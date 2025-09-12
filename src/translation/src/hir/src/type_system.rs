use hashbrown::{HashMap, HashSet};
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

use crate::{TypeId, TypeStore};

#[derive(Serialize, Deserialize)]
pub enum Lifetime {
    ManuallyManaged,
    ProcessLocal,
    CollectorManaged,
    ThreadLocal,
    TaskLocal,
    StackLocal { id: NonZeroU32 },
}

#[derive(Serialize, Deserialize)]
pub struct Reference {
    pub lifetime: Lifetime,
    pub iso: bool,
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
    pub fields: HashMap<IString, TypeId>,
}

#[derive(Serialize, Deserialize, PartialEq, Eq, Hash)]
pub enum EnumAttribute {
    Packed,
}

impl EnumAttribute {
    fn dump(&self, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            EnumAttribute::Packed => write!(o, "packed"),
        }
    }
}

#[derive(Serialize, Deserialize)]
pub struct EnumType {
    pub attributes: HashSet<EnumAttribute>,
    pub variants: HashMap<IString, TypeId>,
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
    Reference(Reference),
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

impl Type {
    fn dump_or_placeholder(
        id: &TypeId,
        storage: &TypeStore,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        if let Some(ty) = storage.get(id) {
            ty.dump(storage, o)
        } else {
            write!(o, "?")
        }
    }

    pub fn dump(
        &self,
        storage: &TypeStore,
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
                Self::dump_or_placeholder(element_type, storage, o)?;
                write!(o, "; {len}]")
            }

            Type::Tuple { elements } => {
                write!(o, "(")?;
                for (i, element) in elements.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    Self::dump_or_placeholder(element, storage, o)?;
                }
                write!(o, ")")
            }

            Type::Slice { element_type } => {
                write!(o, "[")?;
                Self::dump_or_placeholder(element_type, storage, o)?;
                write!(o, "]")
            }

            Type::Struct(struct_type) => {
                write!(o, "struct")?;

                if !struct_type.attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attr) in struct_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attr.dump(o)?;
                    }
                    write!(o, "]")?;
                }

                write!(o, " {{ ")?;
                for (i, (name, ty)) in struct_type.fields.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{name}: ")?;
                    Self::dump_or_placeholder(ty, storage, o)?;
                }
                write!(o, " }}")
            }

            Type::Enum(enum_type) => {
                write!(o, "enum")?;

                if !enum_type.attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attr) in enum_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attr.dump(o)?;
                    }
                    write!(o, "]")?;
                }

                write!(o, " {{ ")?;
                for (i, (name, ty)) in enum_type.variants.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    write!(o, "{name}: ")?;
                    Self::dump_or_placeholder(ty, storage, o)?;
                }
                write!(o, " }}")
            }

            Type::Function(func_type) => {
                write!(o, "fn")?;

                if !func_type.attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, attr) in func_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        attr.dump(o)?;
                    }
                    write!(o, "]")?;
                }

                write!(o, "(")?;
                for (i, param) in func_type.parameters.iter().enumerate() {
                    if i != 0 {
                        write!(o, ", ")?;
                    }

                    Self::dump_or_placeholder(param, storage, o)?;
                }
                write!(o, ") -> ")?;
                Self::dump_or_placeholder(&func_type.return_type, storage, o)
            }

            Type::Reference(reference) => {
                match &reference.lifetime {
                    Lifetime::ManuallyManaged => write!(o, "*")?,
                    Lifetime::ProcessLocal => write!(o, "&'static ")?,
                    Lifetime::CollectorManaged => write!(o, "%")?,
                    Lifetime::ThreadLocal => write!(o, "&'thread ")?,
                    Lifetime::TaskLocal => write!(o, "&'task ")?,
                    Lifetime::StackLocal { id } => write!(o, "&'s{id} ")?,
                }

                if reference.iso {
                    write!(o, "iso ")?;
                } else {
                    write!(o, "poly ")?;
                }

                if reference.mutable {
                    write!(o, "mut ")?;
                } else {
                    write!(o, "const ")?;
                }

                Self::dump_or_placeholder(&reference.to, storage, o)
            }
        }
    }

    pub fn dump_string(&self, storage: &TypeStore) -> String {
        let mut buf = String::new();
        self.dump(storage, &mut buf).ok();
        buf
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

pub struct AlignofOptions {
    pub usize_size: PointerSize,
}

pub enum AlignofError {
    UnknownAlignment,
    Storage404,
}

pub fn get_align_of(
    ty: &Type,
    storage: &TypeStore,
    ptr_size: PointerSize,
) -> Result<u64, AlignofError> {
    match ty {
        Type::Never => Ok(1),
        Type::Bool => Ok(1),
        Type::U8 | Type::I8 | Type::F8 => Ok(1),
        Type::U16 | Type::I16 | Type::F16 => Ok(2),
        Type::U32 | Type::I32 | Type::F32 => Ok(4),
        Type::U64 | Type::I64 | Type::F64 => Ok(8),
        Type::U128 | Type::I128 | Type::F128 => Ok(16),
        Type::USize | Type::ISize => Ok(ptr_size as u64),

        Type::Array { element_type, .. } => {
            // Array alignment is the same as its element type alignment

            let element_type = storage.get(element_type).ok_or(AlignofError::Storage404)?;
            get_align_of(element_type, storage, ptr_size)
        }

        Type::Tuple { elements } => {
            // Tuple alignment is the max alignment among its element types

            let mut max_align = 1;

            for element in elements {
                let element = storage.get(element).ok_or(AlignofError::Storage404)?;
                let element_align = get_align_of(element, storage, ptr_size)?;

                if element_align > max_align {
                    max_align = element_align;
                }
            }

            Ok(max_align)
        }

        Type::Slice { element_type } => {
            // Slice alignment is the same as its element type alignment

            let element_type = storage.get(element_type).ok_or(AlignofError::Storage404)?;
            get_align_of(element_type, storage, ptr_size)
        }

        Type::Struct(struct_type) => {
            // Struct alignment is the same as its largest field alignment
            // unless it is packed, in which case it is 1

            if struct_type.attributes.contains(&StructAttribute::Packed) {
                return Ok(1);
            }

            let mut max_align = 1;

            for (_, field_type) in &struct_type.fields {
                let field_type = storage.get(field_type).ok_or(AlignofError::Storage404)?;
                let field_align = get_align_of(field_type, storage, ptr_size)?;

                if field_align > max_align {
                    max_align = field_align;
                }
            }

            Ok(max_align)
        }

        Type::Enum(enum_type) => {
            // Enum alignment is the same as its largest variant alignment
            // unless it is packed, in which case it is 1

            if enum_type.attributes.contains(&EnumAttribute::Packed) {
                return Ok(1);
            }

            let mut max_align = 1;

            for (_, variant_type) in &enum_type.variants {
                let variant_type = storage.get(variant_type).ok_or(AlignofError::Storage404)?;
                let variant_align = get_align_of(variant_type, storage, ptr_size)?;

                if variant_align > max_align {
                    max_align = variant_align;
                }
            }

            Ok(max_align)
        }

        Type::Function(_) => {
            // A Type::Function represents the literal machine code or whatever,
            // like just like a slice of bytes represents the literal bytes in memory.
            // This is not the same as a reference to a function which has an alignment.
            // We don't know the alignment of machine code, it could vary by platform.
            // This is why the function type itself has no alignment.

            Err(AlignofError::UnknownAlignment)
        }

        Type::Reference(_) => Ok(ptr_size as u64),
    }
}
