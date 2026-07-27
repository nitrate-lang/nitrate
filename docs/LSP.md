# Language Server Protocol (LSP) Implementation

## Overview

The Nitrate LSP server provides IDE-like features for editors that support the Language Server Protocol. It runs as a background process, communicating with the editor over stdin/stdout using JSON-RPC messages. The LSP enables features like syntax highlighting, error diagnostics, code completion, and hover information.

## Architecture

**Crate**: `nitrate_driver` (LSP modules)  
**Key files**: `lsp.rs`, `rpc_server.rs`, `completion.rs`, `document_sync.rs`  
**Protocol**: Language Server Protocol (LSP) via JSON-RPC

## Protocol Implementation

The LSP server implements the standard Language Server Protocol specification. Communication uses JSON-RPC 2.0 over stdin/stdout:

- **Requests**: Method calls from the client (editor) to the server
- **Notifications**: One-way messages (no response expected)
- **Responses**: Results or errors returned for requests

### Server Lifecycle

```
Initialize Request (client → server)
    ├── Server responds with capabilities
    └── Server enters running state

    [Client sends notifications for document changes]
    [Server sends diagnostics, completions, etc.]

Shutdown Request (client → server)
    └── Server enters shutdown state

Exit Notification (client → server)
    └── Server process terminates
```

## RPC Server

The `rpc_server.rs` module implements the JSON-RPC transport layer:

```rust
pub struct RpcServer {
    // Pending requests awaiting responses
    pending: HashMap<RequestId, PendingRequest>,
    // Server state
    state: ServerState,
}

enum ServerState {
    Uninitialized,
    Running,
    Shutdown,
}
```

The RPC server:

1. Reads JSON-RPC messages from stdin
2. Parses the method and parameters
3. Routes to the appropriate handler
4. Sends responses and notifications to stdout

## Document Synchronization

The `document_sync.rs` module manages the open documents in the editor:

```rust
pub struct DocumentSync {
    // Currently open documents
    documents: HashMap<Url, Document>,
    // Compiler log for diagnostics
    log: CompilerLog,
}

struct Document {
    uri: Url,
    version: i32,
    source: String,
    // Cached compilation results
    diagnostics: Vec<Diagnostic>,
}
```

The sync module handles:

- **`textDocument/didOpen`**: A document was opened; start tracking it
- **`textDocument/didChange`**: A document's content changed; update the cached source and recompile
- **`textDocument/didClose`**: A document was closed; stop tracking it
- **`textDocument/didSave`**: A document was saved (optional, for save-triggered features)

When a document changes, the LSP server:

1. Updates the cached source text
2. Runs the compiler pipeline (lex → parse → resolve → type check)
3. Collects all diagnostics
4. Sends a `textDocument/publishDiagnostics` notification to the editor

## Diagnostics

The LSP server translates compiler diagnostics to LSP diagnostics:

| Compiler Diagnostic          | LSP Diagnostic                                |
| ---------------------------- | --------------------------------------------- |
| `SourcePosition` (point)     | `Diagnostic { range: { start, end: start } }` |
| `Span` (range)               | `Diagnostic { range: { start, end } }`        |
| Error code (e.g., `[L0300]`) | `Diagnostic { code: "L0300" }`                |
| Error message                | `Diagnostic { message }`                      |
| Diagnostic group             | `Diagnostic { severity }`                     |

Diagnostic severity mapping:

- Compiler errors → LSP `Error` severity
- Compiler warnings → LSP `Warning` severity
- Compiler notes → LSP `Information` severity

## Code Completion

The `completion.rs` module provides code completion suggestions:

```rust
pub struct CompletionProvider {
    // Symbol table for looking up available names
    tab: SymbolTab,
    // Keywords that are valid at the given position
    keywords: Vec<String>,
}
```

Completion is triggered by the `textDocument/completion` request. The provider:

1. Determines the context (what kind of completion is needed — type, value, keyword, etc.)
2. Filters available symbols based on the context
3. Returns completion items with:
   - The text to insert
   - The kind of symbol (function, variable, keyword, type, etc.)
   - Optional documentation

Completion kinds include:

- **Keywords**: All Nitrate keywords (when valid at the position)
- **Functions**: Function names from the symbol table
- **Types**: Type names (structs, enums, type aliases)
- **Variables**: Local and global variable names
- **Modules**: Module names for import paths

## LSP Server Integration

The LSP server is integrated into the `no3` binary:

```bash
no3 --lsp
```

This starts the LSP server in stdio mode, listening for JSON-RPC messages on stdin and sending responses on stdout.

## VS Code Integration

The Nitrate VS Code extension (`nitrate-lsp`) in `extensions/vscode/nitrate-lsp/` connects to the LSP server:

```typescript
// extension.ts
let client: LanguageClient;

export function activate(context: ExtensionContext) {
  client = new LanguageClient(
    "nitrate-lsp",
    "Nitrate Language Server",
    { command: "no3", args: ["--lsp"] },
    { documentSelector: [{ scheme: "file", language: "nitrate" }] },
  );
  client.start();
}
```

The extension provides:

- **Syntax highlighting** (via `nitrate-syntax` TextMate grammar)
- **Error diagnostics** (via LSP)
- **Code completion** (via LSP)
- **Hover information** (via LSP — future enhancement)
- **Go to definition** (via LSP — future enhancement)

## Supported LSP Methods

| Method                    | Status | Description                                      |
| ------------------------- | ------ | ------------------------------------------------ |
| `initialize`              | ✅     | Server initialization and capability negotiation |
| `initialized`             | ✅     | Post-initialization notification                 |
| `shutdown`                | ✅     | Graceful shutdown                                |
| `exit`                    | ✅     | Process termination                              |
| `textDocument/didOpen`    | ✅     | Document opened notification                     |
| `textDocument/didChange`  | ✅     | Document change notification                     |
| `textDocument/didClose`   | ✅     | Document close notification                      |
| `textDocument/completion` | ✅     | Code completion                                  |
| `textDocument/didSave`    | ✅     | Document save notification                       |
| `textDocument/hover`      | 🔜     | Hover information (planned)                      |
| `textDocument/definition` | 🔜     | Go to definition (planned)                       |
| `textDocument/references` | 🔜     | Find references (planned)                        |
| `textDocument/formatting` | 🔜     | Document formatting (planned)                    |

## Design Decisions

### Why JSON-RPC over stdio?

JSON-RPC with stdio transport is the standard for LSP implementations:

1. **No network setup**: Simple pipe-based communication
2. **No port conflicts**: No network ports to manage
3. **Process isolation**: The server runs as a separate process
4. **Simple debugging**: Messages can be logged and inspected

### Why Full Compilation on Each Change?

Recompiling on each document change (rather than incremental compilation):

1. **Simplicity**: No need to track which parts changed
2. **Correctness**: Always reports complete and accurate diagnostics
3. **Performance is adequate**: For typical file sizes, compilation is fast enough for real-time diagnostics

Future optimization: Incremental compilation that only processes changed functions.
