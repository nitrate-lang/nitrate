use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct TestArgs {}

impl Interpreter<'_> {
    pub fn sc_test(&mut self, _args: &TestArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package test sub-command invoked");
        // TODO: test logic here
        Ok(())
    }
}
