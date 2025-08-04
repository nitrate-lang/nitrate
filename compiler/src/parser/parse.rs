use super::source_model::{CopyrightMetadata, SourceModel};
use crate::lexer::*;
use crate::parsetree::*;
use slog::Logger;
use std::collections::HashMap;

pub struct Parser<'storage, 'a> {
    pub(crate) lexer: Lexer<'a>,
    bb: Builder<'storage, 'a>,
    pub(crate) log: Logger,
}

impl<'storage, 'a> Parser<'storage, 'a> {
    pub fn new(lexer: Lexer<'a>, storage: &'storage mut Storage<'a>, log: Logger) -> Self {
        Parser {
            lexer,
            bb: Builder::new(storage),
            log,
        }
    }

    pub fn parse_expression(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Develop nitrate parser

        self.parse_type();

        None
    }

    pub fn parse(&mut self) -> Option<SourceModel<'a>> {
        let copyright_info = CopyrightMetadata::default();
        let language_version = (1, 0);
        let program = self.parse_expression()?;

        Some(SourceModel::new(
            language_version,
            copyright_info,
            HashMap::new(),
            program,
        ))
    }
}
