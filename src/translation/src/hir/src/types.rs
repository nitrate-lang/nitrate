use hashbrown::HashMap;
use interned_string::IString;
use serde::{Deserialize, Serialize};
use std::num::NonZeroU32;

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
    pub mutable: bool,
    pub iso: bool,
    pub to: TypeId,
}

#[derive(Serialize, Deserialize)]
pub enum StructAttribute {}

#[derive(Serialize, Deserialize)]
pub struct StructType {
    pub attributes: Vec<StructAttribute>,
    pub fields: HashMap<IString, TypeId>,
}

#[derive(Serialize, Deserialize)]
pub enum EnumAttribute {}

#[derive(Serialize, Deserialize)]
pub struct EnumType {
    pub attributes: Vec<EnumAttribute>,
    pub variants: HashMap<IString, TypeId>,
}

#[derive(Serialize, Deserialize)]
pub struct FunctionAttribute {}

#[derive(Serialize, Deserialize)]
pub struct FunctionType {
    pub attributes: Vec<FunctionAttribute>,
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
    /* Function Types                                        */
    Function(Box<FunctionType>),
    Closure(Box<FunctionType>),

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

    pub fn is_nonclosure_function(&self) -> bool {
        matches!(self, Type::Function(_))
    }

    pub fn is_closure(&self) -> bool {
        matches!(self, Type::Closure(_))
    }

    pub fn is_function_or_closure(&self) -> bool {
        matches!(self, Type::Function(_) | Type::Closure(_))
    }

    pub fn is_reference(&self) -> bool {
        matches!(self, Type::Reference(_))
    }
}

#[derive(Clone, Serialize, Deserialize, PartialEq, Eq, Hash)]
pub struct TypeId {
    id: u32,
}

#[derive(Default)]
pub struct TypeStore {
    types: HashMap<TypeId, Type>,
    next_id: u32,
}

impl TypeStore {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn store(&mut self, ty: Type) -> TypeId {
        let id = self.next_id;
        self.types.insert(TypeId { id }, ty);
        self.next_id += 1;
        TypeId { id }
    }

    pub fn get(&self, id: &TypeId) -> Option<&Type> {
        self.types.get(id)
    }

    pub fn get_mut(&mut self, id: &TypeId) -> Option<&mut Type> {
        self.types.get_mut(id)
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
                    for (i, _attr) in struct_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        todo!()
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
                    for (i, _attr) in enum_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        todo!()
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

            Type::Function(func_type) | Type::Closure(func_type) => {
                if matches!(self, Type::Closure(_)) {
                    write!(o, "closure")?;
                } else {
                    write!(o, "fn")?;
                }

                if !func_type.attributes.is_empty() {
                    write!(o, " [")?;
                    for (i, _attr) in func_type.attributes.iter().enumerate() {
                        if i != 0 {
                            write!(o, ", ")?;
                        }

                        todo!()
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
