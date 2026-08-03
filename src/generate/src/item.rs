use crate::Gen;
use nitrate_translation::parsetree::ast::{self, ItemKind};
use std::todo;

impl Gen {
    fn select_item_kind(&mut self) -> ast::ItemKind {
        // TODO: select a random item kind weighted by desired frequency distribution
        todo!()
    }

    pub(crate) fn gen_item(&mut self, argc: Option<u32>) -> ast::Item {
        let kind = match argc {
            Some(_) => ast::ItemKind::Function,
            None => self.select_item_kind(),
        };

        match kind {
            ItemKind::SyntaxError => unreachable!("select_item_kind should not return SyntaxError"),
            ItemKind::Module => self.gen_item_module(),
            ItemKind::Import => self.gen_item_import(),
            ItemKind::TypeAlias => self.gen_item_type_alias(),
            ItemKind::Struct => self.gen_item_struct(),
            ItemKind::Enum => self.gen_item_enum(),
            ItemKind::Trait => self.gen_item_trait(),
            ItemKind::Impl => self.gen_item_impl(),
            ItemKind::Function => self.gen_item_function(argc),
            ItemKind::Variable => self.gen_item_variable(),
        }
    }

    fn gen_item_module(&mut self) -> ast::Item {
        // TODO: generate a module item with nested items
        todo!()
    }

    fn gen_item_import(&mut self) -> ast::Item {
        // TODO: generate an import declaration item
        todo!()
    }

    fn gen_item_type_alias(&mut self) -> ast::Item {
        // TODO: generate a type alias declaration
        todo!()
    }

    fn gen_item_struct(&mut self) -> ast::Item {
        // TODO: generate a struct definition with random fields
        todo!()
    }

    fn gen_item_enum(&mut self) -> ast::Item {
        // TODO: generate an enum definition with random variants
        todo!()
    }

    fn gen_item_trait(&mut self) -> ast::Item {
        // TODO: generate a trait definition with associated items
        todo!()
    }

    fn gen_item_impl(&mut self) -> ast::Item {
        // TODO: generate an impl block with method implementations
        todo!()
    }

    fn gen_item_function(&mut self, argc: Option<u32>) -> ast::Item {
        // if argc is set the function must have that many arguments, argument counts are random
        // TODO: generate a function definition with body, parameters, and return type
        todo!()
    }

    fn gen_item_variable(&mut self) -> ast::Item {
        // TODO: generate a global variable declaration with optional initializer
        todo!()
    }
}
