use nitrate_hir::{SymbolTab, prelude::*};
use nitrate_nstring::NString;
use nitrate_tree::ast::SymbolKind;
use nitrate_tree_resolve::ImportContext;
use std::collections::{HashMap, HashSet};
use std::num::NonZeroU32;

#[derive(Debug)]
pub struct Ast2HirCtx {
    pub tab: SymbolTab,

    pub(crate) ast_symbol_map: HashMap<NString, SymbolKind>,
    pub(crate) current_scope: Vec<NString>,
    pub(crate) ptr_size: PtrSize,
    pub(crate) import_ctx: ImportContext,

    _impl_map: HashMap<TypeId, HashSet<TraitId>>,
    type_infer_id_ctr: NonZeroU32,
    unique_name_ctr: u32,
}

impl Ast2HirCtx {
    pub fn new(ptr_size: PtrSize, import_ctx: ImportContext) -> Self {
        Self {
            tab: SymbolTab::new(ptr_size),
            ast_symbol_map: HashMap::new(),
            current_scope: Vec::new(),
            _impl_map: HashMap::new(),
            type_infer_id_ctr: NonZeroU32::new(1).unwrap(),
            unique_name_ctr: 0,
            ptr_size,
            import_ctx,
        }
    }

    pub fn get_unique_name(&mut self) -> String {
        const COMPILER_RESERVED_PREFIX: &str = "⚙️";

        let name = format!("{}{}", COMPILER_RESERVED_PREFIX, self.unique_name_ctr);
        self.unique_name_ctr += 1;
        name
    }

    pub(crate) fn create_inference_placeholder(&mut self) -> Type {
        let id = self.type_infer_id_ctr;
        self.type_infer_id_ctr = id.checked_add(1).expect("Type infer ID overflow");
        Type::Inferred { id }
    }

    pub(crate) fn qualify_name(&self, item_name: &str) -> String {
        let length = self.current_scope.iter().map(|s| s.len() + 2).sum::<usize>() + item_name.len();
        let mut qualified = String::with_capacity(length);

        for module in &self.current_scope {
            qualified.push_str(module);
            qualified.push_str("::");
        }

        qualified.push_str(item_name);
        qualified
    }
}
