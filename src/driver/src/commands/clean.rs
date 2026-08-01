use crate::Interpreter;
use crate::commands::build::{resolve_manifest, target_dir_for};
use clap::Parser;
use slog::{error, info};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct CleanArgs {
    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<PathBuf>,

    /// Directory for all generated artifacts
    #[arg(long, value_name = "DIRECTORY")]
    pub(crate) target_dir: Option<PathBuf>,

    /// Clean all packages in the workspace
    #[arg(long)]
    pub(crate) workspace: bool,

    /// Clean only this package's library
    #[arg(long)]
    pub(crate) lib: bool,

    /// Whether to clean the release directory
    #[arg(long, short = 'r')]
    pub(crate) release: bool,

    /// Whether to clean the debug directory
    #[arg(long)]
    pub(crate) debug: bool,

    /// Whether to clean the doc directory
    #[arg(long)]
    pub(crate) doc: bool,

    /// Whether to clean the profile directory
    #[arg(long, value_name = "PROFILE-NAME")]
    pub(crate) profile: Option<String>,

    /// Whether to clean all artifacts
    #[arg(long)]
    pub(crate) all: bool,

    // Support `--dry-run` like newer cargo versions.
    #[arg(long, short = 'n')]
    pub(crate) dry_run: bool,

    // Legacy behavior removed; `no3 clean` now removes `target/`.
    #[arg(long, hide = true)]
    _legacy: bool,
}

impl Interpreter<'_> {
    pub(crate) fn sc_clean(&mut self, args: CleanArgs) -> anyhow::Result<()> {
        let manifest = resolve_manifest(args.manifest_path.as_deref())?;
        let target_dir = target_dir_for(&manifest, args.target_dir.as_deref());

        let dir_to_remove = match &args.profile {
            Some(profile) => target_dir.join(profile),
            None if args.release => target_dir.join("release"),
            None if args.debug => target_dir.join("debug"),
            None if args.doc => target_dir.join("doc"),
            None => target_dir.clone(),
        };

        if args.dry_run {
            info!(
                self.log,
                "Would remove build artifacts in '{}'",
                dir_to_remove.display()
            );
            return Ok(());
        }

        if !dir_to_remove.exists() {
            info!(
                self.log,
                "Build artifacts directory '{}' is clean",
                dir_to_remove.display()
            );
            return Ok(());
        }

        if let Err(e) = std::fs::remove_dir_all(&dir_to_remove) {
            error!(
                self.log,
                "Failed to remove build artifacts directory '{}': {}",
                dir_to_remove.display(),
                e
            );
            return Err(e.into());
        }

        info!(self.log, "Removed build artifacts in '{}'", dir_to_remove.display());

        Ok(())
    }
}
