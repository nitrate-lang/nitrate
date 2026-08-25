use crate::Interpreter;
use crate::commands::build::resolve_manifest;
use crate::package::{DEFAULT_REGISTRY, MANIFEST_FILE, validate_package_name};
use clap::Parser;
use slog::{error, info};
use std::path::PathBuf;
use std::str::FromStr;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct AddArgs {
    /// Reference to a package to add as a dependency
    #[arg(value_name = "DEP_ID")]
    pub(crate) dep_id: Vec<String>,

    /// Filesystem path to local crate to add
    #[arg(long)]
    pub(crate) path: Option<String>,

    /// Git URL to add
    #[arg(long)]
    pub(crate) git: Option<String>,

    /// Branch to use when adding from git
    #[arg(long)]
    pub(crate) branch: Option<String>,

    /// Tag to use when adding from git
    #[arg(long)]
    pub(crate) tag: Option<String>,

    /// Specific commit to use when adding from git
    #[arg(long)]
    pub(crate) rev: Option<String>,

    /// Registry to use
    #[arg(long)]
    pub(crate) registry: Option<String>,

    /// Disable the default features
    #[arg(long)]
    pub(crate) no_default_features: bool,

    /// Re-enable the default features
    #[arg(long)]
    pub(crate) default_features: bool,

    /// Space or comma separated list of features to activate
    #[arg(long, short = 'F', value_name = "FEATURES")]
    pub(crate) features: Vec<String>,

    /// Mark the dependency as optional
    #[arg(long)]
    pub(crate) optional: bool,

    /// Mark the dependency as required
    #[arg(long)]
    pub(crate) no_optional: bool,

    /// Rename the dependency
    #[arg(long, value_name = "NAME")]
    pub(crate) rename: Option<String>,

    /// Don't actually write the manifest
    #[arg(long, short = 'n')]
    pub(crate) dry_run: bool,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<PathBuf>,

    /// Package to modify
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Add to dev-dependencies
    #[arg(long)]
    pub(crate) dev: bool,

    /// Add to build-dependencies
    #[arg(long)]
    pub(crate) build: bool,

    /// Add to target-dependencies
    #[arg(long, value_name = "TARGET")]
    pub(crate) target: Option<String>,
}

fn query_latest_version(package_name: &str) -> Option<String> {
    let registry = std::env::var("NO3_REGISTRY").unwrap_or_else(|_| DEFAULT_REGISTRY.to_string());
    let url = format!(
        "{}/api/v1/crates/{}/versions",
        registry.trim_end_matches('/'),
        package_name
    );
    let response = ureq::get(&url).call().ok()?;
    let json: serde_json::Value = response.into_json().ok()?;
    let versions = json.get("versions").and_then(|v| v.as_array())?;
    let max_version = versions
        .iter()
        .filter_map(|v| v.get("num").and_then(|n| n.as_str()))
        .filter_map(|s| semver::Version::parse(s).ok())
        .max()?;
    Some(format!("^{}", max_version))
}

fn dep_value_from_full(full: &crate::package::DependencyFull) -> toml_edit::Value {
    let mut table = toml_edit::InlineTable::new();
    if let Some(v) = &full.version {
        table.insert("version", toml_edit::Value::from(v.as_str()));
    }
    if let Some(v) = &full.path {
        table.insert("path", toml_edit::Value::from(v.as_str()));
    }
    if let Some(v) = &full.git {
        table.insert("git", toml_edit::Value::from(v.as_str()));
    }
    if let Some(v) = &full.branch {
        table.insert("branch", toml_edit::Value::from(v.as_str()));
    }
    if let Some(v) = &full.tag {
        table.insert("tag", toml_edit::Value::from(v.as_str()));
    }
    if let Some(v) = &full.rev {
        table.insert("rev", toml_edit::Value::from(v.as_str()));
    }
    if let Some(v) = &full.registry {
        table.insert("registry", toml_edit::Value::from(v.as_str()));
    }
    if let Some(v) = &full.optional {
        table.insert("optional", toml_edit::Value::from(*v));
    }
    if let Some(v) = &full.default_features {
        table.insert("default-features", toml_edit::Value::from(*v));
    }
    if let Some(features) = &full.features {
        let mut arr = toml_edit::Array::new();
        for f in features {
            arr.push(toml_edit::Value::from(f.as_str()));
        }
        table.insert("features", toml_edit::Value::from(arr));
    }
    toml_edit::Value::InlineTable(table)
}

impl Interpreter<'_> {
    pub(crate) fn sc_add(&mut self, args: AddArgs) -> anyhow::Result<()> {
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

        let section_name = if args.dev {
            "dev-dependencies"
        } else if args.build {
            "build-dependencies"
        } else {
            "dependencies"
        };

        if doc.get(section_name).is_none() {
            doc[section_name] = toml_edit::Item::Table(toml_edit::Table::new());
        }

        let mut added = Vec::new();

        // --path source
        if let Some(path) = &args.path {
            if args.dep_id.is_empty() {
                // `no3 add --path ../foo` infers the name from the directory.
                let inferred = path
                    .trim_end_matches('/')
                    .rsplit('/')
                    .next()
                    .unwrap_or("dep")
                    .to_string();
                let mut full = crate::package::DependencyFull::default();
                full.path = Some(path.clone());
                doc[section_name][&inferred] = toml_edit::Item::Value(dep_value_from_full(&full));
                added.push(inferred);
            } else {
                for dep in &args.dep_id {
                    validate_package_name(dep).map_err(|e| anyhow::anyhow!("{e}"))?;
                    let mut full = crate::package::DependencyFull::default();
                    full.path = Some(path.clone());
                    full.registry = args.registry.clone();
                    let key = args.rename.clone().unwrap_or_else(|| dep.clone());
                    doc[section_name][&key] = toml_edit::Item::Value(dep_value_from_full(&full));
                    added.push(key);
                }
            }
        // --git source
        } else if let Some(git) = &args.git {
            for dep in &args.dep_id {
                validate_package_name(dep).map_err(|e| anyhow::anyhow!("{e}"))?;
                let mut full = crate::package::DependencyFull::default();
                full.git = Some(git.clone());
                full.branch = args.branch.clone();
                full.tag = args.tag.clone();
                full.rev = args.rev.clone();
                let key = args.rename.clone().unwrap_or_else(|| dep.clone());
                doc[section_name][&key] = toml_edit::Item::Value(dep_value_from_full(&full));
                added.push(key);
            }
        // Registry deps: `<name>[@<version-req>]`
        } else {
            for dep in &args.dep_id {
                let (name, version) = match dep.split_once('@') {
                    Some((n, v)) => (n.to_string(), Some(v.to_string())),
                    None => (dep.clone(), None),
                };
                validate_package_name(&name).map_err(|e| anyhow::anyhow!("{e}"))?;

                let version = match version {
                    Some(v) => v,
                    None => {
                        // Fall back to ^0.1.0 if the registry can't be reached.
                        query_latest_version(&name).unwrap_or_else(|| "^0.1.0".to_string())
                    }
                };

                let mut full = crate::package::DependencyFull::default();
                full.version = Some(version);
                full.registry = args.registry.clone();

                if args.no_default_features {
                    full.default_features = Some(false);
                }
                if !args.features.is_empty() {
                    full.features = Some(args.features.clone());
                }
                if args.optional && !args.no_optional {
                    full.optional = Some(true);
                } else if args.no_optional && !args.optional {
                    full.optional = Some(false);
                }

                let key = args.rename.clone().unwrap_or_else(|| name.clone());
                doc[section_name][&key] = toml_edit::Item::Value(dep_value_from_full(&full));
                added.push(key);
            }
        }

        if args.dry_run {
            info!(self.log, "Dry run, not writing manifest.");
            return Ok(());
        }

        std::fs::write(&manifest_path, doc.to_string()).map_err(|e| anyhow::anyhow!("{e}"))?;

        for dep in &added {
            info!(self.log, "Adding {} to [{}]", dep, section_name);
        }

        Ok(())
    }
}
