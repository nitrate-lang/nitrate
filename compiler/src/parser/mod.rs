use crate::lexer::*;
use crate::parsetree::*;
use spdx::LicenseId;
use std::collections::HashMap;

pub struct CopyrightMetadata<'a> {
    author_name: Option<&'a str>,
    copyright_year: Option<u16>,
    copyright_string: Option<&'a str>,
    license_id: Option<LicenseId>,
}

impl<'a> CopyrightMetadata<'a> {
    fn new(
        author_name: Option<&'a str>,
        copyright_year: Option<u16>,
        copyright_string: Option<&'a str>,
        license_id: Option<LicenseId>,
    ) -> Self {
        CopyrightMetadata {
            author_name,
            copyright_year,
            copyright_string,
            license_id,
        }
    }

    pub fn author_name(&self) -> Option<&'a str> {
        self.author_name
    }

    pub fn copyright_year(&self) -> Option<u16> {
        self.copyright_year
    }

    pub fn copyright_string(&self) -> Option<&'a str> {
        self.copyright_string
    }

    pub fn license_name(&self) -> Option<LicenseId> {
        self.license_id
    }
}

pub struct SourceModel<'a> {
    language_version: (u32, u32),
    copyright: CopyrightMetadata<'a>,
    insource_config: HashMap<&'a str, ExprKey<'a>>,
    tree: ExprKey<'a>,
}

impl<'a> SourceModel<'a> {
    fn new(
        language_version: (u32, u32),
        copyright: CopyrightMetadata<'a>,
        insource_config: HashMap<&'a str, ExprKey<'a>>,
        tree: ExprKey<'a>,
    ) -> Self {
        SourceModel {
            language_version,
            copyright,
            insource_config,
            tree,
        }
    }

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
