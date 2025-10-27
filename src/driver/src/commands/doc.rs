use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct DocArgs {}

impl Interpreter<'_> {
    pub fn sc_doc(&mut self, _args: &DocArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package doc sub-command invoked");
        // TODO: doc logic here
        Ok(())
    }
}
