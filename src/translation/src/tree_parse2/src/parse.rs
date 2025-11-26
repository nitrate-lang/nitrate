use nitrate_diagnosis::CompilerLog;
use nitrate_token::Token;
use nitrate_token_lexer::{Lexer, LexerIterator};
use nitrate_tree2::prelude::*;
use std::iter::Peekable;

pub struct Parser<'a, 'log> {
    pub(crate) lexer: Peekable<LexerIterator<'a>>,
    pub(crate) log: &'log CompilerLog,
}

impl<'a, 'log> Parser<'a, 'log> {
    pub fn new(lexer: Lexer<'a>, log: &'log CompilerLog) -> Self {
        Parser {
            lexer: LexerIterator::new(lexer).peekable(),
            log: log,
        }
    }

    pub fn parse_source(&mut self) -> Item {
        let mut items = Vec::new();

        loop {
            let trivia = self.consume_trivia();
            if !self.anymore_tokens() {
                let item = Item::Trivia { trivia };
                items.push(item.into());
                break;
            }

            let item = self.parse_item(trivia);
            items.push(item.into());
        }

        Item::Root {
            items: items.into(),
        }
    }

    pub(crate) fn consume_trivia(&mut self) -> Option<Trivia> {
        let start_offset = self.lexer.peek()?.start_offset;

        while let Some(token) = self.lexer.peek() {
            match token.token {
                Token::Comment(_)
                | Token::HorizontalTab
                | Token::NewLine
                | Token::VerticalTab
                | Token::FormFeed
                | Token::CarriageReturn
                | Token::Space => {
                    self.lexer.next();
                }

                _ => break,
            }
        }

        Some(Trivia::new(start_offset))
    }

    pub(crate) fn anymore_tokens(&mut self) -> bool {
        self.lexer.peek().is_some()
    }
}
