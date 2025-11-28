use crate::commands::lsp::{lsp::*, rpc_server::*};
use slog::{info, warn};

pub struct FileData {
    pub lines: Vec<String>,
}

impl FileData {
    fn new(text: &str) -> Self {
        FileData {
            lines: text.lines().map(|s| s.to_string()).collect(),
        }
    }
}

pub async fn rpc_notify_text_document_did_open(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<()> {
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

pub async fn rpc_notify_text_document_did_change(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<()> {
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

pub async fn rpc_notify_text_document_did_close(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<()> {
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

pub async fn rpc_notify_text_document_did_save(m: &mut LspServer, request: RpcRequest) -> anyhow::Result<()> {
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
