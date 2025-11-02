use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct InitArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_init(&mut self, _args: &InitArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package init sub-command invoked");
        // TODO: init logic here
        Ok(())
    }
}
