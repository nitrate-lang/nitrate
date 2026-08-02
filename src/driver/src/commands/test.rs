use crate::Interpreter;
use crate::commands::build::{CompileOptions, resolve_manifest, target_dir_for};
use clap::Parser;
use slog::info;
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct TestArgs {
    /// Filter tests by this name
    #[arg(value_name = "TESTNAME")]
    pub(crate) testname: Option<String>,

    /// Arguments for the test binary
    #[arg(last = true, value_name = "ARGS")]
    pub(crate) args: Vec<String>,

    /// Compile, but don't run tests
    #[arg(long)]
    pub(crate) no_run: bool,

    /// Run all tests regardless of failure
    #[arg(long)]
    pub(crate) no_fail_fast: bool,

    /// Display one character per test instead of one line
    #[arg(long, short = 'q')]
    pub(crate) quiet: bool,

    /// Build artifacts in release mode, with optimizations
    #[arg(long, short = 'r')]
    pub(crate) release: bool,

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

    /// Package to run tests for
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Test all packages in the workspace
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

    /// Test only this library's documentation
    #[arg(long)]
    pub(crate) doc: bool,
}

impl Interpreter<'_> {
    pub(crate) fn sc_test(&mut self, args: TestArgs) -> anyhow::Result<()> {
        let mut opts = CompileOptions::default();
        opts.release = args.release;
        opts.profile = args.profile;
        opts.target = args.target;
        opts.target_dir = args.target_dir;
        opts.manifest_path = args.manifest_path;

        // Build first: tests compile the package, and running them finds
        // `#[test]`-attributed functions via the compiled artifact.
        let binary_path = self
            .compile_package(&opts)?
            .expect("Failed to compile package for tests");
        if binary_path.as_os_str().is_empty() {
            return Ok(());
        }

        if args.no_run {
            info!(self.log, "tests compiled (not run)");
            return Ok(());
        }

        let manifest = resolve_manifest(opts.manifest_path.as_deref())?;
        let target_dir = target_dir_for(&manifest, opts.target_dir.as_deref());
        let profile = crate::commands::build::profile_dir(&opts);
        let test_binary = if binary_path.is_absolute() || binary_path.exists() {
            binary_path
        } else {
            target_dir.join(&profile).join(&manifest.package.name)
        };

        info!(self.log, "Running tests in `{}`", test_binary.display());

        let mut cmd = std::process::Command::new(&test_binary);
        if let Some(filter) = &args.testname {
            cmd.arg(filter);
        }
        if !args.args.is_empty() {
            cmd.arg("--").args(&args.args);
        }
        if args.quiet {
            cmd.arg("--quiet");
        }

        let status = cmd
            .status()
            .map_err(|e| anyhow::anyhow!("Failed to run test binary: {e}"))?;

        if !status.success() {
            std::process::exit(status.code().unwrap_or(1));
        }

        Ok(())
    }
}
