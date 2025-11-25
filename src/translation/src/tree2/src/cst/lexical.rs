use crate::get_source;
use nitrate_token::{SourcePosition, Token};
use nitrate_token_lexer::{Lexer, LexerIterator};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Trivia {
    offset: u32,
    len: u32,
}

impl Default for Trivia {
    fn default() -> Self {
        Self { offset: 0, len: 0 }
    }
}

impl Trivia {
    pub fn new(offset: u32, len: u32) -> Self {
        Self { offset, len }
    }

    pub fn tokens(&self) -> Vec<Token> {
        get_source(|g| {
            let source = g.full_source;
            let fileid = g.fileid.clone();
            let mut lexer = Lexer::new(source, fileid.clone()).expect("failed to create lexer");

            lexer.rewind(SourcePosition {
                line: 0,
                column: 0,
                offset: self.offset,
                fileid,
            });

            LexerIterator::new(lexer)
                .take_while(|token| token.end_offset <= self.offset + self.len)
                .map(|annotated_token| annotated_token.token)
                .collect()
        })
    }
}

#[derive(Serialize, Deserialize)]
struct TriviaSerHelper {
    offset: u32,
    len: u32,
    tokens: Vec<Token>,
}

impl Serialize for Trivia {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        let tokens = self.tokens();
        let helper = TriviaSerHelper {
            offset: self.offset,
            len: self.len,
            tokens,
        };

        helper.serialize(serializer)
    }
}

impl<'de> Deserialize<'de> for Trivia {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let helper = TriviaSerHelper::deserialize(deserializer)?;
        Ok(Trivia::new(helper.offset, helper.len))
    }
}
