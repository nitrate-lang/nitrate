use nitrate_diagnosis::CompilerLog;
use nitrate_token_lexer::Lexer;
use nitrate_tree2::prelude::*;
use std::path::PathBuf;

pub struct Parser<'a, 'log> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) log: &'log CompilerLog,
}

pub struct ResolveCtx {
    pub package_search_paths: Vec<PathBuf>,
}

impl<'a, 'log> Parser<'a, 'log> {
    pub fn new(lexer: Lexer<'a>, log: &'log CompilerLog) -> Self {
        Parser { lexer, log: log }
    }

    pub fn parse_source(&mut self) -> Item {
        let mut items = Vec::new();

        while !self.lexer.is_eof() {
            let item = self.parse_item();
            items.push(item);
        }

        Item::Root {
            items: items.into(),
        }
    }
}
