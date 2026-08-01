use crate::{Interpreter, package::Manifest};
use clap::Parser;
use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    hir::{Store, prelude as hir, using_storage},
    hir_dump::Dump,
    hir_from_tree::{Ast2HirCtx, convert_ast_to_hir},
    hir_mangle::mangle_symbols,
    hir_validate::{self, ValidateHirItem},
    llvm::{LLVMContext, OptLevel},
    llvm_from_hir::generate_llvmir,
    parsetree::ast,
    token_lexer::{Lexer, LexerError},
    tree_resolve::ImportContext,
};
use slog::{debug, error, info};
use std::collections::HashSet;
use std::io::Read;
use std::path::{Path, PathBuf};

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
    pub(crate) release: bool,

    /// Build artifacts with the specified profile
    #[arg(long, group = "build-profile", value_name = "PROFILE-NAME")]
    pub(crate) profile: Option<String>,

    /// Build for the LLVM target triple
    #[arg(long, value_name = "TRIPLE")]
    pub(crate) target: Option<String>,

    /// Directory for all generated artifacts
    #[arg(long, value_name = "DIRECTORY")]
    pub(crate) target_dir: Option<PathBuf>,

    /// Number of parallel jobs, defaults to # of CPUs.
    #[arg(long, short = 'j', value_name = "N")]
    pub(crate) jobs: Option<usize>,

    /// Do not abort the build as soon as there is an error
    #[arg(long)]
    pub(crate) keep_going: bool,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<PathBuf>,

    /// Build only this package's library
    #[arg(long)]
    pub(crate) lib: bool,

    /// Build all binaries
    #[arg(long)]
    pub(crate) bins: bool,

    /// Build only the specified binary
    #[arg(long, value_name = "NAME")]
    pub(crate) bin: Option<String>,

    /// Build all examples
    #[arg(long)]
    pub(crate) examples: bool,

    /// Build only the specified example
    #[arg(long, value_name = "NAME")]
    pub(crate) example: Option<String>,

    /// Build all tests
    #[arg(long)]
    pub(crate) tests: bool,

    /// Build only the specified test target
    #[arg(long, value_name = "NAME")]
    pub(crate) test: Option<String>,

    /// Build all benchmarks
    #[arg(long)]
    pub(crate) benches: bool,

    /// Build only the specified benchmark target
    #[arg(long, value_name = "NAME")]
    pub(crate) bench: Option<String>,

    /// Build all targets
    #[arg(long)]
    pub(crate) all_targets: bool,

    /// Space or comma separated list of features to activate
    #[arg(long, short = 'F', value_name = "FEATURES")]
    pub(crate) features: Vec<String>,

    /// Activate all available features
    #[arg(long)]
    pub(crate) all_features: bool,

    /// Do not activate the `default` feature
    #[arg(long)]
    pub(crate) no_default_features: bool,

    /// Package to build (see `no3 help pkgid`)
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Build all packages in the workspace
    #[arg(long)]
    pub(crate) workspace: bool,

    /// Exclude packages from the build
    #[arg(long, value_name = "SPEC")]
    pub(crate) exclude: Vec<String>,
}

/// Shared compile options extracted from a cargo-style command.
#[derive(Debug, Clone)]
pub(crate) struct CompileOptions {
    pub(crate) release: bool,
    pub(crate) profile: Option<String>,
    pub(crate) target: Option<String>,
    pub(crate) target_dir: Option<PathBuf>,
    pub(crate) manifest_path: Option<PathBuf>,
    pub(crate) show_ast: bool,
    pub(crate) show_hir: bool,
    pub(crate) show_llvmir: bool,
    pub(crate) show_asm: bool,
    pub(crate) show_obj: bool,
    pub(crate) format_mode: Option<String>,
    /// Stop after HIR validation; don't emit object code or link.
    pub(crate) check_only: bool,
}

impl Default for CompileOptions {
    fn default() -> Self {
        Self {
            release: false,
            profile: None,
            target: None,
            target_dir: None,
            manifest_path: None,
            show_ast: false,
            show_hir: false,
            show_llvmir: false,
            show_asm: false,
            show_obj: false,
            format_mode: None,
            check_only: false,
        }
    }
}

impl From<&BuildArgs> for CompileOptions {
    fn from(args: &BuildArgs) -> Self {
        Self {
            release: args.release,
            profile: args.profile.clone(),
            target: args.target.clone(),
            target_dir: args.target_dir.clone(),
            manifest_path: args.manifest_path.clone(),
            show_ast: args.show_ast,
            show_hir: args.show_hir,
            show_llvmir: args.show_llvmir,
            show_asm: args.show_asm,
            show_obj: args.show_obj,
            format_mode: args.format_mode.clone(),
            check_only: false,
        }
    }
}

/// Resolve the manifest to compile, honoring `--manifest-path` if given.
pub(crate) fn resolve_manifest(manifest_path: Option<&Path>) -> anyhow::Result<Manifest> {
    match manifest_path {
        Some(path) => Manifest::load(path).map_err(|e| anyhow::anyhow!("{e}")),
        None => {
            let cwd = std::env::current_dir().map_err(|e| anyhow::anyhow!("failed to get current dir: {e}"))?;
            Manifest::discover(&cwd).map_err(|e| anyhow::anyhow!("{e}"))
        }
    }
}

/// Compute the target directory: `--target-dir`, or `<manifest_dir>/target`.
pub(crate) fn target_dir_for(manifest: &Manifest, explicit: Option<&Path>) -> PathBuf {
    match explicit {
        Some(dir) => dir.to_path_buf(),
        None => manifest.manifest_dir.join("target"),
    }
}

/// Compute the profile directory name (debug/release or custom profile).
pub(crate) fn profile_dir(opts: &CompileOptions) -> String {
    match &opts.profile {
        Some(p) => p.clone(),
        None if opts.release => "release".to_string(),
        None => "debug".to_string(),
    }
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

    pub(crate) fn parse_source_code(
        &self,
        entrypoint_path: &Path,
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

        let mut source_code_file = match std::fs::File::open(entrypoint_path) {
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

        let source_code_file = intern_file_id(entrypoint_path.to_string_lossy().as_ref()).expect("FileId overflow");

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

        let mut parser = nitrate_translation::parse::Parser::new(lexer, log);

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

    fn create_target_dir(&self, dir: &Path) -> anyhow::Result<()> {
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

    fn opt_level_for(opts: &CompileOptions) -> OptLevel {
        if opts.release {
            OptLevel::Aggressive
        } else {
            match &opts.profile {
                Some(profile_name) if profile_name == "debug" => OptLevel::None,
                Some(profile_name) if profile_name == "release" => OptLevel::Aggressive,
                _ => OptLevel::None,
            }
        }
    }

    fn lower_to_hir(
        &self,
        module: ast::Module,
        ptr_size: u32,
        package_name: &str,
        source_filepath: &Path,
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

    /// Run the full compilation pipeline for a package and produce a binary.
    pub(crate) fn compile_package(&mut self, opts: &CompileOptions) -> anyhow::Result<PathBuf> {
        let manifest = resolve_manifest(opts.manifest_path.as_deref())?;
        let target_dir = target_dir_for(&manifest, opts.target_dir.as_deref());
        let profile = profile_dir(opts);
        let build_dir = target_dir.join(&profile);

        self.create_target_dir(&build_dir)?;
        let log = CompilerLog::new(self.log.clone());

        self.validate_package_edition(manifest.package.edition_major())?;

        let opt_level = Self::opt_level_for(opts);
        let llvm_ctx = self.get_llvm_context(opts.target.clone(), opt_level)?;
        let ptr_size = llvm_ctx.target_data.get_pointer_byte_size(None);

        let ast_module = self.parse_source_code(&manifest.entrypoint(), &manifest.package.name, &log)?;
        if opts.show_ast {
            self.show_ast(&ast_module, &opts.format_mode);
            return Ok(PathBuf::new());
        }

        let store = Store::new();

        using_storage(&store, || {
            let (hir_module, symbol_tab) = self.lower_to_hir(
                ast_module,
                ptr_size,
                &manifest.package.name,
                &manifest.entrypoint(),
                &log,
            )?;

            let mut hir_verifier = hir_validate::ValidateCtx::new(&symbol_tab, &log);
            let valid_hir_module = match hir_module.clone().validate(&mut hir_verifier) {
                Ok(m) => m,
                Err(_) => {
                    error!(
                        self.log,
                        "HIR validation failed for package '{}'", manifest.package.name
                    );

                    if opts.show_hir {
                        println!("{}", hir_module.to_string());
                        return Ok(PathBuf::new());
                    }

                    return Err(anyhow::anyhow!("HIR validation failed"));
                }
            };

            if opts.show_hir {
                println!("{}", valid_hir_module.into_inner().to_string());
                return Ok(PathBuf::new());
            }

            if opts.check_only {
                info!(self.log, "Finished checking `{}`", manifest.package.name);
                return Ok(PathBuf::new());
            }

            // Apply name mangling to all functions and global variables.
            // This sets the `mangled_name` field on each symbol, which is what
            // appears in the object file during LLVM IR generation. The `name`
            // field is preserved for internal symbol lookup.
            let mut symbol_tab = symbol_tab;
            mangle_symbols(&manifest.package.name, &mut symbol_tab);

            let mut llvm_module = generate_llvmir(&manifest.package.name, valid_hir_module, &llvm_ctx, &symbol_tab);

            llvm_ctx.optimize_module(&mut llvm_module);

            if opts.show_llvmir {
                println!("{}", llvm_module.print_to_string().to_string());
                return Ok(PathBuf::new());
            }

            if opts.show_asm {
                if let Err(e) = llvm_ctx.write_asm(&mut llvm_module, &mut std::io::stdout()) {
                    error!(
                        self.log,
                        "Failed to write assembly file for package '{}': {}", manifest.package.name, e
                    );

                    return Err(anyhow::anyhow!("Failed to write assembly file"));
                }
                return Ok(PathBuf::new());
            }

            let (major, minor, patch) = manifest.package.major_minor_patch();
            let object_file = build_dir.join(format!("{}-{}.{}.{}.o", manifest.package.name, major, minor, patch));

            if let Err(e) = llvm_ctx.write_object_file(&mut llvm_module, &object_file) {
                error!(
                    self.log,
                    "Failed to write object file for package '{}': {}", manifest.package.name, e
                );

                return Err(anyhow::anyhow!("Failed to write object file"));
            }

            if opts.show_obj {
                info!(
                    self.log,
                    "Object file for package '{}' written to '{}'",
                    manifest.package.name,
                    object_file.display()
                );
                return Ok(PathBuf::new());
            }

            let binary_path = build_dir.join(&manifest.package.name);

            // Run system linker.
            let status = std::process::Command::new("clang")
                .arg(&object_file)
                .arg("-o")
                .arg(&binary_path)
                .status()
                .map_err(|e| {
                    error!(
                        self.log,
                        "Failed to link final binary for package '{}': {}", manifest.package.name, e
                    );

                    e
                })?;

            if !status.success() {
                error!(
                    self.log,
                    "Linking final binary for package '{}' failed with exit code: {}",
                    manifest.package.name,
                    status.code().unwrap_or(-1),
                );
                return Err(anyhow::anyhow!("Linking final binary failed"));
            }

            info!(
                self.log,
                "Successfully built package '{}' v{}.{}.{}", manifest.package.name, major, minor, patch,
            );

            Ok(binary_path)
        })
    }

    pub(crate) fn sc_build(&mut self, args: BuildArgs) -> anyhow::Result<()> {
        let opts = CompileOptions::from(&args);
        let manifest = resolve_manifest(opts.manifest_path.as_deref())?;
        let target_dir = target_dir_for(&manifest, opts.target_dir.as_deref());

        self.compile_package(&opts)?;

        debug!(
            self.log,
            "Build artifacts in '{}'",
            target_dir.join(profile_dir(&opts)).display()
        );

        Ok(())
    }
}
