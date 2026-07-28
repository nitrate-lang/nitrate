use crate::ty::mangle_type;
use nitrate_hir::prelude::*;

pub fn mangle_function_name(id: &FunctionId) -> String {
    let func = id.borrow();
    format!("_N{}", mangle_type(&func.return_type))
}

pub fn mangle_global_name(id: &GlobalVariableId) -> String {
    let glb = id.borrow();
    format!("_G{}", glb.name)
}
