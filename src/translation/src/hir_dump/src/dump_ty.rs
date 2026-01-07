use std::ops::Deref;

use crate::{Dump, DumpContext, dump_item::dump_attributes};
use nitrate_hir::prelude::*;
use nitrate_token::escape_string;

impl Dump for StructAttribute {
    fn dump(&self, _ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            StructAttribute::Packed => write!(o, "packed"),
        }
    }
}

impl Dump for EnumAttribute {
    fn dump(&self, _ctx: &mut DumpContext, _o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        Ok(())
    }
}

impl Dump for FunctionAttribute {
    fn dump(&self, _ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            FunctionAttribute::CVariadic => write!(o, "c_variadic"),
            FunctionAttribute::NoMangle => write!(o, "no_mangle"),
        }
    }
}

impl Dump for Lifetime {
    fn dump(&self, _ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Lifetime::Static => write!(o, "'static"),
            Lifetime::Gc => write!(o, "'gc"),
            Lifetime::ThreadLocal => write!(o, "'thread"),
            Lifetime::TaskLocal => write!(o, "'task"),
            Lifetime::Inferred => write!(o, "'_"),
        }
    }
}

impl Dump for FunctionType {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        write!(o, "fn")?;

        dump_attributes(&self.attributes, ctx, o)?;

        if self.params.is_empty() {
            write!(o, " ()")?;
        } else {
            write!(o, " (")?;

            for (i, param) in self.params.iter().enumerate() {
                if i != 0 {
                    write!(o, ", ")?;
                }

                write!(o, "{}: ", param.0)?;
                param.1.dump(ctx, o)?;
            }

            write!(o, ")")?;
        }

        write!(o, " -> ")?;
        self.return_type.dump(ctx, o)
    }
}

impl Dump for Type {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
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

            Type::Array { element_type, len } => {
                write!(o, "[")?;
                element_type.dump(ctx, o)?;
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

            Type::Struct { def } => write!(o, "struct {}", escape_string(&def.borrow().name, true)),
            Type::Enum { def } => write!(o, "enum {}", escape_string(&def.borrow().name, true)),

            Type::TypeAlias { def } => {
                write!(o, "type {}", escape_string(&def.borrow().name, true))
            }

            Type::Refine { base, min, max } => {
                base.dump(ctx, o)?;
                write!(o, ": [")?;
                min.dump(ctx, o)?;
                write!(o, ":")?;
                max.dump(ctx, o)?;
                write!(o, "]")
            }

            Type::Function { function_type } => function_type.dump(ctx, o),

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => {
                write!(o, "&")?;

                if lifetime != &Lifetime::Inferred {
                    lifetime.dump(ctx, o)?;
                    write!(o, " ")?;
                }

                match (exclusive, mutable) {
                    (true, true) => write!(o, "mut ")?,
                    (true, false) => write!(o, "iso ")?,
                    (false, true) => write!(o, "poly mut ")?,
                    (false, false) => write!(o, "")?,
                }

                to.dump(ctx, o)
            }

            Type::SliceRef {
                lifetime,
                exclusive,
                mutable,
                element_type,
            } => {
                write!(o, "&")?;

                if lifetime != &Lifetime::Inferred {
                    lifetime.dump(ctx, o)?;
                    write!(o, " ")?;
                }

                match (exclusive, mutable) {
                    (true, true) => write!(o, "mut ")?,
                    (true, false) => write!(o, "iso ")?,
                    (false, true) => write!(o, "poly mut ")?,
                    (false, false) => write!(o, "")?,
                }

                write!(o, "[")?;
                element_type.dump(ctx, o)?;
                write!(o, "]")
            }

            Type::Pointer { exclusive, mutable, to } => {
                write!(o, "*")?;

                match (exclusive, mutable) {
                    (true, true) => write!(o, "mut ")?,
                    (true, false) => write!(o, "iso ")?,
                    (false, true) => write!(o, "poly mut ")?,
                    (false, false) => write!(o, "")?,
                }

                to.dump(ctx, o)
            }

            Type::Parameterized { base, args } => {
                base.dump(ctx, o)?;
                write!(o, "<")?;
                args.dump(ctx, o)?;
                write!(o, ">")
            }

            Type::InferredFloat => write!(o, "?f"),
            Type::InferredInteger => write!(o, "?i"),
            Type::Inferred { id } => write!(o, "?{id}"),
        }
    }
}

impl Dump for TypeId {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        self.deref().dump(ctx, o)
    }
}
