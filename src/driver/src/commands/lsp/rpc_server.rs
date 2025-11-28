use crate::commands::lsp::lsp::{LspServer, handle_rpc_request};
use serde::{Deserialize, Serialize};
use slog::{debug, error, info};
use std::collections::HashMap;
use tokio::io::{AsyncReadExt, AsyncWriteExt};

#[derive(Serialize, Deserialize, Debug)]
pub(crate) struct RpcRequest {
    pub jsonrpc: String,
    pub method: String,
    pub params: serde_json::Value,
    pub id: Option<u64>,
}

#[derive(Serialize, Deserialize, Debug)]
pub(crate) struct RpcResponse {
    pub jsonrpc: String,
    pub result: serde_json::Value,
    pub id: u64,
}

impl From<RpcRequest> for RpcResponse {
    fn from(request: RpcRequest) -> Self {
        RpcResponse {
            jsonrpc: request.jsonrpc,
            result: serde_json::Value::Null,
            id: request.id.unwrap_or(0),
        }
    }
}

pub(crate) enum RpcReply {
    Response(RpcResponse),
    None(()),
}

async fn parse_rpc_frame_headers(socket: &mut tokio::net::TcpStream) -> anyhow::Result<HashMap<String, String>> {
    let mut headers = HashMap::new();
    let mut buffer = Vec::new();
    let mut temp_buf = [0u8; 1];

    loop {
        let n = socket.read(&mut temp_buf).await?;
        if n == 0 {
            return Err(anyhow::anyhow!("Connection closed while reading headers"));
        }

        buffer.push(temp_buf[0]);
        if buffer.ends_with(b"\r\n\r\n") {
            break;
        }
    }

    let header_str = String::from_utf8_lossy(&buffer);
    for line in header_str.lines() {
        if let Some((key, value)) = line.split_once(": ") {
            let value = value.trim_end_matches('\r');
            headers.insert(key.to_string(), value.to_string());
        }
    }

    Ok(headers)
}

pub(crate) async fn handle_tcp_connection(
    mut socket: tokio::net::TcpStream,
    addr: std::net::SocketAddr,
    log: slog::Logger,
) {
    info!(log, "Accepted connection from {}", addr);
    let mut server = LspServer::new(log.clone());

    while server.is_running {
        let Ok(headers) = parse_rpc_frame_headers(&mut socket).await else {
            error!(log, "Failed to parse RPC frame headers from {}", addr);
            return;
        };

        debug!(log, "Parsed RPC frame headers from {}: {:?}", addr, headers);

        let Some(content_length_value) = headers.get("Content-Length") else {
            error!(log, "No Content-Length header found from {}", addr);
            return;
        };

        let Ok(content_length) = content_length_value.parse::<usize>() else {
            error!(log, "Invalid Content-Length header from {}", addr);
            return;
        };

        let mut buffer = vec![0u8; content_length];
        if let Err(e) = socket.read_exact(&mut buffer).await {
            error!(log, "Failed to read RPC message from {}: {}", addr, e);
            return;
        }

        let message = String::from_utf8_lossy(&buffer);

        let Ok(rpc_request) = serde_json::from_str::<RpcRequest>(&message) else {
            error!(log, "Failed to parse RPC request from {}", addr);
            return;
        };

        if rpc_request.jsonrpc != "2.0" {
            error!(log, "Unsupported JSON-RPC version from {}", addr);
            return;
        }

        let rpc_response = match handle_rpc_request(&mut server, rpc_request).await {
            Ok(response) => response,
            Err(e) => {
                error!(log, "Failed to handle RPC request from {}: {}", addr, e);
                continue;
            }
        };

        match rpc_response {
            RpcReply::None(_) => continue,
            RpcReply::Response(rpc_response) => {
                let response_json = serde_json::to_string(&rpc_response).unwrap();
                let response_message = format!("Content-Length: {}\r\n\r\n{}", response_json.len(), response_json);
                if let Err(e) = socket.write_all(response_message.as_bytes()).await {
                    error!(log, "Failed to send RPC response to {}: {}", addr, e);
                    return;
                }

                debug!(log, "Sent RPC response to {}", addr);
            }
        }
    }
}
