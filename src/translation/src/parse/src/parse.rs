use interned_string::IString;
use nitrate_parsetree::kind::{Module, Package};
use nitrate_tokenize::Lexer;

pub struct Parser<'a> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) generic_type_depth: i64,
    pub(crate) generic_type_suffix_terminator_ambiguity: bool,
    failed_bit: bool,
}

impl<'a> Parser<'a> {
    pub fn new(lexer: Lexer<'a>) -> Self {
        Parser {
            lexer,
            generic_type_depth: 0,
            generic_type_suffix_terminator_ambiguity: false,
            failed_bit: false,
        }
    }

    pub(crate) fn set_failed_bit(&mut self) {
        self.failed_bit = true;
    }

    pub fn parse_crate(&mut self, crate_name: IString) -> Result<Package, Package> {
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

        let crate_item = Package {
            name: crate_name,
            root: Module {
                attributes: Vec::new(),
                name: "".into(),
                items,
            },
        };

        if self.failed_bit {
            Err(crate_item)
        } else {
            Ok(crate_item)
        }
    }
}
