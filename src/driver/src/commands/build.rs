use crate::{Interpreter, InterpreterError, package::Package};
use clap::Parser;
use nitrate_diagnosis::CompilerLog;
use nitrate_translation::{TranslationOptions, compile_code};
use slog::{error, info};
use std::collections::HashSet;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct BuildArgs {}

impl Interpreter<'_> {
    fn validate_package_edition(&self, edition: u16) -> Result<(), InterpreterError> {
        let supported_edition = HashSet::from([2026]);

        if !supported_edition.contains(&edition) {
            let supported = supported_edition
                .iter()
                .map(|e| e.to_string())
                .collect::<Vec<String>>()
                .join(", ");

            error!(
                self.log,
                "Unsupported package edition: {}. This release of no3 supports editions: {}",
                edition,
                supported,
            );

            return Err(InterpreterError::UnsupportedPackageEdition(edition));
        }

        Ok(())
    }
    pub(crate) fn sc_build(&mut self, _args: &BuildArgs) -> Result<(), InterpreterError> {
        /*
         * Get to the package directory
         * Find the entry point lib.nit or main.nit
         * call function like 'compile_nitrate_source()' and put artifacts in 'target' folder
         * Relay diagnostics to the user
         */

        let config_file_string = match std::fs::read_to_string("no3.xml") {
            Ok(content) => content,

            Err(e) => {
                error!(
                    self.log,
                    "Failed to read package config file 'no3.xml': {}", e
                );

                return Err(InterpreterError::IoError(e));
            }
        };

        let package = match Package::from_xml(&config_file_string) {
            Ok(pkg) => pkg,

            Err(e) => {
                error!(
                    self.log,
                    "Failed to load package config from 'no3.xml': {}", e
                );

                return Err(InterpreterError::InvalidPackageConfig);
            }
        };

        self.validate_package_edition(package.edition())?;

        let entrypoint_path = package.entrypoint();
        if !entrypoint_path.exists() {
            error!(
                self.log,
                "Package entrypoint '{}' does not exist.",
                entrypoint_path.display()
            );

            return Err(InterpreterError::IoError(std::io::Error::new(
                std::io::ErrorKind::NotFound,
                "package entrypoint not found",
            )));
        }

        let mut source_code = match std::fs::File::open(&entrypoint_path) {
            Ok(file) => file,

            Err(e) => {
                error!(
                    self.log,
                    "Failed to open package entrypoint '{}': {}",
                    entrypoint_path.display(),
                    e
                );

                return Err(InterpreterError::IoError(e));
            }
        };

        let mut machine_code = Vec::new();

        let mut translation_options = TranslationOptions::default();
        translation_options.package_name = package.name().to_string();
        translation_options.source_name_for_debug_messages =
            entrypoint_path.to_string_lossy().to_string();
        translation_options.source_path = Some(entrypoint_path);
        translation_options.log = CompilerLog::new(self.log.clone());

        match compile_code(&mut source_code, &mut machine_code, &translation_options) {
            Ok(_) => {
                info!(
                    self.log,
                    "Successfully built package '{}' version {}.{}.{}",
                    package.name(),
                    package.version().0,
                    package.version().1,
                    package.version().2,
                );

                Ok(())
            }

            Err(_) => {
                error!(
                    self.log,
                    "Build failed for package '{}'. See diagnostics for details.",
                    package.name(),
                );

                Err(InterpreterError::BuildError)
            }
        }
    }
}
