use core::panic;

use nitrate_hir::{
    Store,
    hir::{Lifetime, Type},
};

use crate::string::mangle_string;

pub(crate) fn mangle_type(ty: &Type, store: &Store) -> String {
    match ty {
        Type::Never => "a".to_string(),
        Type::Unit => "b".to_string(),
        Type::Bool => "c".to_string(),

        Type::U8 => "g".to_string(),
        Type::U16 => "h".to_string(),
        Type::U32 => "i".to_string(),
        Type::U64 => "j".to_string(),
        Type::U128 => "k".to_string(),
        Type::USize => "l".to_string(),

        Type::I8 => "q".to_string(),
        Type::I16 => "r".to_string(),
        Type::I32 => "s".to_string(),
        Type::I64 => "t".to_string(),
        Type::I128 => "u".to_string(),

        Type::F32 => "w".to_string(),
        Type::F64 => "x".to_string(),

        Type::Array { element_type, len } => {
            let elem_mangled = mangle_type(&store[element_type], store);
            format!("A{}_{}", len, elem_mangled)
        }

        Type::Tuple { element_types } => {
            let mut mangled = String::new();

            mangled.push_str("T");
            for elem_type in element_types {
                let elem_mangled = mangle_type(&store[elem_type], store);
                mangled.push_str(&elem_mangled);
            }
            mangled.push_str("E");

            mangled
        }

        Type::Struct { struct_type } => {
            let struct_type = &store[struct_type];
            let mut mangled = String::new();

            mangled.push_str("S");
            for field_type in &struct_type.fields {
                let elem_mangled = mangle_type(&store[&field_type.ty], store);
                mangled.push_str(&elem_mangled);
            }
            mangled.push_str("E");

            mangled
        }

        Type::Enum { enum_type } => {
            let enum_type = &store[enum_type];
            let mut mangled = String::new();

            mangled.push_str("M");
            for variant in &enum_type.variants {
                let variant_mangled = mangle_type(&store[&variant.ty], store);
                mangled.push_str(&variant_mangled);
            }
            mangled.push_str("E");

            mangled
        }

        Type::Refine { base, min, max } => {
            let base_mangled = mangle_type(&store[base], store);
            format!("Y{}_{}_{}", store[min], store[max], base_mangled)
        }

        Type::Function { function_type } => {
            let function_type = &store[function_type];
            let mut mangled = String::new();

            mangled.push_str("F");
            let return_mangled = mangle_type(&store[&function_type.return_type], store);
            mangled.push_str(&return_mangled);

            for param_type in &function_type.params {
                let param_mangled = mangle_type(&store[&param_type.1], store);
                mangled.push_str(&param_mangled);
            }
            mangled.push_str("E");

            mangled
        }

        Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
        } => {
            let lifetime_mangled = match lifetime {
                Lifetime::Static => "A",
                Lifetime::Gc => "B",
                Lifetime::ThreadLocal => "C",
                Lifetime::TaskLocal => "D",
                Lifetime::Inferred => panic!("Cannot mangle inferred lifetime"),
            };

            let exmut_mangled = match (exclusive, mutable) {
                (true, true) => "A",
                (true, false) => "B",
                (false, true) => "C",
                (false, false) => "D",
            };

            let to_mangled = mangle_type(&store[to], store);
            format!("R{}{}{}", lifetime_mangled, exmut_mangled, to_mangled)
        }

        Type::SliceRef {
            lifetime,
            exclusive,
            mutable,
            element_type,
        } => {
            let lifetime_mangled = match lifetime {
                Lifetime::Static => "A",
                Lifetime::Gc => "B",
                Lifetime::ThreadLocal => "C",
                Lifetime::TaskLocal => "D",
                Lifetime::Inferred => panic!("Cannot mangle inferred lifetime"),
            };

            let exmut_mangled = match (exclusive, mutable) {
                (true, true) => "A",
                (true, false) => "B",
                (false, true) => "C",
                (false, false) => "D",
            };

            let elem_mangled = mangle_type(&store[element_type], store);
            format!("Q{}{}{}", lifetime_mangled, exmut_mangled, elem_mangled)
        }

        Type::Pointer {
            exclusive,
            mutable,
            to,
        } => {
            let exmut_mangled = match (exclusive, mutable) {
                (true, true) => "A",
                (true, false) => "B",
                (false, true) => "C",
                (false, false) => "D",
            };

            let to_mangled = mangle_type(&store[to], store);
            format!("P{}{}", exmut_mangled, to_mangled)
        }

        Type::Symbol { path } => {
            let mangled_path = mangle_string(path);
            format!("Z{}", mangled_path)
        }

        Type::InferredFloat | Type::InferredInteger | Type::Inferred { .. } => {
            panic!("Cannot mangle inferred type: {:?}", ty);
        }
    }
}

pub(crate) fn demangle_type(_mangled: &mut dyn std::io::Read, _store: &Store) -> Result<Type, ()> {
    // TODO: implement type demangling
    Err(())
}
