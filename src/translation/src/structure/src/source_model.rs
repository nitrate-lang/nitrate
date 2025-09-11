use crate::kind::Expr;
use interned_string::IString;
use spdx::LicenseId;
use std::collections::HashSet;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct CopyrightInfo {
    holder_name: IString,
    copyright_year: u16,
}

impl CopyrightInfo {
    pub fn new(holder_name: IString, copyright_year: u16) -> Self {
        CopyrightInfo {
            holder_name,
            copyright_year,
        }
    }

    #[must_use]
    pub fn holder_name(&self) -> &IString {
        &self.holder_name
    }

    #[must_use]
    pub fn copyright_year(&self) -> u16 {
        self.copyright_year
    }
}

#[derive(Debug, Clone)]
pub struct SourceModel {
    language_version: (u32, u32),
    copyright: Option<CopyrightInfo>,
    license_id: Option<LicenseId>,
    insource_config: HashSet<IString>,
    tree: Expr,
    any_errors: bool,
}

impl SourceModel {
    pub fn new(
        language_version: (u32, u32),
        copyright: Option<CopyrightInfo>,
        license_id: Option<LicenseId>,
        insource_config: HashSet<IString>,
        tree: Expr,
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
    pub fn copyright(&self) -> Option<&CopyrightInfo> {
        self.copyright.as_ref()
    }

    #[must_use]
    pub fn license_name(&self) -> Option<&LicenseId> {
        self.license_id.as_ref()
    }

    #[must_use]
    pub fn insource_config(&self) -> &HashSet<IString> {
        &self.insource_config
    }

    #[must_use]
    pub fn tree(&self) -> Expr {
        self.tree.clone()
    }

    #[must_use]
    pub fn any_errors(&self) -> bool {
        self.any_errors
    }
}
