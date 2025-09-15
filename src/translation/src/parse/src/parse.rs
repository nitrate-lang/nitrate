use interned_string::IString;
use nitrate_diagnosis::DiagnosticCollector;
use nitrate_parsetree::kind::{Module, Package};
use nitrate_tokenize::Lexer;

use crate::bugs::SyntaxBug;

pub struct Parser<'a, 'bugs> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) bugs: &'bugs DiagnosticCollector,
}

impl<'a, 'bugs> Parser<'a, 'bugs> {
    pub fn new(lexer: Lexer<'a>, bugs: &'bugs DiagnosticCollector) -> Self {
        Parser { lexer, bugs }
    }

    pub fn parse_crate(&mut self, crate_name: IString) -> Package {
        let mut items = Vec::new();

        while !self.lexer.is_eof() {
            if self.lexer.next_if_comment().is_some() {
                continue;
            }

            let item = self.parse_item();
            items.push(item);
        }

        Package {
            name: crate_name,
            root: Module {
                attributes: Vec::new(),
                name: "".into(),
                items,
            },
        }
    }
}
