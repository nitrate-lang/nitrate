use crate::parsetree::ExprKey;
use spdx::LicenseId;
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
}

impl<'a> SourceModel<'a> {
    pub(crate) fn new(
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
