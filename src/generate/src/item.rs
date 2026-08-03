use crate::Gen;
use nitrate_translation::parsetree::{
    SrcSpan,
    ast::{self, ItemSyntaxError},
};

impl Gen {
    pub(crate) fn gen_item(&mut self, argc: Option<u32>) -> ast::Item {
        ast::Item::SyntaxError(ItemSyntaxError {
            span: SrcSpan::default(),
        })
    }
}
