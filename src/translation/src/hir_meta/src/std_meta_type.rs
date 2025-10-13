use nitrate_hir::prelude::*;

pub fn create_std_meta_type_instance(ctx: &HirCtx, from: Type) -> Value {
    // TODO: Finish this function
    let std_meta_type = ctx
        .resolve_type(&"::std::meta::Type".into())
        .expect("compiler prelude is missing the defintion of `::std::meta::Type`")
        .to_owned();

    if !ctx[&std_meta_type].is_enum() {
        panic!("compiler prelude has an invalid definition of `::std::meta::Type`");
    }

    let enum_type = match &ctx[&std_meta_type] {
        Type::Enum { enum_type } => enum_type.to_owned(),
        _ => unreachable!(),
    };

    match from {
        Type::Never => Value::EnumVariant {
            enum_type,
            variant: "Never".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::Unit => Value::EnumVariant {
            enum_type,
            variant: "Unit".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::Bool => Value::EnumVariant {
            enum_type,
            variant: "Bool".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::U8 => Value::EnumVariant {
            enum_type,
            variant: "U8".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::U16 => Value::EnumVariant {
            enum_type,
            variant: "U16".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::U32 => Value::EnumVariant {
            enum_type,
            variant: "U32".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::U64 => Value::EnumVariant {
            enum_type,
            variant: "U64".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::U128 => Value::EnumVariant {
            enum_type,
            variant: "U128".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::USize => Value::EnumVariant {
            enum_type,
            variant: "USize".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::I8 => Value::EnumVariant {
            enum_type,
            variant: "I8".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::I16 => Value::EnumVariant {
            enum_type,
            variant: "I16".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::I32 => Value::EnumVariant {
            enum_type,
            variant: "I32".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::I64 => Value::EnumVariant {
            enum_type,
            variant: "I64".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::I128 => Value::EnumVariant {
            enum_type,
            variant: "I128".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::F8 => Value::EnumVariant {
            enum_type,
            variant: "F8".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::F16 => Value::EnumVariant {
            enum_type,
            variant: "F16".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::F32 => Value::EnumVariant {
            enum_type,
            variant: "F32".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::F64 => Value::EnumVariant {
            enum_type,
            variant: "F64".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::F128 => Value::EnumVariant {
            enum_type,
            variant: "F128".into(),
            value: Value::Unit.into_id(ctx.store()),
        },

        Type::Opaque { name } => todo!(),

        Type::Array { element_type, len } => todo!(),

        Type::Tuple { element_types } => todo!(),

        Type::Slice { element_type } => todo!(),

        Type::Struct { struct_type } => todo!(),

        Type::Enum { enum_type } => todo!(),

        Type::Refine { base, min, max } => todo!(),

        Type::Bitfield { base, bits } => todo!(),

        Type::Function { function_type } => todo!(),

        Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
        } => todo!(),

        Type::Pointer {
            exclusive,
            mutable,
            to,
        } => todo!(),

        Type::TypeAlias { name, aliased } => todo!(),

        Type::InferredFloat => todo!(),

        Type::InferredInteger => todo!(),

        Type::Inferred { id } => todo!(),
    }
}
