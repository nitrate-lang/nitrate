use crate::Interpreter;
use clap::Parser;
use slog::info;
use std::net::SocketAddr;

mod lsp;
mod rpc_server;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct LspArgs {
    /// The TCP port to run the LSP server on
    #[arg(short, long)]
    pub port: u16,

    /// The address to bind the LSP server to
    #[arg(long, default_value = "127.0.0.1")]
    pub host: String,
}

impl Interpreter<'_> {
    pub(crate) async fn sc_lsp(&mut self, args: LspArgs) -> anyhow::Result<()> {
        let addr_str = format!("{}:{}", args.host, args.port);
        let addr = addr_str.as_str().parse::<SocketAddr>()?;
        info!(self.log, "Starting LSP server at {}", addr_str);

        let listener = tokio::net::TcpListener::bind(addr).await?;
        loop {
            let (socket, addr) = listener.accept().await?;
            let log = self.log.clone();

            tokio::spawn(async move {
                rpc_server::handle_tcp_connection(socket, addr, log).await;
            });
        }
    }
}
