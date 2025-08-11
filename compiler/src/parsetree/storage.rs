use crate::lexer::{BStringData, StringData};

use super::bin_expr::BinExpr;
use super::block::Block;
use super::control_flow::{
    Assert, Await, Break, Continue, DoWhileLoop, ForEach, If, Return, Switch, WhileLoop,
};
use super::expression::{ExprKind, ExprOwned, ExprRef, ExprRefMut, TypeKind, TypeOwned};
use super::function::Function;
use super::list::ListLit;
use super::number::IntegerLit;
use super::object::ObjectLit;
use super::statement::Statement;
use super::unary_expr::UnaryExpr;
use super::variable::Variable;
use bimap::BiMap;
use hashbrown::HashSet;
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
            id: NonZeroU32::new(variant_bits | index as u64 as u32).expect("ID must be non-zero"),
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

    pub(crate) fn variant_index(self) -> ExprKind {
        let number = ((self.id.get() >> 26) as u8) - 1;
        ExprKind::try_from(number).expect("Invalid variant index")
    }

    fn instance_index(self) -> usize {
        (self.id.get() & 0x03FFFFFF) as usize
    }

    pub fn get<'storage>(self, storage: &'storage Storage<'a>) -> ExprRef<'storage, 'a> {
        storage.get_expr(self)
    }

    pub fn get_mut<'storage>(self, storage: &'storage mut Storage<'a>) -> ExprRefMut<'storage, 'a> {
        storage.get_expr_mut(self)
    }
}

impl<'a> TypeKey<'a> {
    pub(crate) fn new(variant: TypeKind, index: usize) -> Option<Self> {
        assert!((variant as u32) < 64, "Variant index must be less than 64");

        let can_store_index = index < (1 << 26);
        let variant_bits = (variant as u32 + 1) << 26;

        can_store_index.then_some(TypeKey {
            id: NonZeroU32::new(variant_bits | index as u64 as u32).expect("ID must be non-zero"),
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

    pub(crate) fn variant_index(self) -> TypeKind {
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
            x if x == TypeKind::FunctionType as u8 => TypeKind::FunctionType,
            x if x == TypeKind::ManagedRefType as u8 => TypeKind::ManagedRefType,
            x if x == TypeKind::UnmanagedRefType as u8 => TypeKind::UnmanagedRefType,
            x if x == TypeKind::GenericType as u8 => TypeKind::GenericType,
            x if x == TypeKind::OpaqueType as u8 => TypeKind::OpaqueType,

            _ => unreachable!(),
        }
    }

    fn instance_index(self) -> usize {
        (self.id.get() & 0x03FFFFFF) as usize
    }

    pub fn get<'storage>(self, storage: &'storage Storage<'a>) -> &'storage TypeOwned<'a> {
        storage.get_type(self)
    }
}

impl std::fmt::Display for ExprKey<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}({})", self.variant_index(), self.instance_index())
    }
}

impl std::fmt::Display for TypeKey<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{:?}({})", self.variant_index(), self.instance_index())
    }
}

impl<'a> TryInto<TypeKey<'a>> for ExprKey<'a> {
    type Error = ();

    fn try_into(self) -> Result<TypeKey<'a>, Self::Error> {
        let variant = self.variant_index().try_into()?;
        TypeKey::new(variant, self.instance_index()).ok_or(())
    }
}

impl<'a> From<TypeKey<'a>> for ExprKey<'a> {
    fn from(value: TypeKey<'a>) -> Self {
        ExprKey::new(value.variant_index().into(), value.instance_index())
            .expect("TypeRef should be convertible to ExprRef")
    }
}

impl<'a> ExprKey<'a> {
    pub fn is_discard(self) -> bool {
        self.variant_index() == ExprKind::Discard
    }

    pub fn discard(&mut self) {
        *self = ExprKey::new_single(ExprKind::Discard);
    }

    pub fn is_type(self) -> bool {
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
            | ExprKind::FunctionType
            | ExprKind::ManagedRefType
            | ExprKind::UnmanagedRefType
            | ExprKind::GenericType
            | ExprKind::OpaqueType => true,

            ExprKind::Discard
            | ExprKind::IntegerLit
            | ExprKind::FloatLit
            | ExprKind::StringLit
            | ExprKind::BStringLit
            | ExprKind::CharLit
            | ExprKind::ListLit
            | ExprKind::ObjectLit
            | ExprKind::UnaryExpr
            | ExprKind::BinExpr
            | ExprKind::Statement
            | ExprKind::Block
            | ExprKind::Function
            | ExprKind::Variable
            | ExprKind::Identifier
            | ExprKind::If
            | ExprKind::WhileLoop
            | ExprKind::DoWhileLoop
            | ExprKind::Switch
            | ExprKind::Break
            | ExprKind::Continue
            | ExprKind::Return
            | ExprKind::ForEach
            | ExprKind::Await
            | ExprKind::Assert => false,
        }
    }

    pub fn has_parentheses(&self, storage: &Storage<'a>) -> bool {
        storage.has_parentheses(*self)
    }

    pub fn add_parentheses(self, storage: &mut Storage<'a>) {
        storage.add_parentheses(self)
    }
}

impl<'a> TypeKey<'a> {
    pub fn has_parentheses(self, storage: &Storage<'a>) -> bool {
        storage.has_parentheses(self.to_owned().into())
    }

    pub fn add_parentheses(self, storage: &mut Storage<'a>) {
        storage.add_parentheses(self.to_owned().into())
    }
}

#[derive(Debug, Clone)]
pub struct Storage<'a> {
    integers: Vec<IntegerLit>,
    floats: Vec<f64>,
    strings: Vec<StringData<'a>>,
    binaries: Vec<BStringData<'a>>,
    lists: Vec<ListLit<'a>>,
    objects: Vec<ObjectLit<'a>>,

    unary_exprs: Vec<UnaryExpr<'a>>,
    bin_exprs: Vec<BinExpr<'a>>,
    statements: Vec<Statement<'a>>,
    blocks: Vec<Block<'a>>,

    functions: Vec<Function<'a>>,
    variables: Vec<Variable<'a>>,
    identifiers: Vec<&'a str>,

    ifs: Vec<If<'a>>,
    while_loops: Vec<WhileLoop<'a>>,
    do_while_loops: Vec<DoWhileLoop<'a>>,
    switches: Vec<Switch<'a>>,
    breaks: Vec<Break<'a>>,
    continues: Vec<Continue<'a>>,
    returns: Vec<Return<'a>>,
    for_eachs: Vec<ForEach<'a>>,
    awaits: Vec<Await<'a>>,
    asserts: Vec<Assert<'a>>,

    dedup_types: BiMap<TypeKey<'a>, TypeOwned<'a>>,

    has_parentheses: HashSet<ExprKey<'a>>,
}

impl<'a> Default for Storage<'a> {
    fn default() -> Self {
        Self::new()
    }
}

impl<'a> Storage<'a> {
    const STRING_LIT_EMPTY_INDEX: usize = 0;

    pub fn new() -> Self {
        let mut storage = Storage {
            integers: Vec::new(),
            floats: Vec::new(),
            strings: Vec::new(),
            binaries: Vec::new(),
            lists: Vec::new(),
            objects: Vec::new(),

            unary_exprs: Vec::new(),
            bin_exprs: Vec::new(),
            statements: Vec::new(),
            blocks: Vec::new(),

            functions: Vec::new(),
            variables: Vec::new(),
            identifiers: Vec::new(),

            ifs: Vec::new(),
            while_loops: Vec::new(),
            do_while_loops: Vec::new(),
            switches: Vec::new(),
            breaks: Vec::new(),
            continues: Vec::new(),
            returns: Vec::new(),
            for_eachs: Vec::new(),
            awaits: Vec::new(),
            asserts: Vec::new(),

            dedup_types: BiMap::new(),

            has_parentheses: HashSet::new(),
        };

        {
            let empty_string = (Self::STRING_LIT_EMPTY_INDEX, StringData::from_ref(""));
            storage.strings.insert(empty_string.0, empty_string.1);
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
            | ExprKind::InferType
            | ExprKind::TypeName
            | ExprKind::RefinementType
            | ExprKind::TupleType
            | ExprKind::ArrayType
            | ExprKind::MapType
            | ExprKind::SliceType
            | ExprKind::FunctionType
            | ExprKind::ManagedRefType
            | ExprKind::UnmanagedRefType
            | ExprKind::GenericType
            | ExprKind::OpaqueType
            | ExprKind::Discard
            | ExprKind::CharLit => {}

            ExprKind::IntegerLit => self.integers.reserve(additional),
            ExprKind::FloatLit => self.floats.reserve(additional),
            ExprKind::StringLit => self.strings.reserve(additional),
            ExprKind::BStringLit => self.binaries.reserve(additional),
            ExprKind::ListLit => self.lists.reserve(additional),
            ExprKind::ObjectLit => self.objects.reserve(additional),

            ExprKind::UnaryExpr => self.unary_exprs.reserve(additional),
            ExprKind::BinExpr => self.bin_exprs.reserve(additional),
            ExprKind::Statement => self.statements.reserve(additional),
            ExprKind::Block => self.blocks.reserve(additional),

            ExprKind::Function => self.functions.reserve(additional),
            ExprKind::Variable => self.variables.reserve(additional),
            ExprKind::Identifier => self.identifiers.reserve(additional),

            ExprKind::If => self.ifs.reserve(additional),
            ExprKind::WhileLoop => self.while_loops.reserve(additional),
            ExprKind::DoWhileLoop => self.do_while_loops.reserve(additional),
            ExprKind::Switch => self.switches.reserve(additional),
            ExprKind::Break => self.breaks.reserve(additional),
            ExprKind::Continue => self.continues.reserve(additional),
            ExprKind::Return => self.returns.reserve(additional),
            ExprKind::ForEach => self.for_eachs.reserve(additional),
            ExprKind::Await => self.awaits.reserve(additional),
            ExprKind::Assert => self.asserts.reserve(additional),
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
            | ExprOwned::FunctionType(_)
            | ExprOwned::ManagedRefType(_)
            | ExprOwned::UnmanagedRefType(_)
            | ExprOwned::GenericType(_)
            | ExprOwned::OpaqueType(_) => self
                .add_type(expr.try_into().expect("Expected a type node"))
                .map(|key| key.into()),

            ExprOwned::Discard => Some(ExprKey::new_single(ExprKind::Discard)),

            ExprOwned::IntegerLit(node) => ExprKey::new(ExprKind::IntegerLit, self.integers.len())
                .inspect(|_| {
                    self.integers.push(node);
                }),

            ExprOwned::FloatLit(node) => ExprKey::new(ExprKind::FloatLit, self.floats.len())
                .inspect(|_| {
                    self.floats.push(node);
                }),

            ExprOwned::StringLit(node) => match node.get() {
                "" => ExprKey::new(ExprKind::StringLit, Self::STRING_LIT_EMPTY_INDEX),

                _ => ExprKey::new(ExprKind::StringLit, self.strings.len()).inspect(|_| {
                    self.strings.push(node);
                }),
            },

            ExprOwned::BStringLit(node) => ExprKey::new(ExprKind::BStringLit, self.binaries.len())
                .inspect(|_| {
                    self.binaries.push(node);
                }),

            ExprOwned::CharLit(ch) => ExprKey::new(ExprKind::CharLit, ch as usize),

            ExprOwned::ListLit(node) => {
                ExprKey::new(ExprKind::ListLit, self.lists.len()).inspect(|_| {
                    self.lists.push(node);
                })
            }

            ExprOwned::ObjectLit(node) => ExprKey::new(ExprKind::ObjectLit, self.objects.len())
                .inspect(|_| {
                    self.objects.push(node);
                }),

            ExprOwned::UnaryExpr(node) => ExprKey::new(ExprKind::UnaryExpr, self.unary_exprs.len())
                .inspect(|_| {
                    self.unary_exprs.push(node);
                }),

            ExprOwned::BinExpr(node) => ExprKey::new(ExprKind::BinExpr, self.bin_exprs.len())
                .inspect(|_| {
                    self.bin_exprs.push(node);
                }),

            ExprOwned::Statement(node) => ExprKey::new(ExprKind::Statement, self.statements.len())
                .inspect(|_| {
                    self.statements.push(node);
                }),

            ExprOwned::Block(node) => {
                ExprKey::new(ExprKind::Block, self.blocks.len()).inspect(|_| {
                    self.blocks.push(node);
                })
            }

            ExprOwned::Function(node) => ExprKey::new(ExprKind::Function, self.functions.len())
                .inspect(|_| {
                    self.functions.push(node);
                }),

            ExprOwned::Variable(node) => ExprKey::new(ExprKind::Variable, self.variables.len())
                .inspect(|_| {
                    self.variables.push(node);
                }),

            ExprOwned::Identifier(node) => {
                ExprKey::new(ExprKind::Identifier, self.identifiers.len()).inspect(|_| {
                    self.identifiers.push(node);
                })
            }

            ExprOwned::If(node) => ExprKey::new(ExprKind::If, self.ifs.len()).inspect(|_| {
                self.ifs.push(node);
            }),

            ExprOwned::WhileLoop(node) => ExprKey::new(ExprKind::WhileLoop, self.while_loops.len())
                .inspect(|_| {
                    self.while_loops.push(node);
                }),

            ExprOwned::DoWhileLoop(node) => {
                ExprKey::new(ExprKind::DoWhileLoop, self.do_while_loops.len()).inspect(|_| {
                    self.do_while_loops.push(node);
                })
            }

            ExprOwned::Switch(node) => {
                ExprKey::new(ExprKind::Switch, self.switches.len()).inspect(|_| {
                    self.switches.push(node);
                })
            }

            ExprOwned::Break(node) => {
                ExprKey::new(ExprKind::Break, self.breaks.len()).inspect(|_| {
                    self.breaks.push(node);
                })
            }

            ExprOwned::Continue(node) => ExprKey::new(ExprKind::Continue, self.continues.len())
                .inspect(|_| {
                    self.continues.push(node);
                }),

            ExprOwned::Return(node) => {
                ExprKey::new(ExprKind::Return, self.returns.len()).inspect(|_| {
                    self.returns.push(node);
                })
            }

            ExprOwned::ForEach(node) => ExprKey::new(ExprKind::ForEach, self.for_eachs.len())
                .inspect(|_| {
                    self.for_eachs.push(node);
                }),

            ExprOwned::Await(node) => {
                ExprKey::new(ExprKind::Await, self.awaits.len()).inspect(|_| {
                    self.awaits.push(node);
                })
            }

            ExprOwned::Assert(node) => {
                ExprKey::new(ExprKind::Assert, self.asserts.len()).inspect(|_| {
                    self.asserts.push(node);
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

            other => {
                let kind = match other {
                    TypeOwned::Bool
                    | TypeOwned::UInt8
                    | TypeOwned::UInt16
                    | TypeOwned::UInt32
                    | TypeOwned::UInt64
                    | TypeOwned::UInt128
                    | TypeOwned::Int8
                    | TypeOwned::Int16
                    | TypeOwned::Int32
                    | TypeOwned::Int64
                    | TypeOwned::Int128
                    | TypeOwned::Float8
                    | TypeOwned::Float16
                    | TypeOwned::Float32
                    | TypeOwned::Float64
                    | TypeOwned::Float128
                    | TypeOwned::InferType => {
                        unreachable!()
                    }

                    TypeOwned::TypeName(_) => TypeKind::TypeName,
                    TypeOwned::RefinementType(_) => TypeKind::RefinementType,
                    TypeOwned::TupleType(_) => TypeKind::TupleType,
                    TypeOwned::ArrayType(_) => TypeKind::ArrayType,
                    TypeOwned::MapType(_) => TypeKind::MapType,
                    TypeOwned::SliceType(_) => TypeKind::SliceType,
                    TypeOwned::FunctionType(_) => TypeKind::FunctionType,
                    TypeOwned::ManagedRefType(_) => TypeKind::ManagedRefType,
                    TypeOwned::UnmanagedRefType(_) => TypeKind::UnmanagedRefType,
                    TypeOwned::GenericType(_) => TypeKind::GenericType,
                    TypeOwned::OpaqueType(_) => TypeKind::OpaqueType,
                };

                if let Some(type_key) = self.dedup_types.get_by_right(&other) {
                    Some(*type_key)
                } else if let Some(key) = TypeKey::new(kind, self.dedup_types.len()) {
                    self.dedup_types.insert(key, other);
                    Some(key)
                } else {
                    None
                }
            }
        }
    }

    pub fn get_expr(&self, id: ExprKey<'a>) -> ExprRef<'_, 'a> {
        let index = id.instance_index();

        match id.variant_index() {
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
            | ExprKind::FunctionType
            | ExprKind::ManagedRefType
            | ExprKind::UnmanagedRefType
            | ExprKind::GenericType
            | ExprKind::OpaqueType => Some(
                self.get_type(id.try_into().expect("Expected conversion to TypeKey"))
                    .into(),
            ),

            ExprKind::Discard => Some(ExprRef::Discard),

            ExprKind::IntegerLit => self.integers.get(index).map(ExprRef::IntegerLit),
            ExprKind::FloatLit => self.floats.get(index).map(|&f| ExprRef::FloatLit(f)),
            ExprKind::StringLit => self.strings.get(index).map(ExprRef::StringLit),
            ExprKind::BStringLit => self.binaries.get(index).map(ExprRef::BStringLit),
            ExprKind::CharLit => char::from_u32(index as u32).map(ExprRef::CharLit),
            ExprKind::ListLit => self.lists.get(index).map(ExprRef::ListLit),
            ExprKind::ObjectLit => self.objects.get(index).map(ExprRef::ObjectLit),

            ExprKind::UnaryExpr => self.unary_exprs.get(index).map(ExprRef::UnaryExpr),
            ExprKind::BinExpr => self.bin_exprs.get(index).map(ExprRef::BinExpr),
            ExprKind::Statement => self.statements.get(index).map(ExprRef::Statement),
            ExprKind::Block => self.blocks.get(index).map(ExprRef::Block),

            ExprKind::Function => self.functions.get(index).map(ExprRef::Function),
            ExprKind::Variable => self.variables.get(index).map(ExprRef::Variable),
            ExprKind::Identifier => self
                .identifiers
                .get(index)
                .map(|&id| ExprRef::Identifier(id)),

            ExprKind::If => self.ifs.get(index).map(ExprRef::If),
            ExprKind::WhileLoop => self.while_loops.get(index).map(ExprRef::WhileLoop),
            ExprKind::DoWhileLoop => self.do_while_loops.get(index).map(ExprRef::DoWhileLoop),
            ExprKind::Switch => self.switches.get(index).map(ExprRef::Switch),
            ExprKind::Break => self.breaks.get(index).map(ExprRef::Break),
            ExprKind::Continue => self.continues.get(index).map(ExprRef::Continue),
            ExprKind::Return => self.returns.get(index).map(ExprRef::Return),
            ExprKind::ForEach => self.for_eachs.get(index).map(ExprRef::ForEach),
            ExprKind::Await => self.awaits.get(index).map(ExprRef::Await),
            ExprKind::Assert => self.asserts.get(index).map(ExprRef::Assert),
        }
        .expect("Expression not found in storage")
    }

    pub fn get_expr_mut(&mut self, id: ExprKey<'a>) -> ExprRefMut<'_, 'a> {
        let index = id.instance_index();

        match id.variant_index() {
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
            | ExprKind::FunctionType
            | ExprKind::ManagedRefType
            | ExprKind::UnmanagedRefType
            | ExprKind::GenericType
            | ExprKind::OpaqueType => Some(
                self.get_type(id.try_into().expect("Expected conversion to TypeKey"))
                    .into(),
            ),

            ExprKind::Discard => Some(ExprRefMut::Discard),

            ExprKind::IntegerLit => self.integers.get(index).map(ExprRefMut::IntegerLit),
            ExprKind::FloatLit => self.floats.get(index).map(|&f| ExprRefMut::FloatLit(f)),
            ExprKind::StringLit => self.strings.get(index).map(ExprRefMut::StringLit),
            ExprKind::BStringLit => self.binaries.get(index).map(ExprRefMut::BStringLit),
            ExprKind::CharLit => char::from_u32(index as u32).map(ExprRefMut::CharLit),
            ExprKind::ListLit => self.lists.get_mut(index).map(ExprRefMut::ListLit),
            ExprKind::ObjectLit => self.objects.get_mut(index).map(ExprRefMut::ObjectLit),

            ExprKind::UnaryExpr => self.unary_exprs.get_mut(index).map(ExprRefMut::UnaryExpr),
            ExprKind::BinExpr => self.bin_exprs.get_mut(index).map(ExprRefMut::BinExpr),
            ExprKind::Statement => self.statements.get_mut(index).map(ExprRefMut::Statement),
            ExprKind::Block => self.blocks.get_mut(index).map(ExprRefMut::Block),

            ExprKind::Function => self.functions.get_mut(index).map(ExprRefMut::Function),
            ExprKind::Variable => self.variables.get_mut(index).map(ExprRefMut::Variable),
            ExprKind::Identifier => self
                .identifiers
                .get_mut(index)
                .map(|id| ExprRefMut::Identifier(id)),

            ExprKind::If => self.ifs.get_mut(index).map(ExprRefMut::If),
            ExprKind::WhileLoop => self.while_loops.get_mut(index).map(ExprRefMut::WhileLoop),
            ExprKind::DoWhileLoop => self
                .do_while_loops
                .get_mut(index)
                .map(ExprRefMut::DoWhileLoop),
            ExprKind::Switch => self.switches.get_mut(index).map(ExprRefMut::Switch),
            ExprKind::Break => self.breaks.get_mut(index).map(ExprRefMut::Break),
            ExprKind::Continue => self.continues.get_mut(index).map(ExprRefMut::Continue),
            ExprKind::Return => self.returns.get_mut(index).map(ExprRefMut::Return),
            ExprKind::ForEach => self.for_eachs.get_mut(index).map(ExprRefMut::ForEach),
            ExprKind::Await => self.awaits.get_mut(index).map(ExprRefMut::Await),
            ExprKind::Assert => self.asserts.get_mut(index).map(ExprRefMut::Assert),
        }
        .expect("Expression not found in storage")
    }

    pub fn get_type(&self, id: TypeKey<'a>) -> &TypeOwned<'a> {
        match id.variant_index() {
            TypeKind::Bool => &TypeOwned::Bool,
            TypeKind::UInt8 => &TypeOwned::UInt8,
            TypeKind::UInt16 => &TypeOwned::UInt16,
            TypeKind::UInt32 => &TypeOwned::UInt32,
            TypeKind::UInt64 => &TypeOwned::UInt64,
            TypeKind::UInt128 => &TypeOwned::UInt128,
            TypeKind::Int8 => &TypeOwned::Int8,
            TypeKind::Int16 => &TypeOwned::Int16,
            TypeKind::Int32 => &TypeOwned::Int32,
            TypeKind::Int64 => &TypeOwned::Int64,
            TypeKind::Int128 => &TypeOwned::Int128,
            TypeKind::Float8 => &TypeOwned::Float8,
            TypeKind::Float16 => &TypeOwned::Float16,
            TypeKind::Float32 => &TypeOwned::Float32,
            TypeKind::Float64 => &TypeOwned::Float64,
            TypeKind::Float128 => &TypeOwned::Float128,
            TypeKind::InferType => &TypeOwned::InferType,

            _ => self
                .dedup_types
                .get_by_left(&id)
                .expect("TypeKey not found in storage"),
        }
    }

    pub fn has_parentheses(&self, key: ExprKey<'a>) -> bool {
        self.has_parentheses.contains(&key)
    }

    pub fn add_parentheses(&mut self, key: ExprKey<'a>) {
        self.has_parentheses.insert(key);
    }
}
