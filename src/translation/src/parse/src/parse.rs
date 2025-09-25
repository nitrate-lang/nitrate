use nitrate_diagnosis::CompilerLog;
use nitrate_parsetree::kind::Item;
use nitrate_tokenize::Lexer;

pub struct Parser<'a, 'bugs> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) log: &'bugs CompilerLog,
    pub(crate) closure_ctr: u64,
}

impl<'a, 'bugs> Parser<'a, 'bugs> {
    pub fn new(lexer: Lexer<'a>, bugs: &'bugs CompilerLog) -> Self {
        Parser {
            lexer,
            log: bugs,
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
