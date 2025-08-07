use super::parse::Parser;
use super::source_model::CopyrightInfo;
use crate::lexer::*;
use crate::parsetree::*;
use slog::error;
use spdx::LicenseId;
use spdx::license_id;
use std::collections::HashMap;

pub(crate) struct SourcePreamble<'a> {
    pub language_version: (u32, u32),
    pub copyright: CopyrightInfo<'a>,
    pub license_id: Option<LicenseId>,
    pub insource_config: HashMap<&'a str, ExprKey<'a>>,
}

impl<'a> Default for SourcePreamble<'a> {
    fn default() -> Self {
        SourcePreamble {
            language_version: (1, 0),
            copyright: CopyrightInfo::default(),
            license_id: None,
            insource_config: HashMap::default(),
        }
    }
}

impl<'storage, 'a> Parser<'storage, 'a> {
    fn parse_macro_prefix(&mut self) -> Option<(&'a str, Vec<ExprKey<'a>>)> {
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
                        "[P????]: Unable to parse argument expression for macro '{}'\n--> {}",
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
                            "[P????]: Expected ',' or ';' after macro argument expression\n--> {}",
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
                "[P????]: Expected macro name after '@'\n--> {}",
                self.lexer.sync_position()
            );
            None
        }
    }

    fn parse_preamble_version_number(&mut self, number: f64) -> Option<(u32, u32)> {
        let mut pair = (None, None);

        let input = number.to_string();
        if let Some(dot_pos) = input.find(".") {
            let (major_str, minor_str) = input.split_at(dot_pos);

            if let Ok(major) = major_str.parse() {
                pair.0 = Some(major);
            }

            if let Ok(minor) = minor_str[1..].parse() {
                pair.1 = Some(minor);
            }
        } else {
            if let Ok(major) = input.parse() {
                pair.0 = Some(major);
                pair.1 = Some(0);
            }
        }

        pair.0.zip(pair.1).map(|(major, minor)| (major, minor))
    }

    fn parse_preamble_version_macro(&mut self, macro_args: Vec<ExprKey<'a>>) -> (u32, u32) {
        if macro_args.len() != 1 {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: Expected exactly one argument (e.g. '1.0') for 'nitrate' (language version) macro\n--> {}",
                self.lexer.sync_position()
            );
            return (1, 0);
        }

        if let ExprRef::FloatLit(float) = macro_args[0].get(self.storage) {
            if let Some((major, minor)) = self.parse_preamble_version_number(float) {
                (major, minor)
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: Invalid version format '{}'. Expected 'major.minor' (e.g. '1.0') for 'nitrate' (language version) macro argument\n--> {}",
                    float,
                    self.lexer.sync_position()
                );

                (1, 0)
            }
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: Expected a float literal (e.g. '1.0') for 'nitrate' (language version) macro argument\n--> {}",
                self.lexer.sync_position()
            );

            (1, 0)
        }
    }

    fn parse_preamble_copyright_macro(
        &mut self,
        macro_args: Vec<ExprKey<'a>>,
    ) -> CopyrightInfo<'a> {
        if macro_args.len() != 2 {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: Expected exactly two arguments (author name and year) for 'copyright' macro\n--> {}",
                self.lexer.sync_position()
            );
            return CopyrightInfo::default();
        }

        let mut author_name = None;
        let mut copyright_year = None;

        if let ExprRef::StringLit(name) = macro_args[0].get(self.storage) {
            author_name = Some(name.clone().into_inner());
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: Expected a string literal (author name) for 'copyright' macro argument\n--> {}",
                self.lexer.sync_position()
            );
        }

        if let ExprRef::IntegerLit(year) = macro_args[1].get(self.storage) {
            copyright_year = Some(year.get_u128() as u16);
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: Expected an integer literal (year) for 'copyright' macro argument\n--> {}",
                self.lexer.sync_position()
            );
        }

        if author_name.is_none() || copyright_year.is_none() {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: 'copyright' macro requires both author name and year to be specified\n--> {}",
                self.lexer.sync_position()
            );
        }

        CopyrightInfo::new(author_name, copyright_year)
    }

    fn parse_preamble_license_macro(&mut self, macro_args: Vec<ExprKey<'a>>) -> Option<LicenseId> {
        if macro_args.len() != 1 {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: Expected exactly one argument (SPDX license ID string) for 'license' macro\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        }

        if let ExprRef::StringLit(maybe_license) = macro_args[0].get(self.storage) {
            let maybe_license = maybe_license.clone().into_inner();

            if let Some(license_id) = license_id(maybe_license.get()) {
                return Some(license_id);
            } else {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: Unknown SPDX license ID {}. Expected a valid SPDX license ID string for 'license' macro argument\n--> {}",
                    maybe_license,
                    self.lexer.sync_position()
                );
            }
        } else {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: Expected a string literal (SPDX license ID) for 'license' macro argument\n--> {}",
                self.lexer.sync_position()
            );
        }

        None
    }

    fn parse_preamble_insource_macro(
        &mut self,
        _macro_args: Vec<ExprKey<'a>>,
        _config: &mut HashMap<&'a str, ExprKey<'a>>,
    ) {
        // TODO: In-source compiler/optimization/linting configuration options
    }

    pub(crate) fn parse_preamble(&mut self) -> SourcePreamble<'a> {
        let mut preamble = SourcePreamble::default();

        while let Some((macro_name, macro_args)) = self.parse_macro_prefix() {
            match macro_name {
                "nitrate" => {
                    preamble.language_version = self.parse_preamble_version_macro(macro_args);
                }

                "copyright" => {
                    preamble.copyright = self.parse_preamble_copyright_macro(macro_args);
                }

                "license" => {
                    preamble.license_id = self.parse_preamble_license_macro(macro_args);
                }

                "insource" => {
                    self.parse_preamble_insource_macro(macro_args, &mut preamble.insource_config);
                }

                _ => {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "[P????]: Unknown macro '{}'\n--> {}",
                        macro_name,
                        self.lexer.sync_position()
                    );
                }
            }
        }

        preamble
    }
}
