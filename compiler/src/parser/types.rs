use slog::error;

use super::parse::*;
use crate::lexer::*;
use crate::parsetree::*;

#[derive(Default)]
struct TypeConstraints<'a> {
    minimum: Option<ExprKey<'a>>,
    maximum: Option<ExprKey<'a>>,
    width: Option<ExprKey<'a>>,
}

impl<'storage, 'a> Parser<'storage, 'a> {
    fn parse_type_constraints(&mut self) -> Option<TypeConstraints<'a>> {
        // TODO: Type constraint parser
        Some(TypeConstraints::default())
    }

    fn parse_type_primary(&mut self) -> Option<TypeKey<'a>> {
        // Placeholder for primary type parsing logic
        None
    }

    pub fn parse_type(&mut self) -> Option<TypeKey<'a>> {
        let first_token = self.lexer.peek_token();

        match first_token.token() {
            Token::Identifier(_) => {
                // TODO:
                None
            }

            Token::Integer(_) => {
                // TODO:
                None
            }

            Token::Float(_) => {
                // TODO:
                None
            }

            Token::Keyword(_) => {
                // TODO:
                None
            }

            Token::String(_) => {
                // TODO:
                None
            }

            Token::Char(_) => {
                // TODO:
                None
            }

            Token::Punctuation(_) => {
                // TODO:
                None
            }

            Token::Operator(_) => {
                // TODO:
                None
            }

            Token::Comment(_) => {
                // TODO:
                None
            }

            Token::Eof => {
                // TODO:
                None
            }

            Token::Illegal => {
                error!(
                    self.log,
                    "error[P????]: Illegal token encountered during type parsing\n--> {}",
                    first_token.start()
                );

                None
            }
        }
    }
}
