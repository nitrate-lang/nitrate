use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct CheckArgs {}

impl Interpreter<'_> {
    pub fn sc_check(&mut self, _args: &CheckArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package check sub-command invoked");
        // TODO: check logic here
        Ok(())
    }
}
