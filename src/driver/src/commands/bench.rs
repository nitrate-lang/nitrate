use crate::Interpreter;
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct BenchArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_bench(&mut self, _args: BenchArgs) -> anyhow::Result<()> {
        info!(self.log, "package benchmark sub-command invoked");
        // TODO: benchmark logic here
        Ok(())
    }
}
