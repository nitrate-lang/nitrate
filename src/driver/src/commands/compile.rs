use crate::Interpreter;
use crate::commands::build::{CompileOptions, link_binary, opt_level_for, target_cpu_from_opts};
use clap::Parser;
use nitrate_diagnosis::CompilerLog;
use nitrate_translation::MirLowered;
use nitrate_translation::{Pipeline, PipelineConfig, hir, mir};
use slog::{debug, error, info};
use std::num::NonZero;
use std::path::{Path, PathBuf};

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct CompileArgs {
    /// Source file to compile
    #[arg(value_name = "FILE")]
    pub(crate) file: PathBuf,

    /// Write the compiled binary to PATH (default: <FILE stem> in the current directory)
    #[arg(short = 'o', long, value_name = "PATH")]
    pub(crate) output: Option<PathBuf>,

    /// Directory for all generated artifacts (default: <FILE dir>/target)
    #[arg(long, value_name = "DIRECTORY")]
    pub(crate) target_dir: Option<PathBuf>,

    /// Build artifacts in release mode, with optimizations
    #[arg(long, short = 'r', group = "build-profile")]
    pub(crate) release: bool,

    /// Build for the LLVM target triple
    #[arg(long, value_name = "TRIPLE")]
    pub(crate) target: Option<String>,

    /// Number of parallel jobs, defaults to # of CPUs.
    #[arg(long, short = 'j', value_name = "N")]
    pub(crate) jobs: Option<usize>,

    /// Codegen options: -C opt-level=3, -C target-cpu=native, -C passes=inline,constfold
    #[arg(long = "codegen", short = 'C', value_name = "OPTION[=VALUE]", number_of_values = 1)]
    pub(crate) codegen_opts: Vec<String>,

    /// Disable default optimization passes
    #[arg(long)]
    pub(crate) no_default_passes: bool,

    /// Emit the Abstract Syntax Tree (AST) and exit
    #[arg(long, group = "output-mode")]
    pub(crate) emit_ast: bool,

    /// Emit the High-Level Intermediate Representation (HIR) and exit
    #[arg(long, group = "output-mode")]
    pub(crate) emit_hir: bool,

    /// Emit the LLVM Intermediate Representation and exit
    #[arg(long, group = "output-mode")]
    pub(crate) emit_llvmir: bool,

    /// Export the MIR control-flow graph as a Graphviz DOT file
    #[arg(long)]
    pub(crate) emit_mir: bool,

    /// Emit the Assembly Code and exit
    #[arg(long, group = "output-mode")]
    pub(crate) emit_asm: bool,

    /// Emit the object file and exit without linking
    #[arg(long, group = "output-mode")]
    pub(crate) emit_obj: bool,

    /// Analyze the file and report errors, but don't emit object files or link
    #[arg(long)]
    pub(crate) check: bool,
}

impl Interpreter<'_> {
    pub(crate) fn sc_compile(&mut self, args: CompileArgs) -> anyhow::Result<()> {
        let file_path = &args.file;

        if !file_path.exists() {
            error!(
                self.log,
                "Source file '{}' does not exist.",
                file_path.display()
            );
            return Err(anyhow::anyhow!("Source file does not exist"));
        }

        // Derive a package name from the file stem. This is used as the module
        // name during parsing and as the mangling package qualifier.
        let package_name = file_path
            .file_stem()
            .and_then(|stem| stem.to_str())
            .filter(|stem| !stem.is_empty())
            .map(String::from)
            .unwrap_or_else(|| "main".to_string());

        let profile = if args.release { "release" } else { "debug" };
        let target_dir = match &args.target_dir {
            Some(dir) => dir.clone(),
            None => file_path
                .parent()
                .filter(|parent| !parent.as_os_str().is_empty())
                .unwrap_or_else(|| Path::new("."))
                .join("target"),
        };
        let build_dir = target_dir.join(profile);

        self.create_target_dir(&build_dir)?;

        let opts = CompileOptions {
            release: args.release,
            profile: None,
            target: args.target.clone(),
            target_dir: args.target_dir.clone(),
            manifest_path: None,
            emit_ast: args.emit_ast,
            emit_hir: args.emit_hir,
            emit_llvmir: args.emit_llvmir,
            emit_asm: args.emit_asm,
            emit_obj: args.emit_obj,
            format_mode: None,
            emit_mir: args.emit_mir,
            check_only: args.check,
            codegen_opts: args.codegen_opts.clone(),
            no_default_passes: args.no_default_passes,
            jobs: args.jobs,
        };

        // Build pipeline configuration for a single-file compilation.
        let config = PipelineConfig {
            package_name: package_name.clone(),
            target_triple: args.target.clone(),
            target_cpu: target_cpu_from_opts(&opts),
            opt_level: opt_level_for(&opts),
            hir_passes: Vec::new(),
            hir_module_passes: Vec::new(),
            mir_passes: Vec::new(),
            mir_module_passes: Vec::new(),
            no_default_passes: args.no_default_passes,
            thread_count: args
                .jobs
                .and_then(NonZero::new)
                .unwrap_or_else(|| NonZero::new(1).unwrap()),
            log: CompilerLog::new(self.log.clone()),
        };
        let log = config.log.clone();

        // Create the HIR Store upfront
        let mut hir_store = hir::Store::new();
        let mir_store = mir::MirStore::new();

        let pipeline = Pipeline::new(config);

        // Load → Lex → Parse (no Store needed)
        let source = pipeline.load_source(file_path)?;
        let tokenized = source.lex()?;
        let parsed = tokenized.parse()?;

        if args.emit_ast {
            parsed.dump_ast(true);
            return Ok(());
        }

        let ptr_size = std::mem::size_of::<*const u8>() as u32;

        // HIR stages run inside HIR TLS storage
        let mir_lowered = hir::using_storage(&hir_store, || -> anyhow::Result<Option<MirLowered>> {
            let hir_lowered = parsed.lower_hir(ptr_size)?;

            if args.emit_hir {
                hir_lowered.dump_hir();
                return Ok(None);
            }

            let hir_validated = hir_lowered.validate()?;

            if args.check {
                // Run the full semantic pipeline through MIR lowering so the
                // MIR borrow checker (memory-safety enforcement) is exercised.
                let hir_mangled = hir_validated.mangle();
                mir::using_storage(&mir_store, || hir_mangled.lower_mir());
                if log.error_bit() {
                    return Err(anyhow::anyhow!(
                        "check failed: the file has errors (see diagnostics above)"
                    ));
                }
                info!(self.log, "Finished checking `{}`", file_path.display());
                return Ok(None);
            }

            let hir_mangled = hir_validated.mangle();

            // MIR stages need their own TLS storage (separate from HIR)
            let mir_lowered =
                mir::using_storage(&mir_store, || -> MirLowered { hir_mangled.lower_mir() });

            Ok(Some(mir_lowered))
        });

        // Reset the HIR store to free memory before codegen
        hir_store.reset();

        if let Some(mir_lowered) = mir_lowered? {
            mir::using_storage(&mir_store, || -> anyhow::Result<()> {
                if args.emit_mir {
                    let dot_path = build_dir.join(format!("{package_name}.mir.dot"));
                    mir_lowered.dump_mir_dot(&dot_path)?;
                    info!(
                        self.log,
                        "MIR control-flow graph written to '{}'",
                        dot_path.display()
                    );
                    return Ok(());
                }

                let llvm_generated = mir_lowered.codegen()?;

                if args.emit_llvmir {
                    llvm_generated.dump_llvm_ir()?;
                    return Ok(());
                }

                if args.emit_asm {
                    llvm_generated.dump_asm()?;
                    return Ok(());
                }

                let object_file = build_dir.join(format!("{package_name}.o"));
                let emitted = llvm_generated.optimize_llvm().emit_obj(&object_file)?;

                if args.emit_obj {
                    info!(
                        self.log,
                        "Object file for '{}' written to '{}'",
                        file_path.display(),
                        emitted.path().display()
                    );
                    return Ok(());
                }

                let binary_path = args
                    .output
                    .clone()
                    .unwrap_or_else(|| PathBuf::from(&package_name));
                link_binary(self.log, emitted.path(), &binary_path, &package_name)?;

                info!(
                    self.log,
                    "Successfully compiled '{}' -> '{}'",
                    file_path.display(),
                    binary_path.display()
                );

                debug!(
                    self.log,
                    "Build artifacts in '{}'",
                    build_dir.display()
                );

                Ok(())
            })?;
        }

        Ok(())
    }
}

