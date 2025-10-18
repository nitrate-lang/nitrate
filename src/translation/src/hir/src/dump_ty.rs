use crate::prelude::*;

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

impl Dump for StructType {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        write!(o, "struct ")?;
        if !self.attributes.is_empty() {
            write!(o, "[")?;
            for (i, attribute) in self.attributes.iter().enumerate() {
                if i != 0 {
                    write!(o, ", ")?;
                }

                attribute.dump(ctx, o)?;
            }
            write!(o, "] ")?;
        }

        if self.fields.is_empty() {
            write!(o, "{{}}")
        } else {
            write!(o, "{{\n")?;
            ctx.indent += 1;
            for (name, field_type) in &self.fields {
                self.write_indent(ctx, o)?;
                write!(o, "{name}: ")?;
                ctx.store[field_type].dump(ctx, o)?;
                write!(o, ",\n")?;
            }
            ctx.indent -= 1;
            self.write_indent(ctx, o)?;
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
        if !self.attributes.is_empty() {
            write!(o, "[")?;
            for (i, attribute) in self.attributes.iter().enumerate() {
                if i != 0 {
                    write!(o, ", ")?;
                }

                attribute.dump(ctx, o)?;
            }
            write!(o, "] ")?;
        }

        if self.variants.is_empty() {
            write!(o, "{{}}")
        } else {
            write!(o, "{{\n")?;
            ctx.indent += 1;
            for (name, variant_type) in &self.variants {
                self.write_indent(ctx, o)?;
                write!(o, "{name}: ")?;
                ctx.store[variant_type].dump(ctx, o)?;
                write!(o, ",\n")?;
            }
            ctx.indent -= 1;
            self.write_indent(ctx, o)?;
            write!(o, "}}")
        }
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

                    element_type.dump(ctx, o)?;
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

            Type::Struct { struct_type } => {
                let StructType { attributes, fields } = &ctx.store[struct_type];

                write!(o, "struct")?;

                if !attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attribute) in attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attribute.dump(ctx, o)?;
                    }
                    write!(o, "]")?;
                }

                if fields.is_empty() {
                    write!(o, " {{}}")
                } else {
                    write!(o, " {{\n")?;
                    for (name, field_type) in fields {
                        ctx.indent += 1;

                        self.write_indent(ctx, o)?;
                        write!(o, "{name}: ")?;
                        ctx.store[field_type].dump(ctx, o)?;
                        write!(o, ",\n")?;

                        ctx.indent -= 1;
                    }

                    self.write_indent(ctx, o)?;
                    write!(o, "}}")
                }
            }

            Type::Enum { enum_type } => {
                let EnumType {
                    attributes,
                    variants,
                } = &ctx.store[enum_type];

                write!(o, "enum")?;

                if !attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attribute) in attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attribute.dump(ctx, o)?;
                    }
                    write!(o, "]")?;
                }

                if variants.is_empty() {
                    write!(o, " {{}}")
                } else {
                    write!(o, " {{\n")?;
                    for (name, variant_type) in variants {
                        ctx.indent += 1;

                        self.write_indent(ctx, o)?;
                        write!(o, "{name}: ")?;
                        ctx.store[variant_type].dump(ctx, o)?;
                        write!(o, ",\n")?;

                        ctx.indent -= 1;
                    }

                    self.write_indent(ctx, o)?;
                    write!(o, "}}")
                }
            }

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

            Type::Function { function_type } => {
                let FunctionType {
                    attributes,
                    params: parameters,
                    return_type,
                } = &ctx.store[function_type];

                write!(o, "fn")?;

                if !attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attribute) in attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attribute.dump(ctx, o)?;
                    }
                    write!(o, "]")?;
                }

                write!(o, "(")?;
                for (i, param) in parameters.iter().enumerate() {
                    let param = &ctx.store[param].borrow();

                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{}: ", param.name)?;
                    ctx.store[&param.ty].dump(ctx, o)?;
                    if let Some(default) = &param.default_value {
                        write!(o, " = ")?;
                        ctx.store[default].borrow().dump(ctx, o)?;
                    }
                }
                write!(o, ") -> ")?;
                ctx.store[return_type].dump(ctx, o)
            }

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => {
                match lifetime {
                    Lifetime::Static => write!(o, "&'static ")?,
                    Lifetime::Gc => write!(o, "&'gc ")?,
                    Lifetime::ThreadLocal => write!(o, "&'thread ")?,
                    Lifetime::TaskLocal => write!(o, "&'task ")?,
                    Lifetime::Inferred => write!(o, "&'_) ")?,
                }

                if !exclusive {
                    write!(o, "shared ")?;
                }

                if *mutable {
                    write!(o, "mut ")?;
                }

                ctx.store[to].dump(ctx, o)
            }

            Type::Pointer {
                exclusive,
                mutable,
                to,
            } => {
                if !exclusive {
                    write!(o, "shared ")?;
                }

                if *mutable {
                    write!(o, "mut ")?;
                }

                ctx.store[to].dump(ctx, o)
            }

            Type::Symbol { name, link: _ } => write!(o, "`{name}`"),

            Type::InferredFloat => write!(o, "?f"),
            Type::InferredInteger => write!(o, "?u"),
            Type::Inferred { id } => write!(o, "?{id}"),
        }
    }
}
