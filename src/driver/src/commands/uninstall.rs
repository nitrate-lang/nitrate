use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct UninstallArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_uninstall(&mut self, _args: &UninstallArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package uninstall sub-command invoked");
        // TODO: uninstall logic here
        Ok(())
    }
}
