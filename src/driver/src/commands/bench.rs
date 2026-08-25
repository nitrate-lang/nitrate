use crate::Interpreter;
use crate::commands::build::{CompileOptions, resolve_manifest, target_dir_for};
use clap::Parser;
use slog::info;
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct BenchArgs {
    /// Filter benchmarks by this name
    #[arg(value_name = "BENCHNAME")]
    pub(crate) benchname: Option<String>,

    /// Arguments for the bench binary
    #[arg(last = true, value_name = "ARGS")]
    pub(crate) args: Vec<String>,

    /// Compile, but don't run benchmarks
    #[arg(long)]
    pub(crate) no_run: bool,

    /// Run all benchmarks regardless of failure
    #[arg(long)]
    pub(crate) no_fail_fast: bool,

    /// Build artifacts with the specified profile
    #[arg(long, value_name = "PROFILE-NAME")]
    pub(crate) profile: Option<String>,

    /// Build for the target triple
    #[arg(long, value_name = "TRIPLE")]
    pub(crate) target: Option<String>,

    /// Directory for all generated artifacts
    #[arg(long, value_name = "DIRECTORY")]
    pub(crate) target_dir: Option<PathBuf>,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<PathBuf>,

    /// Package to run benchmarks for
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Benchmark all packages in the workspace
    #[arg(long)]
    pub(crate) workspace: bool,

    /// Space or comma separated list of features to activate
    #[arg(long, short = 'F', value_name = "FEATURES")]
    pub(crate) features: Vec<String>,

    /// Activate all available features
    #[arg(long)]
    pub(crate) all_features: bool,

    /// Do not activate the `default` feature
    #[arg(long)]
    pub(crate) no_default_features: bool,

    /// Number of parallel jobs, defaults to # of CPUs
    #[arg(long, short = 'j', value_name = "N")]
    pub(crate) jobs: Option<usize>,
}

impl Interpreter<'_> {
    pub(crate) fn sc_bench(&mut self, args: BenchArgs) -> anyhow::Result<()> {
        let mut opts = CompileOptions::default();
        opts.profile = Some(args.profile.clone().unwrap_or_else(|| "release".to_string()));
        opts.release = args.profile.is_none();
        opts.target = args.target;
        opts.target_dir = args.target_dir;
        opts.manifest_path = args.manifest_path;

        let binary_path = self
            .compile_package(&opts)?
            .expect("Failed to compile package for benchmarks");
        if binary_path.as_os_str().is_empty() {
            return Ok(());
        }

        if args.no_run {
            info!(self.log, "benchmarks compiled (not run)");
            return Ok(());
        }

        let manifest = resolve_manifest(opts.manifest_path.as_deref())?;
        let target_dir = target_dir_for(&manifest, opts.target_dir.as_deref());
        let profile = crate::commands::build::profile_dir(&opts);
        let bench_binary = if binary_path.is_absolute() || binary_path.exists() {
            binary_path
        } else {
            target_dir.join(&profile).join(&manifest.package.name)
        };

        info!(self.log, "Running benchmarks in `{}`", bench_binary.display());

        let mut cmd = std::process::Command::new(&bench_binary);
        if let Some(filter) = &args.benchname {
            cmd.arg(filter);
        }
        if !args.args.is_empty() {
            cmd.arg("--").args(&args.args);
        }

        let status = cmd
            .status()
            .map_err(|e| anyhow::anyhow!("Failed to run bench binary: {e}"))?;

        if !status.success() {
            std::process::exit(status.code().unwrap_or(1));
        }

        Ok(())
    }
}
