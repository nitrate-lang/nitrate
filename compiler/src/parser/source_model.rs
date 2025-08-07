use super::parse::Parser;
use crate::parsetree::ExprKey;
use spdx::LicenseId;
use spdx::license_id;
use std::collections::HashMap;

#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct CopyrightMetadata<'a> {
    author_name: Option<&'a str>,
    copyright_year: Option<u16>,
    license_id: Option<LicenseId>,
}

impl<'a> CopyrightMetadata<'a> {
    pub(crate) fn new(
        author_name: Option<&'a str>,
        copyright_year: Option<u16>,
        license_id: Option<LicenseId>,
    ) -> Self {
        CopyrightMetadata {
            author_name,
            copyright_year,
            license_id,
        }
    }

    pub fn author_name(&self) -> Option<&'a str> {
        self.author_name
    }

    pub fn copyright_year(&self) -> Option<u16> {
        self.copyright_year
    }

    pub fn license_name(&self) -> Option<LicenseId> {
        self.license_id
    }
}

#[derive(Debug, Clone)]
pub struct SourceModel<'a> {
    language_version: (u32, u32),
    copyright: CopyrightMetadata<'a>,
    insource_config: HashMap<&'a str, ExprKey<'a>>,
    tree: ExprKey<'a>,
    any_errors: bool,
}

impl<'a> SourceModel<'a> {
    pub(crate) fn new(
        language_version: (u32, u32),
        copyright: CopyrightMetadata<'a>,
        insource_config: HashMap<&'a str, ExprKey<'a>>,
        tree: ExprKey<'a>,
        any_errors: bool,
    ) -> Self {
        SourceModel {
            language_version,
            copyright,
            insource_config,
            tree,
            any_errors,
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

    pub fn any_errors(&self) -> bool {
        self.any_errors
    }
}

pub struct SourcePreamble<'a> {
    pub language_version: (u32, u32),
    pub copyright: CopyrightMetadata<'a>,
    pub insource_config: HashMap<&'a str, ExprKey<'a>>,
}

impl<'storage, 'a> Parser<'storage, 'a> {
    pub(crate) fn parse_preamble(&mut self) -> SourcePreamble<'a> {
        // TODO: Actually parse preamble from source file

        let language_version = (1, 0);

        let source_license = "MIT";
        let source_author = "Wesley";
        let source_year = 2025;

        SourcePreamble {
            language_version,
            copyright: CopyrightMetadata::new(
                Some(source_author),
                Some(source_year),
                license_id(source_license),
            ),
            insource_config: HashMap::new(),
        }
    }
}
