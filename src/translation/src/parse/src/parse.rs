use interned_string::IString;
use nitrate_diagnosis::DiagnosticCollector;
use nitrate_parsetree::kind::{Module, Package};
use nitrate_tokenize::Lexer;

use crate::bugs::SyntaxError;

pub struct Parser<'a, 'bugs> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) generic_type_depth: i64,
    pub(crate) generic_type_suffix_terminator_ambiguity: bool,
    pub(crate) bugs: &'bugs DiagnosticCollector,
}

impl<'a, 'bugs> Parser<'a, 'bugs> {
    pub fn new(lexer: Lexer<'a>, bugs: &'bugs DiagnosticCollector) -> Self {
        Parser {
            lexer,
            generic_type_depth: 0,
            generic_type_suffix_terminator_ambiguity: false,
            bugs,
        }
    }

    pub(crate) fn set_failed_bit(&mut self) {
        // TODO:

        self.bugs.emit(&SyntaxError::Test);
    }

    pub fn parse_crate(&mut self, crate_name: IString) -> Package {
        let mut items = Vec::new();

        while !self.lexer.is_eof() {
            if self.lexer.next_if_comment().is_some() {
                continue;
            }

            let Some(item) = self.parse_item() else {
                self.set_failed_bit();
                break;
            };

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
