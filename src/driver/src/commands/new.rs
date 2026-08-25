use crate::{Interpreter, package::validate_package_name};
use clap::Parser;
use slog::{error, info};

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct NewArgs {
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

    pub(crate) path: String,
}

impl Interpreter<'_> {
    pub(crate) fn sc_new(&mut self, args: NewArgs) -> anyhow::Result<()> {
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
            "Successfully created new package {} at: {}",
            package_name,
            containing_dir.display()
        );

        Ok(())
    }
}
