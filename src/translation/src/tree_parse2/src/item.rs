use super::parse::Parser;
use crate::diagnosis::SyntaxErr;
use nitrate_nstring::NString;
use nitrate_token::{AnnotatedToken, Token};
use nitrate_tree2::prelude::*;

impl Parser<'_, '_> {
    fn parse_attribute_list(&mut self, leading: Trivia) -> Option<AttributeList> {
        let mut present = AttributeListPresent::empty();

        // Consume '[', or return None if not present
        let open_bracket_token = match self.lexer.peek()? {
            AnnotatedToken {
                token: Token::OpenBracket,
                ..
            } => {
                present.insert(AttributeListPresent::OPEN_BRACKET_PRESENT);
                self.lexer.next().unwrap() // Consume the opening bracket
            }

            _ => return None, // No attribute list found
        };

        let mut attributes = Vec::new();

        loop {
            // Check for ']'
            if let Some(AnnotatedToken {
                token: Token::CloseBracket,
                ..
            }) = self.lexer.peek()
            {
                self.lexer.next(); // Consume the closing bracket
                present.insert(AttributeListPresent::CLOSE_BRACKET_PRESENT);

                return Some(AttributeList {
                    source_offset: open_bracket_token.start_offset,
                    present,
                    trivia: [leading],
                    attributes: attributes.into(),
                });
            }

            present.remove(AttributeListPresent::TRAILING_COMMA_PRESENT);

            // Parse attribute expression
            let expr = self.parse_expression();
            attributes.push(expr.into());

            // Expect ',' or ']'
            match self.lexer.next() {
                Some(AnnotatedToken {
                    token: Token::Comma,
                    ..
                }) => {
                    present.insert(AttributeListPresent::TRAILING_COMMA_PRESENT);
                    continue;
                }

                Some(AnnotatedToken {
                    token: Token::CloseBracket,
                    ..
                }) => {
                    present.insert(AttributeListPresent::CLOSE_BRACKET_PRESENT);
                    break;
                }

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
            present,
            trivia: [leading],
            attributes: attributes.into(),
        })
    }

    fn parse_module(&mut self, leading: Trivia) -> Item {
        // Consume 'mod' token
        let mod_token = self.lexer.next().expect("mod keyword");
        assert!(matches!(mod_token.token, Token::Mod));

        // Parse optional attribute list
        let attribute_list_trivia =
            self.consume_trivia_while(|t| !matches!(t.token, Token::OpenBracket | Token::Name(_)));

        let attribute_list = self.parse_attribute_list(attribute_list_trivia);

        let name_prefix_trivia = match attribute_list {
            Some(_) => self.consume_trivia_while(|t| !matches!(t.token, Token::Name(_))),
            // No attribute list, keep previous trivia
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

        let trivia_2 = self.consume_trivia_while(|t| !matches!(t.token, Token::OpenBrace));
        let mut present = ItemModulePresent::empty();

        // Expect '{'
        match self.lexer.peek() {
            Some(AnnotatedToken {
                token: Token::OpenBrace,
                ..
            }) => {
                self.lexer.next(); // Consume '{'
                present.insert(ItemModulePresent::OPEN_BRACE_PRESENT);
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

        while let Some(_) = self.lexer.peek() {
            // Check for '}'
            if let Some(AnnotatedToken {
                token: Token::CloseBrace,
                ..
            }) = self.lexer.peek()
            {
                self.lexer.next(); // Consume '}'
                present.insert(ItemModulePresent::CLOSE_BRACE_PRESENT);
                break;
            }

            let item = self.parse_item();
            items.push(item.into());
        }

        Item::Module {
            source_offset: mod_token.start_offset,
            present,
            trivia: [leading, name_prefix_trivia, trivia_2],
            attributes: attribute_list.into(),
            name,
            items: items.into(),
        }
    }

    pub fn parse_item(&mut self) -> Item {
        let leading_trivia = self.consume_trivia_while(|t| !matches!(t.token, Token::Mod));

        match self.lexer.peek() {
            Some(AnnotatedToken {
                token: Token::Mod, ..
            }) => self.parse_module(leading_trivia),

            Some(token) => {
                let issue = SyntaxErr::ExpectedItem {
                    pos: Some(token.start().into()),
                };
                self.log.report(&issue);

                Item::Garbage {
                    trivia: leading_trivia,
                }
            }

            None => {
                let issue = SyntaxErr::ExpectedItem { pos: None };
                self.log.report(&issue);

                Item::Garbage {
                    trivia: leading_trivia,
                }
            }
        }
    }
}
