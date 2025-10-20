use crate::{dump::write_indent, dump_item::dump_attributes, prelude::*};

impl Dump for StructAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            StructAttribute::Packed => write!(o, "packed"),
        }
    }
}

impl Dump for EnumAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        Ok(())
    }
}

impl Dump for FunctionAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            FunctionAttribute::Variadic => write!(o, "variadic"),
        }
    }
}

impl Dump for Lifetime {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Lifetime::Static => write!(o, "'static"),
            Lifetime::Gc => write!(o, "'gc"),
            Lifetime::ThreadLocal => write!(o, "'thread"),
            Lifetime::TaskLocal => write!(o, "'task"),
            Lifetime::Inferred => write!(o, "'_"),
        }
    }
}

impl Dump for StructType {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        write!(o, "struct ")?;

        dump_attributes(&self.attributes, ctx, o)?;

        if self.fields.is_empty() {
            write!(o, "{{}}")
        } else {
            write!(o, "{{\n")?;

            for field in &self.fields {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                write!(o, "{}", field.name)?;

                write!(o, ": ")?;
                ctx.store[&field.ty].dump(ctx, o)?;

                write!(o, ",\n")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for EnumType {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        write!(o, "enum ")?;

        dump_attributes(&self.attributes, ctx, o)?;

        if self.variants.is_empty() {
            write!(o, "{{}}")
        } else {
            write!(o, "{{\n")?;

            for variant in &self.variants {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                write!(o, "{}", variant.name)?;

                write!(o, ": ")?;
                ctx.store[&variant.ty].dump(ctx, o)?;

                write!(o, ",\n")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for FunctionType {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        write!(o, "fn")?;

        dump_attributes(&self.attributes, ctx, o)?;

        if self.params.is_empty() {
            write!(o, " ()")?;
        } else {
            write!(o, " (\n")?;

            for param in self.params.iter() {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                param.dump(ctx, o)?;
                write!(o, ",\n")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, ")")?;
        }

        write!(o, " -> ")?;
        ctx.store[&self.return_type].dump(ctx, o)
    }
}

impl Dump for Type {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Type::Never => write!(o, "!"),
            Type::Unit => write!(o, "()"),
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
            Type::F32 => write!(o, "f32"),
            Type::F64 => write!(o, "f64"),
            Type::Opaque { name } => write!(o, "opaque(`{name}`)"),

            Type::Array { element_type, len } => {
                write!(o, "[")?;
                ctx.store[element_type].dump(ctx, o)?;
                write!(o, "; {len}]")
            }

            Type::Tuple { element_types } => {
                write!(o, "(")?;
                for (i, element_type) in element_types.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    ctx.store[element_type].dump(ctx, o)?;
                }

                if element_types.len() == 1 {
                    write!(o, ",")?;
                }

                write!(o, ")")
            }

            Type::Slice { element_type } => {
                write!(o, "[")?;
                ctx.store[element_type].dump(ctx, o)?;
                write!(o, "]")
            }

            Type::Struct { struct_type } => ctx.store[struct_type].dump(ctx, o),

            Type::Enum { enum_type } => ctx.store[enum_type].dump(ctx, o),

            Type::Refine { base, min, max } => {
                ctx.store[base].dump(ctx, o)?;
                write!(o, ": [")?;
                ctx.store[min].dump(ctx, o)?;
                write!(o, ":")?;
                ctx.store[max].dump(ctx, o)?;
                write!(o, "]")
            }

            Type::Bitfield { base, bits } => {
                ctx.store[base].dump(ctx, o)?;
                write!(o, ": {bits}")
            }

            Type::Function { function_type } => ctx.store[function_type].dump(ctx, o),

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => {
                write!(o, "&")?;

                if lifetime != &Lifetime::Inferred {
                    write!(o, " ")?;
                    lifetime.dump(ctx, o)?;
                    write!(o, " ")?;
                }

                match (exclusive, mutable) {
                    (true, true) => write!(o, "mut ")?,
                    (true, false) => write!(o, "iso ")?,
                    (false, true) => write!(o, "poly mut ")?,
                    (false, false) => write!(o, "")?,
                }

                ctx.store[to].dump(ctx, o)
            }

            Type::Pointer {
                exclusive,
                mutable,
                to,
            } => {
                write!(o, "*")?;
                if !exclusive {
                    write!(o, "shared ")?;
                }

                if *mutable {
                    write!(o, "mut ")?;
                }

                ctx.store[to].dump(ctx, o)
            }

            Type::Symbol { path } => write!(o, "{}", path),

            Type::InferredFloat => write!(o, "?f"),
            Type::InferredInteger => write!(o, "?i"),
            Type::Inferred { id } => write!(o, "?{id}"),
        }
    }
}
