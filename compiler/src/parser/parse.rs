use super::source_model::{CopyrightMetadata, SourceModel};
use crate::lexer::*;
use crate::parsetree::*;
use slog::Logger;
use spdx::license_id;
use std::collections::HashMap;

pub struct Parser<'storage, 'a> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) storage: &'storage mut Storage<'a>,
    pub(crate) log: Logger,
    failed_bit: bool,
}

impl<'storage, 'a> Parser<'storage, 'a> {
    pub fn new(lexer: Lexer<'a>, storage: &'storage mut Storage<'a>, log: Logger) -> Self {
        Parser {
            lexer,
            storage,
            log,
            failed_bit: false,
        }
    }

    pub fn get_storage(&self) -> &Storage<'a> {
        self.storage
    }

    pub fn get_storage_mut(&mut self) -> &mut Storage<'a> {
        self.storage
    }

    pub(crate) fn set_failed_bit(&mut self) {
        self.failed_bit = true;
    }

    pub fn has_failed(&self) -> bool {
        self.failed_bit
    }

    pub fn parse_expression(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Develop nitrate parser

        self.parse_type().map(|t| t.into())

        // None
    }

    pub fn parse(&mut self) -> Option<SourceModel<'a>> {
        let language_version = (1, 0);

        let source_license = "MIT";
        let source_author = "Wesley";
        let source_year = 2025;
        let copyright_info = CopyrightMetadata::new(
            Some(source_author),
            Some(source_year),
            license_id(source_license),
        );

        let program = self.parse_expression()?;

        Some(SourceModel::new(
            language_version,
            copyright_info,
            HashMap::new(),
            program,
        ))
    }
}
