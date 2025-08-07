use super::source_model::SourceModel;
use crate::lexer::*;
use crate::parsetree::*;
use slog::Logger;

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

    pub(crate) fn set_failed_bit(&mut self) {
        self.failed_bit = true;
    }

    pub fn has_failed(&self) -> bool {
        self.failed_bit
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

            _ => None,
        }
    }

    pub fn parse(&mut self) -> Option<SourceModel<'a>> {
        let preamble = self.parse_preamble();
        let program = self.parse_type()?.into();

        Some(SourceModel::new(
            preamble.language_version,
            preamble.copyright,
            preamble.insource_config,
            program,
            self.has_failed(),
        ))
    }
}
