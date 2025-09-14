use nitrate_parsetree::kind::{Block, Expr};
use nitrate_tokenize::Lexer;

pub struct Parser<'a> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) generic_type_depth: i64,
    pub(crate) generic_type_suffix_terminator_ambiguity: bool,
    failed_bit: bool,
}

impl<'a> Parser<'a> {
    pub fn new(lexer: Lexer<'a>) -> Self {
        Parser {
            lexer,
            generic_type_depth: 0,
            generic_type_suffix_terminator_ambiguity: false,
            failed_bit: false,
        }
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

    pub fn parse(&mut self) -> Result<Expr, Expr> {
        let mut expressions = Vec::new();

        while !self.lexer.is_eof() {
            if self.lexer.next_if_comment().is_some() {
                continue;
            }

            let Some(expression) = self.parse_expression() else {
                self.set_failed_bit();
                break;
            };

            expressions.push(expression);
        }

        let block = Expr::Block(Box::new(Block {
            elements: expressions,
            ends_with_semi: true,
        }));

        if self.has_failed() {
            Err(block)
        } else {
            Ok(block)
        }
    }
}
