use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct RunArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_run(&mut self, _args: &RunArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package run sub-command invoked");
        // TODO: run logic here
        Ok(())
    }
}
