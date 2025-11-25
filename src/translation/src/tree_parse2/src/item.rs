use crate::diagnosis::SyntaxErr;

use super::parse::Parser;
use nitrate_nstring::NString;
use nitrate_token::{AnnotatedToken, Token};
use nitrate_tree2::prelude::*;

impl Parser<'_, '_> {
    fn parse_attribute_list(&mut self, leading: Option<Trivia>) -> Option<AttributeList> {
        let open_bracket_token = match self.lexer.peek()? {
            AnnotatedToken {
                token: Token::OpenBracket,
                ..
            } => self.lexer.next().unwrap(),
            _ => return None, // No attribute list found
        };

        let mut attributes = Vec::new();

        // Check for immediate closing bracket (empty attribute list)
        if let Some(AnnotatedToken {
            token: Token::CloseBracket,
            ..
        }) = self.lexer.peek()
        {
            self.lexer.next(); // Consume the closing bracket

            return Some(AttributeList {
                source_offset: open_bracket_token.start_offset,
                trivia: leading,
                attributes: attributes.into(),
            });
        }

        loop {
            let trivia = self.consume_trivia();

            // Parse attribute expression
            match self.parse_expression(trivia) {
                Some(expr) => attributes.push(expr.into()),
                None => {
                    let issue = SyntaxErr::ExpectedAttributeExpression {
                        pos: self.lexer.peek().map(|t| t.start().into()),
                    };
                    self.log.report(&issue);
                    break;
                }
            }

            // Expect ',' or ']'
            match self.lexer.next() {
                Some(AnnotatedToken {
                    token: Token::Comma,
                    ..
                }) => {
                    continue;
                }

                Some(AnnotatedToken {
                    token: Token::CloseBracket,
                    ..
                }) => break,

                Some(token) => {
                    let issue = SyntaxErr::ExpectedAttributeDelimiter {
                        pos: Some(token.start().into()),
                    };
                    self.log.report(&issue);
                    break;
                }

                None => {
                    let issue = SyntaxErr::ExpectedAttributeDelimiter { pos: None };
                    self.log.report(&issue);
                    break;
                }
            }
        }

        Some(AttributeList {
            source_offset: open_bracket_token.start_offset,
            trivia: leading,
            attributes: attributes.into(),
        })
    }

    fn parse_module(&mut self, leading: Option<Trivia>) -> Option<Item> {
        // Consume 'mod' token
        let mod_token = self.lexer.next()?;
        let attribute_trivia = self.consume_trivia();
        let attributes = self.parse_attribute_list(attribute_trivia);
        let trivia_1 = self.consume_trivia();

        // Expect module name
        let name: NString = match self.lexer.peek().cloned() {
            Some(AnnotatedToken {
                token: Token::Name(mod_name),
                ..
            }) => {
                self.lexer.next(); // Consume the name token
                mod_name.into()
            }

            Some(token) => {
                let issue = SyntaxErr::ModuleExpectedName {
                    pos: Some(token.start().into()),
                };
                self.log.report(&issue);
                NString::default()
            }

            None => {
                let issue = SyntaxErr::ModuleExpectedName { pos: None };
                self.log.report(&issue);
                NString::default()
            }
        };

        let trivia_2 = self.consume_trivia();

        // Expect '{'
        match self.lexer.next() {
            Some(AnnotatedToken {
                token: Token::OpenBrace,
                ..
            }) => {}

            Some(token) => {
                let issue = SyntaxErr::ExpectedOpenBrace {
                    pos: Some(token.start().into()),
                };
                self.log.report(&issue);
            }

            _ => {
                let issue = SyntaxErr::ExpectedOpenBrace { pos: None };
                self.log.report(&issue);
            }
        };

        // Parse module items
        let mut items = Vec::new();
        loop {
            let trivia = self.consume_trivia();
            if let Some(item) = self.parse_item(trivia) {
                items.push(item.into());
            } else {
                break;
            }
        }

        // Expect '}'
        match self.lexer.next() {
            Some(AnnotatedToken {
                token: Token::CloseBrace,
                ..
            }) => {}

            Some(token) => {
                let issue = SyntaxErr::ExpectedCloseBrace {
                    pos: Some(token.start().into()),
                };
                self.log.report(&issue);
            }

            _ => {
                let issue = SyntaxErr::ExpectedCloseBrace { pos: None };
                self.log.report(&issue);
            }
        };

        Some(Item::Module {
            source_offset: mod_token.start_offset,
            trivia: [leading, trivia_1, trivia_2],
            attributes: attributes.into(),
            name,
            items: items.into(),
        })
    }

    pub fn parse_item(&mut self, leading: Option<Trivia>) -> Option<Item> {
        match self.lexer.peek()?.token {
            Token::Mod => self.parse_module(leading),

            _ => {
                // TODO: Implement item parsing
                None
            }
        }
    }
}
