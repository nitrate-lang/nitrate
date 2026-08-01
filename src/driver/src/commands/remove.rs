use crate::Interpreter;
use crate::commands::build::resolve_manifest;
use crate::package::MANIFEST_FILE;
use clap::Parser;
use slog::{error, info};
use std::path::PathBuf;
use std::str::FromStr;
use std::vec;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct RemoveArgs {
    /// Dependencies to be removed
    #[arg(value_name = "DEP_ID", required = true)]
    pub(crate) dep_id: Vec<String>,

    /// Don't actually write the manifest
    #[arg(long, short = 'n')]
    pub(crate) dry_run: bool,

    /// Remove from dev-dependencies
    #[arg(long)]
    pub(crate) dev: bool,

    /// Remove from build-dependencies
    #[arg(long)]
    pub(crate) build: bool,

    /// Remove from target-dependencies
    #[arg(long, value_name = "TARGET")]
    pub(crate) target: Option<String>,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<PathBuf>,

    /// Package to remove from
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,
}

impl Interpreter<'_> {
    pub(crate) fn sc_remove(&mut self, args: RemoveArgs) -> anyhow::Result<()> {
        let manifest = resolve_manifest(args.manifest_path.as_deref()).map_err(|e| anyhow::anyhow!("{e}"))?;
        let manifest_path = manifest.manifest_dir.join(MANIFEST_FILE);

        if !manifest_path.exists() {
            error!(self.log, "No manifest file found at '{}'", manifest_path.display());
            return Err(anyhow::anyhow!("No manifest file found"));
        }

        let mut doc = toml_edit::DocumentMut::from_str(
            &std::fs::read_to_string(&manifest_path).map_err(|e| anyhow::anyhow!("{e}"))?,
        )
        .map_err(|e| anyhow::anyhow!("{e}"))?;

        let mut removed = Vec::new();

        // Cargo removes a dep from all sections if no section flag is given.
        let section_names: Vec<&str> = if args.dev {
            vec!["dev-dependencies"]
        } else if args.build {
            vec!["build-dependencies"]
        } else {
            vec!["dependencies", "dev-dependencies", "build-dependencies"]
        };

        for section in &section_names {
            if let Some(table) = doc.get_mut(*section).and_then(|t| t.as_table_mut()) {
                for dep in &args.dep_id {
                    if table.contains_key(dep) {
                        table.remove(dep);
                        removed.push((section.to_string(), dep.clone()));
                    }
                }
            }
        }

        if removed.is_empty() {
            error!(
                self.log,
                "Dependency `{}` was not found in the manifest",
                args.dep_id.join(", ")
            );
            return Err(anyhow::anyhow!("Dependency not found"));
        }

        if args.dry_run {
            info!(self.log, "Dry run, not writing manifest.");
            return Ok(());
        }

        std::fs::write(&manifest_path, doc.to_string()).map_err(|e| anyhow::anyhow!("{e}"))?;

        for (section, dep) in &removed {
            info!(self.log, "Removing {} from [{}]", dep, section);
        }

        Ok(())
    }
}
