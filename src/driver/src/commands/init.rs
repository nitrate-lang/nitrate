use crate::{
    Interpreter,
    package::{Manifest, ManifestBuilder, validate_package_name},
};
use clap::Parser;
use slog::{error, info, warn};
use std::fs::OpenOptions;
use std::io::Write;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct InitArgs {
    /// Initialize a new repository for the given version control system
    #[arg(long, value_parser = ["git", "hg", "pijul", "fossil", "none"])]
    pub(crate) vcs: Option<String>,

    /// Use a binary (application) template [default]
    #[arg(long, default_value_t = true)]
    pub(crate) bin: bool,

    /// Use a library template
    #[arg(long)]
    pub(crate) lib: bool,

    /// Edition to set for the package generated
    #[arg(long, default_value = "2026")]
    pub(crate) edition: String,

    /// Set the resulting package name, defaults to the directory name
    #[arg(long)]
    pub(crate) name: Option<String>,

    /// Registry to use
    #[arg(long)]
    pub(crate) registry: Option<String>,

    #[arg(default_value = ".")]
    pub(crate) path: String,
}

impl Interpreter<'_> {
    fn put_default_readme(&self, dir: &std::path::Path) -> anyhow::Result<()> {
        let readme_content = "# Nitrate Package\n\n";
        let readme_path = dir.join("README.md");

        if readme_path.exists() {
            warn!(
                self.log,
                "README.md already exists at {}, skipping creation.",
                readme_path.display()
            );
        } else {
            let mut readme = std::fs::File::create(&readme_path).map_err(|e| {
                error!(self.log, "Failed to create README.md file: {}", e);
                e
            })?;

            readme.write_all(readme_content.as_bytes())?;
        }

        Ok(())
    }

    fn put_default_gitignore(&self, dir: &std::path::Path) -> anyhow::Result<()> {
        let gitignore_content = "/target\n";

        let mut gitignore = OpenOptions::new()
            .create(true)
            .append(true)
            .open(dir.join(".gitignore"))?;

        gitignore.write_all(gitignore_content.as_bytes())?;

        Ok(())
    }

    fn put_no3_toml(&self, dir: &std::path::Path, manifest: &Manifest) -> anyhow::Result<()> {
        let no3_toml_path = dir.join("no3.toml");

        let mut no3_toml_file = std::fs::File::create(&no3_toml_path).map_err(|e| {
            error!(self.log, "Failed to create no3.toml file: {}", e);
            e
        })?;

        no3_toml_file
            .write_all(manifest.to_toml_string().as_bytes())
            .map_err(|e| {
                error!(self.log, "Failed to write to no3.toml file: {}", e);
                e
            })?;

        Ok(())
    }

    fn create_src_directory(&self, dir: &std::path::Path, is_lib: bool) -> anyhow::Result<()> {
        std::fs::create_dir_all(dir.join("src")).map_err(|e| {
            error!(self.log, "Failed to create src directory: {}", e);
            e
        })?;

        let entry_file_path = dir.join("src").join("entry.nit");

        let mut entry_file = std::fs::File::create(entry_file_path).map_err(|e| {
            error!(self.log, "Failed to create entry source file: {}", e);
            e
        })?;

        let entry_file_content = if is_lib {
            include_bytes!("data/default_lib.nit").as_slice()
        } else {
            include_bytes!("data/default_bin.nit").as_slice()
        };

        entry_file.write_all(entry_file_content)?;

        Ok(())
    }

    fn initialize_git_repo(&self, dir: &std::path::Path) -> Result<(), ()> {
        match std::process::Command::new("git")
            .arg("init")
            .arg("-q")
            .arg(dir)
            .status()
        {
            Ok(status) if status.success() => Ok(()),
            Ok(status) => {
                error!(self.log, "Git initialization failed with status: {}", status);
                Err(())
            }
            Err(e) => {
                error!(self.log, "Failed to execute git command: {}", e);
                Err(())
            }
        }
    }

    pub(crate) fn create_package_dir_structure(
        &self,
        containing_dir: &std::path::Path,
        package_name: &str,
        is_lib: bool,
        edition: &str,
    ) -> anyhow::Result<()> {
        std::fs::create_dir_all(containing_dir).map_err(|e| {
            error!(self.log, "Failed to create directories: {}", e);
            e
        })?;

        if !containing_dir.is_dir() {
            error!(
                self.log,
                "Specified path is not a directory: {}",
                containing_dir.display()
            );

            return Err(anyhow::anyhow!("Specified path is not a directory"));
        }

        if self.contains_conflicting_package_files(containing_dir) {
            error!(
                self.log,
                "Unable to initialize package: conflicting package files found in {}",
                containing_dir.display()
            );

            return Err(anyhow::anyhow!("Conflicting package files found"));
        }

        self.create_src_directory(containing_dir, is_lib)?;
        self.put_default_gitignore(containing_dir)?;
        self.put_default_readme(containing_dir)?;
        self.put_no3_toml(
            containing_dir,
            &ManifestBuilder::new(package_name.to_string(), edition)
                .lib(is_lib)
                .build(),
        )?;

        if self.initialize_git_repo(containing_dir).is_err() {
            warn!(
                self.log,
                "Git repository initialization failed, but continuing package creation."
            );
        };

        Ok(())
    }

    fn contains_conflicting_package_files(&self, dir: &std::path::Path) -> bool {
        let conflicting_files = ["no3.toml", "src", "target", "README.md"];

        for file in &conflicting_files {
            let joined = dir.join(file);
            if joined.exists() {
                warn!(self.log, "Conflicting package file found: {}", joined.display());

                return true;
            }
        }

        false
    }

    pub(crate) fn sc_init(&mut self, args: InitArgs) -> anyhow::Result<()> {
        if args.bin && args.lib {
            error!(self.log, "Cannot specify both --bin and --lib");
            return Err(anyhow::anyhow!("Cannot specify both --bin and --lib"));
        }

        let containing_dir = std::path::Path::new(&args.path);
        let package_name = match args.name {
            Some(name) => name,
            None => containing_dir
                .file_name()
                .and_then(|os_str| os_str.to_str())
                .unwrap_or("my_package")
                .to_string(),
        };

        validate_package_name(&package_name).map_err(|e| anyhow::anyhow!("{e}"))?;

        self.create_package_dir_structure(containing_dir, &package_name, args.lib, &args.edition)?;

        info!(
            self.log,
            "Successfully initialized new package {} at: {}",
            package_name,
            containing_dir.display()
        );

        Ok(())
    }
}
