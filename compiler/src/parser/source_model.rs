use crate::lexer::*;
use crate::parsetree::*;
use spdx::LicenseId;
use std::collections::HashMap;

#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct CopyrightInfo<'a> {
    author_name: Option<StringData<'a>>,
    copyright_year: Option<u16>,
}

impl<'a> CopyrightInfo<'a> {
    pub(crate) fn new(author_name: Option<StringData<'a>>, copyright_year: Option<u16>) -> Self {
        CopyrightInfo {
            author_name,
            copyright_year,
        }
    }

    pub fn author_name(&self) -> Option<&StringData<'a>> {
        self.author_name.as_ref()
    }

    pub fn copyright_year(&self) -> Option<u16> {
        self.copyright_year
    }
}

#[derive(Debug, Clone)]
pub struct SourceModel<'a> {
    language_version: (u32, u32),
    copyright: CopyrightInfo<'a>,
    license_id: Option<LicenseId>,
    insource_config: HashMap<&'a str, ExprKey<'a>>,
    tree: ExprKey<'a>,
    any_errors: bool,
}

impl<'a> SourceModel<'a> {
    pub(crate) fn new(
        language_version: (u32, u32),
        copyright: CopyrightInfo<'a>,
        license_id: Option<LicenseId>,
        insource_config: HashMap<&'a str, ExprKey<'a>>,
        tree: ExprKey<'a>,
        any_errors: bool,
    ) -> Self {
        SourceModel {
            language_version,
            copyright,
            license_id,
            insource_config,
            tree,
            any_errors,
        }
    }

    pub fn version(&self) -> (u32, u32) {
        self.language_version
    }

    pub fn copyright(&self) -> &CopyrightInfo<'a> {
        &self.copyright
    }

    pub fn license_name(&self) -> Option<&LicenseId> {
        self.license_id.as_ref()
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
