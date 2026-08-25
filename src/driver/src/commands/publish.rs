use crate::Interpreter;
use crate::commands::build::{resolve_manifest, target_dir_for};
use crate::package::DEFAULT_REGISTRY;
use clap::Parser;
use slog::{error, info};
use std::path::PathBuf;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct PublishArgs {
    /// Perform all checks without uploading
    #[arg(long, short = 'n')]
    pub(crate) dry_run: bool,

    /// Registry index URL to upload the package to
    #[arg(long)]
    pub(crate) index: Option<String>,

    /// Registry to upload the package to
    #[arg(long)]
    pub(crate) registry: Option<String>,

    /// Don't verify the contents by building them
    #[arg(long)]
    pub(crate) no_verify: bool,

    /// Allow dirty working directories to be packaged
    #[arg(long)]
    pub(crate) allow_dirty: bool,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<PathBuf>,

    /// Package(s) to publish
    #[arg(long, short = 'p', value_name = "SPEC")]
    pub(crate) package: Option<String>,

    /// Publish all packages in the workspace
    #[arg(long)]
    pub(crate) workspace: bool,
}

fn create_crate_archive(manifest: &crate::package::Manifest, out_path: &std::path::Path) -> anyhow::Result<()> {
    let src_dir = manifest.manifest_dir.join("src");
    if !src_dir.is_dir() {
        return Err(anyhow::anyhow!("src directory `{}` not found", src_dir.display()));
    }

    let file = std::fs::File::create(out_path)?;
    let mut gz = flate2::write::GzEncoder::new(file, flate2::Compression::default());
    {
        let mut archive = tar::Builder::new(&mut gz);
        let prefix = format!("{}-{}", manifest.package.name, manifest.package.version);

        // Include source files.
        for entry in std::fs::read_dir(&src_dir)? {
            let entry = entry?;
            let path = entry.path();
            if path.is_file() {
                let rel = path
                    .strip_prefix(&manifest.manifest_dir)
                    .map_err(|e| anyhow::anyhow!("{e}"))?;
                let dest = std::path::Path::new(&prefix).join(rel);
                archive.append_path_with_name(&path, &dest)?;
            }
        }

        // Include manifest and readme.
        let manifest_bytes = manifest.to_toml_string();
        let mut header = tar::Header::new_gnu();
        header.set_size(manifest_bytes.len() as u64);
        header.set_mode(0o644);
        header.set_cksum();
        archive.append_data(&mut header, format!("{}/no3.toml", prefix), manifest_bytes.as_bytes())?;

        if manifest.manifest_dir.join("README.md").is_file() {
            archive.append_path_with_name(
                &manifest.manifest_dir.join("README.md"),
                format!("{}/README.md", prefix),
            )?;
        }

        archive.finish()?;
    }
    gz.finish()?;
    Ok(())
}

impl Interpreter<'_> {
    pub(crate) fn sc_publish(&mut self, args: PublishArgs) -> anyhow::Result<()> {
        let manifest = resolve_manifest(args.manifest_path.as_deref()).map_err(|e| anyhow::anyhow!("{e}"))?;

        // Verify the package builds unless `--no-verify`.
        if !args.no_verify {
            info!(self.log, "Verifying package contents (this may take a moment)");
            let mut opts = crate::commands::build::CompileOptions::default();
            opts.manifest_path = args.manifest_path.clone();
            opts.check_only = true;
            self.compile_package(&opts)?;
        }

        let target_dir = target_dir_for(&manifest, None);
        let archive_filename = format!("{}-{}.crate", manifest.package.name, manifest.package.version);
        let archive_path = target_dir.join("package").join(&archive_filename);
        std::fs::create_dir_all(archive_path.parent().unwrap())?;

        create_crate_archive(&manifest, &archive_path)?;
        info!(
            self.log,
            "Packaged {} v{} as {}",
            manifest.package.name,
            manifest.package.version,
            archive_path.display()
        );

        if args.dry_run {
            info!(self.log, "Dry run: not uploading package");
            return Ok(());
        }

        let registry = args
            .registry
            .as_deref()
            .or(args.index.as_deref())
            .unwrap_or(DEFAULT_REGISTRY);

        let url = format!("{}/api/v1/crates/new", registry.trim_end_matches('/'));
        let bytes = std::fs::read(&archive_path)?;
        let response = ureq::post(&url)
            .set("Content-Type", "application/octet-stream")
            .send_bytes(&bytes)
            .map_err(|e| anyhow::anyhow!("Failed to upload package: {e}"))?;

        if response.status() >= 200 && response.status() < 300 {
            info!(
                self.log,
                "Published {} v{} to {}", manifest.package.name, manifest.package.version, registry
            );
        } else {
            error!(
                self.log,
                "Failed to publish package: server responded with status {}",
                response.status()
            );
            return Err(anyhow::anyhow!("Publish failed with status {}", response.status()));
        }

        Ok(())
    }
}
