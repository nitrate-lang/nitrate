use super::parse::*;
use crate::lexer::*;
use crate::parsetree::*;
use slog::error;

impl<'storage, 'logger, 'a> Parser<'storage, 'logger, 'a> {
    pub(crate) fn parse_expression(&mut self) -> Option<ExprKey<'a>> {
        let mut parenthesis_count = 0;
        while self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            parenthesis_count += 1;
        }

        let Some(expr) = self.parse_expression_primary() else {
            self.set_failed_bit();
            return None;
        };

        self.storage.add_parentheses(expr);

        for _ in 0..parenthesis_count {
            if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: expected closing parenthesis\n--> {}",
                    self.lexer.sync_position()
                );
                return None;
            }
        }

        // TODO: Binary expression parsing logic
        // TODO: Unary prefix expression parsing logic
        // TODO: Unary postfix expression parsing logic

        Some(expr)
    }

    fn parse_expression_primary(&mut self) -> Option<ExprKey<'a>> {
        match self.lexer.next_t() {
            Token::Integer(x) => Builder::new(self.storage)
                .create_integer()
                .with_u128(x.value()) // FIXME: Infer integer size?
                .with_kind(x.kind())
                .build(),

            Token::Float(x) => Builder::new(self.storage)
                .create_float()
                .with_value(x.value())
                .build(),

            Token::String(x) => Builder::new(self.storage)
                .create_string()
                .with_string(x)
                .build(),

            Token::Char(x) => Builder::new(self.storage).create_char(x),

            Token::Comment(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: unexpected comment in expression\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Eof => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: unexpected end of file\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            Token::Illegal => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "[P????]: expr: illegal token\n--> {}",
                    self.lexer.sync_position()
                );

                None
            }

            _ => {
                // TODO: Implement primary expression parsing logic
                error!(self.log, "Primary expression parsing not implemented yet");
                self.set_failed_bit();
                None
            }
        }
    }
}
