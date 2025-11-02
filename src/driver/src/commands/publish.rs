use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct PublishArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_publish(&mut self, _args: &PublishArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package publish sub-command invoked");
        // TODO: publish logic here
        Ok(())
    }
}
