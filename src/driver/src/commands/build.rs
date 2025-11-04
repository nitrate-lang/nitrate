use crate::{Interpreter, InterpreterError, package::Package};
use clap::Parser;
use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    hir::prelude as hir,
    hir_from_tree::{Ast2HirCtx, convert_ast_to_hir},
    hir_validate::ValidateHir,
    llvm::{LLVMContext, OptLevel},
    llvm_from_hir::generate_code,
    parse::ResolveCtx,
    parsetree::ast,
    token_lexer::{Lexer, LexerError},
};
use slog::{debug, error, info};
use std::{collections::HashSet, io::Read, path::PathBuf};

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

            return Err(InterpreterError::OperationalError);
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

                return Err(InterpreterError::OperationalError);
            }
        }
    }

    fn get_search_paths(&self) -> Vec<PathBuf> {
        let mut search_paths = Vec::new();

        search_paths.push(std::env::current_dir().unwrap().join(".no3/modules"));
        debug!(self.log, "Package search paths: {:?}", search_paths);

        search_paths
    }

    fn parse_source_code(
        &self,
        entrypoint_path: &std::path::Path,
        package_name: &str,
        log: &CompilerLog,
    ) -> Result<ast::Module, InterpreterError> {
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

                return Err(InterpreterError::OperationalError);
            }
        };

        let mut parser = nitrate_translation::parse::Parser::new(lexer, &log);

        Ok(parser.parse_source(Some(ResolveCtx {
            package_name: package_name.to_string(),
            package_search_paths: self.get_search_paths(),
        })))
    }

    fn lower_to_hir(
        &self,
        module: ast::Module,
        ptr_size: u32,
        log: &CompilerLog,
    ) -> Result<(hir::Module, hir::Store, hir::SymbolTab), InterpreterError> {
        let ptr_size = match ptr_size {
            4 => hir::PtrSize::U32,
            8 => hir::PtrSize::U64,
            _ => {
                error!(self.log, "Unsupported pointer size: {} bytes", ptr_size);
                return Err(InterpreterError::OperationalError);
            }
        };

        let mut ctx = Ast2HirCtx::new(ptr_size);
        let module = match convert_ast_to_hir(module, &mut ctx, log) {
            Err(_) => return Err(InterpreterError::OperationalError),
            Ok(module) => module,
        };

        Ok((module, ctx.store, ctx.tab))
    }

    pub(crate) fn sc_build(&mut self, _args: BuildArgs) -> Result<(), InterpreterError> {
        std::fs::create_dir_all(".no3/build").map_err(|e| {
            error!(
                self.log,
                "Failed to create build directory '.no3/build': {}", e
            );
            InterpreterError::IoError(e)
        })?;

        let package = self.get_package_config()?;
        self.validate_package_edition(package.edition())?;

        let entrypoint_path = package.entrypoint();
        if !entrypoint_path.exists() {
            error!(
                self.log,
                "Package entrypoint '{}' does not exist.",
                entrypoint_path.display()
            );
            return Err(InterpreterError::OperationalError);
        }

        let opt_level = OptLevel::Aggressive;
        let target_triple = "x86_64-pc-linux-gnu";

        let llvm_ctx = LLVMContext::new(target_triple, opt_level).map_err(|e| {
            error!(
                self.log,
                "Failed to create LLVM context for target '{}': {}", target_triple, e
            );

            InterpreterError::OperationalError
        })?;

        let log = CompilerLog::new(self.log.clone());
        let ast_module = self.parse_source_code(&entrypoint_path, package.name(), &log)?;
        let ptr_size = llvm_ctx.target_data.get_pointer_byte_size(None);
        let (hir_module, store, symbol_tab) = self.lower_to_hir(ast_module, ptr_size, &log)?;

        let Ok(valid_hir_module) = hir_module.validate(&store, &symbol_tab) else {
            error!(
                self.log,
                "HIR validation failed for package '{}'.",
                package.name(),
            );

            return Err(InterpreterError::OperationalError);
        };

        let mut llvm_module = generate_code(
            package.name(),
            valid_hir_module,
            &llvm_ctx,
            &store,
            &symbol_tab,
        );

        llvm_ctx.optimize_module(&mut llvm_module);

        let target_file_ll = format!(
            ".no3/build/{}-{}.{}.{}.ll",
            package.name(),
            package.version().0,
            package.version().1,
            package.version().2
        );

        let target_file_s = format!(
            ".no3/build/{}-{}.{}.{}.s",
            package.name(),
            package.version().0,
            package.version().1,
            package.version().2
        );

        let target_file_o = format!(
            ".no3/build/{}-{}.{}.{}.o",
            package.name(),
            package.version().0,
            package.version().1,
            package.version().2
        );

        llvm_module
            .print_to_file(target_file_ll)
            .expect("failed to write to file");

        if let Err(e) = llvm_ctx.write_asm(&mut llvm_module, std::path::Path::new(&target_file_s)) {
            error!(
                self.log,
                "Failed to write assembly file for package '{}': {}",
                package.name(),
                e
            );

            return Err(InterpreterError::OperationalError);
        }

        if let Err(e) =
            llvm_ctx.write_object_file(&mut llvm_module, std::path::Path::new(&target_file_o))
        {
            error!(
                self.log,
                "Failed to write object file for package '{}': {}",
                package.name(),
                e
            );

            return Err(InterpreterError::OperationalError);
        }

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
