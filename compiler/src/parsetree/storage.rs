use hashbrown::HashSet;

use super::array_type::ArrayType;
use super::binary_op::BinaryOp;
use super::block::Block;
use super::character::CharLit;
use super::expression::{ExprKind, ExprOwned, ExprRef, ExprRefMut, TypeKind, TypeOwned, TypeRef};
use super::function::Function;
use super::function_type::FunctionType;
use super::list::ListLit;
use super::map_type::MapType;
use super::number::{FloatLit, IntegerLit};
use super::object::ObjectLit;
use super::refinement_type::RefinementType;
use super::returns::Return;
use super::slice_type::SliceType;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::UnaryOp;
use super::variable::Variable;
use std::num::NonZeroU32;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct ExprKey<'a> {
    id: NonZeroU32,
    _marker: std::marker::PhantomData<&'a ()>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct TypeKey<'a> {
    id: NonZeroU32,
    _marker: std::marker::PhantomData<&'a ()>,
}

impl<'a> ExprKey<'a> {
    pub(crate) fn new(variant: ExprKind, index: usize) -> Option<Self> {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);
        let variant_bits = (variant as u32 + 1) << 26;

        can_store_index.then_some(ExprKey {
            id: NonZeroU32::new(variant_bits | index as u32).expect("ID must be non-zero"),
            _marker: std::marker::PhantomData,
        })
    }

    pub(crate) fn new_single(variant: ExprKind) -> Self {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let variant_bits = (variant as u32 + 1) << 26;

        ExprKey {
            id: NonZeroU32::new(variant_bits).expect("ID must be non-zero"),
            _marker: std::marker::PhantomData,
        }
    }

    pub(crate) fn variant_index(&self) -> ExprKind {
        let number = ((self.id.get() >> 26) as u8) - 1;

        match number {
            x if x == ExprKind::Bool as u8 => ExprKind::Bool,
            x if x == ExprKind::UInt8 as u8 => ExprKind::UInt8,
            x if x == ExprKind::UInt16 as u8 => ExprKind::UInt16,
            x if x == ExprKind::UInt32 as u8 => ExprKind::UInt32,
            x if x == ExprKind::UInt64 as u8 => ExprKind::UInt64,
            x if x == ExprKind::UInt128 as u8 => ExprKind::UInt128,
            x if x == ExprKind::Int8 as u8 => ExprKind::Int8,
            x if x == ExprKind::Int16 as u8 => ExprKind::Int16,
            x if x == ExprKind::Int32 as u8 => ExprKind::Int32,
            x if x == ExprKind::Int64 as u8 => ExprKind::Int64,
            x if x == ExprKind::Int128 as u8 => ExprKind::Int128,
            x if x == ExprKind::Float8 as u8 => ExprKind::Float8,
            x if x == ExprKind::Float16 as u8 => ExprKind::Float16,
            x if x == ExprKind::Float32 as u8 => ExprKind::Float32,
            x if x == ExprKind::Float64 as u8 => ExprKind::Float64,
            x if x == ExprKind::Float128 as u8 => ExprKind::Float128,

            x if x == ExprKind::InferType as u8 => ExprKind::InferType,
            x if x == ExprKind::TypeName as u8 => ExprKind::TypeName,
            x if x == ExprKind::RefinementType as u8 => ExprKind::RefinementType,
            x if x == ExprKind::TupleType as u8 => ExprKind::TupleType,
            x if x == ExprKind::ArrayType as u8 => ExprKind::ArrayType,
            x if x == ExprKind::MapType as u8 => ExprKind::MapType,
            x if x == ExprKind::SliceType as u8 => ExprKind::SliceType,
            x if x == ExprKind::StructType as u8 => ExprKind::StructType,
            x if x == ExprKind::FunctionType as u8 => ExprKind::FunctionType,

            x if x == ExprKind::Discard as u8 => ExprKind::Discard,

            x if x == ExprKind::IntegerLit as u8 => ExprKind::IntegerLit,
            x if x == ExprKind::FloatLit as u8 => ExprKind::FloatLit,
            x if x == ExprKind::StringLit as u8 => ExprKind::StringLit,
            x if x == ExprKind::CharLit as u8 => ExprKind::CharLit,
            x if x == ExprKind::ListLit as u8 => ExprKind::ListLit,
            x if x == ExprKind::ObjectLit as u8 => ExprKind::ObjectLit,

            x if x == ExprKind::UnaryOp as u8 => ExprKind::UnaryOp,
            x if x == ExprKind::BinaryOp as u8 => ExprKind::BinaryOp,
            x if x == ExprKind::Statement as u8 => ExprKind::Statement,
            x if x == ExprKind::Block as u8 => ExprKind::Block,

            x if x == ExprKind::Function as u8 => ExprKind::Function,
            x if x == ExprKind::Variable as u8 => ExprKind::Variable,

            x if x == ExprKind::Return as u8 => ExprKind::Return,

            _ => unreachable!(),
        }
    }

    fn instance_index(&self) -> usize {
        (self.id.get() & 0x03FFFFFF) as usize
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
    pub(crate) fn new(variant: TypeKind, index: usize) -> Option<Self> {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);
        let variant_bits = (variant as u32 + 1) << 26;

        can_store_index.then_some(TypeKey {
            id: NonZeroU32::new(variant_bits | index as u32).expect("ID must be non-zero"),
            _marker: std::marker::PhantomData,
        })
    }

    pub(crate) fn new_single(variant: TypeKind) -> Self {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let variant_bits = (variant as u32 + 1) << 26;

        TypeKey {
            id: NonZeroU32::new(variant_bits).expect("ID must be non-zero"),
            _marker: std::marker::PhantomData,
        }
    }

    pub(crate) fn variant_index(&self) -> TypeKind {
        let number = ((self.id.get() >> 26) as u8) - 1;

        match number {
            x if x == TypeKind::Bool as u8 => TypeKind::Bool,
            x if x == TypeKind::UInt8 as u8 => TypeKind::UInt8,
            x if x == TypeKind::UInt16 as u8 => TypeKind::UInt16,
            x if x == TypeKind::UInt32 as u8 => TypeKind::UInt32,
            x if x == TypeKind::UInt64 as u8 => TypeKind::UInt64,
            x if x == TypeKind::UInt128 as u8 => TypeKind::UInt128,
            x if x == TypeKind::Int8 as u8 => TypeKind::Int8,
            x if x == TypeKind::Int16 as u8 => TypeKind::Int16,
            x if x == TypeKind::Int32 as u8 => TypeKind::Int32,
            x if x == TypeKind::Int64 as u8 => TypeKind::Int64,
            x if x == TypeKind::Int128 as u8 => TypeKind::Int128,
            x if x == TypeKind::Float8 as u8 => TypeKind::Float8,
            x if x == TypeKind::Float16 as u8 => TypeKind::Float16,
            x if x == TypeKind::Float32 as u8 => TypeKind::Float32,
            x if x == TypeKind::Float64 as u8 => TypeKind::Float64,
            x if x == TypeKind::Float128 as u8 => TypeKind::Float128,

            x if x == TypeKind::InferType as u8 => TypeKind::InferType,
            x if x == TypeKind::TypeName as u8 => TypeKind::TypeName,
            x if x == TypeKind::RefinementType as u8 => TypeKind::RefinementType,
            x if x == TypeKind::TupleType as u8 => TypeKind::TupleType,
            x if x == TypeKind::ArrayType as u8 => TypeKind::ArrayType,
            x if x == TypeKind::MapType as u8 => TypeKind::MapType,
            x if x == TypeKind::SliceType as u8 => TypeKind::SliceType,
            x if x == TypeKind::StructType as u8 => TypeKind::StructType,
            x if x == TypeKind::FunctionType as u8 => TypeKind::FunctionType,

            _ => unreachable!(),
        }
    }

    fn instance_index(&self) -> usize {
        (self.id.get() & 0x03FFFFFF) as usize
    }

    pub fn get<'storage>(&self, storage: &'storage Storage<'a>) -> TypeRef<'storage, 'a> {
        storage.get_type(*self)
    }
}

impl<'a> std::fmt::Display for ExprKey<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}({})", self.variant_index(), self.instance_index())
    }
}

impl<'a> std::fmt::Display for TypeKey<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}({})", self.variant_index(), self.instance_index())
    }
}

impl<'a> TryInto<TypeKey<'a>> for ExprKey<'a> {
    type Error = ();

    fn try_into(self) -> Result<TypeKey<'a>, Self::Error> {
        let variant = self.variant_index().try_into()?;
        TypeKey::new(variant, self.instance_index()).ok_or(())
    }
}

impl<'a> Into<ExprKey<'a>> for TypeKey<'a> {
    fn into(self) -> ExprKey<'a> {
        ExprKey::new(self.variant_index().into(), self.instance_index())
            .expect("TypeRef should be convertible to ExprRef")
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
            | ExprKind::TypeName
            | ExprKind::RefinementType
            | ExprKind::TupleType
            | ExprKind::ArrayType
            | ExprKind::MapType
            | ExprKind::SliceType
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

    pub fn has_parentheses(&self, storage: &Storage<'a>) -> bool {
        storage.has_parentheses(*self)
    }

    pub fn add_parentheses(&self, storage: &mut Storage<'a>) {
        storage.add_parentheses(*self)
    }
}

impl<'a> TypeKey<'a> {
    pub fn has_parentheses(&self, storage: &Storage<'a>) -> bool {
        storage.has_parentheses(self.to_owned().into())
    }

    pub fn add_parentheses(&self, storage: &mut Storage<'a>) {
        storage.add_parentheses(self.to_owned().into())
    }
}

#[derive(Debug, Clone)]
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

    type_names: Vec<&'a str>,
    refinement_types: Vec<RefinementType<'a>>,
    tuple_types: Vec<TupleType<'a>>,
    array_types: Vec<ArrayType<'a>>,
    map_types: Vec<MapType<'a>>,
    slice_types: Vec<SliceType<'a>>,
    struct_types: Vec<StructType<'a>>,
    function_types: Vec<FunctionType<'a>>,

    has_parentheses: HashSet<ExprKey<'a>>,
}

impl<'a> Storage<'a> {
    const TUPLE_TYPE_UNIT_INDEX: usize = 0;
    const FUNCTION_TYPE_BASIC_INDEX: usize = 0;
    const STRING_LIT_EMPTY_INDEX: usize = 0;
    const CHAR_LIT_SPACE_INDEX: usize = 0;
    const CHAR_LIT_NEWLINE_INDEX: usize = 1;
    const CHAR_LIT_TAB_INDEX: usize = 2;

    pub fn new() -> Self {
        let mut storage = Storage {
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

            type_names: Vec::new(),
            refinement_types: Vec::new(),
            tuple_types: Vec::new(),
            array_types: Vec::new(),
            map_types: Vec::new(),
            slice_types: Vec::new(),
            struct_types: Vec::new(),
            function_types: Vec::new(),

            has_parentheses: HashSet::new(),
        };

        {
            let empty_tuple = (Self::TUPLE_TYPE_UNIT_INDEX, TupleType::new(Vec::new()));
            storage.tuple_types.insert(empty_tuple.0, empty_tuple.1);
        }

        {
            let basic_fn_ty = (
                Self::FUNCTION_TYPE_BASIC_INDEX,
                FunctionType::new(Vec::new(), None, Vec::new()),
            );
            storage.function_types.insert(basic_fn_ty.0, basic_fn_ty.1);
        }

        {
            let empty_string = (Self::STRING_LIT_EMPTY_INDEX, StringLit::new(""));
            storage.strings.insert(empty_string.0, empty_string.1);
        }

        {
            let space_char = (Self::CHAR_LIT_SPACE_INDEX, CharLit::new(' '));
            let newline_char = (Self::CHAR_LIT_NEWLINE_INDEX, CharLit::new('\n'));
            let tab_char = (Self::CHAR_LIT_TAB_INDEX, CharLit::new('\t'));

            storage.characters.insert(space_char.0, space_char.1);
            storage.characters.insert(newline_char.0, newline_char.1);
            storage.characters.insert(tab_char.0, tab_char.1);
        }

        storage
    }

    pub fn reserve(&mut self, kind: ExprKind, additional: usize) {
        match kind {
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
            | ExprKind::InferType => {}

            ExprKind::TypeName => self.type_names.reserve(additional),
            ExprKind::RefinementType => self.refinement_types.reserve(additional),
            ExprKind::TupleType => self.tuple_types.reserve(additional),
            ExprKind::ArrayType => self.array_types.reserve(additional),
            ExprKind::MapType => self.map_types.reserve(additional),
            ExprKind::SliceType => self.slice_types.reserve(additional),
            ExprKind::StructType => self.struct_types.reserve(additional),
            ExprKind::FunctionType => self.function_types.reserve(additional),

            ExprKind::Discard => {}

            ExprKind::IntegerLit => self.integers.reserve(additional),
            ExprKind::FloatLit => self.floats.reserve(additional),
            ExprKind::StringLit => self.strings.reserve(additional),
            ExprKind::CharLit => self.characters.reserve(additional),
            ExprKind::ListLit => self.lists.reserve(additional),
            ExprKind::ObjectLit => self.objects.reserve(additional),

            ExprKind::UnaryOp => self.unary_ops.reserve(additional),
            ExprKind::BinaryOp => self.binary_ops.reserve(additional),
            ExprKind::Statement => self.statements.reserve(additional),
            ExprKind::Block => self.blocks.reserve(additional),

            ExprKind::Function => self.functions.reserve(additional),
            ExprKind::Variable => self.variables.reserve(additional),

            ExprKind::Return => self.returns.reserve(additional),
        }
    }

    pub(crate) fn add_expr(&mut self, expr: ExprOwned<'a>) -> Option<ExprKey<'a>> {
        match expr {
            ExprOwned::Bool
            | ExprOwned::UInt8
            | ExprOwned::UInt16
            | ExprOwned::UInt32
            | ExprOwned::UInt64
            | ExprOwned::UInt128
            | ExprOwned::Int8
            | ExprOwned::Int16
            | ExprOwned::Int32
            | ExprOwned::Int64
            | ExprOwned::Int128
            | ExprOwned::Float8
            | ExprOwned::Float16
            | ExprOwned::Float32
            | ExprOwned::Float64
            | ExprOwned::Float128
            | ExprOwned::InferType
            | ExprOwned::TypeName(_)
            | ExprOwned::RefinementType(_)
            | ExprOwned::TupleType(_)
            | ExprOwned::ArrayType(_)
            | ExprOwned::MapType(_)
            | ExprOwned::SliceType(_)
            | ExprOwned::StructType(_)
            | ExprOwned::FunctionType(_) => self
                .add_type(expr.try_into().expect("Expected a type node"))
                .map(|key| key.into()),

            ExprOwned::Discard => Some(ExprKey::new_single(ExprKind::Discard)),

            ExprOwned::IntegerLit(node) => match node.try_to_u128() {
                _ => ExprKey::new(ExprKind::IntegerLit, self.integers.len()).and_then(|k| {
                    self.integers.push(node);
                    Some(k)
                }),
            },

            ExprOwned::FloatLit(node) => ExprKey::new(ExprKind::FloatLit, self.floats.len())
                .and_then(|k| {
                    self.floats.push(node);
                    Some(k)
                }),

            ExprOwned::StringLit(node) => match node.get() {
                "" => ExprKey::new(ExprKind::StringLit, Self::STRING_LIT_EMPTY_INDEX),

                _ => ExprKey::new(ExprKind::StringLit, self.strings.len()).and_then(|k| {
                    self.strings.push(node);
                    Some(k)
                }),
            },

            ExprOwned::CharLit(node) => match node.get() {
                ' ' => ExprKey::new(ExprKind::CharLit, Self::CHAR_LIT_SPACE_INDEX),
                '\n' => ExprKey::new(ExprKind::CharLit, Self::CHAR_LIT_NEWLINE_INDEX),
                '\t' => ExprKey::new(ExprKind::CharLit, Self::CHAR_LIT_TAB_INDEX),

                _ => ExprKey::new(ExprKind::CharLit, self.characters.len()).and_then(|k| {
                    self.characters.push(node);
                    Some(k)
                }),
            },

            ExprOwned::ListLit(node) => {
                ExprKey::new(ExprKind::ListLit, self.lists.len()).and_then(|k| {
                    self.lists.push(node);
                    Some(k)
                })
            }

            ExprOwned::ObjectLit(node) => ExprKey::new(ExprKind::ObjectLit, self.objects.len())
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
        match ty {
            TypeOwned::Bool => Some(TypeKey::new_single(TypeKind::Bool)),
            TypeOwned::UInt8 => Some(TypeKey::new_single(TypeKind::UInt8)),
            TypeOwned::UInt16 => Some(TypeKey::new_single(TypeKind::UInt16)),
            TypeOwned::UInt32 => Some(TypeKey::new_single(TypeKind::UInt32)),
            TypeOwned::UInt64 => Some(TypeKey::new_single(TypeKind::UInt64)),
            TypeOwned::UInt128 => Some(TypeKey::new_single(TypeKind::UInt128)),
            TypeOwned::Int8 => Some(TypeKey::new_single(TypeKind::Int8)),
            TypeOwned::Int16 => Some(TypeKey::new_single(TypeKind::Int16)),
            TypeOwned::Int32 => Some(TypeKey::new_single(TypeKind::Int32)),
            TypeOwned::Int64 => Some(TypeKey::new_single(TypeKind::Int64)),
            TypeOwned::Int128 => Some(TypeKey::new_single(TypeKind::Int128)),
            TypeOwned::Float8 => Some(TypeKey::new_single(TypeKind::Float8)),
            TypeOwned::Float16 => Some(TypeKey::new_single(TypeKind::Float16)),
            TypeOwned::Float32 => Some(TypeKey::new_single(TypeKind::Float32)),
            TypeOwned::Float64 => Some(TypeKey::new_single(TypeKind::Float64)),
            TypeOwned::Float128 => Some(TypeKey::new_single(TypeKind::Float128)),

            TypeOwned::InferType => Some(TypeKey::new_single(TypeKind::InferType)),

            TypeOwned::TypeName(node) => TypeKey::new(TypeKind::TypeName, self.type_names.len())
                .and_then(|k| {
                    self.type_names.push(node);
                    Some(k)
                }),

            TypeOwned::RefinementType(node) => {
                TypeKey::new(TypeKind::RefinementType, self.refinement_types.len()).and_then(|k| {
                    self.refinement_types.push(node);
                    Some(k)
                })
            }

            TypeOwned::TupleType(node) => {
                let is_unit_type = node.elements().is_empty();

                if is_unit_type {
                    TypeKey::new(TypeKind::TupleType, Self::TUPLE_TYPE_UNIT_INDEX)
                } else {
                    TypeKey::new(TypeKind::TupleType, self.tuple_types.len()).and_then(|k| {
                        self.tuple_types.push(node);
                        Some(k)
                    })
                }
            }

            TypeOwned::ArrayType(node) => TypeKey::new(TypeKind::ArrayType, self.array_types.len())
                .and_then(|k| {
                    self.array_types.push(node);
                    Some(k)
                }),

            TypeOwned::MapType(node) => TypeKey::new(TypeKind::MapType, self.map_types.len())
                .and_then(|k| {
                    self.map_types.push(node);
                    Some(k)
                }),

            TypeOwned::SliceType(node) => TypeKey::new(TypeKind::SliceType, self.slice_types.len())
                .and_then(|k| {
                    self.slice_types.push(node);
                    Some(k)
                }),

            TypeOwned::StructType(node) => {
                TypeKey::new(TypeKind::StructType, self.struct_types.len()).and_then(|k| {
                    self.struct_types.push(node);
                    Some(k)
                })
            }

            TypeOwned::FunctionType(node) => {
                let is_trivial_callback = node.parameters().is_empty()
                    && node.return_type().is_none()
                    && node.attributes().is_empty();

                if is_trivial_callback {
                    TypeKey::new(TypeKind::FunctionType, Self::FUNCTION_TYPE_BASIC_INDEX)
                } else {
                    TypeKey::new(TypeKind::FunctionType, self.function_types.len()).and_then(|k| {
                        self.function_types.push(node);
                        Some(k)
                    })
                }
            }
        }
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
            ExprKind::TypeName => self
                .type_names
                .get(index)
                .map(|name| ExprRef::TypeName(name)),
            ExprKind::RefinementType => self
                .refinement_types
                .get(index)
                .map(ExprRef::RefinementType),
            ExprKind::TupleType => self.tuple_types.get(index).map(ExprRef::TupleType),
            ExprKind::ArrayType => self.array_types.get(index).map(ExprRef::ArrayType),
            ExprKind::MapType => self.map_types.get(index).map(ExprRef::MapType),
            ExprKind::SliceType => self.slice_types.get(index).map(ExprRef::SliceType),
            ExprKind::StructType => self.struct_types.get(index).map(ExprRef::StructType),
            ExprKind::FunctionType => self.function_types.get(index).map(ExprRef::FunctionType),

            ExprKind::Discard => Some(ExprRef::Discard),

            ExprKind::IntegerLit => self.integers.get(index).map(ExprRef::IntegerLit),
            ExprKind::FloatLit => self.floats.get(index).map(ExprRef::FloatLit),
            ExprKind::StringLit => self.strings.get(index).map(ExprRef::StringLit),
            ExprKind::CharLit => self.characters.get(index).map(ExprRef::CharLit),
            ExprKind::ListLit => self.lists.get(index).map(ExprRef::ListLit),
            ExprKind::ObjectLit => self.objects.get(index).map(ExprRef::ObjectLit),

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
            ExprKind::TypeName => self
                .type_names
                .get(index)
                .map(|name| ExprRefMut::TypeName(name)),
            ExprKind::RefinementType => self
                .refinement_types
                .get_mut(index)
                .map(ExprRefMut::RefinementType),
            ExprKind::TupleType => self.tuple_types.get_mut(index).map(ExprRefMut::TupleType),
            ExprKind::ArrayType => self.array_types.get_mut(index).map(ExprRefMut::ArrayType),
            ExprKind::MapType => self.map_types.get_mut(index).map(ExprRefMut::MapType),
            ExprKind::SliceType => self.slice_types.get_mut(index).map(ExprRefMut::SliceType),
            ExprKind::StructType => self.struct_types.get_mut(index).map(ExprRefMut::StructType),
            ExprKind::FunctionType => self
                .function_types
                .get_mut(index)
                .map(ExprRefMut::FunctionType),

            ExprKind::Discard => Some(ExprRefMut::Discard),

            ExprKind::IntegerLit => self.integers.get_mut(index).map(ExprRefMut::IntegerLit),
            ExprKind::FloatLit => self.floats.get_mut(index).map(ExprRefMut::FloatLit),
            ExprKind::StringLit => self.strings.get_mut(index).map(ExprRefMut::StringLit),
            ExprKind::CharLit => self.characters.get_mut(index).map(ExprRefMut::CharLit),
            ExprKind::ListLit => self.lists.get_mut(index).map(ExprRefMut::ListLit),
            ExprKind::ObjectLit => self.objects.get_mut(index).map(ExprRefMut::ObjectLit),

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
            TypeKind::TypeName => self
                .type_names
                .get(index)
                .map(|name| TypeRef::TypeName(name)),
            TypeKind::RefinementType => self
                .refinement_types
                .get(index)
                .map(TypeRef::RefinementType),
            TypeKind::TupleType => self.tuple_types.get(index).map(TypeRef::TupleType),
            TypeKind::ArrayType => self.array_types.get(index).map(TypeRef::ArrayType),
            TypeKind::MapType => self.map_types.get(index).map(TypeRef::MapType),
            TypeKind::SliceType => self.slice_types.get(index).map(TypeRef::SliceType),
            TypeKind::StructType => self.struct_types.get(index).map(TypeRef::StructType),
            TypeKind::FunctionType => self.function_types.get(index).map(TypeRef::FunctionType),
        }
        .expect("Expression not found in storage")
    }

    pub fn has_parentheses(&self, key: ExprKey<'a>) -> bool {
        self.has_parentheses.contains(&key)
    }

    pub fn add_parentheses(&mut self, key: ExprKey<'a>) {
        self.has_parentheses.insert(key);
    }
}
