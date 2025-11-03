use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct TestArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_test(&mut self, _args: TestArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package test sub-command invoked");
        // TODO: test logic here
        Ok(())
    }
}
