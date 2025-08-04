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
        let first_token = self.lexer.peek();

        match first_token.token() {
            Token::Name(name) => {
                self.lexer.skip();

                let mut bb = Builder::new(self.storage);

                match name.name() {
                    "u1" | "bool" => Some(bb.get_bool()),
                    "u8" => Some(bb.get_u8()),
                    "u16" => Some(bb.get_u16()),
                    "u32" => Some(bb.get_u32()),
                    "u64" => Some(bb.get_u64()),
                    "u128" => Some(bb.get_u128()),
                    "i8" => Some(bb.get_i8()),
                    "i16" => Some(bb.get_i16()),
                    "i32" => Some(bb.get_i32()),
                    "i64" => Some(bb.get_i64()),
                    "i128" => Some(bb.get_i128()),
                    "f8" => Some(bb.get_f8()),
                    "f16" => Some(bb.get_f16()),
                    "f32" => Some(bb.get_f32()),
                    "f64" => Some(bb.get_f64()),
                    "f128" => Some(bb.get_f128()),

                    _ => {
                        self.set_failed_bit();
                        error!(
                            self.log,
                            "Named type parsing not yet implemented: '{}'\n--> {}",
                            name.name(),
                            first_token.start()
                        );

                        // TODO: Create type name identifier
                        None
                    }
                }
            }

            Token::Integer(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected integer token while parsing type\n--> {}",
                    first_token.start()
                );
                None
            }

            Token::Float(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected float token while parsing type\n--> {}",
                    first_token.start()
                );
                None
            }

            Token::Keyword(_) => {
                // TODO: Handle keywords (like 'fn', 'struct', 'opaque', etc.)
                None
            }

            Token::String(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected string token while parsing type\n--> {}",
                    first_token.start()
                );
                None
            }

            Token::Char(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected character token while parsing type\n--> {}",
                    first_token.start()
                );
                None
            }

            Token::Punct(_) => {
                // TODO: Handle punctuation (arrays, functions, tuples, etc.)
                None
            }

            Token::Op(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected operator token while parsing type\n--> {}",
                    first_token.start()
                );
                None
            }

            Token::Comment(_) => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected comment token while parsing type\n--> {}",
                    first_token.start()
                );
                None
            }

            Token::Eof => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Unexpected end of file while parsing type\n--> {}",
                    first_token.start()
                );
                None
            }

            Token::Illegal => {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Illegal token encountered during type parsing\n--> {}",
                    first_token.start()
                );
                None
            }
        }
    }

    pub fn parse_type(&mut self) -> Option<TypeKey<'a>> {
        if self.lexer.skip_if(&Token::Punct(Punct::LeftParen)) {
            let mut inner = self.parse_type();

            if !self.lexer.skip_if(&Token::Punct(Punct::RightParen)) {
                self.set_failed_bit();
                error!(
                    self.log,
                    "error[P????]: Expected right parenthesis after type expression\n--> {}",
                    self.lexer.peek().start()
                );
            }

            inner.inspect(|v| v.add_parentheses(self.storage))
        } else {
            let primary = self.parse_type_primary();
            let constraints = self.parse_type_constraints();

            // TODO: Create wrapper node for types with constraints
            primary
        }
    }
}
