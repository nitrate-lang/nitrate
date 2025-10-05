use hashbrown::{HashMap, HashSet};
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

use crate::{DumpContext, TypeId, dump::Dump};

#[derive(Serialize, Deserialize)]
pub enum Lifetime {
    Static,
    Gc,
    ThreadLocal,
    TaskLocal,
    Stack { id: NonZeroU32 },
}

#[derive(Serialize, Deserialize)]
pub struct Reference {
    pub lifetime: Lifetime,
    pub exclusive: bool,
    pub mutable: bool,
    pub to: TypeId,
}

#[derive(Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum StructAttribute {
    Packed,
}

impl StructAttribute {
    fn dump(&self, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            StructAttribute::Packed => write!(o, "packed"),
        }
    }
}

#[derive(Serialize, Deserialize)]
pub struct StructType {
    pub attributes: HashSet<StructAttribute>,
    pub fields: Vec<(String, TypeId)>,
}

#[derive(Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum EnumAttribute {}

impl EnumAttribute {
    fn dump(&self, _o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            _ => Ok(()),
        }
    }
}

#[derive(Serialize, Deserialize)]
pub struct EnumType {
    pub attributes: HashSet<EnumAttribute>,
    pub variants: HashMap<String, TypeId>,
}

#[derive(Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum FunctionAttribute {
    Variadic,
}

impl FunctionAttribute {
    fn dump(&self, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            FunctionAttribute::Variadic => write!(o, "variadic"),
        }
    }
}

#[derive(Serialize, Deserialize)]
pub struct FunctionType {
    pub attributes: HashSet<FunctionAttribute>,
    pub parameters: Vec<TypeId>,
    pub return_type: TypeId,
}

#[derive(Serialize, Deserialize)]
pub enum Type {
    /* ----------------------------------------------------- */
    /* Primitive Types                                       */
    Never,
    Bool,
    U8,
    U16,
    U32,
    U64,
    U128,
    USize,
    I8,
    I16,
    I32,
    I64,
    I128,
    ISize,
    F8,
    F16,
    F32,
    F64,
    F128,

    /* ----------------------------------------------------- */
    /* Compound Types                                        */
    Array { element_type: TypeId, len: u64 },
    Tuple { elements: Vec<TypeId> },
    Slice { element_type: TypeId },
    Struct(Box<StructType>),
    Enum(Box<EnumType>),

    /* ----------------------------------------------------- */
    /* Function Type                                         */
    Function(Box<FunctionType>),

    /* ----------------------------------------------------- */
    /* Reference Type                                        */
    Reference(Box<Reference>),
}

impl Type {
    #[allow(non_upper_case_globals)]
    pub const Unit: Type = Type::Tuple { elements: vec![] };
}

impl Type {
    pub fn is_never(&self) -> bool {
        matches!(self, Type::Never)
    }

    pub fn is_unit(&self) -> bool {
        matches!(self, Type::Tuple { elements } if elements.is_empty())
    }

    pub fn is_bool(&self) -> bool {
        matches!(self, Type::Bool)
    }

    pub fn is_unsigned_primitive(&self) -> bool {
        matches!(
            self,
            Type::U8 | Type::U16 | Type::U32 | Type::U64 | Type::U128 | Type::USize
        )
    }

    pub fn is_signed_primitive(&self) -> bool {
        matches!(
            self,
            Type::I8 | Type::I16 | Type::I32 | Type::I64 | Type::I128 | Type::ISize
        )
    }

    pub fn is_integer_primitive(&self) -> bool {
        self.is_unsigned_primitive() || self.is_signed_primitive()
    }

    pub fn is_float_primitive(&self) -> bool {
        matches!(
            self,
            Type::F8 | Type::F16 | Type::F32 | Type::F64 | Type::F128
        )
    }

    pub fn is_array(&self) -> bool {
        matches!(self, Type::Array { .. })
    }

    pub fn is_slice(&self) -> bool {
        matches!(self, Type::Slice { .. })
    }

    pub fn is_tuple(&self) -> bool {
        matches!(self, Type::Tuple { .. })
    }

    pub fn is_struct(&self) -> bool {
        matches!(self, Type::Struct { .. })
    }

    pub fn is_enum(&self) -> bool {
        matches!(self, Type::Enum { .. })
    }

    pub fn is_function(&self) -> bool {
        matches!(self, Type::Function(_))
    }

    pub fn is_reference(&self) -> bool {
        matches!(self, Type::Reference(_))
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

                        attribute.dump(o)?;
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

                        attribute.dump(o)?;
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

                        attribute.dump(o)?;
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

#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum PointerSize {
    U8 = 1,
    U16 = 2,
    U32 = 4,
    U64 = 8,
    U128 = 16,
}
