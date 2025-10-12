use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{hir::QualifiedName, prelude::*};
use std::{cell::RefCell, collections::HashMap, num::NonZeroU32};

pub trait TryIntoHir {
    type Hir;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()>;
}

pub struct HirCtx {
    storage: Store,
    symbol_table: HashMap<QualifiedName, SymbolId>,
    type_table: HashMap<QualifiedName, TypeId>,
    type_infer_id_ctr: NonZeroU32,
    unique_name_ctr: u32,
    ptr_size: PtrSize,
}

impl HirCtx {
    pub fn new(ptr_size: PtrSize) -> Self {
        Self {
            storage: Store::new(),
            symbol_table: HashMap::new(),
            type_table: HashMap::new(),
            type_infer_id_ctr: NonZeroU32::new(1).unwrap(),
            unique_name_ctr: 0,
            ptr_size,
        }
    }

    pub(crate) fn next_type_infer_id(&mut self) -> NonZeroU32 {
        let id = self.type_infer_id_ctr;
        self.type_infer_id_ctr = id.checked_add(1).expect("Type infer ID overflow");
        id
    }

    pub(crate) fn next_unique_name(&mut self) -> EntityName {
        let name = format!("⚙️{}", self.unique_name_ctr);
        self.unique_name_ctr += 1;
        EntityName(name.into())
    }

    pub(crate) fn create_inference_placeholder(&mut self) -> Type {
        Type::Inferred {
            id: self.next_type_infer_id(),
        }
    }

    pub fn store(&self) -> &Store {
        &self.storage
    }

    pub fn ptr_size(&self) -> PtrSize {
        self.ptr_size
    }

    pub fn resolve_symbol(&self, name: &QualifiedName) -> Option<&SymbolId> {
        self.symbol_table.get(name)
    }

    pub fn resolve_type(&self, name: &QualifiedName) -> Option<&TypeId> {
        self.type_table.get(name)
    }
}

impl std::ops::Index<&TypeId> for HirCtx {
    type Output = Type;

    fn index(&self, index: &TypeId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&TypeListId> for HirCtx {
    type Output = TypeList;

    fn index(&self, index: &TypeListId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&StructFieldsId> for HirCtx {
    type Output = StructFields;

    fn index(&self, index: &StructFieldsId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&EnumVariantsId> for HirCtx {
    type Output = EnumVariants;

    fn index(&self, index: &EnumVariantsId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&StructAttributesId> for HirCtx {
    type Output = StructAttributes;

    fn index(&self, index: &StructAttributesId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&EnumAttributesId> for HirCtx {
    type Output = EnumAttributes;

    fn index(&self, index: &EnumAttributesId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&FuncAttributesId> for HirCtx {
    type Output = FunctionAttributes;

    fn index(&self, index: &FuncAttributesId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&ItemId> for HirCtx {
    type Output = RefCell<Item>;

    fn index(&self, index: &ItemId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&SymbolId> for HirCtx {
    type Output = RefCell<Symbol>;

    fn index(&self, index: &SymbolId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&ValueId> for HirCtx {
    type Output = RefCell<Value>;

    fn index(&self, index: &ValueId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&LiteralId> for HirCtx {
    type Output = Lit;

    fn index(&self, index: &LiteralId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&BlockId> for HirCtx {
    type Output = RefCell<Block>;

    fn index(&self, index: &BlockId) -> &Self::Output {
        &self.storage[index]
    }
}
