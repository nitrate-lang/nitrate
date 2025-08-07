use super::parse::Parser;
use crate::lexer::*;
use crate::parsetree::*;
use slog::error;
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
    fn new(
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

#[derive(Default)]
pub struct SourcePreamble<'a> {
    pub language_version: (u32, u32),
    pub copyright: CopyrightMetadata<'a>,
    pub insource_config: HashMap<&'a str, ExprKey<'a>>,
}

impl<'storage, 'a> Parser<'storage, 'a> {
    fn parse_macro_prefix(&mut self) -> Option<(&'a str, Vec<ExprKey<'a>>)> {
        // FIXME: Skip comments
        while self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {}

        if !self.lexer.skip_if(&Token::Punct(Punct::AtSign)) {
            return None;
        }

        if let Token::Name(macro_name) = self.lexer.next_t() {
            let mut macro_args = Vec::new();

            self.lexer.skip_if(&Token::Punct(Punct::Comma));
            while !self.lexer.is_eof() {
                if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
                    break;
                }

                if let Some(arg) = self.parse_expression() {
                    macro_args.push(arg);
                } else {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Unable to parse macro argument for macro '{}'\n--> {}",
                        macro_name.name(),
                        self.lexer.sync_position()
                    );
                    break;
                }

                if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                    if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
                        break;
                    } else {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "error[P????]: Expected ',' or ';' after macro argument\n--> {}",
                            self.lexer.sync_position()
                        );
                        break;
                    }
                }
            }

            Some((macro_name.name(), macro_args))
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "error[P????]: Expected macro name after '@'\n--> {}",
                self.lexer.sync_position()
            );
            None
        }
    }

    pub(crate) fn parse_preamble(&mut self) -> SourcePreamble<'a> {
        // TODO: Actually parse preamble from source file
        let mut preamble = SourcePreamble::default();

        while let Some((macro_name, macro_args)) = self.parse_macro_prefix() {
            match macro_name {
                "nitrate" => {
                    if macro_args.len() != 1 {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "error[P????]: Expected exactly one argument (e.g. '1.0') for 'nitrate' (language version) macro\n--> {}",
                            self.lexer.sync_position()
                        );
                        continue;
                    }

                    let argument = macro_args.first().unwrap().get(self.storage);
                    if let ExprRef::FloatLit(float) = argument {
                        // Assuming the float is a version number like 1.0, we can parse it
                        let major = float as u32; // Take int part as major version
                        let minor = ((float - (major as f64)) * 100.0) as u32; // Take fractional part as minor version

                        preamble.language_version = (major, minor);
                    } else {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "error[P????]: Expected a float literal (e.g. '1.0') for 'nitrate' (language version) macro argument\n--> {}",
                            self.lexer.sync_position()
                        );
                    }
                }

                "copyright" => {
                    // TODO: Copyright metadata
                }

                "license" => {
                    if macro_args.len() != 1 {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "error[P????]: Expected exactly one argument (SPDX license ID string) for 'license' macro\n--> {}",
                            self.lexer.sync_position()
                        );
                        continue;
                    }

                    let argument = macro_args.first().unwrap().get(self.storage);
                    if let ExprRef::StringLit(maybe_license) = argument {
                        let maybe_license = maybe_license.clone().into_inner();

                        if let Some(license) = license_id(maybe_license.get()) {
                            preamble.copyright.license_id = Some(license);
                        } else {
                            self.set_failed_bit();
                            error!(
                                self.log,
                                "error[P????]: Unknown SPDX license ID {}. Expected a valid SPDX license ID string for 'license' macro argument\n--> {}",
                                maybe_license,
                                self.lexer.sync_position()
                            );
                        }
                    } else {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "error[P????]: Expected a string literal (SPDX license ID) for 'license' macro argument\n--> {}",
                            self.lexer.sync_position()
                        );
                    }
                }

                "insource" => {
                    // TODO: In-source configuration
                }

                _ => {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "error[P????]: Unknown macro '{}'\n--> {}",
                        macro_name,
                        self.lexer.sync_position()
                    );
                }
            }
        }

        preamble
    }
}
