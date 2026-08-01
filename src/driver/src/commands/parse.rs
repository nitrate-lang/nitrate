use std::io::Read;

use crate::Interpreter;
use crate::commands::build::resolve_manifest;
use clap::Parser;
use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    parse,
    token_lexer::{Lexer, LexerError},
};
use slog::error;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct ParseArgs {
    /// Format mode for printed output
    #[arg(long, value_parser = ["minify", "pretty", "source"])]
    format_mode: Option<String>,

    /// Path to Cargo.toml
    #[arg(long, value_name = "PATH")]
    pub(crate) manifest_path: Option<std::path::PathBuf>,
}

impl Interpreter<'_> {
    pub(crate) fn sc_parse(&mut self, args: ParseArgs) -> anyhow::Result<()> {
        let log = CompilerLog::new(self.log.clone());

        let manifest = resolve_manifest(args.manifest_path.as_deref())?;
        self.validate_package_edition(manifest.package.edition_major())?;
        let entrypoint_path = manifest.entrypoint();

        if !entrypoint_path.exists() {
            error!(
                self.log,
                "Package entrypoint '{}' does not exist.",
                entrypoint_path.display()
            );
            return Err(anyhow::anyhow!("Package entrypoint does not exist"));
        }

        let mut source_code_file = match std::fs::File::open(&entrypoint_path) {
            Ok(file) => file,
            Err(e) => {
                error!(
                    self.log,
                    "Failed to open package entrypoint '{}': {}",
                    entrypoint_path.display(),
                    e
                );

                return Err(anyhow::anyhow!("Failed to open package entrypoint"));
            }
        };

        let mut source_code = Vec::new();
        source_code_file.read_to_end(&mut source_code)?;

        let source_code_file = intern_file_id(entrypoint_path.to_string_lossy().as_ref()).expect("FileId overflow");

        let lexer = match Lexer::new(&source_code, Some(source_code_file)) {
            Ok(lexer) => lexer,

            Err(LexerError::SourceTooBig) => {
                error!(
                    self.log,
                    "Source file '{}' is too large to be processed.",
                    entrypoint_path.display(),
                );

                return Err(anyhow::anyhow!("Source file is too large to be processed."));
            }
        };

        let mut parser = parse::Parser::new(lexer, &log);

        let ast_root = parser.parse_source(manifest.package.name.clone().into());

        match args.format_mode {
            Some(mode) if mode == "minify" => {
                serde_json::to_writer(&mut std::io::stdout(), &ast_root).expect("Failed to write AST to stdout");
            }

            Some(mode) if mode == "pretty" => {
                serde_json::to_writer_pretty(&mut std::io::stdout(), &ast_root).expect("Failed to write AST to stdout");
            }

            Some(mode) if mode == "source" => {
                println!("{:?}", ast_root);
            }

            _ => {
                serde_json::to_writer_pretty(&mut std::io::stdout(), &ast_root).expect("Failed to write AST to stdout");
            }
        }

        Ok(())
    }
}
