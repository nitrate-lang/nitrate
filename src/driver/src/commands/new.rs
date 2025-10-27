use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct NewArgs {}

impl Interpreter<'_> {
    pub fn sc_new(&mut self, _args: &NewArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package new sub-command invoked");
        // TODO: new logic here
        Ok(())
    }
}
