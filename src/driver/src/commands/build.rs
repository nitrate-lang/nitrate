use crate::{Interpreter, InterpreterError, package::Package};
use clap::Parser;
use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    hir::{Dump, DumpContext, prelude as hir},
    hir_from_tree::{Ast2HirCtx, convert_ast_to_hir},
    hir_validate::ValidateHir,
    llvm::{LLVMContext, OptLevel},
    llvm_from_hir::generate_llvmir,
    parse::ResolveCtx,
    parsetree::ast,
    token_lexer::{Lexer, LexerError},
};
use slog::{debug, error, info};
use std::{collections::HashSet, io::Read, path::PathBuf};

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct BuildArgs {
    /// Pretty-print the Abstract Syntax Tree (AST)
    #[arg(long, group = "output")]
    show_ast: bool,

    /// Pretty-print the High-Level Intermediate Representation (HIR)
    #[arg(long, group = "output")]
    show_hir: bool,

    /// Pretty-print the LLVM Intermediate Representation
    #[arg(long, group = "output")]
    show_llvmir: bool,

    /// Pretty-print the Assembly Code
    #[arg(long, group = "output")]
    show_asm: bool,

    /// Dump the Object Code
    #[arg(long, group = "output")]
    show_obj: bool,

    /// Format mode for printed output
    #[arg(long, value_parser = ["minify", "pretty"])]
    format_mode: Option<String>,

    /// Build artifacts in release mode, with optimizations
    #[arg(long, short = 'r', group = "build-profile")]
    release: bool,

    /// Build artifacts with the specified profile
    #[arg(long, group = "build-profile", value_name = "PROFILE-NAME")]
    profile: Option<String>,

    /// Build for the LLVM target triple
    #[arg(long, value_name = "TRIPLE")]
    target: Option<String>,

    /// Directory for all generated artifacts
    #[arg(long, value_name = "DIRECTORY", default_value = ".no3/build")]
    target_dir: PathBuf,
}

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

    fn parse_source_code(
        &self,
        entrypoint_path: &std::path::Path,
        package_name: &str,
        log: &CompilerLog,
    ) -> Result<ast::Module, InterpreterError> {
        if !entrypoint_path.exists() {
            error!(
                self.log,
                "Package entrypoint '{}' does not exist.",
                entrypoint_path.display()
            );
            return Err(InterpreterError::OperationalError);
        }

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

        Ok(parser.parse_source(
            package_name.into(),
            Some(ResolveCtx {
                package_search_paths: self.get_search_paths(),
            }),
        ))
    }

    fn show_ast(&self, module: &ast::Module, format_mode: &Option<String>) {
        match format_mode {
            Some(mode) if mode == "minify" => {
                serde_json::to_writer(&mut std::io::stdout(), &module)
                    .expect("Failed to write AST to stdout");
            }

            Some(mode) if mode == "pretty" => {
                serde_json::to_writer_pretty(&mut std::io::stdout(), &module)
                    .expect("Failed to write AST to stdout");
            }

            _ => {
                serde_json::to_writer_pretty(&mut std::io::stdout(), &module)
                    .expect("Failed to write AST to stdout");
            }
        }
    }

    fn create_target_dir(&self, dir: &PathBuf) -> Result<(), InterpreterError> {
        if let Err(e) = std::fs::create_dir_all(dir) {
            error!(
                self.log,
                "Failed to create build target directory '{}': {}",
                dir.display(),
                e
            );

            return Err(InterpreterError::IoError(e));
        }

        Ok(())
    }

    fn get_llvm_context(
        &self,
        triple: Option<String>,
        opt_level: OptLevel,
    ) -> Result<LLVMContext, InterpreterError> {
        let triple = match triple {
            Some(t) => t,
            None => LLVMContext::default_target_triple(),
        };

        match LLVMContext::new(&triple, opt_level) {
            Ok(ctx) => Ok(ctx),

            Err(e) => {
                error!(
                    self.log,
                    "Failed to create LLVM context for target '{}': {}", triple, e
                );

                Err(InterpreterError::OperationalError)
            }
        }
    }

    fn get_search_paths(&self) -> Vec<PathBuf> {
        let mut search_paths = Vec::new();

        search_paths.push(std::env::current_dir().unwrap().join(".no3/modules"));
        debug!(self.log, "Package search paths: {:?}", search_paths);

        search_paths
    }

    fn lower_to_hir(
        &self,
        module: ast::Module,
        ptr_size: u32,
        package_name: &str,
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

        let mut ctx = Ast2HirCtx::new(ptr_size, package_name.into());
        let module = match convert_ast_to_hir(module, &mut ctx, log) {
            Err(_) => return Err(InterpreterError::OperationalError),
            Ok(module) => module,
        };

        Ok((module, ctx.store, ctx.tab))
    }

    pub(crate) fn sc_build(&mut self, args: BuildArgs) -> Result<(), InterpreterError> {
        self.create_target_dir(&args.target_dir)?;
        let log = CompilerLog::new(self.log.clone());

        let package = self.get_package_config()?;
        self.validate_package_edition(package.edition())?;

        let ast_module = self.parse_source_code(&package.entrypoint(), package.name(), &log)?;
        if args.show_ast {
            self.show_ast(&ast_module, &args.format_mode);
            return Ok(());
        }

        let llvm_ctx = self.get_llvm_context(args.target, package.optimization_level())?;
        let ptr_size = llvm_ctx.target_data.get_pointer_byte_size(None);

        let (hir_module, store, symbol_tab) =
            self.lower_to_hir(ast_module, ptr_size, package.name(), &log)?;
        if args.show_hir {
            hir_module.dump(&mut DumpContext::new(&store), &mut std::io::stdout())?;
            return Ok(());
        }

        let valid_hir_module = match hir_module.validate(&store, &symbol_tab) {
            Ok(m) => m,
            Err(_) => return Err(InterpreterError::OperationalError),
        };

        let mut llvm_module = generate_llvmir(
            package.name(),
            valid_hir_module,
            &llvm_ctx,
            &store,
            &symbol_tab,
        );

        if args.show_llvmir {
            println!("{}", llvm_module.print_to_string().to_string());
            return Ok(());
        }

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
