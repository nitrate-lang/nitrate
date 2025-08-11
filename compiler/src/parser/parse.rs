use super::source_model::SourceModel;
use crate::lexer::*;
use crate::parsetree::*;
use slog::{Logger, error, info};

pub struct Parser<'storage, 'logger, 'a> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) storage: &'storage mut Storage<'a>,
    pub(crate) log: &'logger mut Logger,
    // Source files are 2^32 bytes max, so this won't overflow
    pub(crate) generic_type_depth: i64,
    pub(crate) generic_type_suffix_terminator_ambiguity: bool,
    failed_bit: bool,
}

impl<'storage, 'logger, 'a> Parser<'storage, 'logger, 'a> {
    pub fn new(
        lexer: Lexer<'a>,
        storage: &'storage mut Storage<'a>,
        log: &'logger mut Logger,
    ) -> Self {
        Parser {
            lexer,
            storage,
            log,
            generic_type_depth: 0,
            generic_type_suffix_terminator_ambiguity: false,
            failed_bit: false,
        }
    }

    pub fn get_storage(&self) -> &Storage<'a> {
        self.storage
    }

    pub fn get_storage_mut(&mut self) -> &mut Storage<'a> {
        self.storage
    }

    pub fn has_failed(&self) -> bool {
        self.failed_bit
    }

    pub(crate) fn set_failed_bit(&mut self) {
        self.failed_bit = true;
    }

    pub fn is_supported(&self, language_version: (u32, u32)) -> bool {
        matches!(language_version, (1, _))
    }

    pub fn parse(&mut self) -> Option<SourceModel<'a>> {
        let preamble = self.parse_preamble();

        if !self.is_supported(preamble.language_version) {
            self.set_failed_bit();
            error!(
                self.log,
                "[P????]: This compiler does not support Nitrate version {}.{}.",
                preamble.language_version.0,
                preamble.language_version.1
            );
            info!(
                self.log,
                "[P????]: Consider upgrading to a newer version of the compiler."
            );

            return None;
        }

        let mut expressions = Vec::new();
        while !self.lexer.is_eof() {
            let Some(expression) = self.parse_expression() else {
                // Resynchronize the lexer to the next semicolon
                let before_pos = self.lexer.sync_position();
                while !self.lexer.is_eof() && self.lexer.next_t() != Token::Punct(Punct::Semicolon)
                {
                }

                if before_pos == self.lexer.sync_position() {
                    self.set_failed_bit();
                    break;
                }

                continue;
            };

            expressions.push(expression);
        }

        let block = Builder::new(self.storage)
            .create_block()
            .add_expressions(expressions)
            .build();

        Some(SourceModel::new(
            preamble.language_version,
            preamble.copyright,
            preamble.license_id,
            preamble.insource_config,
            block,
            self.has_failed(),
        ))
    }
}
