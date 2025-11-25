use std::num::NonZeroU32;

use crate::get_source;
use nitrate_diagnosis::FileId;
use nitrate_token::{AnnotatedToken, SourcePosition, Token};
use nitrate_token_lexer::{Lexer, LexerIterator};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Trivia {
    pub position: NonZeroU32,
}

impl Trivia {
    pub fn new(start_offset: u32) -> Self {
        Self {
            position: match start_offset {
                0 => NonZeroU32::MAX,
                offset => NonZeroU32::new(offset).unwrap(),
            },
        }
    }

    pub fn tokens_iter<'a>(
        &self,
        full_source: &'a str,
        fileid: Option<FileId>,
    ) -> impl Iterator<Item = Token> {
        let mut lexer =
            Lexer::new(full_source.as_bytes(), fileid.clone()).expect("failed to create lexer");

        lexer.rewind(SourcePosition {
            line: 0,
            column: 0,
            offset: match self.position.get() {
                u32::MAX => 0,
                offset => offset,
            },
            fileid,
        });

        LexerIterator::new(lexer)
            .map(|annotated_token: AnnotatedToken| annotated_token.token)
            .take_while(|token: &Token| match token {
                Token::Comment(_) => true,
                _ => false,
            })
    }
}

#[derive(Serialize, Deserialize)]
struct TriviaSerHelper {
    offset: u32,
    tokens: Vec<Token>,
}

impl Serialize for Trivia {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: serde::Serializer,
    {
        get_source(|g| {
            let tokens = self
                .tokens_iter(g.full_source, g.fileid.clone())
                .collect::<Vec<_>>();

            let helper = TriviaSerHelper {
                offset: self.position.get(),
                tokens,
            };

            helper.serialize(serializer)
        })
    }
}

impl<'de> Deserialize<'de> for Trivia {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let helper = TriviaSerHelper::deserialize(deserializer)?;
        Ok(Trivia::new(helper.offset))
    }
}
