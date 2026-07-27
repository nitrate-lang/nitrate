use std::io::Read;

use crate::Interpreter;
use clap::Parser;
use nitrate_diagnosis::intern_file_id;
use nitrate_translation::{
    token::AnnotatedToken,
    token_lexer::{Lexer, LexerError, LexerIterator},
};
use slog::error;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct LexArgs {
    /// Format mode for printed output
    #[arg(long, value_parser = ["minify", "pretty"])]
    format_mode: Option<String>,
}

impl Interpreter<'_> {
    pub(crate) fn sc_lex(&mut self, args: LexArgs) -> anyhow::Result<()> {
        let package = self.get_package_config()?;
        self.validate_package_edition(package.edition())?;
        let entrypoint_path = package.entrypoint();

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

        let lexer_iter = LexerIterator::new(lexer);

        let tokens: Vec<AnnotatedToken> = lexer_iter.collect();

        match args.format_mode {
            Some(mode) if mode == "minify" => {
                serde_json::to_writer(&mut std::io::stdout(), &tokens).expect("Failed to write AST to stdout");
            }

            Some(mode) if mode == "pretty" => {
                serde_json::to_writer_pretty(&mut std::io::stdout(), &tokens).expect("Failed to write AST to stdout");
            }

            _ => {
                serde_json::to_writer_pretty(&mut std::io::stdout(), &tokens).expect("Failed to write AST to stdout");
            }
        }

        Ok(())
    }
}
