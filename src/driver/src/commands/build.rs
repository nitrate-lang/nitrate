use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct BuildArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_build(&mut self, _args: &BuildArgs) -> Result<(), InterpreterError> {
        /*
         * Get to the package directory
         * Find the entry point lib.nit or main.nit
         * call function like 'compile_nitrate_source()' and put artifacts in 'target' folder
         * Relay diagnostics to the user
         */

        info!(self.log, "package build sub-command invoked");
        // TODO: build logic here
        Ok(())
    }
}
