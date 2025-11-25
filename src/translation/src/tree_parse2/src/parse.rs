use nitrate_diagnosis::CompilerLog;
use nitrate_token::{AnnotatedToken, Token};
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
            match self.parse_item() {
                Some(item) => items.push(item.into()),
                None => break,
            }
        }

        Item::Root {
            items: items.into(),
        }
    }

    pub(crate) fn consume_trivia(&mut self) -> Trivia {
        let start_offset = match self.lexer.peek().map(|t| t.start_offset) {
            Some(offset) => offset,
            None => return Trivia::default(),
        };

        let mut end_offset = start_offset;

        while let Some(token) = self.lexer.peek() {
            match token {
                AnnotatedToken {
                    token:
                        Token::Comment(_)
                        | Token::HorizontalTab
                        | Token::NewLine
                        | Token::VerticalTab
                        | Token::FormFeed
                        | Token::CarriageReturn
                        | Token::Space,
                    ..
                } => {
                    end_offset = token.end_offset;
                    self.lexer.next();
                }

                _ => break,
            }
        }

        Trivia::new(start_offset, end_offset - start_offset)
    }

    pub(crate) fn consume_while(&mut self, f: impl Fn(&AnnotatedToken) -> bool) -> Trivia {
        let start_offset = match self.lexer.peek().map(|t| t.start_offset) {
            Some(offset) => offset,
            None => return Trivia::default(),
        };

        let mut end_offset = start_offset;

        while let Some(token) = self.lexer.peek() {
            if f(token) {
                end_offset = token.end_offset;
                self.lexer.next();
            } else {
                break;
            }
        }

        Trivia::new(start_offset, end_offset - start_offset)
    }
}
