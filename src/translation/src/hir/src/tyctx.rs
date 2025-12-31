use nitrate_nstring::NString;

use crate::{FunctionId, PtrSize, TypeId};
use std::collections::{HashMap, HashSet};

#[derive(Debug)]
pub struct TyCtx {
    impls: HashMap<TypeId, HashSet<()>>,
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

    pub fn get_method_impl(&self, _ty: &TypeId, _method: &NString) -> Option<FunctionId> {
        // TODO: Implement method lookup
        None
    }
}
