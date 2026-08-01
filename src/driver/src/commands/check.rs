use crate::Interpreter;
use crate::commands::build::CompileOptions;
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct CheckArgs {
    /// Build artifacts in release mode, with optimizations
    #[arg(long, short = 'r', group = "build-profile")]
    pub(crate) release: bool,

    /// Build artifacts with the specified profile
    #[arg(long, group = "build-profile", value_name = "PROFILE-NAME")]
    pub(crate) profile: Option<String>,

    /// Check for the target triple
    #[arg(long, value_name = "TRIPLE")]
    pub(crate) target: Option<String>,

    /// Directory for all generated artifacts
    #[arg(long, value_name = "DIRECTORY")]
    pub(crate) target_dir: Option<std::path::PathBuf>,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<std::path::PathBuf>,

    /// Number of parallel jobs, defaults to # of CPUs.
    #[arg(long, short = 'j', value_name = "N")]
    pub(crate) jobs: Option<usize>,

    /// Do not abort the build as soon as there is an error
    #[arg(long)]
    pub(crate) keep_going: bool,

    /// Package(s) to check
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Check all packages in the workspace
    #[arg(long)]
    pub(crate) workspace: bool,

    /// Exclude packages from the check
    #[arg(long, value_name = "SPEC")]
    pub(crate) exclude: Vec<String>,

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
    pub(crate) fn sc_check(&mut self, args: CheckArgs) -> anyhow::Result<()> {
        let mut opts = CompileOptions::default();
        opts.release = args.release;
        opts.profile = args.profile;
        opts.target = args.target;
        opts.target_dir = args.target_dir;
        opts.manifest_path = args.manifest_path;
        opts.check_only = true;

        self.compile_package(&opts)?;

        info!(self.log, "no3 check completed successfully");
        Ok(())
    }
}
