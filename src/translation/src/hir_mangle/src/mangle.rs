use crate::string::{demangle_string, mangle_string};
use crate::ty::{demangle_type, mangle_type};
use nitrate_hir::prelude::*;

/// The prefix used for all Nitrate-mangled symbols.
const MANGLE_PREFIX: &str = "_N";

/// Mangles a symbol name.
///
/// The mangled name has the form:
///
/// ```text
/// _N<package><name><type>
/// ```
///
/// where:
/// - `<package>` is the package name, mangled as a string
/// - `<name>` is the symbol's name, mangled as a string
/// - `<type>` is the type encoding (for functions, this includes the full
///   function signature; for globals, it's the global's type)
///
/// All output characters are from the C99 identifier charset `[A-Za-z0-9_]`.
pub fn mangle_name(package_name: &str, name: &str, ty: &Type) -> String {
    let mut result = String::with_capacity(package_name.len() + name.len() + 16);
    result.push_str(MANGLE_PREFIX);
    result.push_str(&mangle_string(package_name));
    result.push_str(&mangle_string(name));
    result.push_str(&mangle_type(ty));
    result
}

/// Demangles a symbol name back into its original form.
///
/// Returns `(package_name, name, type)`.
pub fn demangle_name(mangled: &str) -> Result<(String, String, Type), ()> {
    let bytes = mangled.as_bytes();
    if !bytes.starts_with(MANGLE_PREFIX.as_bytes()) {
        return Err(());
    }

    let mut input = &bytes[MANGLE_PREFIX.len()..];
    let package_name = demangle_string(&mut input)?;
    let name = demangle_string(&mut input)?;
    let ty = demangle_type(&mut input)?;

    if !input.is_empty() {
        return Err(());
    }

    Ok((package_name, name, ty))
}

/// Mangles a module name.
///
/// The module name is mangled as a string, prefixed with `_N`.
pub fn mangle_module(module_name: &str) -> String {
    let mut result = String::with_capacity(module_name.len() + 2);
    result.push_str(MANGLE_PREFIX);
    result.push_str(&mangle_string(module_name));
    result
}

/// Demangles a module name back into its original form.
pub fn demangle_module(mangled: &str) -> Result<String, ()> {
    let bytes = mangled.as_bytes();
    if !bytes.starts_with(MANGLE_PREFIX.as_bytes()) {
        return Err(());
    }

    let mut input = &bytes[MANGLE_PREFIX.len()..];
    demangle_string(&mut input)
}

#[cfg(test)]
mod tests {
    use nitrate_tree::ByteSpan;
    use thin_vec::ThinVec;

    use super::*;

    fn with_store<R>(f: impl FnOnce() -> R) -> R {
        let store = Store::new();
        using_storage(&store, f)
    }

    fn test_type() -> Type {
        Type::Function {
            span: ByteSpan::default(),
            function_type: FunctionType {
                attributes: std::collections::BTreeSet::new(),
                params: ThinVec::new(),
                return_type: Type::Unit {
                    span: ByteSpan::default(),
                }
                .into(),
            }
            .into(),
        }
    }

    #[test]
    fn test_mangle_name_roundtrip() {
        with_store(|| {
            let cases = [
                ("my_package", "main", test_type()),
                ("my_package", "foo::bar", test_type()),
                ("my_package", "λ", test_type()),
                ("my_package", "foo-bar", test_type()),
            ];

            for (package, name, ty) in cases {
                let mangled = mangle_name(package, name, &ty);
                let (demangled_package, demangled_name, demangled_ty) = demangle_name(&mangled).unwrap();
                assert_eq!(demangled_package, package);
                assert_eq!(demangled_name, name);
                assert_eq!(demangled_ty, ty);
            }
        })
    }

    #[test]
    fn test_mangle_name_c99_charset() {
        with_store(|| {
            let cases = [
                ("my_package", "main", test_type()),
                ("my_package", "λ", test_type()),
                ("my_package", "foo-bar", test_type()),
                (
                    "my_package",
                    "a very long name that exceeds the compression threshold and should be compressed to save space in the symbol table",
                    test_type(),
                ),
            ];

            for (package, name, ty) in cases {
                let mangled = mangle_name(package, name, &ty);
                assert!(
                    mangled.bytes().all(|b| b.is_ascii_alphanumeric() || b == b'_'),
                    "mangled name contains non-C99 chars: {}",
                    mangled
                );
            }
        })
    }

    #[test]
    fn test_mangle_module_roundtrip() {
        let cases = ["my_package", "my_package::my_module", "λ::foo-bar"];

        for case in cases {
            let mangled = mangle_module(case);
            let demangled = demangle_module(&mangled).unwrap();
            assert_eq!(demangled, case);
        }
    }

    #[test]
    fn test_demangle_invalid() {
        assert!(demangle_name("not_mangled").is_err());
        assert!(demangle_module("not_mangled").is_err());
    }
}
