use crate::{string::mangle_string, ty::mangle_type};
use nitrate_hir::prelude::*;
use std::{format, unimplemented};

pub fn mangle_name(package_name: &str, name: &str, ty: &Type) -> String {
    format!(
        "_NIT_{}_{}_{}",
        mangle_string(package_name),
        mangle_string(name),
        mangle_type(ty)
    )
}

pub struct DemangledName {
    pub package: String,
    pub name: String,
    pub ty: Type,
}

pub fn demangle_name(mangled: &str) -> Result<DemangledName, ()> {
    unimplemented!(
        "demangling is not yet implemented, but the mangled name is: {}",
        mangled
    );
}
