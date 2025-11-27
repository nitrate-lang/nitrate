use crate::Interpreter;
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct CheckArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_check(&mut self, _args: CheckArgs) -> anyhow::Result<()> {
        info!(self.log, "package check sub-command invoked");
        // TODO: check logic here
        Ok(())
    }
}
