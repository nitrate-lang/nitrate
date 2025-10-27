use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct UpdateArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_update(&mut self, _args: &UpdateArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package update sub-command invoked");
        // TODO: update logic here
        Ok(())
    }
}
