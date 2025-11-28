use serde_json::json;
use slog::{error, info};

use crate::commands::lsp::rpc_server::{RpcRequest, RpcResponse};

pub(crate) async fn handle_rpc_request(request: RpcRequest, log: &slog::Logger) -> anyhow::Result<RpcResponse> {
    match request.method.as_str() {
        "initialize" => {
            info!(log, "Handling 'initialize' RPC method");
            let result = json!({
                "capabilities": {
                    "textDocumentSync": 1,
                }
            });

            Ok(RpcResponse {
                jsonrpc: "2.0".to_string(),
                result,
                id: request.id,
            })
        }

        _ => {
            error!(log, "Unknown RPC method: {}", request.method);
            Err(anyhow::anyhow!("Unknown RPC method"))
        }
    }
}
