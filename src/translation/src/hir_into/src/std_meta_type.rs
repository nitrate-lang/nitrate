use crate::from_nitrate_expression;
use interned_string::IString;
use nitrate_hir::prelude::*;
use nitrate_tokenize::escape_string;
use std::fmt::Write;

fn metatype_source_encode(store: &Store, from: &Type, o: &mut dyn Write) -> std::fmt::Result {
    match from {
        Type::Never => write!(o, "::std::meta::Type::Never"),
        Type::Unit => write!(o, "::std::meta::Type::Unit"),
        Type::Bool => write!(o, "::std::meta::Type::Bool"),
        Type::U8 => write!(o, "::std::meta::Type::U8"),
        Type::U16 => write!(o, "::std::meta::Type::U16"),
        Type::U32 => write!(o, "::std::meta::Type::U32"),
        Type::U64 => write!(o, "::std::meta::Type::U64"),
        Type::U128 => write!(o, "::std::meta::Type::U128"),
        Type::I8 => write!(o, "::std::meta::Type::I8"),
        Type::I16 => write!(o, "::std::meta::Type::I16"),
        Type::I32 => write!(o, "::std::meta::Type::I32"),
        Type::I64 => write!(o, "::std::meta::Type::I64"),
        Type::I128 => write!(o, "::std::meta::Type::I128"),
        Type::F32 => write!(o, "::std::meta::Type::F32"),
        Type::F64 => write!(o, "::std::meta::Type::F64"),
        Type::USize => write!(o, "::std::meta::Type::USize"),

        Type::Opaque { name } => write!(
            o,
            "::std::meta::Type::Opaque {{ name: String::from({}) }}",
            escape_string(name, true)
        ),

        Type::Array { element_type, len } => {
            write!(o, "::std::meta::Type::Array {{ element_type: ")?;
            metatype_source_encode(store, &store[element_type], o)?;
            write!(o, ", len: {} }}", len)
        }

        Type::Tuple { element_types } => {
            write!(o, "::std::meta::Type::Tuple {{ element_types: Vec::from([")?;
            for elem in element_types {
                metatype_source_encode(store, elem, o)?;
                write!(o, ",")?;
            }
            write!(o, "]) }}")
        }

        Type::Slice { element_type } => {
            write!(o, "::std::meta::Type::Slice {{ element_type: ")?;
            metatype_source_encode(store, &store[element_type], o)?;
            write!(o, " }}")
        }

        Type::Struct { struct_type } => {
            let struct_type = &store[struct_type];
            write!(o, "::std::meta::Type::Struct {{ fields: Vec::from([")?;
            for field in &struct_type.fields {
                write!(
                    o,
                    "::std::meta::StructField {{ name: String::from({}), ty: ",
                    escape_string(&field.0, true)
                )?;
                metatype_source_encode(store, &store[field.1], o)?;
                write!(o, " }},")?;
            }
            write!(o, "]), attributes: Vec::from([")?;
            for attr in &struct_type.attributes {
                match attr {
                    StructAttribute::Packed => write!(o, "::std::meta::StructAttribute::Packed,")?,
                }
                write!(o, ",")?;
            }
            write!(o, "]) }}")
        }

        Type::Enum { enum_type } => {
            let enum_type = &store[enum_type];
            write!(o, "::std::meta::Type::Enum {{ variants: Vec::from([")?;
            for variant in &enum_type.variants {
                write!(
                    o,
                    "::std::meta::EnumVariant {{ name: String::from({}), ty: ",
                    escape_string(&variant.0, true)
                )?;
                metatype_source_encode(store, &store[variant.1], o)?;
                write!(o, " }},")?;
            }
            write!(o, "]), attributes: Vec::from([")?;
            for attr in &enum_type.attributes {
                match attr {
                    EnumAttribute::Placeholder => {
                        write!(o, "::std::meta::EnumAttribute::Placeholder,")?
                    }
                }
                write!(o, ",")?;
            }
            write!(o, "]) }}")
        }

        Type::Refine {
            base: _,
            min: _,
            max: _,
        } => {
            // TODO: Finish this
            todo!()
        }

        Type::Bitfield { base: _, bits: _ } => {
            // TODO: Finish this
            todo!()
        }

        Type::Function { function_type: _ } => {
            // TODO: Finish this
            todo!()
        }

        Type::Reference {
            lifetime: _,
            exclusive: _,
            mutable: _,
            to: _,
        } => {
            // TODO: Finish this
            todo!()
        }

        Type::Pointer {
            exclusive: _,
            mutable: _,
            to: _,
        } => {
            // TODO: Finish this
            todo!()
        }

        Type::TypeAlias {
            name: _,
            aliased: _,
        } => {
            // TODO: Finish this
            todo!()
        }

        Type::InferredFloat => {
            // TODO: Finish this
            todo!()
        }

        Type::InferredInteger => {
            // TODO: Finish this
            todo!()
        }

        Type::Inferred { id: _ } => {
            // TODO: Finish this
            todo!()
        }
    }
}

pub(crate) fn metatype_encode(ctx: &mut HirCtx, from: Type) -> Value {
    let std_meta_type = ctx
        .resolve_type(&"::std::meta::Type".into())
        .expect("compiler prelude is missing the defintion of `::std::meta::Type`")
        .to_owned();

    if !ctx[&std_meta_type].is_enum() {
        panic!("compiler prelude has an invalid definition of `::std::meta::Type`");
    }

    let mut repr = String::new();
    metatype_source_encode(ctx.store(), &from, &mut repr).unwrap();

    let hir_meta_object = from_nitrate_expression(ctx, &repr)
        .expect("failed to lower auto-generated std::meta::Type expression");

    hir_meta_object
}
