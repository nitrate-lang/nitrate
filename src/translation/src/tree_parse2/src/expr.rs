use crate::diagnosis::SyntaxErr;

use super::parse::Parser;
use nitrate_token::{AnnotatedToken, Token};
use nitrate_tree2::prelude::*;

impl Parser<'_, '_> {
    pub fn parse_expression(&mut self, leading: Option<Trivia>) -> Expr {
        match self.lexer.peek().cloned() {
            Some(AnnotatedToken {
                token: Token::Integer(integer),
                ..
            }) => {
                let token = self.lexer.next().unwrap(); // Consume token

                Expr::Integer {
                    source_offset: token.start_offset,
                    trivia: leading,
                    value: integer.value(),
                }
            }

            Some(AnnotatedToken { token: Token::True, .. }) => {
                let token = self.lexer.next().unwrap(); // Consume token

                Expr::Boolean {
                    source_offset: token.start_offset,
                    trivia: leading,
                    value: true,
                }
            }

            Some(AnnotatedToken {
                token: Token::False, ..
            }) => {
                let token = self.lexer.next().unwrap(); // Consume token

                Expr::Boolean {
                    source_offset: token.start_offset,
                    trivia: leading,
                    value: false,
                }
            }

            Some(token) => {
                self.lexer.next(); // Consume unexpected token

                let issue = SyntaxErr::ExpectedExpr {
                    pos: Some(token.start().into()),
                };
                self.log.report(&issue);

                Expr::Trivia { trivia: leading }
            }

            None => {
                let issue = SyntaxErr::ExpectedExpr { pos: None };
                self.log.report(&issue);

                Expr::Trivia { trivia: leading }
            }
        }
    }
}
