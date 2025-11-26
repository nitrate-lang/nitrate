use nitrate_diagnosis::CompilerLog;
use nitrate_token::{AnnotatedToken, Token};
use nitrate_token_lexer::{Lexer, LexerIterator};
use nitrate_tree2::prelude::*;
use std::iter::Peekable;

use crate::diagnosis::SyntaxErr;

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

        while let Some(_) = self.lexer.peek() {
            let item = self.parse_item();
            items.push(item.into());
        }

        Item::Root {
            items: items.into(),
        }
    }

    pub(crate) fn consume_trivia_while(&mut self, f: impl Fn(&AnnotatedToken) -> bool) -> Trivia {
        let start_offset = match self.lexer.peek().map(|t| t.start_offset) {
            Some(offset) => offset,
            None => return Trivia::default(),
        };

        let mut end_offset = start_offset;

        while let Some(token) = self.lexer.peek().cloned() {
            if f(&token) {
                let is_trivia = matches!(
                    token.token,
                    Token::Comment(_)
                        | Token::HorizontalTab
                        | Token::NewLine
                        | Token::VerticalTab
                        | Token::FormFeed
                        | Token::CarriageReturn
                        | Token::Space
                );

                if !is_trivia {
                    let issue = SyntaxErr::UnexpectedToken {
                        pos: token.start().into(),
                        token: token.token,
                    };
                    self.log.report(&issue);
                }

                end_offset = token.end_offset;
                self.lexer.next();
            } else {
                break;
            }
        }

        Trivia::new(start_offset, end_offset - start_offset)
    }
}
