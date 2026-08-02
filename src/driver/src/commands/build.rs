use crate::{Interpreter, package::Manifest};
use clap::Parser;
use nitrate_diagnosis::CompilerLog;
use nitrate_translation::{LlvmGenerated, MirLowered};
use nitrate_translation::{Pipeline, PipelineConfig, hir, llvm::OptLevel, mir};
use slog::{debug, error, info};
use std::collections::HashSet;
use std::num::NonZero;
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

    /// Codegen options: -C opt-level=3, -C target-cpu=native, -C passes=inline,constfold
    #[arg(long = "codegen", short = 'C', value_name = "OPTION[=VALUE]", number_of_values = 1)]
    pub(crate) codegen_opts: Vec<String>,

    /// Disable default optimization passes
    #[arg(long)]
    pub(crate) no_default_passes: bool,
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
    /// Raw `-C` codegen options to parse.
    pub(crate) codegen_opts: Vec<String>,
    /// Disable default optimization passes.
    pub(crate) no_default_passes: bool,
    /// Number of parallel jobs.
    pub(crate) jobs: Option<usize>,
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
            codegen_opts: Vec::new(),
            no_default_passes: false,
            jobs: None,
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
            codegen_opts: args.codegen_opts.clone(),
            no_default_passes: args.no_default_passes,
            jobs: args.jobs,
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

/// Compute the optimization level from compile options.
pub(crate) fn opt_level_for(opts: &CompileOptions) -> OptLevel {
    match opts
        .codegen_opts
        .iter()
        .find_map(|opt| opt.strip_prefix("opt-level=").and_then(|v| v.parse::<u8>().ok()))
    {
        Some(0) => OptLevel::None,
        Some(1) => OptLevel::Default,
        Some(_) | None if opts.release => OptLevel::Aggressive,
        _ => OptLevel::None,
    }
}

/// Extract the target CPU from `-C target-cpu=...` codegen options.
pub(crate) fn target_cpu_from_opts(opts: &CompileOptions) -> Option<String> {
    opts.codegen_opts
        .iter()
        .find_map(|opt| opt.strip_prefix("target-cpu=").map(String::from))
}

/// Build a `PipelineConfig` from compile options and manifest.
pub(crate) fn pipeline_config_from_opts(opts: &CompileOptions, manifest: &Manifest) -> PipelineConfig {
    let log = CompilerLog::new(slog::Logger::root(slog::Discard, slog::o!()));
    let opt_level = opt_level_for(opts);
    let target_cpu = target_cpu_from_opts(opts);

    PipelineConfig {
        package_name: manifest.package.name.clone(),
        target_triple: opts.target.clone(),
        target_cpu,
        opt_level,
        hir_passes: Vec::new(),
        hir_module_passes: Vec::new(),
        mir_passes: Vec::new(),
        mir_module_passes: Vec::new(),
        no_default_passes: opts.no_default_passes,
        thread_count: opts
            .jobs
            .and_then(|j| NonZero::new(j))
            .unwrap_or_else(|| NonZero::new(1).unwrap()),
        log,
    }
}

/// Helper: link an object file into a binary using the system linker.
fn link_binary(log: &slog::Logger, object_file: &Path, binary_path: &Path, package_name: &str) -> anyhow::Result<()> {
    let status = std::process::Command::new("clang")
        .arg(object_file)
        .arg("-o")
        .arg(binary_path)
        .status()
        .map_err(|e| {
            error!(log, "Failed to link final binary for package '{}': {}", package_name, e);
            e
        })?;

    if !status.success() {
        error!(
            log,
            "Linking final binary for package '{}' failed with exit code: {}",
            package_name,
            status.code().unwrap_or(-1),
        );
        return Err(anyhow::anyhow!("Linking final binary failed"));
    }

    Ok(())
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

    /// Run the full compilation pipeline for a package and produce a binary.
    ///
    /// Uses the centralized `Pipeline` type-state builder from `nitrate_translation`.
    pub(crate) fn compile_package(&mut self, opts: &CompileOptions) -> anyhow::Result<Option<PathBuf>> {
        let manifest = resolve_manifest(opts.manifest_path.as_deref())?;
        let target_dir = target_dir_for(&manifest, opts.target_dir.as_deref());
        let profile = profile_dir(opts);
        let build_dir = target_dir.join(&profile);

        self.create_target_dir(&build_dir)?;
        self.validate_package_edition(manifest.package.edition_major())?;

        // Build pipeline configuration
        let mut config = pipeline_config_from_opts(opts, &manifest);
        config.log = CompilerLog::new(self.log.clone());

        let entrypoint = manifest.entrypoint();
        let package_name = manifest.package.name.clone();

        // Create the HIR Store upfront
        let mut hir_store = hir::Store::new();
        let mut mir_store = mir::MirStore::new();

        let pipeline = Pipeline::new(config);

        // Load → Lex → Parse (no Store needed)
        let source = pipeline.load_source(&entrypoint)?;
        let tokenized = source.lex()?;
        let parsed = tokenized.parse()?;

        if opts.show_ast {
            let pretty = opts.format_mode.as_deref() != Some("minify");
            parsed.dump_ast(pretty);
            return Ok(None);
        }

        let ptr_size = std::mem::size_of::<*const u8>() as u32;

        // HIR stages run inside HIR TLS storage
        let mir_lowered = hir::using_storage(&hir_store, || -> anyhow::Result<Option<MirLowered>> {
            let hir_lowered = parsed.lower_hir(ptr_size)?;

            if opts.show_hir {
                hir_lowered.dump_hir();
                return Ok(None);
            }

            let hir_validated = hir_lowered.validate()?;

            if opts.check_only {
                hir_validated.finish_check();
                info!(self.log, "Finished checking `{}`", package_name);
                return Ok(None);
            }

            let hir_mangled = hir_validated.mangle();

            // MIR stages need their own TLS storage (separate from HIR)
            let mir_lowered = mir::using_storage(&mir_store, || -> MirLowered { hir_mangled.lower_mir() });

            Ok(Some(mir_lowered))
        });

        // Reset the HIR store to free memory before codegen
        hir_store.reset();

        if let Some(mir_lowered) = mir_lowered? {
            let llvm_generated = mir::using_storage(&mir_store, || -> anyhow::Result<LlvmGenerated> {
                let llvm_generated = mir_lowered.codegen()?;
                Ok(llvm_generated)
            })?;

            // Reset the MIR store to free memory before codegen
            mir_store.reset();

            if opts.show_llvmir {
                llvm_generated.dump_llvm_ir();
                return Ok(None);
            }

            if opts.show_asm {
                llvm_generated.dump_asm()?;
                return Ok(None);
            }

            let (major, minor, patch) = manifest.package.major_minor_patch();
            let object_file = build_dir.join(format!("{}-{}.{}.{}.o", package_name, major, minor, patch));

            let emitted = llvm_generated.optimize_llvm().emit_obj(&object_file)?;

            if opts.show_obj {
                info!(
                    self.log,
                    "Object file for package '{}' written to '{}'",
                    package_name,
                    emitted.path().display()
                );
                return Ok(None);
            }

            let binary_path = build_dir.join(&package_name);
            link_binary(self.log, emitted.path(), &binary_path, &package_name)?;

            info!(
                self.log,
                "Successfully built package '{}' v{}.{}.{}", package_name, major, minor, patch,
            );

            Ok(Some(binary_path))
        } else {
            Ok(None)
        }
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
