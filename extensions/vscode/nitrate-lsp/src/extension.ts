import { workspace, ExtensionContext } from 'vscode';
import { statSync } from 'fs';
import { delimiter, join } from 'path';
import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';

function FindExecutablePath(executable_name: string): string | undefined {
	const env_path = process.env.PATH + delimiter;
	const exe_paths = env_path.split(delimiter);

	for (const exe_path of exe_paths) {
		const full_path = join(exe_path, executable_name);
		console.info(`Checking path: ${full_path}`);

		try {
			const stats = statSync(full_path);
			if (!stats.isFile()) {
				continue;
			}

			// Check if the file is executable
			if (stats.mode & 0o111) {
				console.info(`Found executable: ${full_path}`);
				return full_path;
			} else {
				console.info(`Found file but not executable: ${full_path}`);
			}
		} catch { }
	}

	return undefined;
}

let gLanguageClient: LanguageClient;

export async function activate(context: ExtensionContext) {
	const home_dir = require('os').homedir();
	const log_file = join(home_dir, 'nitrate-lsp.log');
	const lsp_app = FindExecutablePath('nitrate');
	if (lsp_app == undefined) {
		console.error('Could not find "nitrate" binary in $PATH');
		throw new Error('Could not find "nitrate" binary in $PATH');
	}

	console.log(`Using nitrate binary: ${lsp_app}`);
	console.log(`Using log file: ${log_file}`);
	console.log(`Using home directory: ${home_dir}`);
	console.log(`Using OS: ${process.platform}`);
	console.log(`Using architecture: ${process.arch}`);
	console.log(`Using Node.js version: ${process.versions.node}`);
	console.log(`Using V8 version: ${process.versions.v8}`);
	console.log(`Using Electron version: ${process.versions.electron}`);

	const LSPserverOptions: ServerOptions = {
		command: lsp_app,
		args: ['lsp', "--log", log_file],
		options: { env: { "NO_COLOR": "1" } },
		transport: TransportKind.stdio
	};

	const LSPclientOptions: LanguageClientOptions = {
		documentSelector: [{ scheme: 'file', language: 'nitrate' }],
		synchronize: {
			fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
		}
	};

	gLanguageClient = new LanguageClient(
		'nitrateLanguageServer',
		'Nitrate: Language Server',
		LSPserverOptions,
		LSPclientOptions
	);

	console.log('Starting Nitrate Language Server...');
	return await gLanguageClient.start().catch((err) => {
		console.error('Error starting Nitrate Language Server:', err);
		throw err;
	});
}

export function deactivate(): Thenable<void> | undefined {
	return gLanguageClient ? gLanguageClient.stop() : undefined;
}
