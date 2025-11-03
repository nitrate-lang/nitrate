use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::{error, info};

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct NewArgs {
    /// Use a binary (application) template [default]
    #[arg(long, default_value_t = true)]
    bin: bool,

    /// Use a library template
    #[arg(long)]
    lib: bool,

    /// Specify the edition for the new package
    #[arg(long, default_value = "2025")]
    edition: u16,

    /// Set the resulting package name, defaults to the directory name
    #[arg(long)]
    name: Option<String>,

    path: String,
}

impl Interpreter<'_> {
    pub(crate) fn sc_new(&mut self, args: NewArgs) -> Result<(), InterpreterError> {
        if args.bin && args.lib {
            error!(self.log, "Cannot specify both --bin and --lib");
            return Err(InterpreterError::CLISemanticError);
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

        self.create_package_dir_structure(containing_dir, &package_name, args.lib, args.edition)?;

        info!(
            self.log,
            "Successfully created new package {} at: {}",
            package_name,
            containing_dir.display()
        );

        Ok(())
    }
}
