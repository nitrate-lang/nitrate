use crate::lexical::StringData;
use crate::parsetree::Expr;
use spdx::LicenseId;
use std::collections::HashSet;
use std::sync::Arc;

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

    #[must_use]
    pub fn holder_name(&self) -> &StringData<'a> {
        &self.holder_name
    }

    #[must_use]
    pub fn copyright_year(&self) -> u16 {
        self.copyright_year
    }
}

#[derive(Debug, Clone)]
pub struct SourceModel<'a> {
    language_version: (u32, u32),
    copyright: Option<CopyrightInfo<'a>>,
    license_id: Option<LicenseId>,
    insource_config: HashSet<StringData<'a>>,
    tree: Arc<Expr<'a>>,
    any_errors: bool,
}

impl<'a> SourceModel<'a> {
    pub(crate) fn new(
        language_version: (u32, u32),
        copyright: Option<CopyrightInfo<'a>>,
        license_id: Option<LicenseId>,
        insource_config: HashSet<StringData<'a>>,
        tree: Arc<Expr<'a>>,
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

    #[must_use]
    pub fn version(&self) -> (u32, u32) {
        self.language_version
    }

    #[must_use]
    pub fn copyright(&self) -> Option<&CopyrightInfo<'a>> {
        self.copyright.as_ref()
    }

    #[must_use]
    pub fn license_name(&self) -> Option<&LicenseId> {
        self.license_id.as_ref()
    }

    #[must_use]
    pub fn insource_config(&self) -> &HashSet<StringData<'a>> {
        &self.insource_config
    }

    #[must_use]
    pub fn tree(&self) -> Arc<Expr<'a>> {
        self.tree.clone()
    }

    #[must_use]
    pub fn any_errors(&self) -> bool {
        self.any_errors
    }
}
