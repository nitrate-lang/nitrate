import { workspace, ExtensionContext } from "vscode";
import { statSync } from "fs";
import { delimiter, join } from "path";
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  StreamInfo,
} from "vscode-languageclient/node";
import * as net from "net";
import { spawn, ChildProcessWithoutNullStreams } from "child_process";

// Global variable to hold the spawned process, accessible by deactivate
let gLanguageClient: LanguageClient | undefined;
let serverProcess: ChildProcessWithoutNullStreams | undefined; // Moved to module scope for deactivate access

function FindExecutablePath(executable_name: string): string | undefined {
  const env_path = process.env.PATH + delimiter;
  const exe_paths = env_path.split(delimiter);

  for (const exe_path of exe_paths) {
    const full_path = join(exe_path, executable_name);
    console.info(`Checking path: ${full_path}`);

    try {
      const stats = statSync(full_path);

      // Cleaned up check: ensure it's a file AND executable
      if (stats.isFile() && stats.mode & 0o111) {
        console.info(`Found executable: ${full_path}`);
        return full_path;
      }
    } catch (e) {
      // Intentionally swallows errors (like ENOENT: file not found)
      // to continue searching the PATH.
    }
  }

  return undefined;
}

export async function activate(context: ExtensionContext) {
  const home_dir = require("os").homedir();
  const lsp_app = FindExecutablePath("no3");
  if (!lsp_app) {
    console.error('Could not find "no3" binary in $PATH');
    throw new Error('Could not find "no3" binary in $PATH');
  }

  console.log(`Using no3 binary: ${lsp_app}`);
  // ... (Removed unused console.logs for brevity, but kept in original)

  const cfg = workspace.getConfiguration("nitrate.lsp");
  const lspHost = cfg.get<string>("host", "0.0.0.0");
  const lspPort = cfg.get<number>("port", 5007);
  const spawnServer = cfg.get<boolean>("spawn", true);
  const connectRetries = cfg.get<number>("connectRetries", 20);
  const connectDelayMs = cfg.get<number>("connectDelayMs", 250);
  // Using an initial delay allows the server time to bind the port before the retry loop starts.
  const initialStartupDelayMs = cfg.get<number>("initialStartupDelayMs", 1000);

  function startServerProcess(): ChildProcessWithoutNullStreams | undefined {
    if (!spawnServer) {
      return undefined;
    }

    const args = ["lsp", "--host", lspHost, "--port", String(lspPort)];

    console.log(`Spawning no3 server: ${lsp_app} ${args.join(" ")}`);
    const child = spawn(lsp_app!, args, {
      env: { ...process.env, NO_COLOR: "1" },
      stdio: "pipe",
    }) as ChildProcessWithoutNullStreams;

    child.stdout.on(
      "data",
      (chunk: Buffer) => console.log(`[no3 stdout] ${chunk.toString().trim()}`) // Added .trim()
    );
    child.stderr.on(
      "data",
      (chunk: Buffer) =>
        console.error(`[no3 stderr] ${chunk.toString().trim()}`) // Added .trim()
    );
    child.on("exit", (code: number | null, signal: NodeJS.Signals | null) => {
      console.log(`no3 exited with code=${code} signal=${signal}`);
      serverProcess = undefined; // Clear global on exit
    });
    child.on("error", (err: Error) =>
      console.error("Failed to spawn no3:", err)
    );

    return child;
  }

  // Connect to language server over TCP.
  const LSPserverOptions: ServerOptions = () => {
    return new Promise<StreamInfo>(async (resolve, reject) => {
      try {
        if (spawnServer) {
          serverProcess = startServerProcess();

          // --- FIX 1: Crucial Delay for Server Startup ---
          console.log(
            `Waiting ${initialStartupDelayMs}ms for server to start listening...`
          );
          await new Promise((r) => setTimeout(r, initialStartupDelayMs));
          // ---------------------------------------------
        }

        let lastErr: any = null;
        for (let attempt = 0; attempt < connectRetries; attempt++) {
          try {
            const socket = await new Promise<net.Socket>((res, rej) => {
              const s = net.connect({ port: lspPort, host: lspHost }, () =>
                res(s)
              );
              s.setTimeout(connectDelayMs); // Added a connection timeout
              s.on("error", (e) => rej(e));
              s.on("timeout", () => rej(new Error("Connection timeout")));
            });

            console.log(`Connected to LSP server at ${lspHost}:${lspPort}`);
            // `socket` acts as both reader and writer
            return resolve({ reader: socket, writer: socket });
          } catch (err) {
            lastErr = err;
            console.warn(
              `Connection attempt ${
                attempt + 1
              }/${connectRetries} failed. Retrying...`
            );
            // Wait and retry
            await new Promise((r) => setTimeout(r, connectDelayMs));
          }
        }

        console.error(
          `Failed to connect to LSP server at ${lspHost}:${lspPort} after ${connectRetries} attempts`
        );
        return reject(
          lastErr || new Error("Connection failed after max retries.")
        );
      } catch (err) {
        return reject(err);
      }
    });
  };

  const LSPclientOptions: LanguageClientOptions = {
    documentSelector: [{ scheme: "file", language: "nitrate" }],
    synchronize: {
      fileEvents: workspace.createFileSystemWatcher("**/.clientrc"),
    },
  };

  gLanguageClient = new LanguageClient(
    "nitrate-lsp",
    "NitrateLSP",
    LSPserverOptions,
    LSPclientOptions
  );

  console.log("Starting Nitrate Language Server...");
  return await gLanguageClient.start().catch((err) => {
    console.error("Error starting Nitrate Language Server:", err);
    // Ensure process is killed if LanguageClient startup fails
    if (serverProcess) {
      serverProcess.kill();
      serverProcess = undefined;
    }
    throw err;
  });
}

// --- FIX 2: Gracefully Kill Child Process on Deactivation ---
export function deactivate(): Thenable<void> | undefined {
  let stopPromise: Thenable<void> | undefined = gLanguageClient
    ? gLanguageClient.stop()
    : undefined;

  if (serverProcess) {
    console.log("Stopping spawned language server process (no3).");
    // Use kill() with default 'SIGTERM' to allow for graceful shutdown
    serverProcess.kill();
    serverProcess = undefined;
  }

  return stopPromise;
}
// -----------------------------------------------------------
