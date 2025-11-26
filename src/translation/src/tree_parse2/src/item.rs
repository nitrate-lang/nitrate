use super::parse::Parser;
use crate::diagnosis::SyntaxErr;
use nitrate_nstring::NString;
use nitrate_token::{AnnotatedToken, Token};
use nitrate_tree2::prelude::*;

impl Parser<'_, '_> {
    fn parse_attribute_list(&mut self, leading: Option<Trivia>) -> Option<AttributeList> {
        // Consume '[', or return None if not present
        let open_bracket_token = match self.lexer.peek()? {
            AnnotatedToken {
                token: Token::OpenBracket,
                ..
            } => {
                self.lexer.next().unwrap() // Consume the opening bracket
            }

            _ => return None, // No attribute list found
        };

        let mut attributes = Vec::new();
        let mut flags = AttributeListFlags::empty();

        loop {
            // Check for ']'
            if let Some(AnnotatedToken {
                token: Token::CloseBracket,
                ..
            }) = self.lexer.peek()
            {
                self.lexer.next(); // Consume the closing bracket

                return Some(AttributeList {
                    source_offset: open_bracket_token.start_offset,
                    flags,
                    trivia: [leading],
                    attributes: attributes.into(),
                });
            }

            flags.remove(AttributeListFlags::TRAILING_COMMA_PRESENT);

            // Parse attribute expression
            let expr = self.parse_expression();
            attributes.push(expr.into());

            // Expect ',' or ']'
            match self.lexer.next() {
                Some(AnnotatedToken {
                    token: Token::Comma,
                    ..
                }) => {
                    flags.insert(AttributeListFlags::TRAILING_COMMA_PRESENT);
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
            flags,
            trivia: [leading],
            attributes: attributes.into(),
        })
    }

    fn parse_module(&mut self, leading: Option<Trivia>) -> Item {
        // Consume 'mod' token
        let mod_token = self.lexer.next().expect("mod keyword");
        assert!(matches!(mod_token.token, Token::Mod));

        // Parse optional attribute list
        let attribute_list_trivia = self.consume_trivia();
        let attribute_list = self.parse_attribute_list(attribute_list_trivia);

        let name_leading_trivia = match attribute_list {
            Some(_) => self.consume_trivia(),
            None => attribute_list_trivia,
        };

        // Parse module name
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

        let open_brace_leading_trivia = self.consume_trivia();

        // Expect '{'
        match self.lexer.peek() {
            Some(AnnotatedToken {
                token: Token::OpenBrace,
                ..
            }) => {
                self.lexer.next(); // Consume '{'
            }

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

        while self.anymore_tokens() {
            let trivia = self.consume_trivia();

            // Check for '}'
            if let Some(AnnotatedToken {
                token: Token::CloseBrace,
                ..
            }) = self.lexer.peek()
            {
                let item = Item::Trivia { trivia };
                items.push(item.into());

                self.lexer.next(); // Consume '}'
                break;
            }

            let item = self.parse_item(trivia);
            items.push(item.into());
        }

        Item::Module {
            source_offset: mod_token.start_offset,
            trivia: [leading, name_leading_trivia, open_brace_leading_trivia],
            attributes: attribute_list.into(),
            name,
            items: items.into(),
        }
    }

    pub fn parse_item(&mut self, leading: Option<Trivia>) -> Item {
        match self.lexer.peek().cloned() {
            Some(AnnotatedToken {
                token: Token::Mod, ..
            }) => self.parse_module(leading),

            Some(AnnotatedToken {
                token: Token::Pub | Token::Pro | Token::Sec,
                ..
            }) => {
                let visibility_token = self.lexer.next().unwrap(); // Consume visibility token
                let vis = match visibility_token.token {
                    Token::Pub => Vis::Pub,
                    Token::Pro => Vis::Pro,
                    Token::Sec => Vis::Sec,
                    _ => unreachable!(),
                };

                let new_leading = self.consume_trivia();
                let item = self.parse_item(new_leading);

                Item::Visibility {
                    source_offset: visibility_token.start_offset,
                    trivia: [leading],
                    vis,
                    item: item.into(),
                }
            }

            Some(token) => {
                self.lexer.next(); // Consume unexpected token

                let issue = SyntaxErr::ExpectedItem {
                    pos: Some(token.start().into()),
                };
                self.log.report(&issue);

                Item::Trivia { trivia: leading }
            }

            None => {
                let issue = SyntaxErr::ExpectedItem { pos: None };
                self.log.report(&issue);

                Item::Trivia { trivia: leading }
            }
        }
    }
}
