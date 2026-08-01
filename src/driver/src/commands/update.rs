use crate::Interpreter;
use crate::commands::build::resolve_manifest;
use crate::package::{DEFAULT_REGISTRY, LockedPackage};
use clap::Parser;
use slog::info;
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct UpdateArgs {
    /// Don't actually write the lockfile
    #[arg(long, short = 'n')]
    pub(crate) dry_run: bool,

    /// Force updating all dependencies of [SPEC]... as well
    #[arg(long)]
    pub(crate) recursive: bool,

    /// Update [SPEC] to exactly PRECISE
    #[arg(long, value_name = "PRECISE")]
    pub(crate) precise: Option<String>,

    /// Package to update
    #[arg(value_name = "SPEC")]
    pub(crate) spec: Vec<String>,

    /// Only update the workspace packages
    #[arg(long, short = 'w')]
    pub(crate) workspace: bool,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<PathBuf>,
}

/// Resolve a dependency to its latest matching version for the lockfile.
fn resolve_dependency(name: &str, req: &crate::package::DependencyReq) -> Result<LockedPackage, String> {
    let version_req = req
        .version_req()
        .ok_or_else(|| format!("dependency `{}` has an invalid version requirement", name))?;

    // Local path dependency: no registry query needed.
    if let crate::package::DependencyReq::Full(f) = req {
        if let Some(path) = &f.path {
            return Ok(LockedPackage {
                name: name.to_string(),
                version: "0.1.0".to_string(),
                dependencies: vec![],
                source: format!("path+{}", path),
                checksum: None,
            });
        }
    }

    let registry = std::env::var("NO3_REGISTRY").unwrap_or_else(|_| DEFAULT_REGISTRY.to_string());
    let url = format!("{}/api/v1/crates/{}/versions", registry.trim_end_matches('/'), name);
    let response = ureq::get(&url).call().map_err(|e| e.to_string())?;
    let json: serde_json::Value = response.into_json().map_err(|e| e.to_string())?;
    let versions = json
        .get("versions")
        .and_then(|v| v.as_array())
        .ok_or_else(|| format!("no versions found for `{}`", name))?;

    let best = versions
        .iter()
        .filter_map(|v| {
            let num = v.get("num").and_then(|n| n.as_str())?;
            let version = semver::Version::parse(num).ok()?;
            if version_req.matches(&version) {
                Some(version)
            } else {
                None
            }
        })
        .max()
        .ok_or_else(|| format!("no version of `{}` matches requirement", name))?;

    let checksum = json.get("checksum").and_then(|c| c.as_str()).map(|s| s.to_string());

    Ok(LockedPackage {
        name: name.to_string(),
        version: best.to_string(),
        dependencies: vec![],
        source: registry,
        checksum,
    })
}

impl Interpreter<'_> {
    pub(crate) fn sc_update(&mut self, args: UpdateArgs) -> anyhow::Result<()> {
        let manifest = resolve_manifest(args.manifest_path.as_deref()).map_err(|e| anyhow::anyhow!("{e}"))?;

        let mut lockfile = manifest.load_lockfile().unwrap_or_default();
        // Keep only entries still present in the manifest.
        lockfile
            .packages
            .retain(|pkg| manifest.existing_dependency(&pkg.name).is_some());

        // Apply `--precise` overrides.
        if let (Some(spec), Some(precise)) = (args.spec.first(), &args.precise) {
            if let Some(pkg) = lockfile.packages.iter_mut().find(|p| &p.name == spec) {
                pkg.version = precise.clone();
            }
        }

        // Resolve any dependencies missing from the lockfile.
        for (name, req) in manifest.all_dependencies() {
            if lockfile.packages.iter().any(|p| &p.name == name) {
                continue;
            }
            let locked = resolve_dependency(name, req).map_err(|e| anyhow::anyhow!("{e}"))?;
            lockfile.packages.push(locked);
        }

        lockfile.packages.sort_by(|a, b| a.name.cmp(&b.name));

        if args.dry_run {
            info!(self.log, "Dry run: lockfile would be updated.");
            return Ok(());
        }

        manifest.save_lockfile(&lockfile).map_err(|e| anyhow::anyhow!("{e}"))?;
        info!(self.log, "Updated no3.lock ({} package(s))", lockfile.packages.len());

        Ok(())
    }
}
