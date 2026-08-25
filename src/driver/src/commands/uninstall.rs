use crate::Interpreter;
use clap::Parser;
use slog::{error, info};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct UninstallArgs {
    /// Binary to uninstall
    #[arg(value_name = "SPEC")]
    pub(crate) spec: Vec<String>,

    /// Directory to uninstall packages from
    #[arg(long, value_name = "DIR")]
    pub(crate) root: Option<PathBuf>,

    /// Package to uninstall
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Only uninstall the binary NAME
    #[arg(long, value_name = "NAME")]
    pub(crate) bin: Option<String>,
}

fn uninstall_root(explicit: Option<&PathBuf>) -> PathBuf {
    if let Some(dir) = explicit {
        return dir.clone();
    }
    if let Ok(root) = std::env::var("NO3_INSTALL_ROOT") {
        return PathBuf::from(root);
    }
    let home = std::env::var("HOME").unwrap_or_else(|_| ".".to_string());
    PathBuf::from(home).join(".no3").join("bin")
}

impl Interpreter<'_> {
    pub(crate) fn sc_uninstall(&mut self, args: UninstallArgs) -> anyhow::Result<()> {
        let root = uninstall_root(args.root.as_ref());

        let names: Vec<String>;
        if !args.spec.is_empty() {
            names = args.spec.clone();
        } else if let Some(pkg) = &args.package {
            names = vec![pkg.clone()];
        } else if let Some(bin) = &args.bin {
            names = vec![bin.clone()];
        } else {
            names = Vec::new();
        }

        if names.is_empty() {
            return Err(anyhow::anyhow!(
                "no packages specified (use `no3 uninstall <name>` or `no3 uninstall --package <name>`)"
            ));
        }

        let mut removed = false;
        for name in &names {
            let candidate = root.join(name);
            if candidate.exists() {
                std::fs::remove_file(&candidate)
                    .map_err(|e| anyhow::anyhow!("Failed to remove '{}': {}", candidate.display(), e))?;
                info!(self.log, "Removed `{}` from {}", name, root.display());
                removed = true;
            } else {
                error!(self.log, "binary `{}` is not installed", name);
            }
        }

        if !removed {
            return Err(anyhow::anyhow!("No binaries were uninstalled"));
        }

        Ok(())
    }
}
