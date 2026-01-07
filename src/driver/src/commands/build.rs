use crate::{Interpreter, package::Package};
use clap::Parser;
use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    hir::{Store, prelude as hir, using_storage},
    hir_dump::Dump,
    hir_from_tree::{Ast2HirCtx, convert_ast_to_hir},
    hir_validate::{self, ValidateHirItem},
    llvm::{LLVMContext, OptLevel},
    llvm_from_hir::generate_llvmir,
    parsetree::ast,
    token_lexer::{Lexer, LexerError},
    tree_resolve::ImportContext,
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
    pub(crate) fn validate_package_edition(&self, edition: u16) -> anyhow::Result<()> {
        let supported_edition = HashSet::from([2026]);

        if !supported_edition.contains(&edition) {
            let supported = supported_edition
                .iter()
                .map(|e| e.to_string())
                .collect::<Vec<String>>()
                .join(", ");

            error!(
                self.log,
                "Unsupported package edition: {}. This release of no3 supports editions: {}", edition, supported,
            );

            return Err(anyhow::anyhow!("Unsupported package edition"));
        }

        Ok(())
    }

    pub(crate) fn get_package_config(&self) -> anyhow::Result<Package> {
        let config_file_string = match std::fs::read_to_string("no3.xml") {
            Ok(content) => content,

            Err(e) => {
                error!(self.log, "Failed to read package config file 'no3.xml': {}", e);

                return Err(anyhow::anyhow!("Failed to read package config file 'no3.xml'"));
            }
        };

        match Package::from_xml(&config_file_string) {
            Ok(pkg) => Ok(pkg),

            Err(e) => {
                error!(self.log, "Failed to load package config from 'no3.xml': {}", e);

                return Err(anyhow::anyhow!("Failed to load package config from 'no3.xml'"));
            }
        }
    }

    pub(crate) fn parse_source_code(
        &self,
        entrypoint_path: &std::path::Path,
        package_name: &str,
        log: &CompilerLog,
    ) -> anyhow::Result<ast::Module> {
        if !entrypoint_path.exists() {
            error!(
                self.log,
                "Package entrypoint '{}' does not exist.",
                entrypoint_path.display()
            );
            return Err(anyhow::anyhow!("Package entrypoint does not exist"));
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

                return Err(e.into());
            }
        };

        let mut source_code = Vec::new();
        source_code_file.read_to_end(&mut source_code)?;

        let source_code_file = intern_file_id(&entrypoint_path.to_string_lossy().to_string()).expect("FileId overflow");

        let lexer = match Lexer::new(&source_code, Some(source_code_file)) {
            Ok(lexer) => lexer,

            Err(LexerError::SourceTooBig) => {
                error!(
                    self.log,
                    "Source file '{}' is too large to be processed.",
                    entrypoint_path.display(),
                );

                return Err(anyhow::anyhow!("Source file too large"));
            }
        };

        let mut parser = nitrate_translation::parse::Parser::new(lexer, &log);

        Ok(parser.parse_source(package_name.into()))
    }

    pub(crate) fn show_ast(&self, module: &ast::Module, format_mode: &Option<String>) {
        match format_mode {
            Some(mode) if mode == "minify" => {
                serde_json::to_writer(&mut std::io::stdout(), &module).expect("Failed to write AST to stdout");
            }

            Some(mode) if mode == "pretty" => {
                serde_json::to_writer_pretty(&mut std::io::stdout(), &module).expect("Failed to write AST to stdout");
            }

            _ => {
                serde_json::to_writer_pretty(&mut std::io::stdout(), &module).expect("Failed to write AST to stdout");
            }
        }
    }

    fn create_target_dir(&self, dir: &PathBuf) -> anyhow::Result<()> {
        if let Err(e) = std::fs::create_dir_all(dir) {
            error!(
                self.log,
                "Failed to create build target directory '{}': {}",
                dir.display(),
                e
            );
            return Err(e.into());
        }

        Ok(())
    }

    fn get_llvm_context(&self, triple: Option<String>, opt_level: OptLevel) -> anyhow::Result<LLVMContext> {
        let triple = match triple {
            Some(t) => t,
            None => LLVMContext::default_target_triple(),
        };

        match LLVMContext::new(&triple, opt_level) {
            Ok(ctx) => Ok(ctx),

            Err(e) => {
                error!(self.log, "Failed to create LLVM context for target '{}': {}", triple, e);

                Err(anyhow::anyhow!("Failed to create LLVM context"))
            }
        }
    }

    fn _get_search_paths(&self) -> Vec<PathBuf> {
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
        source_filepath: &std::path::Path,
        log: &CompilerLog,
    ) -> anyhow::Result<(hir::Module, hir::SymbolTab)> {
        let ptr_size = match ptr_size {
            4 => hir::PtrSize::U32,
            8 => hir::PtrSize::U64,
            _ => {
                error!(self.log, "Unsupported pointer size: {} bytes", ptr_size);
                return Err(anyhow::anyhow!("Unsupported pointer size"));
            }
        };

        let import_ctx = ImportContext::new(package_name.into(), source_filepath.into());
        let mut ctx = Ast2HirCtx::new(ptr_size, import_ctx);
        let module = match convert_ast_to_hir(module, &mut ctx, log) {
            Err(_) => return Err(anyhow::anyhow!("Failed to convert AST to HIR")),
            Ok(module) => module,
        };

        Ok((module, ctx.tab))
    }

    pub(crate) fn sc_build(&mut self, args: BuildArgs) -> anyhow::Result<()> {
        self.create_target_dir(&args.target_dir)?;
        let log = CompilerLog::new(self.log.clone());

        let package = self.get_package_config()?;
        self.validate_package_edition(package.edition())?;

        let ast_module = self.parse_source_code(&package.entrypoint(), package.name(), &log)?;
        if args.show_ast {
            self.show_ast(&ast_module, &args.format_mode);
            return Ok(());
        }

        let opt_level = if args.release {
            OptLevel::Aggressive
        } else {
            match &args.profile {
                Some(profile_name) if profile_name == "debug" => OptLevel::None,
                Some(profile_name) if profile_name == "release" => OptLevel::Aggressive,
                Some(profile_name) => {
                    error!(
                        self.log,
                        "Unknown build profile '{}'. Supported profiles are 'debug' and 'release'.", profile_name
                    );
                    return Err(anyhow::anyhow!("Unknown build profile"));
                }
                None => OptLevel::None,
            }
        };

        let llvm_ctx = self.get_llvm_context(args.target, opt_level)?;
        let ptr_size = llvm_ctx.target_data.get_pointer_byte_size(None);

        let store = Store::new();

        using_storage(&store, || {
            let (hir_module, symbol_tab) =
                self.lower_to_hir(ast_module, ptr_size, package.name(), &package.entrypoint(), &log)?;

            let mut hir_verifier = hir_validate::ValidateCtx::new(&symbol_tab);
            let valid_hir_module = match hir_module.clone().validate(&mut hir_verifier) {
                Ok(m) => m,
                Err(_) => {
                    error!(self.log, "HIR validation failed for package '{}'", package.name());

                    if args.show_hir {
                        println!("{}", hir_module.to_string());
                        return Ok(());
                    }

                    return Err(anyhow::anyhow!("HIR validation failed"));
                }
            };

            if args.show_hir {
                println!("{}", valid_hir_module.into_inner().to_string());
                return Ok(());
            }

            let mut llvm_module = generate_llvmir(package.name(), valid_hir_module, &llvm_ctx, &symbol_tab);

            llvm_ctx.optimize_module(&mut llvm_module);

            if args.show_llvmir {
                println!("{}", llvm_module.print_to_string().to_string());
                return Ok(());
            }

            if args.show_asm {
                if let Err(e) = llvm_ctx.write_asm(&mut llvm_module, &mut std::io::stdout()) {
                    error!(
                        self.log,
                        "Failed to write assembly file for package '{}': {}",
                        package.name(),
                        e
                    );

                    return Err(anyhow::anyhow!("Failed to write assembly file"));
                }
                return Ok(());
            }

            let target_file_o = format!(
                ".no3/build/{}-{}.{}.{}.o",
                package.name(),
                package.version().0,
                package.version().1,
                package.version().2
            );

            if let Err(e) = llvm_ctx.write_object_file(&mut llvm_module, std::path::Path::new(&target_file_o)) {
                error!(
                    self.log,
                    "Failed to write object file for package '{}': {}",
                    package.name(),
                    e
                );

                return Err(anyhow::anyhow!("Failed to write object file"));
            }

            if args.show_obj {
                info!(
                    self.log,
                    "Object file for package '{}' written to '{}'",
                    package.name(),
                    target_file_o
                );
                return Ok(());
            }

            // run system command
            let status = std::process::Command::new("clang")
                .args(&[&target_file_o, "-o"])
                .arg(format!(
                    "{}-{}.{}.{}",
                    package.name(),
                    package.version().0,
                    package.version().1,
                    package.version().2
                ))
                .status()
                .map_err(|e| {
                    error!(
                        self.log,
                        "Failed to link final binary for package '{}': {}",
                        package.name(),
                        e
                    );

                    e
                })?;

            if !status.success() {
                error!(
                    self.log,
                    "Linking final binary for package '{}' failed with exit code: {}",
                    package.name(),
                    status.code().unwrap_or(-1),
                );
                return Err(anyhow::anyhow!("Linking final binary failed"));
            }

            info!(
                self.log,
                "Successfully built package '{}' version {}.{}.{}",
                package.name(),
                package.version().0,
                package.version().1,
                package.version().2,
            );

            Ok(())
        })
    }
}
