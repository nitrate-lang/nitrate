use super::array_type::ArrayType;
use super::binary_op::BinaryOp;
use super::block::Block;
use super::character::CharLit;
use super::expression::{ExprOwned, ExprRef, ExprRefMut, TypeOwned, TypeRef, TypeRefMut};
use super::function::Function;
use super::function_type::FunctionType;
use super::list::ListLit;
use super::number::{FloatLit, IntegerLit};
use super::object::ObjectLit;
use super::returns::Return;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::UnaryOp;
use super::variable::Variable;

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
enum ExprKind {
    Bool,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Int8,
    Int16,
    Int32,
    Int64,
    Int128,
    Float8,
    Float16,
    Float32,
    Float64,
    Float128,

    InferType,
    TupleType,
    ArrayType,
    StructType,
    FunctionType,

    Discard,

    Integer,
    Float,
    String,
    Char,
    List,
    Object,

    UnaryOp,
    BinaryOp,
    Statement,
    Block,

    Function,
    Variable,

    Return,
}

#[derive(Debug, PartialEq, Eq, Clone, Copy)]
enum TypeKind {
    Bool,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Int8,
    Int16,
    Int32,
    Int64,
    Int128,
    Float8,
    Float16,
    Float32,
    Float64,
    Float128,

    InferType,
    TupleType,
    ArrayType,
    StructType,
    FunctionType,
}

impl TryInto<TypeKind> for ExprKind {
    type Error = ();

    fn try_into(self) -> Result<TypeKind, Self::Error> {
        match self {
            ExprKind::Bool => Ok(TypeKind::Bool),
            ExprKind::UInt8 => Ok(TypeKind::UInt8),
            ExprKind::UInt16 => Ok(TypeKind::UInt16),
            ExprKind::UInt32 => Ok(TypeKind::UInt32),
            ExprKind::UInt64 => Ok(TypeKind::UInt64),
            ExprKind::UInt128 => Ok(TypeKind::UInt128),
            ExprKind::Int8 => Ok(TypeKind::Int8),
            ExprKind::Int16 => Ok(TypeKind::Int16),
            ExprKind::Int32 => Ok(TypeKind::Int32),
            ExprKind::Int64 => Ok(TypeKind::Int64),
            ExprKind::Int128 => Ok(TypeKind::Int128),
            ExprKind::Float8 => Ok(TypeKind::Float8),
            ExprKind::Float16 => Ok(TypeKind::Float16),
            ExprKind::Float32 => Ok(TypeKind::Float32),
            ExprKind::Float64 => Ok(TypeKind::Float64),
            ExprKind::Float128 => Ok(TypeKind::Float128),

            ExprKind::InferType => Ok(TypeKind::InferType),
            ExprKind::TupleType => Ok(TypeKind::TupleType),
            ExprKind::ArrayType => Ok(TypeKind::ArrayType),
            ExprKind::StructType => Ok(TypeKind::StructType),
            ExprKind::FunctionType => Ok(TypeKind::FunctionType),

            ExprKind::Discard
            | ExprKind::Integer
            | ExprKind::Float
            | ExprKind::String
            | ExprKind::Char
            | ExprKind::List
            | ExprKind::Object
            | ExprKind::UnaryOp
            | ExprKind::BinaryOp
            | ExprKind::Statement
            | ExprKind::Block
            | ExprKind::Function
            | ExprKind::Variable
            | ExprKind::Return => Err(()),
        }
    }
}

impl Into<ExprKind> for TypeKind {
    fn into(self) -> ExprKind {
        match self {
            TypeKind::Bool => ExprKind::Bool,
            TypeKind::UInt8 => ExprKind::UInt8,
            TypeKind::UInt16 => ExprKind::UInt16,
            TypeKind::UInt32 => ExprKind::UInt32,
            TypeKind::UInt64 => ExprKind::UInt64,
            TypeKind::UInt128 => ExprKind::UInt128,
            TypeKind::Int8 => ExprKind::Int8,
            TypeKind::Int16 => ExprKind::Int16,
            TypeKind::Int32 => ExprKind::Int32,
            TypeKind::Int64 => ExprKind::Int64,
            TypeKind::Int128 => ExprKind::Int128,
            TypeKind::Float8 => ExprKind::Float8,
            TypeKind::Float16 => ExprKind::Float16,
            TypeKind::Float32 => ExprKind::Float32,
            TypeKind::Float64 => ExprKind::Float64,
            TypeKind::Float128 => ExprKind::Float128,

            TypeKind::InferType => ExprKind::InferType,
            TypeKind::TupleType => ExprKind::TupleType,
            TypeKind::ArrayType => ExprKind::ArrayType,
            TypeKind::StructType => ExprKind::StructType,
            TypeKind::FunctionType => ExprKind::FunctionType,
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub struct ExprKey<'a> {
    id: u32,
    _marker: std::marker::PhantomData<&'a ()>,
}

#[derive(Debug, Clone, Copy)]
pub struct TypeKey<'a> {
    id: u32,
    _marker: std::marker::PhantomData<&'a ()>,
}

impl<'a> ExprKey<'a> {
    fn new(variant: ExprKind, index: usize) -> Option<Self> {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);

        can_store_index.then_some(ExprKey {
            id: (variant as u32) << 26 | index as u32,
            _marker: std::marker::PhantomData,
        })
    }

    fn new_single(variant: ExprKind) -> Self {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        ExprKey {
            id: (variant as u32) << 26,
            _marker: std::marker::PhantomData,
        }
    }

    fn variant_index(&self) -> ExprKind {
        let number = (self.id >> 26) as u8;

        // SAFETY: The number is guaranteed to be in the range of ExprKind
        unsafe { std::mem::transmute::<u8, ExprKind>(number) }
    }

    fn instance_index(&self) -> usize {
        (self.id & 0x03FFFFFF) as usize
    }

    pub fn get<'storage>(&self, storage: &'storage Storage<'a>) -> ExprRef<'storage, 'a> {
        storage.get_expr(*self)
    }

    pub fn get_mut<'storage>(
        &mut self,
        storage: &'storage mut Storage<'a>,
    ) -> ExprRefMut<'storage, 'a> {
        storage.get_expr_mut(*self)
    }
}

impl<'a> TypeKey<'a> {
    fn new(variant: TypeKind, index: usize) -> Option<Self> {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);

        can_store_index.then_some(TypeKey {
            id: (variant as u32) << 26 | index as u32,
            _marker: std::marker::PhantomData,
        })
    }

    fn variant_index(&self) -> TypeKind {
        let number = (self.id >> 26) as u8;

        // SAFETY: The number is guaranteed to be in the range of TypeKind
        unsafe { std::mem::transmute::<u8, TypeKind>(number) }
    }

    fn instance_index(&self) -> usize {
        (self.id & 0x03FFFFFF) as usize
    }

    pub fn get<'storage>(&self, storage: &'storage Storage<'a>) -> TypeRef<'storage, 'a> {
        storage.get_type(*self)
    }

    pub fn get_mut<'storage>(
        &mut self,
        storage: &'storage mut Storage<'a>,
    ) -> TypeRefMut<'storage, 'a> {
        storage.get_type_mut(*self)
    }
}

impl<'a> Into<ExprKey<'a>> for TypeKey<'a> {
    fn into(self) -> ExprKey<'a> {
        ExprKey::new(self.variant_index().into(), self.instance_index())
            .expect("TypeRef should be convertible to ExprRef")
    }
}

impl<'a> TryInto<TypeKey<'a>> for ExprKey<'a> {
    type Error = ();

    fn try_into(self) -> Result<TypeKey<'a>, Self::Error> {
        let variant = self.variant_index().try_into()?;
        TypeKey::new(variant, self.instance_index()).ok_or(())
    }
}

#[derive(Debug, Clone, Default)]
pub struct Storage<'a> {
    integers: Vec<IntegerLit>,
    floats: Vec<FloatLit>,
    strings: Vec<StringLit<'a>>,
    characters: Vec<CharLit>,
    lists: Vec<ListLit<'a>>,
    objects: Vec<ObjectLit<'a>>,

    unary_ops: Vec<UnaryOp<'a>>,
    binary_ops: Vec<BinaryOp<'a>>,
    statements: Vec<Statement<'a>>,
    blocks: Vec<Block<'a>>,

    functions: Vec<Function<'a>>,
    variables: Vec<Variable<'a>>,

    returns: Vec<Return<'a>>,

    tuple_types: Vec<TupleType<'a>>,
    array_types: Vec<ArrayType<'a>>,
    struct_types: Vec<StructType<'a>>,
    function_types: Vec<FunctionType<'a>>,
}

impl<'a> Storage<'a> {
    pub(crate) fn add_expr(&mut self, expr: ExprOwned<'a>) -> Option<ExprKey<'a>> {
        match expr {
            ExprOwned::Bool => Some(ExprKey::new_single(ExprKind::Bool)),
            ExprOwned::UInt8 => Some(ExprKey::new_single(ExprKind::UInt8)),
            ExprOwned::UInt16 => Some(ExprKey::new_single(ExprKind::UInt16)),
            ExprOwned::UInt32 => Some(ExprKey::new_single(ExprKind::UInt32)),
            ExprOwned::UInt64 => Some(ExprKey::new_single(ExprKind::UInt64)),
            ExprOwned::UInt128 => Some(ExprKey::new_single(ExprKind::UInt128)),
            ExprOwned::Int8 => Some(ExprKey::new_single(ExprKind::Int8)),
            ExprOwned::Int16 => Some(ExprKey::new_single(ExprKind::Int16)),
            ExprOwned::Int32 => Some(ExprKey::new_single(ExprKind::Int32)),
            ExprOwned::Int64 => Some(ExprKey::new_single(ExprKind::Int64)),
            ExprOwned::Int128 => Some(ExprKey::new_single(ExprKind::Int128)),
            ExprOwned::Float8 => Some(ExprKey::new_single(ExprKind::Float8)),
            ExprOwned::Float16 => Some(ExprKey::new_single(ExprKind::Float16)),
            ExprOwned::Float32 => Some(ExprKey::new_single(ExprKind::Float32)),
            ExprOwned::Float64 => Some(ExprKey::new_single(ExprKind::Float64)),
            ExprOwned::Float128 => Some(ExprKey::new_single(ExprKind::Float128)),

            ExprOwned::InferType => Some(ExprKey::new_single(ExprKind::InferType)),

            ExprOwned::TupleType(node) => ExprKey::new(ExprKind::TupleType, self.tuple_types.len())
                .and_then(|k| {
                    self.tuple_types.push(node);
                    Some(k)
                }),

            ExprOwned::ArrayType(node) => ExprKey::new(ExprKind::ArrayType, self.array_types.len())
                .and_then(|k| {
                    self.array_types.push(node);
                    Some(k)
                }),

            ExprOwned::StructType(node) => {
                ExprKey::new(ExprKind::StructType, self.struct_types.len()).and_then(|k| {
                    self.struct_types.push(node);
                    Some(k)
                })
            }

            ExprOwned::FunctionType(node) => {
                ExprKey::new(ExprKind::FunctionType, self.function_types.len()).and_then(|k| {
                    self.function_types.push(node);
                    Some(k)
                })
            }

            ExprOwned::Discard => Some(ExprKey::new_single(ExprKind::Discard)),

            ExprOwned::IntegerLit(node) => ExprKey::new(ExprKind::Integer, self.integers.len())
                .and_then(|k| {
                    self.integers.push(node);
                    Some(k)
                }),

            ExprOwned::FloatLit(node) => {
                ExprKey::new(ExprKind::Float, self.floats.len()).and_then(|k| {
                    self.floats.push(node);
                    Some(k)
                })
            }

            ExprOwned::StringLit(node) => ExprKey::new(ExprKind::String, self.strings.len())
                .and_then(|k| {
                    self.strings.push(node);
                    Some(k)
                }),

            ExprOwned::CharLit(node) => ExprKey::new(ExprKind::Char, self.characters.len())
                .and_then(|k| {
                    self.characters.push(node);
                    Some(k)
                }),

            ExprOwned::ListLit(node) => {
                ExprKey::new(ExprKind::List, self.lists.len()).and_then(|k| {
                    self.lists.push(node);
                    Some(k)
                })
            }

            ExprOwned::ObjectLit(node) => ExprKey::new(ExprKind::Object, self.objects.len())
                .and_then(|k| {
                    self.objects.push(node);
                    Some(k)
                }),

            ExprOwned::UnaryOp(node) => ExprKey::new(ExprKind::UnaryOp, self.unary_ops.len())
                .and_then(|k| {
                    self.unary_ops.push(node);
                    Some(k)
                }),

            ExprOwned::BinaryOp(node) => ExprKey::new(ExprKind::BinaryOp, self.binary_ops.len())
                .and_then(|k| {
                    self.binary_ops.push(node);
                    Some(k)
                }),

            ExprOwned::Statement(node) => ExprKey::new(ExprKind::Statement, self.statements.len())
                .and_then(|k| {
                    self.statements.push(node);
                    Some(k)
                }),

            ExprOwned::Block(node) => {
                ExprKey::new(ExprKind::Block, self.blocks.len()).and_then(|k| {
                    self.blocks.push(node);
                    Some(k)
                })
            }

            ExprOwned::Function(node) => ExprKey::new(ExprKind::Function, self.functions.len())
                .and_then(|k| {
                    self.functions.push(node);
                    Some(k)
                }),

            ExprOwned::Variable(node) => ExprKey::new(ExprKind::Variable, self.variables.len())
                .and_then(|k| {
                    self.variables.push(node);
                    Some(k)
                }),

            ExprOwned::Return(node) => {
                ExprKey::new(ExprKind::Return, self.returns.len()).and_then(|k| {
                    self.returns.push(node);
                    Some(k)
                })
            }
        }
    }

    pub(crate) fn add_type(&mut self, ty: TypeOwned<'a>) -> Option<TypeKey<'a>> {
        self.add_expr(ty.into()).and_then(|expr_ref| {
            /*
             * All TypeKinds have analogous ExprKinds, so we can use the same
             * variant index for both.
             */

            let variant = expr_ref
                .variant_index()
                .try_into()
                .expect("Failed to convert ExprKind to TypeKind");
            let index = expr_ref.instance_index();

            TypeKey::new(variant, index)
        })
    }

    pub fn get_expr(&self, id: ExprKey<'a>) -> ExprRef<'_, 'a> {
        let index = id.instance_index() as usize;

        match id.variant_index() {
            ExprKind::Bool => Some(ExprRef::Bool),
            ExprKind::UInt8 => Some(ExprRef::UInt8),
            ExprKind::UInt16 => Some(ExprRef::UInt16),
            ExprKind::UInt32 => Some(ExprRef::UInt32),
            ExprKind::UInt64 => Some(ExprRef::UInt64),
            ExprKind::UInt128 => Some(ExprRef::UInt128),
            ExprKind::Int8 => Some(ExprRef::Int8),
            ExprKind::Int16 => Some(ExprRef::Int16),
            ExprKind::Int32 => Some(ExprRef::Int32),
            ExprKind::Int64 => Some(ExprRef::Int64),
            ExprKind::Int128 => Some(ExprRef::Int128),
            ExprKind::Float8 => Some(ExprRef::Float8),
            ExprKind::Float16 => Some(ExprRef::Float16),
            ExprKind::Float32 => Some(ExprRef::Float32),
            ExprKind::Float64 => Some(ExprRef::Float64),
            ExprKind::Float128 => Some(ExprRef::Float128),

            ExprKind::InferType => Some(ExprRef::InferType),
            ExprKind::TupleType => self.tuple_types.get(index).map(ExprRef::TupleType),
            ExprKind::ArrayType => self.array_types.get(index).map(ExprRef::ArrayType),
            ExprKind::StructType => self.struct_types.get(index).map(ExprRef::StructType),
            ExprKind::FunctionType => self.function_types.get(index).map(ExprRef::FunctionType),

            ExprKind::Discard => Some(ExprRef::Discard),

            ExprKind::Integer => self.integers.get(index).map(ExprRef::IntegerLit),
            ExprKind::Float => self.floats.get(index).map(ExprRef::FloatLit),
            ExprKind::String => self.strings.get(index).map(ExprRef::StringLit),
            ExprKind::Char => self.characters.get(index).map(ExprRef::CharLit),
            ExprKind::List => self.lists.get(index).map(ExprRef::ListLit),
            ExprKind::Object => self.objects.get(index).map(ExprRef::ObjectLit),

            ExprKind::UnaryOp => self.unary_ops.get(index).map(ExprRef::UnaryOp),
            ExprKind::BinaryOp => self.binary_ops.get(index).map(ExprRef::BinaryOp),
            ExprKind::Statement => self.statements.get(index).map(ExprRef::Statement),
            ExprKind::Block => self.blocks.get(index).map(ExprRef::Block),

            ExprKind::Function => self.functions.get(index).map(ExprRef::Function),
            ExprKind::Variable => self.variables.get(index).map(ExprRef::Variable),

            ExprKind::Return => self.returns.get(index).map(ExprRef::Return),
        }
        .expect("Expression not found in storage")
    }

    pub fn get_expr_mut(&mut self, id: ExprKey<'a>) -> ExprRefMut<'_, 'a> {
        let index = id.instance_index() as usize;

        match id.variant_index() {
            ExprKind::Bool => Some(ExprRefMut::Bool),
            ExprKind::UInt8 => Some(ExprRefMut::UInt8),
            ExprKind::UInt16 => Some(ExprRefMut::UInt16),
            ExprKind::UInt32 => Some(ExprRefMut::UInt32),
            ExprKind::UInt64 => Some(ExprRefMut::UInt64),
            ExprKind::UInt128 => Some(ExprRefMut::UInt128),
            ExprKind::Int8 => Some(ExprRefMut::Int8),
            ExprKind::Int16 => Some(ExprRefMut::Int16),
            ExprKind::Int32 => Some(ExprRefMut::Int32),
            ExprKind::Int64 => Some(ExprRefMut::Int64),
            ExprKind::Int128 => Some(ExprRefMut::Int128),
            ExprKind::Float8 => Some(ExprRefMut::Float8),
            ExprKind::Float16 => Some(ExprRefMut::Float16),
            ExprKind::Float32 => Some(ExprRefMut::Float32),
            ExprKind::Float64 => Some(ExprRefMut::Float64),
            ExprKind::Float128 => Some(ExprRefMut::Float128),

            ExprKind::InferType => Some(ExprRefMut::InferType),
            ExprKind::TupleType => self.tuple_types.get_mut(index).map(ExprRefMut::TupleType),
            ExprKind::ArrayType => self.array_types.get_mut(index).map(ExprRefMut::ArrayType),
            ExprKind::StructType => self.struct_types.get_mut(index).map(ExprRefMut::StructType),
            ExprKind::FunctionType => self
                .function_types
                .get_mut(index)
                .map(ExprRefMut::FunctionType),

            ExprKind::Discard => Some(ExprRefMut::Discard),

            ExprKind::Integer => self.integers.get_mut(index).map(ExprRefMut::IntegerLit),
            ExprKind::Float => self.floats.get_mut(index).map(ExprRefMut::FloatLit),
            ExprKind::String => self.strings.get_mut(index).map(ExprRefMut::StringLit),
            ExprKind::Char => self.characters.get_mut(index).map(ExprRefMut::CharLit),
            ExprKind::List => self.lists.get_mut(index).map(ExprRefMut::ListLit),
            ExprKind::Object => self.objects.get_mut(index).map(ExprRefMut::ObjectLit),

            ExprKind::UnaryOp => self.unary_ops.get_mut(index).map(ExprRefMut::UnaryOp),
            ExprKind::BinaryOp => self.binary_ops.get_mut(index).map(ExprRefMut::BinaryOp),
            ExprKind::Statement => self.statements.get_mut(index).map(ExprRefMut::Statement),
            ExprKind::Block => self.blocks.get_mut(index).map(ExprRefMut::Block),

            ExprKind::Function => self.functions.get_mut(index).map(ExprRefMut::Function),
            ExprKind::Variable => self.variables.get_mut(index).map(ExprRefMut::Variable),

            ExprKind::Return => self.returns.get_mut(index).map(ExprRefMut::Return),
        }
        .expect("Expression not found in storage")
    }

    pub fn get_type(&self, id: TypeKey<'a>) -> TypeRef<'_, 'a> {
        let index = id.instance_index() as usize;

        match id.variant_index() {
            TypeKind::Bool => Some(TypeRef::Bool),
            TypeKind::UInt8 => Some(TypeRef::UInt8),
            TypeKind::UInt16 => Some(TypeRef::UInt16),
            TypeKind::UInt32 => Some(TypeRef::UInt32),
            TypeKind::UInt64 => Some(TypeRef::UInt64),
            TypeKind::UInt128 => Some(TypeRef::UInt128),
            TypeKind::Int8 => Some(TypeRef::Int8),
            TypeKind::Int16 => Some(TypeRef::Int16),
            TypeKind::Int32 => Some(TypeRef::Int32),
            TypeKind::Int64 => Some(TypeRef::Int64),
            TypeKind::Int128 => Some(TypeRef::Int128),
            TypeKind::Float8 => Some(TypeRef::Float8),
            TypeKind::Float16 => Some(TypeRef::Float16),
            TypeKind::Float32 => Some(TypeRef::Float32),
            TypeKind::Float64 => Some(TypeRef::Float64),
            TypeKind::Float128 => Some(TypeRef::Float128),

            TypeKind::InferType => Some(TypeRef::InferType),
            TypeKind::TupleType => self.tuple_types.get(index).map(TypeRef::TupleType),
            TypeKind::ArrayType => self.array_types.get(index).map(TypeRef::ArrayType),
            TypeKind::StructType => self.struct_types.get(index).map(TypeRef::StructType),
            TypeKind::FunctionType => self.function_types.get(index).map(TypeRef::FunctionType),
        }
        .expect("Expression not found in storage")
    }

    pub fn get_type_mut(&mut self, id: TypeKey<'a>) -> TypeRefMut<'_, 'a> {
        let index = id.instance_index() as usize;

        match id.variant_index() {
            TypeKind::Bool => Some(TypeRefMut::Bool),
            TypeKind::UInt8 => Some(TypeRefMut::UInt8),
            TypeKind::UInt16 => Some(TypeRefMut::UInt16),
            TypeKind::UInt32 => Some(TypeRefMut::UInt32),
            TypeKind::UInt64 => Some(TypeRefMut::UInt64),
            TypeKind::UInt128 => Some(TypeRefMut::UInt128),
            TypeKind::Int8 => Some(TypeRefMut::Int8),
            TypeKind::Int16 => Some(TypeRefMut::Int16),
            TypeKind::Int32 => Some(TypeRefMut::Int32),
            TypeKind::Int64 => Some(TypeRefMut::Int64),
            TypeKind::Int128 => Some(TypeRefMut::Int128),
            TypeKind::Float8 => Some(TypeRefMut::Float8),
            TypeKind::Float16 => Some(TypeRefMut::Float16),
            TypeKind::Float32 => Some(TypeRefMut::Float32),
            TypeKind::Float64 => Some(TypeRefMut::Float64),
            TypeKind::Float128 => Some(TypeRefMut::Float128),

            TypeKind::InferType => Some(TypeRefMut::InferType),
            TypeKind::TupleType => self.tuple_types.get_mut(index).map(TypeRefMut::TupleType),
            TypeKind::ArrayType => self.array_types.get_mut(index).map(TypeRefMut::ArrayType),
            TypeKind::StructType => self.struct_types.get_mut(index).map(TypeRefMut::StructType),
            TypeKind::FunctionType => self
                .function_types
                .get_mut(index)
                .map(TypeRefMut::FunctionType),
        }
        .expect("Expression not found in storage")
    }
}
