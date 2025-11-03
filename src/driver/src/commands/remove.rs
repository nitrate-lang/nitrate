use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct RemoveArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_remove(&mut self, _args: RemoveArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package remove sub-command invoked");
        // TODO: remove logic here
        Ok(())
    }
}
