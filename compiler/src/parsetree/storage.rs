use super::array_type::ArrayType;
use super::binary_op::BinaryExpr;
use super::block::Block;
use super::character::CharLit;
use super::expression::Expr;
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

#[derive(Debug, Clone, Copy)]
#[repr(u8)]
pub enum ExprKind {
    Discard,

    /* Primitive Expressions */
    Integer,
    Float,
    String,
    Char,
    List,
    Object,

    /* Compound Expressions */
    UnaryOp,
    BinaryOp,
    Statement,
    Block,

    /* Definition */
    Function,
    Variable,

    /* Control Flow */
    Return,

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
    TupleType,
    ArrayType,
    StructType,
    FunctionType,
}

#[derive(Debug, Clone, Copy)]
pub struct ExprRef<'a> {
    id: u32,
    _marker: std::marker::PhantomData<&'a ()>,
}

impl<'a> ExprRef<'a> {
    /*
     * First 6 bits are the variant index.
     * Next 26 bits are the instance index.
     */

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

// TODO: Amoritze the allocation overhead of parsing by using a vector of vectors structure

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

    pub fn add(&mut self, expr: Expr<'a>) -> Option<ExprRef> {
        match expr {
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
        }
    }

    pub fn get(&self, id: ExprRef) -> &Expr<'a> {
        let index = id.instance_index() as usize;

        panic!("Storage::get is not implemented yet, id: {:?}", id);

        // let instance = match id.variant_index() {
        //     ExprKind::Discard => {
        //         // TODO:
        //         // self.get(index)
        //         panic!("Discard variant is not implemented")
        //     }

        //     ExprKind::Integer => self.integers.get(index),
        //     ExprKind::Float => self.floats.get(index),
        //     ExprKind::String => self.strings.get(index),
        //     ExprKind::Char => self.characters.get(index),
        //     ExprKind::List => self.lists.get(index),
        //     ExprKind::Object => self.nodes.get(index),

        //     ExprKind::UnaryOp => self.unary_ops.get(index),
        //     ExprKind::BinaryOp => self.binary_ops.get(index),
        //     ExprKind::Statement => self.statements.get(index),
        //     ExprKind::Block => self.blocks.get(index),

        //     ExprKind::Function => self.functions.get(index),
        //     ExprKind::Variable => self.variables.get(index),

        //     ExprKind::Return => self.returns.get(index),

        //     ExprKind::Bool => {
        //         // TODO:
        //     }
        //     ExprKind::UInt8 => {
        //         // TODO:
        //     }
        //     ExprKind::UInt16 => {
        //         // TODO:
        //     }
        //     ExprKind::UInt32 => {
        //         // TODO:
        //     }
        //     ExprKind::UInt64 => {
        //         // TODO:
        //     }
        //     ExprKind::UInt128 => {
        //         // TODO:
        //     }
        //     ExprKind::Int8 => {
        //         // TODO:
        //     }
        //     ExprKind::Int16 => {
        //         // TODO:
        //     }
        //     ExprKind::Int32 => {
        //         // TODO:
        //     }
        //     ExprKind::Int64 => {
        //         // TODO:
        //     }
        //     ExprKind::Int128 => {
        //         // TODO:
        //     }
        //     ExprKind::Float8 => {
        //         // TODO:
        //     }
        //     ExprKind::Float16 => {
        //         // TODO:
        //     }
        //     ExprKind::Float32 => {
        //         // TODO:
        //     }
        //     ExprKind::Float64 => {
        //         // TODO:
        //     }
        //     ExprKind::Float128 => {
        //         // TODO:
        //     }

        //     ExprKind::InferType => {
        //         // TODO:
        //     }
        //     ExprKind::TupleType => self.tuple_types.get(index),
        //     ExprKind::ArrayType => self.array_types.get(index),
        //     ExprKind::StructType => self.struct_types.get(index),
        //     ExprKind::FunctionType => self.function_types.get(index),
        // };

        // instance.expect("Expression not found in Storage")
    }
}
