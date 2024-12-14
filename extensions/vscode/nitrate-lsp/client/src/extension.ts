import { workspace, ExtensionContext } from 'vscode';
const fs = require('fs');
const path = require('path');

import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';

let client: LanguageClient;

function findExecutablePath(executableName: string): string | undefined {
	const envPath = process.env.PATH;
	const pathDirs = envPath.split(path.delimiter);

	for (let i = 0; i < pathDirs.length; i++) {
		const fullPath = path.join(pathDirs[i], executableName);

		try {
			const stats = fs.statSync(fullPath);
			if (stats.isFile() && (stats.mode & 0o111) !== 0) {
				return fullPath;
			}
		} catch (err) {
			continue;
		}
	}

	return undefined;
}

export function activate(context: ExtensionContext) {
	const homeDir = require('os').homedir();
	const lspLogPath = path.join(homeDir, 'nitrated-lsp.log');
	const lspBinaryPath = findExecutablePath('nitrate');
	if (!lspBinaryPath) {
		throw new Error('Could not find nitrate binary in PATH');
	}

	const serverOptions: ServerOptions = {
		command: lspBinaryPath,
		args: ['lsp', "--log", lspLogPath],
		options: { env: { "NO_COLOR": "1" } },
		transport: TransportKind.stdio
	};

	const clientOptions: LanguageClientOptions = {
		documentSelector: [{ scheme: 'file', language: 'nitrate' }],
		synchronize: {
			fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
		}
	};

	// Create the language client and start the client.
	client = new LanguageClient(
		'nitrateLanguageServer',
		'Nitrate Language Server',
		serverOptions,
		clientOptions
	);

	// Start the client. This will also launch the server
	client.start();
}

export function deactivate(): Thenable<void> | undefined {
	if (!client) {
		return undefined;
	}
	return client.stop();
}
