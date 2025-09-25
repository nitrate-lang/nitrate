use nitrate_diagnosis::CompilerLog;
use nitrate_parsetree::kind::Item;
use nitrate_tokenize::Lexer;

pub struct Parser<'a, 'log> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) log: &'log CompilerLog,
    pub(crate) closure_ctr: u64,
}

impl<'a, 'log> Parser<'a, 'log> {
    pub fn new(lexer: Lexer<'a>, log: &'log CompilerLog) -> Self {
        Parser {
            lexer,
            log: log,
            closure_ctr: 0,
        }
    }

    pub fn parse_source(&mut self) -> Vec<Item> {
        let mut items = Vec::new();

        while !self.lexer.is_eof() {
            let item = self.parse_item();
            items.push(item);
        }

        items
    }
}
