use crate::{
    string::{demangle_string, mangle_string},
    ty::{demangle_type, mangle_type},
};
use nitrate_hir::prelude::*;

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
    let mut buf = [0u8; 1];

    let Some(mangled) = mangled.strip_prefix("_NIT_") else {
        return Err(());
    };

    let read: &mut dyn std::io::Read = &mut mangled.as_bytes();

    let package = demangle_string(read)?;

    if read.read_exact(&mut buf).is_err() || buf[0] != b'_' {
        return Err(());
    }

    let name = demangle_string(read)?;

    if read.read_exact(&mut buf).is_err() || buf[0] != b'_' {
        return Err(());
    }

    let ty = demangle_type(read)?;

    Ok(DemangledName { package, name, ty })
}
