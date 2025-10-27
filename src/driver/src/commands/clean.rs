use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct CleanArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_clean(&mut self, _args: &CleanArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package clean sub-command invoked");
        // TODO: clean logic here
        Ok(())
    }
}
