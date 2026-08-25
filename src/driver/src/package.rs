use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;
use std::matches;
use std::path::{Path, PathBuf};

pub const MANIFEST_FILE: &str = "no3.toml";
pub const LOCK_FILE: &str = "no3.lock";
pub const DEFAULT_REGISTRY: &str = "https://registry.nitrate.dev";

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
#[serde(untagged)]
pub enum DependencyReq {
    Simple(String),
    Full(DependencyFull),
}

#[derive(Debug, Clone, Default, Serialize, Deserialize, PartialEq)]
pub struct DependencyFull {
    #[serde(skip_serializing_if = "Option::is_none")]
    pub version: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub path: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub git: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub branch: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub tag: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub rev: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub registry: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub features: Option<Vec<String>>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub optional: Option<bool>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub default_features: Option<bool>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub package: Option<String>,
}

impl DependencyReq {
    pub fn version_req(&self) -> Option<semver::VersionReq> {
        let s = match self {
            DependencyReq::Simple(v) => v.as_str(),
            DependencyReq::Full(f) => f.version.as_deref()?,
        };
        semver::VersionReq::parse(s).ok()
    }

    pub fn name(&self) -> Option<&str> {
        match self {
            DependencyReq::Full(f) => f.package.as_deref(),
            DependencyReq::Simple(_) => None,
        }
    }

    pub fn is_optional(&self) -> bool {
        matches!(self, DependencyReq::Full(f) if f.optional == Some(true))
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub struct LockedPackage {
    pub name: String,
    pub version: String,
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    pub dependencies: Vec<String>,
    #[serde(default = "default_registry_source")]
    pub source: String,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub checksum: Option<String>,
}

fn default_registry_source() -> String {
    DEFAULT_REGISTRY.to_string()
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub struct Lockfile {
    #[serde(default = "default_lock_version")]
    pub version: u32,
    #[serde(default, rename = "package")]
    pub packages: Vec<LockedPackage>,
}

impl Default for Lockfile {
    fn default() -> Self {
        Self {
            version: 4,
            packages: Vec::new(),
        }
    }
}

fn default_lock_version() -> u32 {
    4
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub struct WorkspaceSection {
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    pub members: Vec<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub exclude: Option<Vec<String>>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub struct Manifest {
    pub package: Package,
    #[serde(default, skip_serializing_if = "BTreeMap::is_empty")]
    pub dependencies: BTreeMap<String, DependencyReq>,
    #[serde(default, rename = "dev-dependencies", skip_serializing_if = "BTreeMap::is_empty")]
    pub dev_dependencies: BTreeMap<String, DependencyReq>,
    #[serde(default, rename = "build-dependencies", skip_serializing_if = "BTreeMap::is_empty")]
    pub build_dependencies: BTreeMap<String, DependencyReq>,
    #[serde(default, skip_serializing_if = "BTreeMap::is_empty")]
    pub features: BTreeMap<String, Vec<String>>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub workspace: Option<WorkspaceSection>,
    #[serde(skip)]
    pub manifest_dir: PathBuf,
}

/// The `[package]` section.
#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub struct Package {
    pub name: String,
    pub version: String,
    pub edition: String,
    #[serde(default, rename = "rust-version", skip_serializing_if = "Option::is_none")]
    pub rust_version: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub description: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub authors: Option<Vec<String>>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub license: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub readme: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub repository: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub homepage: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub documentation: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub keywords: Option<Vec<String>>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub categories: Option<Vec<String>>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub publish: Option<bool>,
}

impl Package {
    pub fn semver(&self) -> Result<semver::Version, semver::Error> {
        semver::Version::parse(&self.version)
    }

    pub fn major_minor_patch(&self) -> (u64, u64, u64) {
        let v = self.semver().unwrap_or_else(|_| semver::Version::new(0, 1, 0));
        (v.major, v.minor, v.patch)
    }

    pub fn edition_major(&self) -> u16 {
        self.edition
            .split('.')
            .next()
            .and_then(|s| s.parse().ok())
            .unwrap_or(2026)
    }
}

impl Lockfile {
    pub fn from_str(s: &str) -> Result<Self, toml::de::Error> {
        toml::from_str(s)
    }

    pub fn to_toml_string(&self) -> String {
        let mut out = format!("version = {}\n\n", self.version);
        for pkg in &self.packages {
            out.push_str("[[package]]\n");
            out.push_str(&format!("name = \"{}\"\n", pkg.name));
            out.push_str(&format!("version = \"{}\"\n", pkg.version));
            out.push_str(&format!("source = \"{}\"\n", pkg.source));
            if !pkg.dependencies.is_empty() {
                out.push_str("dependencies = [\n");
                for dep in &pkg.dependencies {
                    out.push_str(&format!("    \"{}\",\n", dep));
                }
                out.push_str("]\n");
            }
            if let Some(checksum) = &pkg.checksum {
                out.push_str(&format!("checksum = \"{}\"\n", checksum));
            }
            out.push('\n');
        }
        out
    }
}

#[derive(Debug, thiserror::Error)]
pub enum ManifestError {
    #[error("failed to parse manifest: {0}")]
    Parse(toml::de::Error),
    #[error("failed to read manifest '{0}': {1}")]
    Io(PathBuf, std::io::Error),
    #[error("could not find `no3.toml` in `{0}` or any parent directory")]
    NotFound(PathBuf),
}

/// Find `no3.toml` starting at `dir` and walking up parents.
pub fn find_manifest(start_dir: &Path) -> Result<PathBuf, ManifestError> {
    let mut dir = Some(start_dir);
    while let Some(d) = dir {
        let candidate = d.join(MANIFEST_FILE);
        if candidate.is_file() {
            return Ok(candidate);
        }
        dir = d.parent();
    }
    Err(ManifestError::NotFound(start_dir.to_path_buf()))
}

impl Manifest {
    /// Load the manifest, searching upward from `start_dir` for `no3.toml`.
    pub fn discover(start_dir: &Path) -> Result<Self, ManifestError> {
        let manifest_path = find_manifest(start_dir)?;
        Self::load(&manifest_path)
    }

    /// Load a manifest from an explicit `--manifest-path`.
    pub fn load(path: &Path) -> Result<Self, ManifestError> {
        let text = std::fs::read_to_string(path).map_err(|e| ManifestError::Io(path.to_path_buf(), e))?;
        let mut manifest: Manifest = toml::from_str(&text).map_err(ManifestError::Parse)?;
        manifest.manifest_dir = path
            .parent()
            .map(|p| p.to_path_buf())
            .unwrap_or_else(|| PathBuf::from("."));
        Ok(manifest)
    }

    pub fn to_toml_string(&self) -> String {
        let mut manifest = self.clone();
        manifest.manifest_dir = PathBuf::new();
        toml::to_string(&manifest).expect("failed to serialize manifest")
    }

    /// The default source entrypoint for this package.
    pub fn entrypoint(&self) -> PathBuf {
        self.manifest_dir.join("src").join("entry.nit")
    }

    pub fn all_dependencies(&self) -> impl Iterator<Item = (&String, &DependencyReq)> {
        self.dependencies
            .iter()
            .chain(self.dev_dependencies.iter())
            .chain(self.build_dependencies.iter())
    }

    pub fn existing_dependency(&self, name: &str) -> Option<(&String, &DependencyReq)> {
        self.all_dependencies().find(|(n, _)| *n == name)
    }

    /// The lockfile path next to the manifest.
    pub fn lockfile_path(&self) -> PathBuf {
        self.manifest_dir.join(LOCK_FILE)
    }

    pub fn load_lockfile(&self) -> Result<Lockfile, ManifestError> {
        let path = self.lockfile_path();
        let text = std::fs::read_to_string(&path).map_err(|e| ManifestError::Io(path, e))?;
        Lockfile::from_str(&text).map_err(ManifestError::Parse)
    }

    pub fn save_lockfile(&self, lockfile: &Lockfile) -> Result<(), ManifestError> {
        let path = self.lockfile_path();
        std::fs::write(&path, lockfile.to_toml_string()).map_err(|e| ManifestError::Io(path, e))
    }
}

/// Builder for `no3 new` / `no3 init`.
pub struct ManifestBuilder {
    manifest: Manifest,
    is_lib: bool,
}

impl ManifestBuilder {
    pub fn new(name: String, edition: &str) -> Self {
        Self {
            manifest: Manifest {
                package: Package {
                    name,
                    version: "0.1.0".to_string(),
                    edition: edition.to_string(),
                    rust_version: None,
                    description: None,
                    authors: Some(vec![default_author()]),
                    license: None,
                    readme: None,
                    repository: None,
                    homepage: None,
                    documentation: None,
                    keywords: None,
                    categories: None,
                    publish: None,
                },
                dependencies: BTreeMap::new(),
                dev_dependencies: BTreeMap::new(),
                build_dependencies: BTreeMap::new(),
                features: BTreeMap::new(),
                workspace: None,
                manifest_dir: PathBuf::from("."),
            },
            is_lib: false,
        }
    }

    pub fn lib(mut self, is_lib: bool) -> Self {
        self.is_lib = is_lib;
        self
    }

    pub fn build(self) -> Manifest {
        self.manifest
    }

    pub fn is_lib(&self) -> bool {
        self.is_lib
    }
}

fn default_author() -> String {
    match std::env::var("USER") {
        Ok(user) => {
            let name_env = std::env::var("NITRATE_AUTHOR_NAME")
                .or_else(|_| std::env::var("CARGO_AUTHOR_NAME"))
                .ok();
            let email_env = std::env::var("NITRATE_AUTHOR_EMAIL")
                .or_else(|_| std::env::var("CARGO_AUTHOR_EMAIL"))
                .ok();
            match (name_env, email_env) {
                (Some(name), Some(email)) => format!("{} <{}>", name, email),
                (Some(name), None) => name,
                (None, Some(email)) => format!("{} <{}>", user, email),
                (None, None) => user,
            }
        }
        Err(_) => "Unknown".to_string(),
    }
}

/// Sanitize a package name to only contain alphanumeric, `-`, and `_`.
pub fn sanitize_package_name(name: &str) -> String {
    let mut out = String::with_capacity(name.len());
    for c in name.chars() {
        if c.is_alphanumeric() || c == '-' || c == '_' {
            out.push(c);
        } else {
            out.push('_');
        }
    }
    out
}

/// Validate a package name against cargo-like rules.
pub fn validate_package_name(name: &str) -> Result<(), String> {
    if name.is_empty() {
        return Err("package name cannot be empty".to_string());
    }
    if !name.chars().all(|c| c.is_alphanumeric() || c == '-' || c == '_') {
        return Err(format!(
            "invalid package name `{}`: characters must be alphanumeric, `-`, or `_`",
            name
        ));
    }
    if name.starts_with('-') || name.ends_with('-') {
        return Err(format!("invalid package name `{}`: cannot start or end with `-`", name));
    }
    if name == "no3" {
        return Err("package name `no3` is reserved".to_string());
    }
    Ok(())
}
