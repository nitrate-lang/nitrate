use nitrate_diagnosis::DiagnosticCollector;
use nitrate_parsetree::{
    kind::{Module, Package, Visibility},
    tag::{intern_module_name_id, intern_package_name_id},
};
use nitrate_tokenize::Lexer;

pub struct Parser<'a, 'bugs> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) bugs: &'bugs DiagnosticCollector,
}

impl<'a, 'bugs> Parser<'a, 'bugs> {
    pub fn new(lexer: Lexer<'a>, bugs: &'bugs DiagnosticCollector) -> Self {
        Parser { lexer, bugs }
    }

    pub fn parse_crate(&mut self, package_name: &str) -> Package {
        let mut items = Vec::new();

        while !self.lexer.is_eof() {
            let item = self.parse_item();
            items.push(item);
        }

        let package_name = intern_package_name_id(package_name.to_string());
        let module_name = intern_module_name_id("".into());

        Package {
            name: package_name,
            root: Module {
                visibility: Some(Visibility::Public),
                attributes: None,
                name: module_name,
                items,
            },
        }
    }
}
