use super::parse::*;
use crate::lexer::*;
use crate::parsetree::*;
use slog::error;

impl<'storage, 'logger, 'a> Parser<'storage, 'logger, 'a> {
    pub(crate) fn parse_expression(&mut self) -> Option<ExprKey<'a>> {
        match self.lexer.peek_t() {
            Token::Punct(Punct::LeftParen) => {
                self.lexer.next();

                let Some(expr) = self.parse_expression() else {
                    self.set_failed_bit();
                    return None;
                };

                if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                    self.set_failed_bit();
                    error!(
                        self.log,
                        "expr: expected closing parenthesis\n--> {}",
                        self.lexer.sync_position()
                    );
                    return None;
                }

                self.storage.add_parentheses(expr);

                Some(expr)
            }

            Token::Comment(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "expr: unexpected comment\n--> {}",
                    self.lexer.sync_position()
                );
                None
            }

            Token::Eof => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "expr: unexpected end of file\n--> {}",
                    self.lexer.sync_position()
                );
                None
            }

            Token::Illegal => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "expr: illegal token\n--> {}",
                    self.lexer.sync_position()
                );
                None
            }

            _ => {
                // TODO: Implement expression parsing logic
                error!(self.log, "Expression parsing not implemented yet");
                None
            }
        }
    }
}
