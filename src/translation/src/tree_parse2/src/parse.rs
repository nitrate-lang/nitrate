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
            match self.parse_item(trivia) {
                Some(item) => items.push(item.into()),
                None => break,
            }
        }

        Item::Root {
            items: items.into(),
        }
    }

    pub(crate) fn consume_trivia(&mut self) -> Option<Trivia> {
        let start_offset = self.lexer.peek().map(|t| t.start_offset)?;
        let mut found = false;

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
                    found = true;
                }
                _ => break,
            }
        }

        match found {
            false => None,
            true => Some(Trivia::new(start_offset)),
        }
    }
}
