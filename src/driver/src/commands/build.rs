use crate::{Interpreter, InterpreterError, package::Package};
use clap::Parser;
use slog::{error, info};

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

        let config_file_string = std::fs::read_to_string("no3.xml").map_err(|e| {
            error!(
                self.log,
                "Failed to read package config file 'no3.xml': {}", e
            );
            InterpreterError::IoError(e)
        })?;

        let package = Package::from_xml(&config_file_string).map_err(|e| {
            error!(
                self.log,
                "Failed to load package config from 'no3.xml': {}", e
            );
            InterpreterError::InvalidPackageConfig
        })?;

        println!("package = {:#?}", package);

        info!(self.log, "package build sub-command invoked");
        // TODO: build logic here
        Ok(())
    }
}
