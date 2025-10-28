use crate::{Interpreter, InterpreterError, package::Package};
use clap::Parser;
use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    hir::{self, hir::PtrSize},
    parse::ResolveCtx,
    parsetree::ast,
    source_into_hir::{Ast2HirCtx, convert_ast_to_hir},
    token_lexer::{Lexer, LexerError},
};
use slog::{error, info};
use std::{collections::HashSet, io::Read};

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

    fn get_package_config(&self) -> Result<Package, InterpreterError> {
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

        match Package::from_xml(&config_file_string) {
            Ok(pkg) => Ok(pkg),

            Err(e) => {
                error!(
                    self.log,
                    "Failed to load package config from 'no3.xml': {}", e
                );

                return Err(InterpreterError::InvalidPackageConfig);
            }
        }
    }

    fn parse_source_code(
        &self,
        entrypoint_path: &std::path::Path,
        package_name: &str,
        log: &CompilerLog,
    ) -> Result<ast::Module, InterpreterError> {
        /* FIXME: Implement package search paths */
        let package_search_paths = Vec::new();

        let mut source_code_file = match std::fs::File::open(&entrypoint_path) {
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

        let mut source_code = Vec::new();
        source_code_file
            .read_to_end(&mut source_code)
            .map_err(|e| InterpreterError::IoError(e))?;

        let source_code_file = intern_file_id(&entrypoint_path.to_string_lossy().to_string())
            .expect("FileId overflow");

        let lexer = match Lexer::new(&source_code, Some(source_code_file)) {
            Ok(lexer) => lexer,

            Err(LexerError::SourceTooBig) => {
                error!(
                    self.log,
                    "Source file '{}' is too large to be processed.",
                    entrypoint_path.display(),
                );

                return Err(InterpreterError::BuildError);
            }
        };

        let mut parser = nitrate_translation::parse::Parser::new(lexer, &log);

        Ok(parser.parse_source(Some(ResolveCtx {
            package_name: package_name.to_string(),
            package_search_paths,
        })))
    }

    fn lower_to_hir(
        &self,
        module: ast::Module,
        log: &CompilerLog,
    ) -> Result<hir::prelude::Module, InterpreterError> {
        /* FIXME: Parameterize this */
        const PTR_SIZE: PtrSize = PtrSize::U32;

        let mut ctx = Ast2HirCtx::new(PTR_SIZE);
        convert_ast_to_hir(module, &mut ctx, log).map_err(|_| InterpreterError::BuildError)
    }

    pub(crate) fn sc_build(&mut self, _args: &BuildArgs) -> Result<(), InterpreterError> {
        let package = self.get_package_config()?;

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

        let log = CompilerLog::new(self.log.clone());
        let module = self.parse_source_code(&entrypoint_path, package.name(), &log)?;
        let module_hir = self.lower_to_hir(module, &log)?;

        /* TODO: Perform mandatory checks on the HIR */
        /* TODO: Lower to LLVM IR */
        /* TODO: Link LLVM IR into final binary or shared library */

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
}
