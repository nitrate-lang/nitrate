use crate::lexer::*;
use crate::parsetree::*;
use std::collections::HashMap;

pub struct CopyrightMetadata<'a> {
    pub author_name: Option<&'a str>,
    pub copyright_year: Option<u16>,
    pub copyright_string: Option<&'a str>,
    pub some_license_name: Option<&'a str>,
}

pub struct SourceModel<'a> {
    language_version: (u32, u32),
    copyright: CopyrightMetadata<'a>,
    insource_config: HashMap<&'a str, ExprKey<'a>>,
    tree: ExprKey<'a>,
}

impl<'a> SourceModel<'a> {
    pub fn version(&self) -> (u32, u32) {
        self.language_version
    }

    pub fn copyright(&self) -> &CopyrightMetadata<'a> {
        &self.copyright
    }

    pub fn insource_config(&self) -> &HashMap<&'a str, ExprKey<'a>> {
        &self.insource_config
    }

    pub fn tree(&self) -> ExprKey<'a> {
        self.tree
    }
}

pub struct Parser<'storage, 'lexer, 'a> {
    lexer: &'lexer mut Lexer<'a>,
    bb: Builder<'storage, 'a>,
}

impl<'storage, 'lexer, 'a> Parser<'storage, 'lexer, 'a> {
    pub fn new(lexer: &'lexer mut Lexer<'a>, storage: &'storage mut Storage<'a>) -> Self {
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
        // self.lexer.clone();
        // self.parse_expression()
        None
    }
}
