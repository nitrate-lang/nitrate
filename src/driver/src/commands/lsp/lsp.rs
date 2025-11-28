use std::{collections::HashMap, fs::File};

use crate::commands::lsp::rpc_server::{RpcReply, RpcRequest, RpcResponse};
use serde_json::json;
use slog::{error, info, warn};

struct FileData {
    lines: Vec<String>,
}

impl FileData {
    fn new(text: &str) -> Self {
        FileData {
            lines: text.lines().map(|s| s.to_string()).collect(),
        }
    }
}

pub struct LspServer {
    pub(crate) is_running: bool,

    log: slog::Logger,
    filesystem: HashMap<String, FileData>,
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
            "textDocumentSync": TEXT_DOCUMENT_SYNC_KIND_FULL
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
    let params = request.params.clone();
    if let Some(text_document) = params.get("textDocument") {
        if let Some(uri) = text_document.get("uri").and_then(|u| u.as_str()) {
            if let Some(text) = text_document.get("text").and_then(|t| t.as_str()) {
                let file_data = FileData::new(text);
                m.filesystem.insert(uri.to_string(), file_data);
                info!(m.log, "Stored document: {}", uri);
                return Ok(());
            }
        }
    }

    warn!(m.log, "Failed to open document from request: {:?}", request);
    anyhow::bail!("Invalid didOpen parameters");
}

async fn rpc_notify_text_document_did_change(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<()> {
    let params = request.params.clone();
    if let Some(text_document) = params.get("textDocument") {
        if let Some(uri) = text_document.get("uri").and_then(|u| u.as_str()) {
            if let Some(content_changes) = params.get("contentChanges").and_then(|c| c.as_array()) {
                if let Some(first_change) = content_changes.first() {
                    if let Some(text) = first_change.get("text").and_then(|t| t.as_str()) {
                        let file_data = FileData::new(text);
                        m.filesystem.insert(uri.to_string(), file_data);
                        info!(m.log, "Updated document: {}", uri);
                        return Ok(());
                    }
                }
            }
        }
    }

    warn!(m.log, "Failed to change document from request: {:?}", request);
    anyhow::bail!("Invalid didChange parameters");
}

async fn rpc_notify_text_document_did_close(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<()> {
    let params = request.params.clone();
    if let Some(text_document) = params.get("textDocument") {
        if let Some(uri) = text_document.get("uri").and_then(|u| u.as_str()) {
            if m.filesystem.remove(uri).is_some() {
                info!(m.log, "Removed document: {}", uri);
                return Ok(());
            }

            warn!(m.log, "Document not found for removal: {}", uri);
            return Ok(());
        }
    }

    warn!(m.log, "Failed to close document from request: {:?}", request);
    anyhow::bail!("Invalid didClose parameters");
}

async fn rpc_notify_text_document_did_save(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<()> {
    let params = request.params.clone();
    if let Some(text_document) = params.get("textDocument") {
        if let Some(uri) = text_document.get("uri").and_then(|u| u.as_str()) {
            info!(m.log, "Document saved: {}", uri);
            return Ok(());
        }
    }

    warn!(m.log, "Failed to save document from request: {:?}", request);
    anyhow::bail!("Invalid didSave parameters");
}

pub(crate) async fn handle_rpc_request(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<RpcReply> {
    match request.method.as_str() {
        "initialize" => Ok(RpcReply::Response(rpc_method_initialize(m, request).await?)),
        "initialized" => Ok(RpcReply::None(rpc_notify_initialized(m, request).await?)),
        "shutdown" => Ok(RpcReply::Response(rpc_method_shutdown(m, request).await?)),
        "exit" => Ok(RpcReply::None(rpc_notify_exit(m, request).await?)),
        "textDocument/didOpen" => Ok(RpcReply::None(rpc_notify_text_document_did_open(m, request).await?)),
        "textDocument/didChange" => Ok(RpcReply::None(rpc_notify_text_document_did_change(m, request).await?)),
        "textDocument/didClose" => Ok(RpcReply::None(rpc_notify_text_document_did_close(m, request).await?)),
        "textDocument/didSave" => Ok(RpcReply::None(rpc_notify_text_document_did_save(m, request).await?)),

        _ => {
            error!(m.log, "Unknown RPC method: {}", request.method);
            return Err(anyhow::anyhow!("Unknown RPC method"));
        }
    }
}
