use crate::prelude::{hir::*, *};

impl Dump for Type {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Type::Never => write!(o, "!"),
            Type::Bool => write!(o, "bool"),
            Type::U8 => write!(o, "u8"),
            Type::U16 => write!(o, "u16"),
            Type::U32 => write!(o, "u32"),
            Type::U64 => write!(o, "u64"),
            Type::U128 => write!(o, "u128"),
            Type::USize => write!(o, "usize"),
            Type::I8 => write!(o, "i8"),
            Type::I16 => write!(o, "i16"),
            Type::I32 => write!(o, "i32"),
            Type::I64 => write!(o, "i64"),
            Type::I128 => write!(o, "i128"),
            Type::ISize => write!(o, "isize"),
            Type::F8 => write!(o, "f8"),
            Type::F16 => write!(o, "f16"),
            Type::F32 => write!(o, "f32"),
            Type::F64 => write!(o, "f64"),
            Type::F128 => write!(o, "f128"),

            Type::Array { element_type, len } => {
                write!(o, "[")?;
                ctx.store[element_type].dump(ctx, o)?;
                write!(o, "; {len}]")
            }

            Type::Tuple { elements } => {
                write!(o, "(")?;
                for (i, element_type) in elements.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    ctx.store[element_type].dump(ctx, o)?;
                }
                write!(o, ")")
            }

            Type::Slice { element_type } => {
                write!(o, "[")?;
                ctx.store[element_type].dump(ctx, o)?;
                write!(o, "]")
            }

            Type::Struct(struct_type) => {
                write!(o, "struct")?;

                if !struct_type.attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attribute) in struct_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attribute.dump(ctx, o)?;
                    }
                    write!(o, "]")?;
                }

                write!(o, " {{ ")?;
                for (i, (name, field_type)) in struct_type.fields.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{name}: ")?;
                    ctx.store[field_type].dump(ctx, o)?;
                }
                write!(o, " }}")
            }

            Type::Enum(enum_type) => {
                write!(o, "enum")?;

                if !enum_type.attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attribute) in enum_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attribute.dump(ctx, o)?;
                    }
                    write!(o, "]")?;
                }

                write!(o, " {{ ")?;
                for (i, (name, variant_type)) in enum_type.variants.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{name}: ")?;
                    ctx.store[variant_type].dump(ctx, o)?;
                }
                write!(o, " }}")
            }

            Type::Function(func_type) => {
                write!(o, "fn")?;

                if !func_type.attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attribute) in func_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attribute.dump(ctx, o)?;
                    }
                    write!(o, "]")?;
                }

                write!(o, "(")?;
                for (i, param_type) in func_type.parameters.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    ctx.store[param_type].dump(ctx, o)?;
                }
                write!(o, ") -> ")?;
                ctx.store[&func_type.return_type].dump(ctx, o)
            }

            Type::Reference(reference) => {
                match &reference.lifetime {
                    Lifetime::Static => write!(o, "&'static ")?,
                    Lifetime::Gc => write!(o, "&'gc ")?,
                    Lifetime::ThreadLocal => write!(o, "&'thread ")?,
                    Lifetime::TaskLocal => write!(o, "&'task ")?,
                    Lifetime::Stack { id } => write!(o, "&'s{id} ")?,
                }

                if !reference.exclusive {
                    write!(o, "shared ")?;
                }

                if reference.mutable {
                    write!(o, "mut ")?;
                }

                ctx.store[&reference.to].dump(ctx, o)
            }
        }
    }
}
