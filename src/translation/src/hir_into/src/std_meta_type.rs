use crate::from_nitrate_expression;
use nitrate_hir::prelude::*;
use nitrate_tokenize::escape_string;
use std::fmt::Write;

pub(crate) enum EncodeErr {
    CannotEncodeInferredType,
}

fn metatype_source_encode(store: &Store, from: &Type, o: &mut dyn Write) -> Result<(), EncodeErr> {
    // FIXME: std::meta::Type transcoding | code is stringy => perform manual testing
    // FIXME: Update implementation in accordance with the ratified definition of std::meta::Type within standard library.

    match from {
        Type::Never => {
            write!(o, "::std::meta::Type::Never").unwrap();
            Ok(())
        }

        Type::Unit => {
            write!(o, "::std::meta::Type::Unit").unwrap();
            Ok(())
        }

        Type::Bool => {
            write!(o, "::std::meta::Type::Bool").unwrap();
            Ok(())
        }

        Type::U8 => {
            write!(o, "::std::meta::Type::U8").unwrap();
            Ok(())
        }

        Type::U16 => {
            write!(o, "::std::meta::Type::U16").unwrap();
            Ok(())
        }

        Type::U32 => {
            write!(o, "::std::meta::Type::U32").unwrap();
            Ok(())
        }

        Type::U64 => {
            write!(o, "::std::meta::Type::U64").unwrap();
            Ok(())
        }

        Type::U128 => {
            write!(o, "::std::meta::Type::U128").unwrap();
            Ok(())
        }

        Type::I8 => {
            write!(o, "::std::meta::Type::I8").unwrap();
            Ok(())
        }

        Type::I16 => {
            write!(o, "::std::meta::Type::I16").unwrap();
            Ok(())
        }

        Type::I32 => {
            write!(o, "::std::meta::Type::I32").unwrap();
            Ok(())
        }

        Type::I64 => {
            write!(o, "::std::meta::Type::I64").unwrap();
            Ok(())
        }

        Type::I128 => {
            write!(o, "::std::meta::Type::I128").unwrap();
            Ok(())
        }

        Type::F32 => {
            write!(o, "::std::meta::Type::F32").unwrap();
            Ok(())
        }

        Type::F64 => {
            write!(o, "::std::meta::Type::F64").unwrap();
            Ok(())
        }

        Type::USize => {
            write!(o, "::std::meta::Type::USize").unwrap();
            Ok(())
        }

        Type::Opaque { name } => {
            write!(
                o,
                "::std::meta::Type::Opaque {{ name: String::from({}) }}",
                escape_string(name, true)
            )
            .unwrap();
            Ok(())
        }

        Type::Array { element_type, len } => {
            write!(o, "::std::meta::Type::Array {{ element_type: ").unwrap();
            metatype_source_encode(store, &store[element_type], o)?;
            write!(o, ", len: {} }}", len).unwrap();
            Ok(())
        }

        Type::Tuple { element_types } => {
            write!(o, "::std::meta::Type::Tuple {{ element_types: Vec::from([").unwrap();
            for elem in element_types {
                metatype_source_encode(store, elem, o)?;
                write!(o, ",").unwrap();
            }
            write!(o, "]) }}").unwrap();
            Ok(())
        }

        Type::Slice { element_type } => {
            write!(o, "::std::meta::Type::Slice {{ element_type: ").unwrap();
            metatype_source_encode(store, &store[element_type], o)?;
            write!(o, " }}").unwrap();
            Ok(())
        }

        Type::Struct { struct_type } => {
            let struct_type = &store[struct_type];
            write!(o, "::std::meta::Type::Struct {{ fields: Vec::from([").unwrap();
            for field in &struct_type.fields {
                write!(
                    o,
                    "::std::meta::StructField {{ name: String::from({}), ty: ",
                    escape_string(&field.0, true)
                )
                .unwrap();
                metatype_source_encode(store, &store[field.1], o)?;
                write!(o, " }},").unwrap();
            }
            write!(o, "]), attributes: Vec::from([").unwrap();
            for attr in &struct_type.attributes {
                match attr {
                    StructAttribute::Packed => {
                        write!(o, "::std::meta::StructAttribute::Packed,").unwrap()
                    }
                };
                write!(o, ",").unwrap();
            }
            write!(o, "]) }}").unwrap();
            Ok(())
        }

        Type::Enum { enum_type } => {
            let enum_type = &store[enum_type];
            write!(o, "::std::meta::Type::Enum {{ variants: Vec::from([").unwrap();
            for variant in &enum_type.variants {
                write!(
                    o,
                    "::std::meta::EnumVariant {{ name: String::from({}), ty: ",
                    escape_string(&variant.0, true)
                )
                .unwrap();
                metatype_source_encode(store, &store[variant.1], o)?;
                write!(o, " }},").unwrap();
            }
            write!(o, "]), attributes: Vec::from([").unwrap();
            for attr in &enum_type.attributes {
                match attr {
                    EnumAttribute::Placeholder => {
                        write!(o, "::std::meta::EnumAttribute::Placeholder,").unwrap()
                    }
                };
                write!(o, ",").unwrap();
            }
            write!(o, "]) }}").unwrap();
            Ok(())
        }

        Type::Refine { base, min, max } => {
            write!(o, "::std::meta::Type::Refine {{ base: ").unwrap();
            metatype_source_encode(store, &store[base], o)?;
            write!(o, ", min: {}, max: {} }}", &store[min], &store[max]).unwrap();
            Ok(())
        }

        Type::Bitfield { base, bits } => {
            write!(o, "::std::meta::Type::Bitfield {{ base: ").unwrap();
            metatype_source_encode(store, &store[base], o)?;
            write!(o, ", bits: {} }}", bits).unwrap();
            Ok(())
        }

        Type::Function { function_type } => {
            let function_type = &store[function_type];
            write!(o, "::std::meta::Type::Function {{ parameters: Vec::from([").unwrap();
            for param_id in &function_type.params {
                let param = &store[param_id].borrow();
                write!(
                    o,
                    "::std::meta::FunctionParameter {{ name: String::from({}) , ty: ",
                    escape_string(&param.name, true)
                )
                .unwrap();
                metatype_source_encode(store, &store[&param.ty], o)?;
                write!(o, " }},").unwrap();
            }
            write!(o, "]), return_type: ").unwrap();
            metatype_source_encode(store, &store[&function_type.return_type], o)?;
            write!(o, ", attributes: Vec::from([").unwrap();
            for attr in &function_type.attributes {
                match attr {
                    FunctionAttribute::Variadic => {
                        write!(o, "::std::meta::FunctionAttribute::Variadic,").unwrap()
                    }
                };
                write!(o, ",").unwrap();
            }
            write!(o, "]) }}").unwrap();
            Ok(())
        }

        Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
        } => {
            write!(o, "::std::meta::Type::Reference {{ lifetime: ").unwrap();
            match lifetime {
                Lifetime::Static => write!(o, "::std::meta::Lifetime::Static").unwrap(),
                Lifetime::Gc => write!(o, "::std::meta::Lifetime::Gc").unwrap(),
                Lifetime::ThreadLocal => write!(o, "::std::meta::Lifetime::ThreadLocal").unwrap(),
                Lifetime::TaskLocal => write!(o, "::std::meta::Lifetime::TaskLocal").unwrap(),
                Lifetime::Inferred => write!(o, "::std::meta::Lifetime::Inferred").unwrap(),
            };
            write!(o, ", exclusive: {}, mutable: {}, to: ", exclusive, mutable).unwrap();
            metatype_source_encode(store, &store[to], o)?;
            write!(o, " }}").unwrap();
            Ok(())
        }

        Type::Pointer {
            exclusive,
            mutable,
            to,
        } => {
            write!(
                o,
                "::std::meta::Type::Pointer {{ exclusive: {}, mutable: {}, to: ",
                exclusive, mutable
            )
            .unwrap();
            metatype_source_encode(store, &store[to], o)?;
            write!(o, " }}").unwrap();
            Ok(())
        }

        Type::TypeAlias { name: _, aliased } => metatype_source_encode(store, &store[aliased], o),

        Type::InferredFloat => Err(EncodeErr::CannotEncodeInferredType),
        Type::InferredInteger => Err(EncodeErr::CannotEncodeInferredType),
        Type::Inferred { id: _ } => Err(EncodeErr::CannotEncodeInferredType),
    }
}

pub(crate) fn metatype_encode(ctx: &mut HirCtx, from: Type) -> Result<Value, EncodeErr> {
    let mut repr = String::new();
    metatype_source_encode(ctx.store(), &from, &mut repr)?;

    let hir_meta_object = from_nitrate_expression(ctx, &repr)
        .expect("failed to lower auto-generated std::meta::Type expression");

    Ok(hir_meta_object)
}
