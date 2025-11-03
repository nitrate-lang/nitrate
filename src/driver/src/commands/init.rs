use crate::{
    Interpreter, InterpreterError,
    package::{Package, PackageBuilder},
};
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
    fn put_default_readme(&self, dir: &std::path::Path) -> Result<(), InterpreterError> {
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
                InterpreterError::IoError(e)
            })?;

            readme.write_all(readme_content.as_bytes())?;
        }

        Ok(())
    }

    fn put_default_gitignore(&self, dir: &std::path::Path) -> Result<(), InterpreterError> {
        let gitignore_content = "# Nitrate NO3 files\nno3_modules/\n\n";

        let mut gitignore = OpenOptions::new()
            .write(true)
            .create(true)
            .append(true)
            .open(dir.join(".gitignore"))?;

        gitignore.write_all(gitignore_content.as_bytes())?;

        Ok(())
    }

    fn put_no3_xml(
        &self,
        dir: &std::path::Path,
        package: &Package,
    ) -> Result<(), InterpreterError> {
        let no3_xml_path = dir.join("no3.xml");

        let mut no3_xml_file = std::fs::File::create(&no3_xml_path).map_err(|e| {
            error!(self.log, "Failed to create no3.xml file: {}", e);
            InterpreterError::IoError(e)
        })?;

        no3_xml_file
            .write_all(package.xml_serialize().as_bytes())
            .map_err(|e| {
                error!(self.log, "Failed to write to no3.xml file: {}", e);
                InterpreterError::IoError(e)
            })?;

        Ok(())
    }

    fn create_src_directory(
        &self,
        dir: &std::path::Path,
        is_lib: bool,
    ) -> Result<(), InterpreterError> {
        std::fs::create_dir_all(dir.join("src")).map_err(|e| {
            error!(self.log, "Failed to create src directory: {}", e);
            InterpreterError::IoError(e)
        })?;

        let entry_file_path = dir.join("src").join("entry.nit");

        let mut entry_file = std::fs::File::create(entry_file_path).map_err(|e| {
            error!(self.log, "Failed to create entry source file: {}", e);
            InterpreterError::IoError(e)
        })?;

        let entry_file_content = if is_lib {
            include_bytes!("data/default_lib.nit").as_slice()
        } else {
            include_bytes!("data/default_bin.nit").as_slice()
        };

        entry_file.write_all(entry_file_content)?;

        Ok(())
    }

    pub(crate) fn create_package_dir_structure(
        &self,
        containing_dir: &std::path::Path,
        package_name: &str,
        is_lib: bool,
        edition: u16,
    ) -> Result<(), InterpreterError> {
        std::fs::create_dir_all(containing_dir).map_err(|e| {
            error!(self.log, "Failed to create directories: {}", e);
            InterpreterError::IoError(e)
        })?;

        if !containing_dir.is_dir() {
            error!(
                self.log,
                "Specified path is not a directory: {}",
                containing_dir.display()
            );

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

        self.create_src_directory(containing_dir, is_lib)?;
        self.put_default_gitignore(containing_dir)?;
        self.put_default_readme(containing_dir)?;
        self.put_no3_xml(
            containing_dir,
            &PackageBuilder::new(package_name.to_string())
                .edition(edition)
                .build(),
        )?;

        Ok(())
    }

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
            "Successfully initialized new package {} at: {}",
            package_name,
            containing_dir.display()
        );

        Ok(())
    }
}
