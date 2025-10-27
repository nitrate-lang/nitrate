use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct BuildArgs {}

impl Interpreter<'_> {
    pub fn sc_build(&mut self, _args: &BuildArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package build sub-command invoked");
        // TODO: build logic here
        Ok(())
    }
}
