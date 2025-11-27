use crate::{Interpreter, InterpreterError};
use clap::Parser;
use slog::info;

#[derive(Parser, Debug)]
#[command(about, long_about = None)]
pub(crate) struct LspArgs {}

impl Interpreter<'_> {
    pub(crate) fn sc_lsp(&mut self, _args: LspArgs) -> Result<(), InterpreterError> {
        info!(self.log, "package lsp sub-command invoked");
        // TODO: update logic here
        Ok(())
    }
}
