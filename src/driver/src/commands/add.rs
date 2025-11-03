use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct AddArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_add(&mut self, _args: AddArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package add dependency sub-command invoked");
        // TODO: dependency addition logic here
        Ok(())
    }
}
