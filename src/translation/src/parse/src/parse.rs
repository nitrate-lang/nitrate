use nitrate_diagnosis::DiagnosticCollector;
use nitrate_parsetree::kind::{Module, Package, Visibility};
use nitrate_tokenize::Lexer;

pub struct Parser<'a, 'bugs> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) bugs: &'bugs DiagnosticCollector,
}

impl<'a, 'bugs> Parser<'a, 'bugs> {
    pub fn new(lexer: Lexer<'a>, bugs: &'bugs DiagnosticCollector) -> Self {
        Parser { lexer, bugs }
    }

    pub fn parse_crate(&mut self, crate_name: String) -> Package {
        let mut items = Vec::new();

        while !self.lexer.is_eof() {
            let item = self.parse_item();
            items.push(item);
        }

        Package {
            name: crate_name,
            root: Module {
                visibility: Some(Visibility::Public),
                attributes: Vec::new(),
                name: "".into(),
                items,
            },
        }
    }
}
