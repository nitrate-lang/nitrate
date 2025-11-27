use crate::Interpreter;
use clap::Parser;
use jsonrpsee::{
    core::{RpcResult, async_trait},
    proc_macros::rpc,
    server::{ServerBuilder, ServerHandle},
    types::{ErrorObject, error::ErrorCode},
};
use slog::info;
use std::net::SocketAddr;

#[rpc(server)]
pub trait GreeterRpc {
    /// RPC method to greet a user by name.
    #[method(name = "say_hello")]
    async fn say_hello(&self, name: String) -> RpcResult<String>;
}

pub struct GreeterServerImpl;

#[async_trait]
impl GreeterRpcServer for GreeterServerImpl {
    async fn say_hello(&self, name: String) -> RpcResult<String> {
        if name.is_empty() {
            return Err(ErrorObject::owned(
                ErrorCode::InvalidParams.code(),
                "Name cannot be empty.",
                None::<()>,
            ));
        }
        Ok(format!("Hello, {}!", name))
    }
}

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct LspArgs {
    /// The TCP port to run the LSP server on
    #[arg(short, long)]
    pub port: u16,

    /// The address to bind the LSP server to
    #[arg(short, long, default_value = "127.0.0.1")]
    pub address: String,
}

impl Interpreter<'_> {
    pub(crate) async fn sc_lsp(&mut self, args: LspArgs) -> anyhow::Result<()> {
        let addr_str = format!("{}:{}", args.address, args.port);
        let addr = addr_str.as_str().parse::<SocketAddr>()?;
        info!(self.log, "Starting LSP server at {}", addr_str);

        let server = ServerBuilder::default().build(addr).await?;
        let greeter = GreeterServerImpl;
        let handle: ServerHandle = server.start(greeter.into_rpc());

        info!(self.log, "🌍 JSON-RPC server running at {}", addr);

        handle.stopped().await;

        Ok(())
    }
}
