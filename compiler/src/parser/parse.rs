use super::source_model::{CopyrightMetadata, SourceModel};
use crate::lexer::*;
use crate::parsetree::*;
use std::collections::HashMap;

pub struct Parser<'storage, 'a> {
    lexer: Lexer<'a>,
    bb: Builder<'storage, 'a>,
}

impl<'storage, 'a> Parser<'storage, 'a> {
    pub fn new(lexer: Lexer<'a>, storage: &'storage mut Storage<'a>) -> Self {
        storage.reserve(ExprKind::Block, 1);

        Parser {
            lexer,
            bb: Builder::new(storage),
        }
    }

    pub fn parse_expression(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Develop nitrate parser

        None
    }

    pub fn parse_type(&mut self) -> Option<TypeKey<'a>> {
        // TODO: Develop nitrate type parser

        None
    }

    pub fn parse(&mut self) -> Option<SourceModel<'a>> {
        Some(SourceModel::new(
            (0, 0),
            CopyrightMetadata::new(
                Some("Author Name"),
                Some(2023),
                Some("Copyright String"),
                spdx::license_id("MIT"),
            ),
            HashMap::new(),
            self.bb.create_block().build()?,
        ))
    }
}
