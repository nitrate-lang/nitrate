use crate::Gen;
use nitrate_translation::parsetree::ast::{self, ItemKind};
use std::todo;

impl Gen {
    fn select_item_kind(&mut self) -> ast::ItemKind {
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
        todo!()
    }

    fn gen_item_import(&mut self) -> ast::Item {
        todo!()
    }

    fn gen_item_type_alias(&mut self) -> ast::Item {
        todo!()
    }

    fn gen_item_struct(&mut self) -> ast::Item {
        todo!()
    }

    fn gen_item_enum(&mut self) -> ast::Item {
        todo!()
    }

    fn gen_item_trait(&mut self) -> ast::Item {
        todo!()
    }

    fn gen_item_impl(&mut self) -> ast::Item {
        todo!()
    }

    fn gen_item_function(&mut self, argc: Option<u32>) -> ast::Item {
        // if argc is set the function must have that many arguments, argument counts are random
        todo!()
    }

    fn gen_item_variable(&mut self) -> ast::Item {
        todo!()
    }
}
