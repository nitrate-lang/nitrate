use crate::diagnosis::SyntaxErr;

use super::parse::Parser;
use nitrate_nstring::NString;
use nitrate_token::{AnnotatedToken, Token};
use nitrate_tree2::prelude::*;

impl Parser<'_, '_> {
    fn parse_attribute_list(&mut self) -> Vec<ExprId> {
        let attributes = Vec::new();

        // TODO: Implement attribute list parsing

        attributes
    }

    fn parse_module(&mut self, leading: Option<Trivia>) -> Item {
        // Consume 'mod' token
        let mod_token = self.lexer.next().expect("expected mod keyword");
        let trivia_1 = self.consume_trivia();
        let attributes = self.parse_attribute_list();
        let trivia_2 = self.consume_trivia();

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

        let trivia_3 = self.consume_trivia();

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

        Item::Module {
            source_offset: mod_token.start_offset,
            trivia: [leading, trivia_1, trivia_2, trivia_3],
            attributes: attributes.into(),
            name,
            items: items.into(),
        }
    }

    pub fn parse_item(&mut self, leading: Option<Trivia>) -> Option<Item> {
        match self.lexer.peek()?.token {
            Token::Mod => Some(self.parse_module(leading)),

            _ => {
                // TODO: Implement item parsing
                None
            }
        }
    }
}
