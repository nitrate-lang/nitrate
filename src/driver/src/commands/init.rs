use crate::{Interpreter, InterpreterError, package::PackageBuilder};
use clap::Parser;
use slog::{debug, error, info, warn};
use std::{fs::OpenOptions, io::Write};

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct InitArgs {
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

    #[arg(default_value = ".")]
    path: String,
}

impl Interpreter<'_> {
    fn contains_conflicting_package_files(&self, dir: &std::path::Path) -> bool {
        let conflicting_files = ["no3.xml", "src", "no3_modules"];

        for file in &conflicting_files {
            let joined = dir.join(file);
            if joined.exists() {
                warn!(
                    self.log,
                    "Conflicting package file found: {}",
                    joined.display()
                );

                return true;
            }
        }

        false
    }

    pub(crate) fn sc_init(&mut self, args: InitArgs) -> Result<(), InterpreterError> {
        if args.bin && args.lib {
            error!(self.log, "Cannot specify both --bin and --lib");
            return Err(InterpreterError::CLISemanticError);
        }

        let containing_dir = std::path::Path::new(&args.path);
        if !containing_dir.exists() {
            debug!(self.log, "Recursively creating directory: {}", args.path);
            std::fs::create_dir_all(containing_dir).map_err(|e| {
                error!(self.log, "Failed to create directory: {}", e);
                InterpreterError::IoError(e)
            })?;
        }

        if !containing_dir.is_dir() {
            error!(self.log, "Specified path is not a directory: {}", args.path);
            return Err(InterpreterError::OperationalError);
        }

        if self.contains_conflicting_package_files(containing_dir) {
            error!(
                self.log,
                "Unable to initialize package: conflicting package files found in {}",
                containing_dir.display()
            );
            return Err(InterpreterError::OperationalError);
        }

        let package_name = match args.name {
            Some(name) => name,

            None => containing_dir
                .file_name()
                .and_then(|os_str| os_str.to_str())
                .unwrap_or("my_package")
                .to_string(),
        };

        {
            /***************************************************************************/
            // Create "src" directory
            std::fs::create_dir_all(containing_dir.join("src")).map_err(|e| {
                error!(self.log, "Failed to create src directory: {}", e);
                InterpreterError::IoError(e)
            })?;

            let main_file_path = if args.lib {
                containing_dir.join("src").join("lib.nit")
            } else {
                containing_dir.join("src").join("main.nit")
            };

            let mut main_file = std::fs::File::create(main_file_path).map_err(|e| {
                error!(self.log, "Failed to create main source file: {}", e);
                InterpreterError::IoError(e)
            })?;

            let main_file_content = if args.lib {
                include_bytes!("data/default_lib.nit").as_slice()
            } else {
                include_bytes!("data/default_main.nit").as_slice()
            };

            main_file.write_all(main_file_content)?;
        }

        {
            /***************************************************************************/
            // Create "no3_modules" directory
            std::fs::create_dir_all(containing_dir.join("no3_modules")).map_err(|e| {
                error!(self.log, "Failed to create no3_modules directory: {}", e);
                InterpreterError::IoError(e)
            })?;
        }

        {
            /***************************************************************************/
            // Create "no3.xml" file
            let package = PackageBuilder::new(package_name.clone())
                .edition(args.edition)
                .build();

            let mut no3_xml_file =
                std::fs::File::create(containing_dir.join("no3.xml")).map_err(|e| {
                    error!(self.log, "Failed to create no3.xml file: {}", e);
                    InterpreterError::IoError(e)
                })?;

            no3_xml_file.write_all(package.xml_serialize().as_bytes())?;
        }

        {
            /***************************************************************************/
            // Append to ".gitignore" file

            let gitignore_content = "# Nitrate NO3 files\nno3_modules/\n\n";

            let mut gitignore = OpenOptions::new()
                .write(true)
                .create(true)
                .append(true)
                .open(containing_dir.join(".gitignore"))?;

            gitignore.write_all(gitignore_content.as_bytes())?;
        }

        {
            /***************************************************************************/
            // Create README.md file

            let readme_content = "# Nitrate Package\n\n";
            let readme_path = containing_dir.join("README.md");

            if readme_path.exists() {
                warn!(
                    self.log,
                    "README.md already exists at {}, skipping creation.",
                    readme_path.display()
                );
            } else {
                let mut readme = std::fs::File::create(&readme_path).map_err(|e| {
                    error!(self.log, "Failed to create README.md file: {}", e);
                    InterpreterError::IoError(e)
                })?;

                readme.write_all(readme_content.as_bytes())?;
            }
        }

        info!(
            self.log,
            "Successfully initialized new package {} at: {}",
            package_name,
            containing_dir.display()
        );

        Ok(())
    }
}
