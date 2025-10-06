use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{hir::QualifiedName, prelude::*};
use std::{collections::HashMap, num::NonZeroU32};

pub trait TryIntoHir {
    type Error;
    type Hir;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, Self::Error>;
}

pub struct HirCtx {
    storage: Store,
    symbol_table: HashMap<QualifiedName, SymbolId>,
    type_table: HashMap<QualifiedName, TypeId>,
    type_infer_id_ctr: NonZeroU32,
    ptr_size: PtrSize,
}

impl HirCtx {
    pub fn new(ptr_size: PtrSize) -> Self {
        Self {
            storage: Store::new(),
            symbol_table: HashMap::new(),
            type_table: HashMap::new(),
            type_infer_id_ctr: NonZeroU32::new(1).unwrap(),
            ptr_size,
        }
    }

    pub(crate) fn next_type_infer_id(&mut self) -> NonZeroU32 {
        let id = self.type_infer_id_ctr;
        self.type_infer_id_ctr = id.checked_add(1).expect("Type infer ID overflow");
        id
    }

    pub fn store(&self) -> &Store {
        &self.storage
    }

    pub fn store_mut(&mut self) -> &mut Store {
        &mut self.storage
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
    type Output = Item;

    fn index(&self, index: &ItemId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::IndexMut<&ItemId> for HirCtx {
    fn index_mut(&mut self, index: &ItemId) -> &mut Self::Output {
        &mut self.storage[index]
    }
}

impl std::ops::Index<&SymbolId> for HirCtx {
    type Output = Symbol;

    fn index(&self, index: &SymbolId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::IndexMut<&SymbolId> for HirCtx {
    fn index_mut(&mut self, index: &SymbolId) -> &mut Self::Output {
        &mut self.storage[index]
    }
}

impl std::ops::Index<&ValueId> for HirCtx {
    type Output = Value;

    fn index(&self, index: &ValueId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::IndexMut<&ValueId> for HirCtx {
    fn index_mut(&mut self, index: &ValueId) -> &mut Self::Output {
        &mut self.storage[index]
    }
}

impl std::ops::Index<&LiteralId> for HirCtx {
    type Output = Literal;

    fn index(&self, index: &LiteralId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::Index<&BlockId> for HirCtx {
    type Output = Block;

    fn index(&self, index: &BlockId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::IndexMut<&BlockId> for HirCtx {
    fn index_mut(&mut self, index: &BlockId) -> &mut Self::Output {
        &mut self.storage[index]
    }
}

impl std::ops::Index<&PlaceId> for HirCtx {
    type Output = Place;

    fn index(&self, index: &PlaceId) -> &Self::Output {
        &self.storage[index]
    }
}

impl std::ops::IndexMut<&PlaceId> for HirCtx {
    fn index_mut(&mut self, index: &PlaceId) -> &mut Self::Output {
        &mut self.storage[index]
    }
}
