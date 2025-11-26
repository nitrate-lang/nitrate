use std::io::Read;

use crate::{Interpreter, InterpreterError};
use clap::Parser;
use nitrate_diagnosis::{CompilerLog, intern_file_id};
use nitrate_translation::{
    parse2,
    parsetree2::{self, GlobalSource, using_source, using_storage},
    token_lexer::{Lexer, LexerError},
};
use slog::error;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct ParseArgs {
    /// Format mode for printed output
    #[arg(long, value_parser = ["minify", "pretty", "source"])]
    format_mode: Option<String>,
}

impl Interpreter<'_> {
    pub(crate) fn sc_parse(&mut self, args: ParseArgs) -> Result<(), InterpreterError> {
        let log = CompilerLog::new(self.log.clone());

        let package = self.get_package_config()?;
        self.validate_package_edition(package.edition())?;
        let entrypoint_path = package.entrypoint();

        if !entrypoint_path.exists() {
            error!(
                self.log,
                "Package entrypoint '{}' does not exist.",
                entrypoint_path.display()
            );
            return Err(InterpreterError::OperationalError);
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

                return Err(InterpreterError::IoError(e));
            }
        };

        let mut source_code = Vec::new();
        source_code_file
            .read_to_end(&mut source_code)
            .map_err(|e| InterpreterError::IoError(e))?;

        let source_code_file = intern_file_id(&entrypoint_path.to_string_lossy().to_string())
            .expect("FileId overflow");

        using_source(
            &GlobalSource {
                full_source: &source_code,
                fileid: Some(source_code_file.clone()),
            },
            || {
                using_storage(&parsetree2::Store::new(), || {
                    let lexer = match Lexer::new(&source_code, Some(source_code_file)) {
                        Ok(lexer) => lexer,

                        Err(LexerError::SourceTooBig) => {
                            error!(
                                self.log,
                                "Source file '{}' is too large to be processed.",
                                entrypoint_path.display(),
                            );

                            return Err(InterpreterError::OperationalError);
                        }
                    };

                    let mut parser = parse2::Parser::new(lexer, &log);

                    let ast_root = parser.parse_source();

                    match args.format_mode {
                        Some(mode) if mode == "minify" => {
                            serde_json::to_writer(&mut std::io::stdout(), &ast_root)
                                .expect("Failed to write AST to stdout");
                        }

                        Some(mode) if mode == "pretty" => {
                            serde_json::to_writer_pretty(&mut std::io::stdout(), &ast_root)
                                .expect("Failed to write AST to stdout");
                        }

                        Some(mode) if mode == "source" => {
                            println!("{}", ast_root);
                        }

                        _ => {
                            serde_json::to_writer_pretty(&mut std::io::stdout(), &ast_root)
                                .expect("Failed to write AST to stdout");
                        }
                    }

                    Ok(())
                })
            },
        )
    }
}
