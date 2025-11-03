use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct SearchArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_search(&mut self, _args: SearchArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package search sub-command invoked");
        // TODO: search logic here
        Ok(())
    }
}
