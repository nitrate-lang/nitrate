use super::array_type::ArrayType;
use super::binary_op::BinaryOp;
use super::block::Block;
use super::character::CharLit;
use super::expression::{MutRefExpr, MutRefType, OwnedExpr, OwnedType, RefExpr, RefType};
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
pub struct ExprRef<'a> {
    id: u32,
    _marker: std::marker::PhantomData<&'a ()>,
}

#[derive(Debug, Clone, Copy)]
pub struct TypeRef<'a> {
    id: u32,
    _marker: std::marker::PhantomData<&'a ()>,
}

impl<'a> ExprRef<'a> {
    fn new(variant: ExprKind, index: usize) -> Option<Self> {
        // FIXME: Enforce at compile time using reflection
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);

        can_store_index.then_some(ExprRef {
            id: (variant as u32) << 26 | index as u32,
            _marker: std::marker::PhantomData,
        })
    }

    fn new_single(variant: ExprKind) -> Self {
        // FIXME: Enforce at compile time using reflection
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        ExprRef {
            id: (variant as u32) << 26,
            _marker: std::marker::PhantomData,
        }
    }

    fn variant_index(&self) -> ExprKind {
        let number = (self.id >> 26) as u8;

        // FIXME: SAFETY: The number is guaranteed to be in the range of ExprKind
        unsafe { std::mem::transmute::<u8, ExprKind>(number) }
    }

    fn instance_index(&self) -> usize {
        (self.id & 0x03FFFFFF) as usize
    }

    pub fn get(&self) -> RefExpr<'_, 'a> {
        // TODO: Implement this method to retrieve the expression from thread-local storage
        panic!("get() not implemented for ExprRef");
    }

    pub fn get_mut(&mut self) -> MutRefExpr<'_, 'a> {
        // TODO: Implement this method to retrieve the expression from thread-local storage
        panic!("get_mut() not implemented for ExprRef");
    }
}

impl<'a> TypeRef<'a> {
    fn new(variant: TypeKind, index: usize) -> Option<Self> {
        // FIXME: Enforce at compile time using reflection
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);

        can_store_index.then_some(TypeRef {
            id: (variant as u32) << 26 | index as u32,
            _marker: std::marker::PhantomData,
        })
    }

    fn variant_index(&self) -> TypeKind {
        let number = (self.id >> 26) as u8;

        // FIXME: SAFETY: The number is guaranteed to be in the range of TypeKind
        unsafe { std::mem::transmute::<u8, TypeKind>(number) }
    }

    fn instance_index(&self) -> usize {
        (self.id & 0x03FFFFFF) as usize
    }

    pub fn get(&self) -> RefType<'_, 'a> {
        // TODO: Implement this method to retrieve the expression from thread-local storage
        panic!("get() not implemented for TypeRef");
    }

    pub fn get_mut(&mut self) -> MutRefType<'_, 'a> {
        // TODO: Implement this method to retrieve the expression from thread-local storage
        panic!("get_mut() not implemented for TypeRef");
    }
}

impl<'a> Into<ExprRef<'a>> for TypeRef<'a> {
    fn into(self) -> ExprRef<'a> {
        ExprRef::new(self.variant_index().into(), self.instance_index())
            .expect("TypeRef must be convertible to ExprRef")
    }
}

impl<'a> TryInto<TypeRef<'a>> for ExprRef<'a> {
    type Error = ();

    fn try_into(self) -> Result<TypeRef<'a>, Self::Error> {
        let variant = self.variant_index().try_into()?;
        TypeRef::new(variant, self.instance_index()).ok_or(())
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
    pub(crate) fn add_expr(&mut self, expr: OwnedExpr<'a>) -> Option<ExprRef<'a>> {
        match expr {
            OwnedExpr::Bool => Some(ExprRef::new_single(ExprKind::Bool)),
            OwnedExpr::UInt8 => Some(ExprRef::new_single(ExprKind::UInt8)),
            OwnedExpr::UInt16 => Some(ExprRef::new_single(ExprKind::UInt16)),
            OwnedExpr::UInt32 => Some(ExprRef::new_single(ExprKind::UInt32)),
            OwnedExpr::UInt64 => Some(ExprRef::new_single(ExprKind::UInt64)),
            OwnedExpr::UInt128 => Some(ExprRef::new_single(ExprKind::UInt128)),
            OwnedExpr::Int8 => Some(ExprRef::new_single(ExprKind::Int8)),
            OwnedExpr::Int16 => Some(ExprRef::new_single(ExprKind::Int16)),
            OwnedExpr::Int32 => Some(ExprRef::new_single(ExprKind::Int32)),
            OwnedExpr::Int64 => Some(ExprRef::new_single(ExprKind::Int64)),
            OwnedExpr::Int128 => Some(ExprRef::new_single(ExprKind::Int128)),
            OwnedExpr::Float8 => Some(ExprRef::new_single(ExprKind::Float8)),
            OwnedExpr::Float16 => Some(ExprRef::new_single(ExprKind::Float16)),
            OwnedExpr::Float32 => Some(ExprRef::new_single(ExprKind::Float32)),
            OwnedExpr::Float64 => Some(ExprRef::new_single(ExprKind::Float64)),
            OwnedExpr::Float128 => Some(ExprRef::new_single(ExprKind::Float128)),

            OwnedExpr::InferType => Some(ExprRef::new_single(ExprKind::InferType)),

            OwnedExpr::TupleType(node) => ExprRef::new(ExprKind::TupleType, self.tuple_types.len())
                .and_then(|k| {
                    self.tuple_types.push(node);
                    Some(k)
                }),

            OwnedExpr::ArrayType(node) => ExprRef::new(ExprKind::ArrayType, self.array_types.len())
                .and_then(|k| {
                    self.array_types.push(node);
                    Some(k)
                }),

            OwnedExpr::StructType(node) => {
                ExprRef::new(ExprKind::StructType, self.struct_types.len()).and_then(|k| {
                    self.struct_types.push(node);
                    Some(k)
                })
            }

            OwnedExpr::FunctionType(node) => {
                ExprRef::new(ExprKind::FunctionType, self.function_types.len()).and_then(|k| {
                    self.function_types.push(node);
                    Some(k)
                })
            }

            OwnedExpr::Discard => Some(ExprRef::new_single(ExprKind::Discard)),

            OwnedExpr::IntegerLit(node) => ExprRef::new(ExprKind::Integer, self.integers.len())
                .and_then(|k| {
                    self.integers.push(node);
                    Some(k)
                }),

            OwnedExpr::FloatLit(node) => {
                ExprRef::new(ExprKind::Float, self.floats.len()).and_then(|k| {
                    self.floats.push(node);
                    Some(k)
                })
            }

            OwnedExpr::StringLit(node) => ExprRef::new(ExprKind::String, self.strings.len())
                .and_then(|k| {
                    self.strings.push(node);
                    Some(k)
                }),

            OwnedExpr::CharLit(node) => ExprRef::new(ExprKind::Char, self.characters.len())
                .and_then(|k| {
                    self.characters.push(node);
                    Some(k)
                }),

            OwnedExpr::ListLit(node) => {
                ExprRef::new(ExprKind::List, self.lists.len()).and_then(|k| {
                    self.lists.push(node);
                    Some(k)
                })
            }

            OwnedExpr::ObjectLit(node) => ExprRef::new(ExprKind::Object, self.objects.len())
                .and_then(|k| {
                    self.objects.push(node);
                    Some(k)
                }),

            OwnedExpr::UnaryOp(node) => ExprRef::new(ExprKind::UnaryOp, self.unary_ops.len())
                .and_then(|k| {
                    self.unary_ops.push(node);
                    Some(k)
                }),

            OwnedExpr::BinaryOp(node) => ExprRef::new(ExprKind::BinaryOp, self.binary_ops.len())
                .and_then(|k| {
                    self.binary_ops.push(node);
                    Some(k)
                }),

            OwnedExpr::Statement(node) => ExprRef::new(ExprKind::Statement, self.statements.len())
                .and_then(|k| {
                    self.statements.push(node);
                    Some(k)
                }),

            OwnedExpr::Block(node) => {
                ExprRef::new(ExprKind::Block, self.blocks.len()).and_then(|k| {
                    self.blocks.push(node);
                    Some(k)
                })
            }

            OwnedExpr::Function(node) => ExprRef::new(ExprKind::Function, self.functions.len())
                .and_then(|k| {
                    self.functions.push(node);
                    Some(k)
                }),

            OwnedExpr::Variable(node) => ExprRef::new(ExprKind::Variable, self.variables.len())
                .and_then(|k| {
                    self.variables.push(node);
                    Some(k)
                }),

            OwnedExpr::Return(node) => {
                ExprRef::new(ExprKind::Return, self.returns.len()).and_then(|k| {
                    self.returns.push(node);
                    Some(k)
                })
            }
        }
    }

    pub(crate) fn add_type(&mut self, ty: OwnedType<'a>) -> Option<TypeRef<'a>> {
        self.add_expr(ty.into()).and_then(|expr_ref| {
            /*
             * All TypeKinds have analogous ExprKinds, so we can use the same
             * variant index for both.
             */

            let variant = expr_ref
                .variant_index()
                .try_into()
                .expect("The expr was a type");
            let index = expr_ref.instance_index();

            TypeRef::new(variant, index)
        })
    }

    pub fn get_expr(&self, id: ExprRef<'a>) -> Option<RefExpr<'_, 'a>> {
        let index = id.instance_index() as usize;

        match id.variant_index() {
            ExprKind::Bool => Some(RefExpr::Bool),
            ExprKind::UInt8 => Some(RefExpr::UInt8),
            ExprKind::UInt16 => Some(RefExpr::UInt16),
            ExprKind::UInt32 => Some(RefExpr::UInt32),
            ExprKind::UInt64 => Some(RefExpr::UInt64),
            ExprKind::UInt128 => Some(RefExpr::UInt128),
            ExprKind::Int8 => Some(RefExpr::Int8),
            ExprKind::Int16 => Some(RefExpr::Int16),
            ExprKind::Int32 => Some(RefExpr::Int32),
            ExprKind::Int64 => Some(RefExpr::Int64),
            ExprKind::Int128 => Some(RefExpr::Int128),
            ExprKind::Float8 => Some(RefExpr::Float8),
            ExprKind::Float16 => Some(RefExpr::Float16),
            ExprKind::Float32 => Some(RefExpr::Float32),
            ExprKind::Float64 => Some(RefExpr::Float64),
            ExprKind::Float128 => Some(RefExpr::Float128),

            ExprKind::InferType => Some(RefExpr::InferType),
            ExprKind::TupleType => self.tuple_types.get(index).map(RefExpr::TupleType),
            ExprKind::ArrayType => self.array_types.get(index).map(RefExpr::ArrayType),
            ExprKind::StructType => self.struct_types.get(index).map(RefExpr::StructType),
            ExprKind::FunctionType => self.function_types.get(index).map(RefExpr::FunctionType),

            ExprKind::Discard => Some(RefExpr::Discard),

            ExprKind::Integer => self.integers.get(index).map(RefExpr::IntegerLit),
            ExprKind::Float => self.floats.get(index).map(RefExpr::FloatLit),
            ExprKind::String => self.strings.get(index).map(RefExpr::StringLit),
            ExprKind::Char => self.characters.get(index).map(RefExpr::CharLit),
            ExprKind::List => self.lists.get(index).map(RefExpr::ListLit),
            ExprKind::Object => self.objects.get(index).map(RefExpr::ObjectLit),

            ExprKind::UnaryOp => self.unary_ops.get(index).map(RefExpr::UnaryOp),
            ExprKind::BinaryOp => self.binary_ops.get(index).map(RefExpr::BinaryOp),
            ExprKind::Statement => self.statements.get(index).map(RefExpr::Statement),
            ExprKind::Block => self.blocks.get(index).map(RefExpr::Block),

            ExprKind::Function => self.functions.get(index).map(RefExpr::Function),
            ExprKind::Variable => self.variables.get(index).map(RefExpr::Variable),

            ExprKind::Return => self.returns.get(index).map(RefExpr::Return),
        }
    }

    pub fn get_expr_mut(&mut self, id: ExprRef<'a>) -> Option<MutRefExpr<'_, 'a>> {
        let index = id.instance_index() as usize;

        match id.variant_index() {
            ExprKind::Bool => Some(MutRefExpr::Bool),
            ExprKind::UInt8 => Some(MutRefExpr::UInt8),
            ExprKind::UInt16 => Some(MutRefExpr::UInt16),
            ExprKind::UInt32 => Some(MutRefExpr::UInt32),
            ExprKind::UInt64 => Some(MutRefExpr::UInt64),
            ExprKind::UInt128 => Some(MutRefExpr::UInt128),
            ExprKind::Int8 => Some(MutRefExpr::Int8),
            ExprKind::Int16 => Some(MutRefExpr::Int16),
            ExprKind::Int32 => Some(MutRefExpr::Int32),
            ExprKind::Int64 => Some(MutRefExpr::Int64),
            ExprKind::Int128 => Some(MutRefExpr::Int128),
            ExprKind::Float8 => Some(MutRefExpr::Float8),
            ExprKind::Float16 => Some(MutRefExpr::Float16),
            ExprKind::Float32 => Some(MutRefExpr::Float32),
            ExprKind::Float64 => Some(MutRefExpr::Float64),
            ExprKind::Float128 => Some(MutRefExpr::Float128),

            ExprKind::InferType => Some(MutRefExpr::InferType),
            ExprKind::TupleType => self.tuple_types.get_mut(index).map(MutRefExpr::TupleType),
            ExprKind::ArrayType => self.array_types.get_mut(index).map(MutRefExpr::ArrayType),
            ExprKind::StructType => self.struct_types.get_mut(index).map(MutRefExpr::StructType),
            ExprKind::FunctionType => self
                .function_types
                .get_mut(index)
                .map(MutRefExpr::FunctionType),

            ExprKind::Discard => Some(MutRefExpr::Discard),

            ExprKind::Integer => self.integers.get_mut(index).map(MutRefExpr::IntegerLit),
            ExprKind::Float => self.floats.get_mut(index).map(MutRefExpr::FloatLit),
            ExprKind::String => self.strings.get_mut(index).map(MutRefExpr::StringLit),
            ExprKind::Char => self.characters.get_mut(index).map(MutRefExpr::CharLit),
            ExprKind::List => self.lists.get_mut(index).map(MutRefExpr::ListLit),
            ExprKind::Object => self.objects.get_mut(index).map(MutRefExpr::ObjectLit),

            ExprKind::UnaryOp => self.unary_ops.get_mut(index).map(MutRefExpr::UnaryOp),
            ExprKind::BinaryOp => self.binary_ops.get_mut(index).map(MutRefExpr::BinaryOp),
            ExprKind::Statement => self.statements.get_mut(index).map(MutRefExpr::Statement),
            ExprKind::Block => self.blocks.get_mut(index).map(MutRefExpr::Block),

            ExprKind::Function => self.functions.get_mut(index).map(MutRefExpr::Function),
            ExprKind::Variable => self.variables.get_mut(index).map(MutRefExpr::Variable),

            ExprKind::Return => self.returns.get_mut(index).map(MutRefExpr::Return),
        }
    }

    pub fn get_type(&self, id: TypeRef<'a>) -> Option<RefType<'_, 'a>> {
        let index = id.instance_index() as usize;

        match id.variant_index() {
            TypeKind::Bool => Some(RefType::Bool),
            TypeKind::UInt8 => Some(RefType::UInt8),
            TypeKind::UInt16 => Some(RefType::UInt16),
            TypeKind::UInt32 => Some(RefType::UInt32),
            TypeKind::UInt64 => Some(RefType::UInt64),
            TypeKind::UInt128 => Some(RefType::UInt128),
            TypeKind::Int8 => Some(RefType::Int8),
            TypeKind::Int16 => Some(RefType::Int16),
            TypeKind::Int32 => Some(RefType::Int32),
            TypeKind::Int64 => Some(RefType::Int64),
            TypeKind::Int128 => Some(RefType::Int128),
            TypeKind::Float8 => Some(RefType::Float8),
            TypeKind::Float16 => Some(RefType::Float16),
            TypeKind::Float32 => Some(RefType::Float32),
            TypeKind::Float64 => Some(RefType::Float64),
            TypeKind::Float128 => Some(RefType::Float128),

            TypeKind::InferType => Some(RefType::InferType),
            TypeKind::TupleType => self.tuple_types.get(index).map(RefType::TupleType),
            TypeKind::ArrayType => self.array_types.get(index).map(RefType::ArrayType),
            TypeKind::StructType => self.struct_types.get(index).map(RefType::StructType),
            TypeKind::FunctionType => self.function_types.get(index).map(RefType::FunctionType),
        }
    }

    pub fn get_type_mut(&mut self, id: TypeRef<'a>) -> Option<MutRefType<'_, 'a>> {
        let index = id.instance_index() as usize;

        match id.variant_index() {
            TypeKind::Bool => Some(MutRefType::Bool),
            TypeKind::UInt8 => Some(MutRefType::UInt8),
            TypeKind::UInt16 => Some(MutRefType::UInt16),
            TypeKind::UInt32 => Some(MutRefType::UInt32),
            TypeKind::UInt64 => Some(MutRefType::UInt64),
            TypeKind::UInt128 => Some(MutRefType::UInt128),
            TypeKind::Int8 => Some(MutRefType::Int8),
            TypeKind::Int16 => Some(MutRefType::Int16),
            TypeKind::Int32 => Some(MutRefType::Int32),
            TypeKind::Int64 => Some(MutRefType::Int64),
            TypeKind::Int128 => Some(MutRefType::Int128),
            TypeKind::Float8 => Some(MutRefType::Float8),
            TypeKind::Float16 => Some(MutRefType::Float16),
            TypeKind::Float32 => Some(MutRefType::Float32),
            TypeKind::Float64 => Some(MutRefType::Float64),
            TypeKind::Float128 => Some(MutRefType::Float128),

            TypeKind::InferType => Some(MutRefType::InferType),
            TypeKind::TupleType => self.tuple_types.get_mut(index).map(MutRefType::TupleType),
            TypeKind::ArrayType => self.array_types.get_mut(index).map(MutRefType::ArrayType),
            TypeKind::StructType => self.struct_types.get_mut(index).map(MutRefType::StructType),
            TypeKind::FunctionType => self
                .function_types
                .get_mut(index)
                .map(MutRefType::FunctionType),
        }
    }
}
