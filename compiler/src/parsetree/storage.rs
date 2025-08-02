use super::array_type::ArrayType;
use super::binary_op::BinaryExpr;
use super::block::Block;
use super::character::CharLit;
use super::expression::{Expr, RefExpr, RefType, Type};
use super::function::Function;
use super::function_type::FunctionType;
use super::list::List;
use super::number::{FloatLit, IntegerLit};
use super::object::Object;
use super::returns::Return;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::UnaryExpr;
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
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);

        can_store_index.then_some(ExprRef {
            id: (variant as u32) << 26 | index as u32,
            _marker: std::marker::PhantomData,
        })
    }

    fn new_single(variant: ExprKind) -> Self {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        ExprRef {
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
}

impl<'a> TypeRef<'a> {
    fn new(variant: TypeKind, index: usize) -> Option<Self> {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);

        can_store_index.then_some(TypeRef {
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
}

#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct Storage<'a> {
    integers: Vec<IntegerLit>,
    floats: Vec<FloatLit>,
    strings: Vec<StringLit<'a>>,
    characters: Vec<CharLit>,
    lists: Vec<List<'a>>,
    objects: Vec<Object<'a>>,

    unary_ops: Vec<UnaryExpr<'a>>,
    binary_ops: Vec<BinaryExpr<'a>>,
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
    pub fn new() -> Self {
        Storage {
            integers: Vec::new(),
            floats: Vec::new(),
            strings: Vec::new(),
            characters: Vec::new(),
            lists: Vec::new(),
            objects: Vec::new(),

            unary_ops: Vec::new(),
            binary_ops: Vec::new(),
            statements: Vec::new(),
            blocks: Vec::new(),

            functions: Vec::new(),
            variables: Vec::new(),

            returns: Vec::new(),

            tuple_types: Vec::new(),
            array_types: Vec::new(),
            struct_types: Vec::new(),
            function_types: Vec::new(),
        }
    }

    pub fn add_expr(&mut self, expr: Expr<'a>) -> Option<ExprRef<'a>> {
        match expr {
            Expr::Bool => Some(ExprRef::new_single(ExprKind::Bool)),
            Expr::UInt8 => Some(ExprRef::new_single(ExprKind::UInt8)),
            Expr::UInt16 => Some(ExprRef::new_single(ExprKind::UInt16)),
            Expr::UInt32 => Some(ExprRef::new_single(ExprKind::UInt32)),
            Expr::UInt64 => Some(ExprRef::new_single(ExprKind::UInt64)),
            Expr::UInt128 => Some(ExprRef::new_single(ExprKind::UInt128)),
            Expr::Int8 => Some(ExprRef::new_single(ExprKind::Int8)),
            Expr::Int16 => Some(ExprRef::new_single(ExprKind::Int16)),
            Expr::Int32 => Some(ExprRef::new_single(ExprKind::Int32)),
            Expr::Int64 => Some(ExprRef::new_single(ExprKind::Int64)),
            Expr::Int128 => Some(ExprRef::new_single(ExprKind::Int128)),
            Expr::Float8 => Some(ExprRef::new_single(ExprKind::Float8)),
            Expr::Float16 => Some(ExprRef::new_single(ExprKind::Float16)),
            Expr::Float32 => Some(ExprRef::new_single(ExprKind::Float32)),
            Expr::Float64 => Some(ExprRef::new_single(ExprKind::Float64)),
            Expr::Float128 => Some(ExprRef::new_single(ExprKind::Float128)),

            Expr::InferType => Some(ExprRef::new_single(ExprKind::InferType)),

            Expr::TupleType(node) => ExprRef::new(ExprKind::TupleType, self.tuple_types.len())
                .and_then(|k| {
                    self.tuple_types.push(node);
                    Some(k)
                }),

            Expr::ArrayType(node) => ExprRef::new(ExprKind::ArrayType, self.array_types.len())
                .and_then(|k| {
                    self.array_types.push(node);
                    Some(k)
                }),

            Expr::StructType(node) => ExprRef::new(ExprKind::StructType, self.struct_types.len())
                .and_then(|k| {
                    self.struct_types.push(node);
                    Some(k)
                }),

            Expr::FunctionType(node) => {
                ExprRef::new(ExprKind::FunctionType, self.function_types.len()).and_then(|k| {
                    self.function_types.push(node);
                    Some(k)
                })
            }

            Expr::Discard => Some(ExprRef::new_single(ExprKind::Discard)),

            Expr::Integer(node) => {
                ExprRef::new(ExprKind::Integer, self.integers.len()).and_then(|k| {
                    self.integers.push(node);
                    Some(k)
                })
            }

            Expr::Float(node) => ExprRef::new(ExprKind::Float, self.floats.len()).and_then(|k| {
                self.floats.push(node);
                Some(k)
            }),

            Expr::String(node) => {
                ExprRef::new(ExprKind::String, self.strings.len()).and_then(|k| {
                    self.strings.push(node);
                    Some(k)
                })
            }

            Expr::Char(node) => ExprRef::new(ExprKind::Char, self.characters.len()).and_then(|k| {
                self.characters.push(node);
                Some(k)
            }),

            Expr::List(node) => ExprRef::new(ExprKind::List, self.lists.len()).and_then(|k| {
                self.lists.push(node);
                Some(k)
            }),

            Expr::Object(node) => {
                ExprRef::new(ExprKind::Object, self.objects.len()).and_then(|k| {
                    self.objects.push(node);
                    Some(k)
                })
            }

            Expr::UnaryOp(node) => {
                ExprRef::new(ExprKind::UnaryOp, self.unary_ops.len()).and_then(|k| {
                    self.unary_ops.push(node);
                    Some(k)
                })
            }

            Expr::BinaryOp(node) => ExprRef::new(ExprKind::BinaryOp, self.binary_ops.len())
                .and_then(|k| {
                    self.binary_ops.push(node);
                    Some(k)
                }),

            Expr::Statement(node) => ExprRef::new(ExprKind::Statement, self.statements.len())
                .and_then(|k| {
                    self.statements.push(node);
                    Some(k)
                }),

            Expr::Block(node) => ExprRef::new(ExprKind::Block, self.blocks.len()).and_then(|k| {
                self.blocks.push(node);
                Some(k)
            }),

            Expr::Function(node) => ExprRef::new(ExprKind::Function, self.functions.len())
                .and_then(|k| {
                    self.functions.push(node);
                    Some(k)
                }),

            Expr::Variable(node) => ExprRef::new(ExprKind::Variable, self.variables.len())
                .and_then(|k| {
                    self.variables.push(node);
                    Some(k)
                }),

            Expr::Return(node) => {
                ExprRef::new(ExprKind::Return, self.returns.len()).and_then(|k| {
                    self.returns.push(node);
                    Some(k)
                })
            }
        }
    }

    pub fn add_type(&mut self, ty: Type<'a>) -> Option<TypeRef<'a>> {
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

            ExprKind::Integer => self.integers.get(index).map(RefExpr::Integer),
            ExprKind::Float => self.floats.get(index).map(RefExpr::Float),
            ExprKind::String => self.strings.get(index).map(RefExpr::String),
            ExprKind::Char => self.characters.get(index).map(RefExpr::Char),
            ExprKind::List => self.lists.get(index).map(RefExpr::List),
            ExprKind::Object => self.objects.get(index).map(RefExpr::Object),

            ExprKind::UnaryOp => self.unary_ops.get(index).map(RefExpr::UnaryOp),
            ExprKind::BinaryOp => self.binary_ops.get(index).map(RefExpr::BinaryOp),
            ExprKind::Statement => self.statements.get(index).map(RefExpr::Statement),
            ExprKind::Block => self.blocks.get(index).map(RefExpr::Block),

            ExprKind::Function => self.functions.get(index).map(RefExpr::Function),
            ExprKind::Variable => self.variables.get(index).map(RefExpr::Variable),

            ExprKind::Return => self.returns.get(index).map(RefExpr::Return),
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
}
