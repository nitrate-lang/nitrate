use std::collections::HashMap;

use crate::commands::lsp::rpc_server::{RpcReply, RpcRequest, RpcResponse};
use serde_json::json;
use slog::{error, info};

pub struct LspServer {
    pub(crate) is_running: bool,

    log: slog::Logger,
    filesystem: HashMap<String, Vec<u8>>,
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

    const TEXT_DOCUMENT_SYNC_KIND_INCREMENTAL: u32 = 2;

    let mut response = RpcResponse::from(request);
    response.result = json!({
        "capabilities": {
            "textDocumentSync": TEXT_DOCUMENT_SYNC_KIND_INCREMENTAL
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

async fn rpc_notify_text_document_did_open(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<()> {
    info!(m.log, "LSP textDocument/didOpen received");
    let params = request.params.clone();
    if let Some(text_document) = params.get("textDocument") {
        if let Some(uri) = text_document.get("uri").and_then(|u| u.as_str()) {
            if let Some(text) = text_document.get("text").and_then(|t| t.as_str()) {
                m.filesystem.insert(uri.to_string(), text.as_bytes().to_vec());
                info!(m.log, "Stored document: {}", uri);
            }
        }
    }
    Ok(())
}

pub(crate) async fn handle_rpc_request(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<RpcReply> {
    match request.method.as_str() {
        "initialize" => Ok(RpcReply::Response(rpc_method_initialize(m, request).await?)),
        "initialized" => Ok(RpcReply::None(rpc_notify_initialized(m, request).await?)),
        "shutdown" => Ok(RpcReply::Response(rpc_method_shutdown(m, request).await?)),
        "exit" => Ok(RpcReply::None(rpc_notify_exit(m, request).await?)),
        "textDocument/didOpen" => Ok(RpcReply::None(rpc_notify_text_document_did_open(m, request).await?)),

        _ => {
            error!(m.log, "Unknown RPC method: {}", request.method);
            return Err(anyhow::anyhow!("Unknown RPC method"));
        }
    }
}
