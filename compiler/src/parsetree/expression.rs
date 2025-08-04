use super::array_type::ArrayType;
use super::binary_op::BinaryOp;
use super::block::Block;
use super::character::CharLit;
use super::function::Function;
use super::function_type::FunctionType;
use super::list::ListLit;
use super::number::{FloatLit, IntegerLit};
use super::object::ObjectLit;
use super::returns::Return;
use super::statement::Statement;
use super::storage::{ExprKey, ExprKind, Storage, TypeKey, TypeKind};
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::UnaryOp;
use super::variable::Variable;

#[derive(Debug, Clone)]
pub(crate) enum ExprOwned<'a> {
    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    StructType(StructType<'a>),
    FunctionType(FunctionType<'a>),

    Discard,

    /* Literal Expressions */
    IntegerLit(IntegerLit),
    FloatLit(FloatLit),
    StringLit(StringLit<'a>),
    CharLit(CharLit),
    ListLit(ListLit<'a>),
    ObjectLit(ObjectLit<'a>),

    /* Compound Expressions */
    UnaryOp(UnaryOp<'a>),
    BinaryOp(BinaryOp<'a>),
    Statement(Statement<'a>),
    Block(Block<'a>),

    /* Definition */
    Function(Function<'a>),
    Variable(Variable<'a>),

    /* Control Flow */
    Return(Return<'a>),
}

#[derive(Debug, Clone)]
pub(crate) enum TypeOwned<'a> {
    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    StructType(StructType<'a>),
    FunctionType(FunctionType<'a>),
}

#[derive(Debug, Clone, Copy)]
pub enum ExprRef<'storage, 'a> {
    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    StructType(&'storage StructType<'a>),
    FunctionType(&'storage FunctionType<'a>),

    Discard,

    /* Literal Expressions */
    IntegerLit(&'storage IntegerLit),
    FloatLit(&'storage FloatLit),
    StringLit(&'storage StringLit<'a>),
    CharLit(&'storage CharLit),
    ListLit(&'storage ListLit<'a>),
    ObjectLit(&'storage ObjectLit<'a>),

    /* Compound Expressions */
    UnaryOp(&'storage UnaryOp<'a>),
    BinaryOp(&'storage BinaryOp<'a>),
    Statement(&'storage Statement<'a>),
    Block(&'storage Block<'a>),

    /* Definition */
    Function(&'storage Function<'a>),
    Variable(&'storage Variable<'a>),

    /* Control Flow */
    Return(&'storage Return<'a>),
}

#[derive(Debug)]
pub enum ExprRefMut<'storage, 'a> {
    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(&'storage mut TupleType<'a>),
    ArrayType(&'storage mut ArrayType<'a>),
    StructType(&'storage mut StructType<'a>),
    FunctionType(&'storage mut FunctionType<'a>),

    Discard,

    /* Literal Expressions */
    IntegerLit(&'storage mut IntegerLit),
    FloatLit(&'storage mut FloatLit),
    StringLit(&'storage mut StringLit<'a>),
    CharLit(&'storage mut CharLit),
    ListLit(&'storage mut ListLit<'a>),
    ObjectLit(&'storage mut ObjectLit<'a>),

    /* Compound Expressions */
    UnaryOp(&'storage mut UnaryOp<'a>),
    BinaryOp(&'storage mut BinaryOp<'a>),
    Statement(&'storage mut Statement<'a>),
    Block(&'storage mut Block<'a>),

    /* Definition */
    Function(&'storage mut Function<'a>),
    Variable(&'storage mut Variable<'a>),

    /* Control Flow */
    Return(&'storage mut Return<'a>),
}

#[derive(Debug, Clone, Copy)]
pub enum TypeRef<'storage, 'a> {
    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(&'storage TupleType<'a>),
    ArrayType(&'storage ArrayType<'a>),
    StructType(&'storage StructType<'a>),
    FunctionType(&'storage FunctionType<'a>),
}

#[derive(Debug)]
pub enum TypeRefMut<'storage, 'a> {
    /* Primitive Types */
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

    /* Compound Types */
    InferType,
    TupleType(&'storage mut TupleType<'a>),
    ArrayType(&'storage mut ArrayType<'a>),
    StructType(&'storage mut StructType<'a>),
    FunctionType(&'storage mut FunctionType<'a>),
}

impl<'a> TryInto<TypeOwned<'a>> for ExprOwned<'a> {
    type Error = Self;

    fn try_into(self) -> Result<TypeOwned<'a>, Self::Error> {
        match self {
            ExprOwned::Bool => Ok(TypeOwned::Bool),
            ExprOwned::UInt8 => Ok(TypeOwned::UInt8),
            ExprOwned::UInt16 => Ok(TypeOwned::UInt16),
            ExprOwned::UInt32 => Ok(TypeOwned::UInt32),
            ExprOwned::UInt64 => Ok(TypeOwned::UInt64),
            ExprOwned::UInt128 => Ok(TypeOwned::UInt128),
            ExprOwned::Int8 => Ok(TypeOwned::Int8),
            ExprOwned::Int16 => Ok(TypeOwned::Int16),
            ExprOwned::Int32 => Ok(TypeOwned::Int32),
            ExprOwned::Int64 => Ok(TypeOwned::Int64),
            ExprOwned::Int128 => Ok(TypeOwned::Int128),
            ExprOwned::Float8 => Ok(TypeOwned::Float8),
            ExprOwned::Float16 => Ok(TypeOwned::Float16),
            ExprOwned::Float32 => Ok(TypeOwned::Float32),
            ExprOwned::Float64 => Ok(TypeOwned::Float64),
            ExprOwned::Float128 => Ok(TypeOwned::Float128),

            ExprOwned::InferType => Ok(TypeOwned::InferType),
            ExprOwned::TupleType(x) => Ok(TypeOwned::TupleType(x)),
            ExprOwned::ArrayType(x) => Ok(TypeOwned::ArrayType(x)),
            ExprOwned::StructType(x) => Ok(TypeOwned::StructType(x)),
            ExprOwned::FunctionType(x) => Ok(TypeOwned::FunctionType(x)),

            ExprOwned::Discard
            | ExprOwned::IntegerLit(_)
            | ExprOwned::FloatLit(_)
            | ExprOwned::StringLit(_)
            | ExprOwned::CharLit(_)
            | ExprOwned::ListLit(_)
            | ExprOwned::ObjectLit(_)
            | ExprOwned::UnaryOp(_)
            | ExprOwned::BinaryOp(_)
            | ExprOwned::Statement(_)
            | ExprOwned::Block(_)
            | ExprOwned::Function(_)
            | ExprOwned::Variable(_)
            | ExprOwned::Return(_) => Err(self),
        }
    }
}

impl<'a> Into<ExprOwned<'a>> for TypeOwned<'a> {
    fn into(self) -> ExprOwned<'a> {
        match self {
            TypeOwned::Bool => ExprOwned::Bool,
            TypeOwned::UInt8 => ExprOwned::UInt8,
            TypeOwned::UInt16 => ExprOwned::UInt16,
            TypeOwned::UInt32 => ExprOwned::UInt32,
            TypeOwned::UInt64 => ExprOwned::UInt64,
            TypeOwned::UInt128 => ExprOwned::UInt128,
            TypeOwned::Int8 => ExprOwned::Int8,
            TypeOwned::Int16 => ExprOwned::Int16,
            TypeOwned::Int32 => ExprOwned::Int32,
            TypeOwned::Int64 => ExprOwned::Int64,
            TypeOwned::Int128 => ExprOwned::Int128,
            TypeOwned::Float8 => ExprOwned::Float8,
            TypeOwned::Float16 => ExprOwned::Float16,
            TypeOwned::Float32 => ExprOwned::Float32,
            TypeOwned::Float64 => ExprOwned::Float64,
            TypeOwned::Float128 => ExprOwned::Float128,

            TypeOwned::InferType => ExprOwned::InferType,
            TypeOwned::TupleType(x) => ExprOwned::TupleType(x),
            TypeOwned::ArrayType(x) => ExprOwned::ArrayType(x),
            TypeOwned::StructType(x) => ExprOwned::StructType(x),
            TypeOwned::FunctionType(x) => ExprOwned::FunctionType(x),
        }
    }
}

impl<'storage, 'a> TryInto<TypeRef<'storage, 'a>> for ExprRef<'storage, 'a> {
    type Error = Self;

    fn try_into(self) -> Result<TypeRef<'storage, 'a>, Self::Error> {
        match self {
            ExprRef::Bool => Ok(TypeRef::Bool),
            ExprRef::UInt8 => Ok(TypeRef::UInt8),
            ExprRef::UInt16 => Ok(TypeRef::UInt16),
            ExprRef::UInt32 => Ok(TypeRef::UInt32),
            ExprRef::UInt64 => Ok(TypeRef::UInt64),
            ExprRef::UInt128 => Ok(TypeRef::UInt128),
            ExprRef::Int8 => Ok(TypeRef::Int8),
            ExprRef::Int16 => Ok(TypeRef::Int16),
            ExprRef::Int32 => Ok(TypeRef::Int32),
            ExprRef::Int64 => Ok(TypeRef::Int64),
            ExprRef::Int128 => Ok(TypeRef::Int128),
            ExprRef::Float8 => Ok(TypeRef::Float8),
            ExprRef::Float16 => Ok(TypeRef::Float16),
            ExprRef::Float32 => Ok(TypeRef::Float32),
            ExprRef::Float64 => Ok(TypeRef::Float64),
            ExprRef::Float128 => Ok(TypeRef::Float128),

            ExprRef::InferType => Ok(TypeRef::InferType),
            ExprRef::TupleType(x) => Ok(TypeRef::TupleType(x)),
            ExprRef::ArrayType(x) => Ok(TypeRef::ArrayType(x)),
            ExprRef::StructType(x) => Ok(TypeRef::StructType(x)),
            ExprRef::FunctionType(x) => Ok(TypeRef::FunctionType(x)),

            ExprRef::Discard
            | ExprRef::IntegerLit(_)
            | ExprRef::FloatLit(_)
            | ExprRef::StringLit(_)
            | ExprRef::CharLit(_)
            | ExprRef::ListLit(_)
            | ExprRef::ObjectLit(_)
            | ExprRef::UnaryOp(_)
            | ExprRef::BinaryOp(_)
            | ExprRef::Statement(_)
            | ExprRef::Block(_)
            | ExprRef::Function(_)
            | ExprRef::Variable(_)
            | ExprRef::Return(_) => Err(self),
        }
    }
}

impl<'storage, 'a> TryInto<TypeRefMut<'storage, 'a>> for ExprRefMut<'storage, 'a> {
    type Error = Self;

    fn try_into(self) -> Result<TypeRefMut<'storage, 'a>, Self::Error> {
        match self {
            ExprRefMut::Bool => Ok(TypeRefMut::Bool),
            ExprRefMut::UInt8 => Ok(TypeRefMut::UInt8),
            ExprRefMut::UInt16 => Ok(TypeRefMut::UInt16),
            ExprRefMut::UInt32 => Ok(TypeRefMut::UInt32),
            ExprRefMut::UInt64 => Ok(TypeRefMut::UInt64),
            ExprRefMut::UInt128 => Ok(TypeRefMut::UInt128),
            ExprRefMut::Int8 => Ok(TypeRefMut::Int8),
            ExprRefMut::Int16 => Ok(TypeRefMut::Int16),
            ExprRefMut::Int32 => Ok(TypeRefMut::Int32),
            ExprRefMut::Int64 => Ok(TypeRefMut::Int64),
            ExprRefMut::Int128 => Ok(TypeRefMut::Int128),
            ExprRefMut::Float8 => Ok(TypeRefMut::Float8),
            ExprRefMut::Float16 => Ok(TypeRefMut::Float16),
            ExprRefMut::Float32 => Ok(TypeRefMut::Float32),
            ExprRefMut::Float64 => Ok(TypeRefMut::Float64),
            ExprRefMut::Float128 => Ok(TypeRefMut::Float128),

            ExprRefMut::InferType => Ok(TypeRefMut::InferType),
            ExprRefMut::TupleType(x) => Ok(TypeRefMut::TupleType(x)),
            ExprRefMut::ArrayType(x) => Ok(TypeRefMut::ArrayType(x)),
            ExprRefMut::StructType(x) => Ok(TypeRefMut::StructType(x)),
            ExprRefMut::FunctionType(x) => Ok(TypeRefMut::FunctionType(x)),

            ExprRefMut::Discard
            | ExprRefMut::IntegerLit(_)
            | ExprRefMut::FloatLit(_)
            | ExprRefMut::StringLit(_)
            | ExprRefMut::CharLit(_)
            | ExprRefMut::ListLit(_)
            | ExprRefMut::ObjectLit(_)
            | ExprRefMut::UnaryOp(_)
            | ExprRefMut::BinaryOp(_)
            | ExprRefMut::Statement(_)
            | ExprRefMut::Block(_)
            | ExprRefMut::Function(_)
            | ExprRefMut::Variable(_)
            | ExprRefMut::Return(_) => Err(self),
        }
    }
}

impl<'storage, 'a> Into<ExprRef<'storage, 'a>> for TypeRef<'storage, 'a> {
    fn into(self) -> ExprRef<'storage, 'a> {
        match self {
            TypeRef::Bool => ExprRef::Bool,
            TypeRef::UInt8 => ExprRef::UInt8,
            TypeRef::UInt16 => ExprRef::UInt16,
            TypeRef::UInt32 => ExprRef::UInt32,
            TypeRef::UInt64 => ExprRef::UInt64,
            TypeRef::UInt128 => ExprRef::UInt128,
            TypeRef::Int8 => ExprRef::Int8,
            TypeRef::Int16 => ExprRef::Int16,
            TypeRef::Int32 => ExprRef::Int32,
            TypeRef::Int64 => ExprRef::Int64,
            TypeRef::Int128 => ExprRef::Int128,
            TypeRef::Float8 => ExprRef::Float8,
            TypeRef::Float16 => ExprRef::Float16,
            TypeRef::Float32 => ExprRef::Float32,
            TypeRef::Float64 => ExprRef::Float64,
            TypeRef::Float128 => ExprRef::Float128,

            TypeRef::InferType => ExprRef::InferType,
            TypeRef::TupleType(x) => ExprRef::TupleType(x),
            TypeRef::ArrayType(x) => ExprRef::ArrayType(x),
            TypeRef::StructType(x) => ExprRef::StructType(x),
            TypeRef::FunctionType(x) => ExprRef::FunctionType(x),
        }
    }
}

impl<'storage, 'a> Into<ExprRefMut<'storage, 'a>> for TypeRefMut<'storage, 'a> {
    fn into(self) -> ExprRefMut<'storage, 'a> {
        match self {
            TypeRefMut::Bool => ExprRefMut::Bool,
            TypeRefMut::UInt8 => ExprRefMut::UInt8,
            TypeRefMut::UInt16 => ExprRefMut::UInt16,
            TypeRefMut::UInt32 => ExprRefMut::UInt32,
            TypeRefMut::UInt64 => ExprRefMut::UInt64,
            TypeRefMut::UInt128 => ExprRefMut::UInt128,
            TypeRefMut::Int8 => ExprRefMut::Int8,
            TypeRefMut::Int16 => ExprRefMut::Int16,
            TypeRefMut::Int32 => ExprRefMut::Int32,
            TypeRefMut::Int64 => ExprRefMut::Int64,
            TypeRefMut::Int128 => ExprRefMut::Int128,
            TypeRefMut::Float8 => ExprRefMut::Float8,
            TypeRefMut::Float16 => ExprRefMut::Float16,
            TypeRefMut::Float32 => ExprRefMut::Float32,
            TypeRefMut::Float64 => ExprRefMut::Float64,
            TypeRefMut::Float128 => ExprRefMut::Float128,

            TypeRefMut::InferType => ExprRefMut::InferType,
            TypeRefMut::TupleType(x) => ExprRefMut::TupleType(x),
            TypeRefMut::ArrayType(x) => ExprRefMut::ArrayType(x),
            TypeRefMut::StructType(x) => ExprRefMut::StructType(x),
            TypeRefMut::FunctionType(x) => ExprRefMut::FunctionType(x),
        }
    }
}

impl<'a> ExprKey<'a> {
    pub fn is_discard(&self) -> bool {
        self.variant_index() == ExprKind::Discard
    }

    pub fn discard(&mut self) {
        *self = ExprKey::new_single(ExprKind::Discard);
    }

    pub fn is_type(&self) -> bool {
        match self.variant_index() {
            ExprKind::Bool
            | ExprKind::UInt8
            | ExprKind::UInt16
            | ExprKind::UInt32
            | ExprKind::UInt64
            | ExprKind::UInt128
            | ExprKind::Int8
            | ExprKind::Int16
            | ExprKind::Int32
            | ExprKind::Int64
            | ExprKind::Int128
            | ExprKind::Float8
            | ExprKind::Float16
            | ExprKind::Float32
            | ExprKind::Float64
            | ExprKind::Float128
            | ExprKind::InferType
            | ExprKind::TupleType
            | ExprKind::ArrayType
            | ExprKind::StructType
            | ExprKind::FunctionType => true,

            ExprKind::Discard
            | ExprKind::IntegerLit
            | ExprKind::FloatLit
            | ExprKind::StringLit
            | ExprKind::CharLit
            | ExprKind::ListLit
            | ExprKind::ObjectLit
            | ExprKind::UnaryOp
            | ExprKind::BinaryOp
            | ExprKind::Statement
            | ExprKind::Block
            | ExprKind::Function
            | ExprKind::Variable
            | ExprKind::Return => false,
        }
    }

    pub fn has_parentheses(&self, _storage: &Storage<'a>) -> bool {
        match self.variant_index() {
            ExprKind::Bool
            | ExprKind::UInt8
            | ExprKind::UInt16
            | ExprKind::UInt32
            | ExprKind::UInt64
            | ExprKind::UInt128
            | ExprKind::Int8
            | ExprKind::Int16
            | ExprKind::Int32
            | ExprKind::Int64
            | ExprKind::Int128
            | ExprKind::Float8
            | ExprKind::Float16
            | ExprKind::Float32
            | ExprKind::Float64
            | ExprKind::Float128
            | ExprKind::InferType
            | ExprKind::TupleType
            | ExprKind::ArrayType
            | ExprKind::StructType => false,

            ExprKind::FunctionType => {
                // TODO: Check if the expression has parentheses
                false
            }

            ExprKind::Discard
            | ExprKind::IntegerLit
            | ExprKind::FloatLit
            | ExprKind::StringLit
            | ExprKind::CharLit
            | ExprKind::ListLit
            | ExprKind::ObjectLit => false,

            ExprKind::UnaryOp => {
                // TODO: Check if the expression has parentheses
                false
            }

            ExprKind::BinaryOp => {
                // TODO: Check if the expression has parentheses
                false
            }

            ExprKind::Statement => {
                // TODO: Check if the expression has parentheses
                false
            }

            ExprKind::Block => false,

            ExprKind::Function => {
                // TODO: Check if the expression has parentheses
                false
            }
            ExprKind::Variable => {
                // TODO: Check if the expression has parentheses
                false
            }

            ExprKind::Return => {
                // TODO: Check if the expression has parentheses
                false
            }
        }
    }

    pub fn set_parentheses(&mut self, _storage: &Storage<'a>, _has_parentheses: bool) {
        match self.variant_index() {
            ExprKind::Bool
            | ExprKind::UInt8
            | ExprKind::UInt16
            | ExprKind::UInt32
            | ExprKind::UInt64
            | ExprKind::UInt128
            | ExprKind::Int8
            | ExprKind::Int16
            | ExprKind::Int32
            | ExprKind::Int64
            | ExprKind::Int128
            | ExprKind::Float8
            | ExprKind::Float16
            | ExprKind::Float32
            | ExprKind::Float64
            | ExprKind::Float128
            | ExprKind::InferType
            | ExprKind::TupleType
            | ExprKind::ArrayType
            | ExprKind::StructType => {}

            ExprKind::FunctionType => {
                // TODO: Set if the expression has parentheses
            }

            ExprKind::Discard
            | ExprKind::IntegerLit
            | ExprKind::FloatLit
            | ExprKind::StringLit
            | ExprKind::CharLit
            | ExprKind::ListLit
            | ExprKind::ObjectLit => {}

            ExprKind::UnaryOp => {
                // TODO: Set if the expression has parentheses
            }

            ExprKind::BinaryOp => {
                // TODO: Set if the expression has parentheses
            }

            ExprKind::Statement => {
                // TODO: Set if the expression has parentheses
            }

            ExprKind::Block => {}

            ExprKind::Function => {
                // TODO: Set if the expression has parentheses
            }
            ExprKind::Variable => {
                // TODO: Set if the expression has parentheses
            }

            ExprKind::Return => {
                // TODO: Set if the expression has parentheses
            }
        }
    }
}

impl<'a> TypeKey<'a> {
    pub fn has_parentheses(&self, _storage: &Storage<'a>) -> bool {
        match self.variant_index() {
            TypeKind::Bool
            | TypeKind::UInt8
            | TypeKind::UInt16
            | TypeKind::UInt32
            | TypeKind::UInt64
            | TypeKind::UInt128
            | TypeKind::Int8
            | TypeKind::Int16
            | TypeKind::Int32
            | TypeKind::Int64
            | TypeKind::Int128
            | TypeKind::Float8
            | TypeKind::Float16
            | TypeKind::Float32
            | TypeKind::Float64
            | TypeKind::Float128
            | TypeKind::InferType
            | TypeKind::TupleType
            | TypeKind::ArrayType
            | TypeKind::StructType => false,

            TypeKind::FunctionType => {
                // TODO: Check if the expression has parentheses
                false
            }
        }
    }
}
