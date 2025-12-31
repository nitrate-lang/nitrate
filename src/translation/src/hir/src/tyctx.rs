use nitrate_nstring::NString;

use crate::{FunctionId, PtrSize, TypeId};
use std::collections::{HashMap, HashSet};

#[derive(Debug)]
pub struct TyCtx {
    impls: HashMap<TypeId, HashSet<FunctionId>>,
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

    pub fn add_method_impl(&mut self, ty: TypeId, func_id: FunctionId) {
        self.impls.entry(ty).or_insert_with(HashSet::new).insert(func_id);
    }

    pub fn get_method_impl(&self, ty: &TypeId, method: &NString) -> Option<FunctionId> {
        self.impls
            .get(ty)?
            .iter()
            .find(|func_id| func_id.borrow().name == *method)
            .map(|func_id| func_id.to_owned())
    }
}
