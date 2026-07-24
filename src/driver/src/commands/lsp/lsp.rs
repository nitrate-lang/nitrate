use crate::commands::lsp::{completion::rpc_method_text_document_completion, document_sync::*, rpc_server::*};
use serde_json::json;
use slog::{error, info, warn};
use std::collections::HashMap;

pub struct LspServer {
    pub is_running: bool,
    pub log: slog::Logger,
    pub filesystem: HashMap<String, FileData>,
}

impl LspServer {
    pub fn new(log: slog::Logger) -> Self {
        LspServer {
            is_running: true,
            log,
            filesystem: HashMap::new(),
        }
    }
}

async fn rpc_method_initialize(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<RpcResponse> {
    info!(m.log, "LSP client initializing");

    const TEXT_DOCUMENT_SYNC_KIND_FULL: u32 = 1;

    let mut response = RpcResponse::from(request);
    response.result = json!({
        "capabilities": {
            "textDocumentSync": TEXT_DOCUMENT_SYNC_KIND_FULL,
            "completionProvider": {
                "resolveProvider": false,
                "triggerCharacters": [".", ":"]
            }
        }
    });

    Ok(response)
}

async fn rpc_notify_initialized(m: &mut LspServer, _request: RpcRequest) -> anyhow::Result<()> {
    info!(m.log, "LSP client initialized");
    Ok(())
}

async fn rpc_method_shutdown(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<RpcResponse> {
    info!(m.log, "LSP client shutdown requested");
    let response = RpcResponse::from(request);
    Ok(response)
}

async fn rpc_notify_exit(m: &mut LspServer, _request: RpcRequest) -> anyhow::Result<()> {
    info!(m.log, "LSP client exit requested");
    m.is_running = false;
    Ok(())
}

pub(crate) async fn handle_rpc_request(m: &mut LspServer, req: RpcRequest) -> anyhow::Result<RpcReply> {
    match req.method.as_str() {
        "initialize" => Ok(RpcReply::Response(rpc_method_initialize(m, req).await?)),
        "initialized" => Ok(RpcReply::None(rpc_notify_initialized(m, req).await?)),
        "shutdown" => Ok(RpcReply::Response(rpc_method_shutdown(m, req).await?)),
        "exit" => Ok(RpcReply::None(rpc_notify_exit(m, req).await?)),

        /* Handle document synchronization */
        "textDocument/didOpen" => Ok(RpcReply::None(rpc_notify_text_document_did_open(m, req).await?)),
        "textDocument/didChange" => Ok(RpcReply::None(rpc_notify_text_document_did_change(m, req).await?)),
        "textDocument/didClose" => Ok(RpcReply::None(rpc_notify_text_document_did_close(m, req).await?)),
        "textDocument/didSave" => Ok(RpcReply::None(rpc_notify_text_document_did_save(m, req).await?)),

        /* Language server features */
        "textDocument/completion" => Ok(RpcReply::Response(rpc_method_text_document_completion(m, req).await?)),

        method if method.starts_with("$/") => {
            // Handle custom notifications or requests starting with $/
            warn!(m.log, "Custom RPC method not implemented: {}", req.method);
            Err(anyhow::anyhow!("Custom RPC methods not implemented"))
        }

        _ => {
            error!(m.log, "Unknown RPC method: {}", req.method);
            return Err(anyhow::anyhow!("Unknown RPC method"));
        }
    }
}
