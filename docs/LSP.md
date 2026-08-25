# Language Server Protocol (LSP) Implementation

## Overview

The Nitrate LSP server provides IDE-like features for editors that support the Language Server Protocol. It runs as a background process communicating with the editor over stdin/stdout using JSON-RPC messages. The LSP enables real-time error diagnostics, code completion, hover information, and document synchronization.

## Architecture

**Crate**: `nitrate_driver` (LSP modules)  
**Key files**: `lsp.rs`, `rpc_server.rs`, `completion.rs`, `document_sync.rs`  
**Protocol**: Language Server Protocol (LSP) via JSON-RPC 2.0

## Server Lifecycle

The server follows the standard LSP lifecycle: the client sends an `initialize` request, the server responds with capabilities, the server enters the running state where it processes document changes and returns diagnostics, then the client sends `shutdown` and `exit` to terminate.

## RPC Server

The `rpc_server.rs` module implements the JSON-RPC transport layer, reading messages from stdin, parsing the method and parameters, routing to the appropriate handler, and sending responses and notifications to stdout. Server state transitions through `Uninitialized` → `Running` → `Shutdown`.

## Document Synchronization

The `document_sync.rs` module manages open documents. When a document changes, the LSP server updates the cached source text, runs the compiler pipeline (lex → parse → resolve → type check), collects all diagnostics, and sends a `textDocument/publishDiagnostics` notification to the editor. This enables real-time error reporting as the user types.

## Diagnostics

Compiler diagnostics are translated to LSP diagnostics: `SourcePosition` maps to a point diagnostic range, `Span` maps to a range diagnostic, error codes become LSP diagnostic codes, and diagnostic groups map to severity levels (error → Error, warning → Warning, notes → Information).

## Code Completion

The `completion.rs` module provides code completion by determining the context (what kind of completion is needed — type, value, keyword), filtering available symbols based on context, and returning completion items with insert text, symbol kind, and optional documentation. Completion kinds include keywords, functions, types, variables, and modules.

## Supported Methods

The server implements `initialize`, `initialized`, `shutdown`, `exit`, `textDocument/didOpen`, `textDocument/didChange`, `textDocument/didClose`, `textDocument/didSave`, and `textDocument/completion`. Hover information, go to definition, find references, and document formatting are planned as future enhancements.

## VS Code Integration

The `nitrate-lsp` VS Code extension starts the LSP server using `no3 --lsp` as the server command, configured for the Nitrate language. The extension bundles with `nitrate-syntax` for syntax highlighting.

## Design Rationale

JSON-RPC with stdio transport follows the LSP standard, requiring no network setup, avoiding port conflicts, providing process isolation, and enabling simple debugging. Full recompilation on each change is used for simplicity and correctness, with incremental compilation as a planned future optimization.
