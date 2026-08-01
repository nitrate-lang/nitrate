use crate::Interpreter;
use crate::commands::build::{CompileOptions, resolve_manifest, target_dir_for};
use clap::Parser;
use slog::info;
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct RunArgs {
    /// Arguments for the binary or example to run
    #[arg(trailing_var_arg = true, allow_hyphen_values = true)]
    pub(crate) args: Vec<String>,

    /// Name of the bin target to run
    #[arg(long, value_name = "NAME")]
    pub(crate) bin: Option<String>,

    /// Name of the example target to run
    #[arg(long, value_name = "NAME")]
    pub(crate) example: Option<String>,

    /// Package with the target to run
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Build artifacts in release mode, with optimizations
    #[arg(long, short = 'r', group = "build-profile")]
    pub(crate) release: bool,

    /// Build artifacts with the specified profile
    #[arg(long, group = "build-profile", value_name = "PROFILE-NAME")]
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

    /// Number of parallel jobs, defaults to # of CPUs.
    #[arg(long, short = 'j', value_name = "N")]
    pub(crate) jobs: Option<usize>,

    /// Do not abort the build as soon as there is an error
    #[arg(long)]
    pub(crate) keep_going: bool,

    /// Space or comma separated list of features to activate
    #[arg(long, short = 'F', value_name = "FEATURES")]
    pub(crate) features: Vec<String>,

    /// Activate all available features
    #[arg(long)]
    pub(crate) all_features: bool,

    /// Do not activate the `default` feature
    #[arg(long)]
    pub(crate) no_default_features: bool,
}

impl Interpreter<'_> {
    pub(crate) fn sc_run(&mut self, args: RunArgs) -> anyhow::Result<()> {
        let mut opts = CompileOptions::default();
        opts.release = args.release;
        opts.profile = args.profile;
        opts.target = args.target;
        opts.target_dir = args.target_dir;
        opts.manifest_path = args.manifest_path;

        let binary_path = self.compile_package(&opts)?;
        if binary_path.as_os_str().is_empty() {
            return Ok(());
        }

        let manifest = resolve_manifest(opts.manifest_path.as_deref())?;
        let target_dir = target_dir_for(&manifest, opts.target_dir.as_deref());
        let profile = crate::commands::build::profile_dir(&opts);
        let final_binary = if binary_path.is_absolute() || binary_path.exists() {
            binary_path
        } else {
            target_dir.join(&profile).join(&manifest.package.name)
        };

        info!(self.log, "Running `{}`", final_binary.display());

        let status = std::process::Command::new(&final_binary)
            .args(&args.args)
            .status()
            .map_err(|e| anyhow::anyhow!("Failed to launch '{}': {}", final_binary.display(), e))?;

        if !status.success() {
            std::process::exit(status.code().unwrap_or(1));
        }

        Ok(())
    }
}
