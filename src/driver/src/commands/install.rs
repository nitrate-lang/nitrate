use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub struct InstallArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_install(&mut self, _args: &InstallArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package install sub-command invoked");
        // TODO: install logic here
        Ok(())
    }
}
