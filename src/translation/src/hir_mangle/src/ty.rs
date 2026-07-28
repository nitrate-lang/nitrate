use std::format;

use nitrate_hir::prelude::*;

pub fn mangle_type(ty: &Type) -> String {
    match ty {
        Type::Never { .. } => "a".to_string(),
        Type::Unit { .. } => "b".to_string(),
        Type::Bool { .. } => "c".to_string(),
        Type::U8 { .. } => "g".to_string(),
        Type::U16 { .. } => "h".to_string(),
        Type::U32 { .. } => "i".to_string(),
        Type::U64 { .. } => "j".to_string(),
        Type::U128 { .. } => "k".to_string(),
        Type::USize { .. } => "l".to_string(),
        Type::I8 { .. } => "d".to_string(),
        Type::I16 { .. } => "e".to_string(),
        Type::I32 { .. } => "f".to_string(),
        Type::I64 { .. } => "m".to_string(),
        Type::I128 { .. } => "n".to_string(),
        Type::F32 { .. } => "o".to_string(),
        Type::F64 { .. } => "p".to_string(),

        Type::Array { element_type, len, .. } => {
            format!("q{}e{}", mangle_type(element_type), len)
        }

        Type::Tuple { element_types, .. } => {
            let mut result = "t".to_string();
            for element in element_types {
                result.push_str(&mangle_type(element));
            }
            result.push('e');
            result
        }

        Type::Struct { def, .. } => {
            let name = &def.borrow().name;
            format!("s{}e", name)
        }

        Type::Enum { def, .. } => {
            let name = &def.borrow().name;
            format!("u{}e", name)
        }

        Type::TypeAlias { def, .. } => {
            let name = &def.borrow().name;
            format!("x{}e", name)
        }

        Type::Refine { base, .. } => {
            format!("z{}", mangle_type(base))
        }

        Type::Function { function_type, .. } => {
            let mut result = "F".to_string();
            for (_, param) in function_type.params.iter() {
                result.push_str(&mangle_type(param));
            }
            result.push('e');
            result.push_str(&mangle_type(&function_type.return_type));
            result
        }

        Type::Reference {
            exclusive, mutable, to, ..
        } => {
            let prefix = if *exclusive { "R" } else { "r" };
            let mutability = if *mutable { "v" } else { "x" };
            format!("{}{}{}", prefix, mutability, mangle_type(to))
        }

        Type::SliceRef {
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let prefix = if *exclusive { "S" } else { "s" };
            let mutability = if *mutable { "v" } else { "x" };
            format!("{}{}{}", prefix, mutability, mangle_type(element_type))
        }

        Type::Pointer {
            exclusive, mutable, to, ..
        } => {
            let prefix = if *exclusive { "P" } else { "p" };
            let mutability = if *mutable { "v" } else { "x" };
            format!("{}{}{}", prefix, mutability, mangle_type(to))
        }

        Type::SlicePtr {
            exclusive,
            mutable,
            element_type,
            ..
        } => {
            let prefix = if *exclusive { "Q" } else { "q" };
            let mutability = if *mutable { "v" } else { "x" };
            format!("{}{}{}", prefix, mutability, mangle_type(element_type))
        }

        Type::TraitObject { .. } => "O".to_string(),
        Type::Parameterized { base, .. } => mangle_type(base),
        Type::GenericParam { index, .. } => format!("g{}", index),
        Type::Inferred { .. } => "i".to_string(),
        Type::InferredFloat { .. } => "f".to_string(),
        Type::InferredInteger { .. } => "w".to_string(),
    }
}
