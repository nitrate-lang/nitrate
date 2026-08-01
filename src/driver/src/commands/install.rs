use crate::Interpreter;
use crate::package::DEFAULT_REGISTRY;
use clap::Parser;
use slog::{error, info};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct InstallArgs {
    /// Select the package from the given source
    #[arg(value_name = "CRATE")]
    pub(crate) crate_spec: Vec<String>,

    /// Specify a version to install
    #[arg(long, value_name = "VERSION")]
    pub(crate) version: Option<String>,

    /// Registry index to install from
    #[arg(long)]
    pub(crate) index: Option<String>,

    /// Registry to use
    #[arg(long)]
    pub(crate) registry: Option<String>,

    /// Git URL to install the specified crate from
    #[arg(long)]
    pub(crate) git: Option<String>,

    /// Branch to use when installing from git
    #[arg(long)]
    pub(crate) branch: Option<String>,

    /// Tag to use when installing from git
    #[arg(long)]
    pub(crate) tag: Option<String>,

    /// Specific commit to use when installing from git
    #[arg(long)]
    pub(crate) rev: Option<String>,

    /// Filesystem path to local crate to install from
    #[arg(long)]
    pub(crate) path: Option<String>,

    /// Directory to install packages into
    #[arg(long, value_name = "DIR")]
    pub(crate) root: Option<PathBuf>,

    /// Force overwriting existing crates or binaries
    #[arg(long, short = 'f')]
    pub(crate) force: bool,

    /// Perform all checks without installing
    #[arg(long, short = 'n')]
    pub(crate) dry_run: bool,

    /// List all installed packages and their versions
    #[arg(long)]
    pub(crate) list: bool,

    /// Build in debug mode instead of release mode
    #[arg(long)]
    pub(crate) debug: bool,

    /// Only install the specified binary
    #[arg(long, value_name = "NAME")]
    pub(crate) bin: Option<String>,

    /// Install all binaries
    #[arg(long)]
    pub(crate) bins: bool,
}

fn install_root(explicit: Option<&PathBuf>) -> PathBuf {
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
    pub(crate) fn sc_install(&mut self, args: InstallArgs) -> anyhow::Result<()> {
        let root = install_root(args.root.as_ref());

        if args.list {
            if let Ok(entries) = std::fs::read_dir(&root) {
                for entry in entries.flatten() {
                    println!("{}", entry.file_name().to_string_lossy());
                }
            }
            return Ok(());
        }

        if args.crate_spec.is_empty() && args.path.is_none() && args.git.is_none() {
            return Err(anyhow::anyhow!(
                "no crates specified (use `no3 install <crate>` or `no3 install --path <path>`)"
            ));
        }

        std::fs::create_dir_all(&root).map_err(|e| anyhow::anyhow!("Failed to create install dir: {e}"))?;

        // Local path install: build and copy the binary.
        if let Some(path) = &args.path {
            let manifest_path = PathBuf::from(path).join(crate::package::MANIFEST_FILE);
            let mut opts = crate::commands::build::CompileOptions::default();
            opts.manifest_path = Some(manifest_path);
            opts.release = !args.debug;
            let binary_path = self.compile_package(&opts)?;
            if binary_path.as_os_str().is_empty() {
                return Ok(());
            }
            let manifest = crate::commands::build::resolve_manifest(opts.manifest_path.as_deref())?;
            let dest = root.join(&manifest.package.name);
            copy_binary(&binary_path, &dest, args.force, &self.log)?;
            info!(self.log, "Installed `{}` to {}", manifest.package.name, dest.display());
            return Ok(());
        }

        // Registry install: download, extract, build, copy.
        for spec in &args.crate_spec {
            let (name, ver) = match spec.split_once('@') {
                Some((n, v)) => (n.to_string(), Some(v.to_string())),
                None => (spec.clone(), args.version.clone()),
            };

            let registry = args
                .registry
                .as_deref()
                .or(args.index.as_deref())
                .unwrap_or(DEFAULT_REGISTRY);

            let url = format!(
                "{}/api/v1/crates/{}/{}",
                registry.trim_end_matches('/'),
                name,
                ver.as_deref().unwrap_or("latest")
            );

            info!(self.log, "Downloading {} from {}", name, registry);

            if args.dry_run {
                continue;
            }

            let reader = ureq::get(&url)
                .call()
                .map_err(|e| anyhow::anyhow!("Failed to download {}: {}", name, e))?
                .into_reader();

            let work_dir = std::env::temp_dir().join(format!("no3-install-{}", name));
            std::fs::create_dir_all(&work_dir)?;
            let mut archive = tar::Archive::new(flate2::read::GzDecoder::new(reader));
            archive.unpack(&work_dir).map_err(|e| anyhow::anyhow!("{e}"))?;

            let pkg_dir = std::fs::read_dir(&work_dir)?
                .flatten()
                .find(|e| e.path().is_dir())
                .map(|e| e.path())
                .ok_or_else(|| anyhow::anyhow!("Package archive was empty"))?;

            let mut opts = crate::commands::build::CompileOptions::default();
            opts.manifest_path = Some(pkg_dir.join(crate::package::MANIFEST_FILE));
            opts.release = !args.debug;
            let binary_path = self.compile_package(&opts)?;
            if binary_path.as_os_str().is_empty() {
                continue;
            }
            let dest = root.join(&name);
            copy_binary(&binary_path, &dest, args.force, &self.log)?;
            info!(self.log, "Installed `{}` to {}", name, dest.display());

            let _ = std::fs::remove_dir_all(&work_dir);
        }

        Ok(())
    }
}

fn copy_binary(src: &std::path::Path, dest: &std::path::Path, force: bool, log: &slog::Logger) -> anyhow::Result<()> {
    if dest.exists() && !force {
        error!(
            log,
            "binary `{}` already exists; use `--force` to overwrite",
            dest.display()
        );
        return Err(anyhow::anyhow!("Binary already exists"));
    }
    std::fs::copy(src, dest).map_err(|e| anyhow::anyhow!("Failed to install binary: {e}"))?;
    #[cfg(unix)]
    {
        use std::os::unix::fs::PermissionsExt;
        let mut perms = std::fs::metadata(dest)?.permissions();
        perms.set_mode(0o755);
        std::fs::set_permissions(dest, perms)?;
    }
    Ok(())
}
