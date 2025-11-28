use std::collections::HashMap;

use crate::commands::lsp::rpc_server::{RpcRequest, RpcResponse};
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

async fn rpc_notify_initialized(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<RpcResponse> {
    info!(m.log, "LSP client initialized");
    let response = RpcResponse::from(request);
    Ok(response)
}

async fn rpc_method_shutdown(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<RpcResponse> {
    info!(m.log, "LSP client shutdown requested");
    let response = RpcResponse::from(request);
    Ok(response)
}

async fn rpc_notify_exit(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<RpcResponse> {
    info!(m.log, "LSP client exit requested");
    m.is_running = false;
    let response = RpcResponse::from(request);
    Ok(response)
}

pub(crate) async fn handle_rpc_request(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<RpcResponse> {
    match request.method.as_str() {
        "initialize" => rpc_method_initialize(m, request).await,
        "initialized" => rpc_notify_initialized(m, request).await,
        "shutdown" => rpc_method_shutdown(m, request).await,
        "exit" => rpc_notify_exit(m, request).await,

        _ => {
            error!(m.log, "Unknown RPC method: {}", request.method);
            Err(anyhow::anyhow!("Unknown RPC method"))
        }
    }
}
