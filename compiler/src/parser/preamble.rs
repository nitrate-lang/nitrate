use super::parse::Parser;
use super::source_model::CopyrightInfo;
use crate::lexer::{Punct, StringData, Token};
use crate::parsetree::{ExprKey, ExprRef};
use log::error;
use spdx::{LicenseId, license_id};
use std::collections::HashSet;

pub(crate) struct SourcePreamble<'a> {
    pub language_version: (u32, u32),
    pub copyright: Option<CopyrightInfo<'a>>,
    pub license_id: Option<LicenseId>,
    pub insource_config: HashSet<StringData<'a>>,
}

impl<'a> Parser<'a, '_, '_> {
    fn parse_macro_prefix(&mut self) -> Option<(&'a str, Vec<ExprKey<'a>>)> {
        while self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {}
        if !self.lexer.skip_if(&Token::Punct(Punct::AtSign)) {
            return None;
        }

        let Some(macro_name) = self.lexer.next_if_name() else {
            self.set_failed_bit();
            error!(
                "[P????]: Expected macro name after '@'\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let mut macro_args = Vec::new();

        self.lexer.skip_if(&Token::Punct(Punct::Comma));
        while !self.lexer.is_eof() {
            if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
                break;
            }

            let Some(macro_argument) = self.parse_expression() else {
                self.set_failed_bit();
                error!(
                    "[P????]: Unable to parse argument expression for macro '{}'\n--> {}",
                    macro_name.name(),
                    self.lexer.sync_position()
                );
                break;
            };

            macro_args.push(macro_argument);

            if !self.lexer.skip_if(&Token::Punct(Punct::Comma)) {
                if !self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
                    self.set_failed_bit();
                    error!(
                        "[P????]: Expected ',' or ';' after macro argument expression\n--> {}",
                        self.lexer.sync_position()
                    );
                }
                break;
            }
        }

        Some((macro_name.name(), macro_args))
    }

    fn parse_preamble_version_number(&self, number: f64) -> Option<(u32, u32)> {
        let mut pair = (None, None);

        let input = number.to_string();
        if let Some(dot_pos) = input.find('.') {
            let (major_str, minor_str) = input.split_at(dot_pos);

            if let Ok(major) = major_str.parse() {
                pair.0 = Some(major);
            }

            if let Ok(minor) = minor_str[1..].parse() {
                pair.1 = Some(minor);
            }
        } else if let Ok(major) = input.parse() {
            pair.0 = Some(major);
            pair.1 = Some(0);
        }

        pair.0.zip(pair.1)
    }

    fn parse_preamble_version_macro(&mut self, macro_args: Vec<ExprKey<'a>>) -> Option<(u32, u32)> {
        if macro_args.len() != 1 {
            self.set_failed_bit();
            error!(
                "[P????]: Expected exactly one argument (e.g. '1.0') for 'nitrate' (language version) macro\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        }

        let ExprRef::FloatLit(float) = macro_args[0].get(self.storage) else {
            self.set_failed_bit();
            error!(
                "[P????]: Expected a float literal (e.g. '1.0') for 'nitrate' (language version) macro argument\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let Some((major, minor)) = self.parse_preamble_version_number(float) else {
            self.set_failed_bit();
            error!(
                "[P????]: Invalid version format '{}'. Expected 'major.minor' (e.g. '1.0') for 'nitrate' (language version) macro argument\n--> {}",
                float,
                self.lexer.sync_position()
            );

            return None;
        };

        Some((major, minor))
    }

    fn parse_preamble_copyright_macro(
        &mut self,
        macro_args: Vec<ExprKey<'a>>,
    ) -> Option<CopyrightInfo<'a>> {
        if macro_args.len() != 2 {
            self.set_failed_bit();
            error!(
                "[P????]: Expected exactly two arguments (year and holder's name) for @copyright\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        }

        let ExprRef::IntegerLit(copyright_year) = macro_args[0].get(self.storage) else {
            self.set_failed_bit();
            error!(
                "[P????]: Unable to parse @copyright; expected an integer literal for the first argument (copyright year)\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let ExprRef::StringLit(holder_name) = macro_args[1].get(self.storage) else {
            self.set_failed_bit();
            error!(
                "[P????]: Unable to parse @copyright; expected a string literal for the second argument (copyright holder's name)\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        };

        Some(CopyrightInfo::new(
            holder_name.to_owned(),
            copyright_year.get_u128() as u16,
        ))
    }

    fn parse_preamble_license_macro(&mut self, macro_args: Vec<ExprKey<'a>>) -> Option<LicenseId> {
        if macro_args.len() != 1 {
            self.set_failed_bit();
            error!(
                "[P????]: Expected exactly one argument (SPDX license ID string) for 'license' macro\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        }

        let ExprRef::StringLit(license_name) = macro_args[0].get(self.storage) else {
            self.set_failed_bit();
            error!(
                "[P????]: Expected a string literal (SPDX license ID) for 'license' macro argument\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        };

        let Some(license_id) = license_id(license_name.get()) else {
            error!(
                "[P????]: The SPDX license database does not contain \"{}\". Only valid SPDX license IDs are allowed.\n--> {}",
                license_name.get(),
                self.lexer.sync_position()
            );
            self.set_failed_bit();

            return None;
        };

        Some(license_id)
    }

    fn parse_preamble_insource_macro(
        &mut self,
        macro_args: Vec<ExprKey<'a>>,
    ) -> Option<HashSet<StringData<'a>>> {
        if macro_args.len() != 1 {
            self.set_failed_bit();
            error!(
                "[P????]: Expected a list of strings for 'insource' macro\n--> {}",
                self.lexer.sync_position()
            );
            return None;
        }

        let ExprRef::ListLit(list) = macro_args[0].get(self.storage) else {
            self.set_failed_bit();
            error!(
                "[P????]: Expected a list of strings for 'insource' macro argument\n--> {}",
                self.lexer.sync_position()
            );

            return None;
        };

        let mut config = HashSet::new();
        for element in list.elements() {
            let ExprRef::StringLit(option) = element.get(self.storage) else {
                self.set_failed_bit();
                error!(
                    "[P????]: Expected a string literal in the list for 'insource' macro argument\n--> {}",
                    self.lexer.sync_position()
                );

                return None;
            };

            config.insert(option.to_owned());
        }

        Some(config)
    }

    pub(crate) fn parse_preamble(&mut self) -> SourcePreamble<'a> {
        let mut language_version = (1, 0);
        let mut copyright = None;
        let mut license_id = None;
        let mut insource_config = None;

        while let Some((macro_name, macro_args)) = self.parse_macro_prefix() {
            match macro_name {
                "nitrate" => {
                    self.parse_preamble_version_macro(macro_args)
                        .inspect(|version| {
                            language_version = version.to_owned();
                        });
                }

                "copyright" => {
                    copyright = self.parse_preamble_copyright_macro(macro_args);
                }

                "license" => {
                    license_id = self.parse_preamble_license_macro(macro_args);
                }

                "insource" => {
                    insource_config = self.parse_preamble_insource_macro(macro_args);
                }

                _ => {
                    self.set_failed_bit();
                    error!(
                        "[P????]: Unknown macro '{}'\n--> {}",
                        macro_name,
                        self.lexer.sync_position()
                    );
                }
            }
        }

        SourcePreamble {
            language_version,
            copyright,
            license_id,
            insource_config: insource_config.unwrap_or_default(),
        }
    }
}
