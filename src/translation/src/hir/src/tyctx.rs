use nitrate_nstring::NString;

use crate::{FunctionId, PtrSize, TypeId};
use std::collections::HashMap;

#[derive(Debug)]
pub struct TyCtx {
    impls: HashMap<TypeId, HashMap<NString, FunctionId>>,
    ptr_size: PtrSize,
}

impl TyCtx {
    pub fn new(ptr_size: PtrSize) -> Self {
        Self {
            impls: HashMap::new(),
            ptr_size,
        }
    }

    pub fn ptr_size(&self) -> PtrSize {
        self.ptr_size
    }

    pub fn add_method_impl(&mut self, ty: TypeId, method: NString, func_id: FunctionId) {
        self.impls
            .entry(ty)
            .or_insert_with(HashMap::new)
            .insert(method, func_id);
    }

    pub fn get_method_impl(&self, ty: &TypeId, method: &NString) -> Option<&FunctionId> {
        self.impls.get(ty)?.get(method)
    }
}
