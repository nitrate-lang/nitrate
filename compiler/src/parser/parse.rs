use super::source_model::SourceModel;
use crate::lexer::*;
use crate::parsetree::*;
use slog::{Logger, error, info};

pub struct Parser<'storage, 'a> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) storage: &'storage mut Storage<'a>,
    pub(crate) log: Logger,
    // Source files are 2^32 bytes max, so this won't overflow
    pub(crate) generic_type_depth: i64,
    pub(crate) generic_type_suffix_terminator_ambiguity: bool,
    failed_bit: bool,
}

impl<'storage, 'a> Parser<'storage, 'a> {
    pub fn new(lexer: Lexer<'a>, storage: &'storage mut Storage<'a>, log: Option<Logger>) -> Self {
        let log = log.unwrap_or_else(|| slog::Logger::root(slog::Discard, slog::o!()));

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

    pub fn parse_expression(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Develop nitrate expression parser

        match self.lexer.peek_t() {
            Token::Integer(int) => {
                self.lexer.skip();

                Builder::new(self.storage)
                    .create_integer()
                    .with_u128(int.value())
                    .build()
            }

            Token::Float(float) => {
                self.lexer.skip();

                Builder::new(self.storage)
                    .create_float()
                    .with_value(float.value())
                    .build()
            }

            Token::String(string) => {
                self.lexer.skip();

                Builder::new(self.storage)
                    .create_string()
                    .with_utf8string(string)
                    .build()
            }

            _ => self.parse_type().map(|t| t.into()),
        }
    }

    pub fn is_parser_compatible(&self, language_version: (u32, u32)) -> bool {
        match language_version {
            // Future major versions might introduce breaking syntactic changes
            (1, _) => true,

            _ => false,
        }
    }

    pub fn parse(&mut self) -> Option<SourceModel<'a>> {
        let preamble = self.parse_preamble();

        if !self.is_parser_compatible(preamble.language_version) {
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
            self.set_failed_bit();
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
            .build()?;

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
