use crate::lexer::*;
use crate::parsetree::*;
use spdx::LicenseId;
use std::collections::HashMap;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct CopyrightInfo<'a> {
    holder_name: StringData<'a>,
    copyright_year: u16,
}

impl<'a> CopyrightInfo<'a> {
    pub(crate) fn new(holder_name: StringData<'a>, copyright_year: u16) -> Self {
        CopyrightInfo {
            holder_name,
            copyright_year,
        }
    }

    pub fn holder_name(&self) -> &StringData<'a> {
        &self.holder_name
    }

    pub fn copyright_year(&self) -> u16 {
        self.copyright_year
    }
}

#[derive(Debug, Clone)]
pub struct SourceModel<'a> {
    language_version: (u32, u32),
    copyright: Option<CopyrightInfo<'a>>,
    license_id: Option<LicenseId>,
    insource_config: HashMap<&'a str, ExprKey<'a>>,
    tree: ExprKey<'a>,
    any_errors: bool,
}

impl<'a> SourceModel<'a> {
    pub(crate) fn new(
        language_version: (u32, u32),
        copyright: Option<CopyrightInfo<'a>>,
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

    pub fn copyright(&self) -> Option<&CopyrightInfo<'a>> {
        self.copyright.as_ref()
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
