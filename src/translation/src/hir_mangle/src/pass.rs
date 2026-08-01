use crate::mangle::mangle_name;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use std::ops::Deref;

/// Applies name mangling to all functions and global variables in the symbol
/// table.
///
/// This pass sets the `mangled_name` field on every `Function` and
/// `GlobalVariable` in the symbol table. The `name` field is preserved as the
/// internal symbol lookup key, while `mangled_name` is what appears in the
/// object file during LLVM IR generation.
///
/// Symbols with the `NoMangle` attribute use the bare (unqualified) final
/// path segment as the mangled name, so that the symbol appears in the
/// object file exactly as the user wrote it (e.g. `main` for an entry point,
/// or `printf` for an extern C function).
pub fn mangle_symbols(package_name: &str, tab: &mut SymbolTab) {
    // Mangle all functions.
    let function_ids: Vec<FunctionId> = tab.functions().cloned().collect();
    for function_id in function_ids {
        let mut function = function_id.borrow_mut();
        if function.attributes.contains(&FunctionAttribute::NoMangle) {
            function.mangled_name = NString::from(last_segment(&function.name));
        } else {
            let ty = function.get_type();
            // HIR symbol names are fully qualified (e.g.
            // `test-package::add`). The package name is already encoded
            // separately by `mangle_name`, so strip it to avoid duplication.
            let name = strip_package(package_name, &function.name);
            let mangled = mangle_name(
                package_name,
                name,
                &Type::Function {
                    span: function.span,
                    function_type: ty.into(),
                },
            );
            function.mangled_name = NString::from(mangled);
        }
    }

    // Mangle all global variables.
    let global_ids: Vec<GlobalVariableId> = tab.globals().cloned().collect();
    for global_id in global_ids {
        let mut global = global_id.borrow_mut();
        if global.attributes.contains(&GlobalVariableAttribute::NoMangle) {
            global.mangled_name = NString::from(last_segment(&global.name));
        } else {
            let ty = global.ty.deref().clone();
            let name = strip_package(package_name, &global.name);
            let mangled = mangle_name(package_name, name, &ty);
            global.mangled_name = NString::from(mangled);
        }
    }
}

/// Strips the package qualifier from a fully-qualified HIR symbol name.
///
/// HIR symbol names are fully qualified (e.g. `test-package::add` or
/// `test-package::module::fn`). The package name is encoded separately by
/// `mangle_name`, so we strip the leading package qualifier to avoid
/// duplicating it in the mangled output.
fn strip_package<'a>(package_name: &str, name: &'a str) -> &'a str {
    name.strip_prefix(package_name)
        .and_then(|rest| rest.strip_prefix("::"))
        .unwrap_or(name)
}

/// Returns the final path segment of a potentially fully-qualified name.
fn last_segment(name: &str) -> &str {
    name.rsplit("::").next().unwrap_or(name)
}
