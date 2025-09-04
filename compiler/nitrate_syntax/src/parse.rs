use super::source_model::SourceModel;
use super::symbol_table::SymbolTable;
use log::{error, info};
use nitrate_lexical::{Lexer, Punct, Token};
use nitrate_parsetree::{Builder, kind::QualifiedScope};
use smallvec::SmallVec;

pub struct Parser<'a, 'symbol_table> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) symtab: &'symbol_table mut SymbolTable<'a>,
    pub(crate) scope: QualifiedScope<'a>,
    pub(crate) generic_type_depth: i64,
    pub(crate) generic_type_suffix_terminator_ambiguity: bool,
    failed_bit: bool,
}

impl<'a, 'symbol_table> Parser<'a, 'symbol_table> {
    pub fn new(lexer: Lexer<'a>, symbol_table: &'symbol_table mut SymbolTable<'a>) -> Self {
        Parser {
            lexer,
            symtab: symbol_table,
            scope: QualifiedScope::new(SmallVec::new()),
            generic_type_depth: 0,
            generic_type_suffix_terminator_ambiguity: false,
            failed_bit: false,
        }
    }

    #[must_use]
    pub fn get_symbol_table(&self) -> &SymbolTable<'a> {
        self.symtab
    }

    pub fn get_symbol_table_mut(&mut self) -> &mut SymbolTable<'a> {
        self.symtab
    }

    #[must_use]
    pub fn has_failed(&self) -> bool {
        self.failed_bit
    }

    pub(crate) fn set_failed_bit(&mut self) {
        self.failed_bit = true;
    }

    #[must_use]
    pub fn is_supported(&self, language_version: (u32, u32)) -> bool {
        matches!(language_version, (1, _))
    }

    pub fn parse(&mut self) -> Option<SourceModel<'a>> {
        let preamble = self.parse_preamble();

        if !self.is_supported(preamble.language_version) {
            self.set_failed_bit();
            error!(
                "[P????]: This compiler does not support Nitrate version {}.{}.",
                preamble.language_version.0, preamble.language_version.1
            );
            info!("[P????]: Consider upgrading to a newer version of the compiler.");

            return None;
        }

        let mut expressions = Vec::new();
        while !self.lexer.is_eof() {
            if self.lexer.skip_if(&Token::Punct(Punct::Semicolon))
                || self.lexer.next_if_comment().is_some()
            {
                continue;
            }

            let Some(mut expression) = self.parse_expression() else {
                let before_pos = self.lexer.sync_position();
                loop {
                    match self.lexer.next_t() {
                        Token::Punct(Punct::Semicolon) | Token::Illegal | Token::Eof => {
                            // Resynchronize the lexer to the next semicolon
                            break;
                        }

                        _ => {}
                    }
                }

                if before_pos == self.lexer.sync_position() {
                    self.set_failed_bit();
                    break;
                }

                continue;
            };

            if self.lexer.skip_if(&Token::Punct(Punct::Semicolon)) {
                expression = Builder::create_statement()
                    .with_expression(expression)
                    .build();
            }

            expressions.push(expression);
        }

        let block = Builder::create_block().add_expressions(expressions).build();

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
