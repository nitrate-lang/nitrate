use nitrate_diagnosis::CompilerLog;
use nitrate_nstring::NString;
use nitrate_token_lexer::Lexer;
use nitrate_tree::ast::Module;
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

    pub fn parse_source(&mut self, package_name: NString) -> Module {
        let mut items = Vec::new();

        while !self.lexer.is_eof() {
            let item = self.parse_item();
            items.push(item);
        }

        Module {
            name: package_name.clone(),
            visibility: None,
            items,
            attributes: None,
        }
    }
}
