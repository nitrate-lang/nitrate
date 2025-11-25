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
        let mod_token = self.lexer.next().unwrap();
        let trivia_1 = self.consume_trivia();
        let attributes = self.parse_attribute_list();
        let trivia_2 = self.consume_trivia();

        // Expect module name
        let name: NString = match self.lexer.next().unwrap() {
            AnnotatedToken {
                token: Token::Name(mod_name),
                ..
            } => mod_name.into(),

            token => {
                self.log.report(&SyntaxErr::ModuleExpectedName {
                    pos: token.start().into(),
                });
                NString::default()
            }
        };

        let trivia_3 = self.consume_trivia();

        // Expect '{'
        match self.lexer.next() {
            Some(token) if token.token == Token::OpenBrace => {}
            _ => self.log.report(&SyntaxErr::ExpectedOpenBrace {
                pos: mod_token.end().into(),
            }),
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
            Some(token) if token.token == Token::CloseBrace => {}
            _ => self.log.report(&SyntaxErr::ExpectedCloseBrace {
                pos: mod_token.end().into(),
            }),
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
