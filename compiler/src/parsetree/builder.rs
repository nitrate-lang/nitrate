use ordered_float::NotNan;

use super::builder_helper::{
    ArrayTypeBuilder, AssertBuilder, AwaitBuilder, BinExprBuilder, BlockBuilder, BreakBuilder,
    ContinueBuilder, DirectCallBuilder, DoWhileLoopBuilder, ForEachBuilder, FunctionBuilder,
    FunctionTypeBuilder, GenericTypeBuilder, IfBuilder, IndirectCallBuilder, IntegerBuilder,
    ListBuilder, ManagedRefTypeBuilder, MapTypeBuilder, ObjectBuilder, RefinementTypeBuilder,
    ReturnBuilder, SliceTypeBuilder, StatementBuilder, SwitchBuilder, TupleTypeBuilder,
    UnaryExprBuilder, UnmanagedRefTypeBuilder, VariableBuilder, WhileLoopBuilder,
};
use super::expression::VariableKind;
use super::node::{Expr, Type};
use super::types::TupleType;
use crate::lexical::{BStringData, StringData};
use crate::parsetree::builder_helper::ScopeBuilder;
use std::rc::Rc;

#[derive(Debug, Default)]
pub struct Builder {}

impl<'a> Builder {
    #[must_use]
    pub fn new() -> Self {
        Builder {}
    }

    #[must_use]
    pub fn get_discard() -> Expr<'a> {
        Expr::Discard
    }

    #[must_use]
    pub fn create_boolean(value: bool) -> Expr<'a> {
        Expr::BooleanLit(value)
    }

    #[must_use]
    pub fn create_integer() -> IntegerBuilder {
        IntegerBuilder::new()
    }

    #[must_use]
    pub fn create_float(x: NotNan<f64>) -> Expr<'a> {
        Expr::FloatLit(x)
    }

    #[must_use]
    pub fn create_string_from_ref(str: &'a str) -> Expr<'a> {
        Expr::StringLit(Rc::new(StringData::from_ref(str)))
    }

    #[must_use]
    pub fn create_string(str: String) -> Expr<'a> {
        Expr::StringLit(Rc::new(StringData::from_dyn(str)))
    }

    #[must_use]
    pub fn create_string_from(storage: StringData<'a>) -> Expr<'a> {
        Expr::StringLit(Rc::new(storage))
    }

    #[must_use]
    pub fn create_bstring_from_ref(bytes: &'a [u8]) -> Expr<'a> {
        Expr::BStringLit(Rc::new(BStringData::from_ref(bytes)))
    }

    #[must_use]
    pub fn create_bstring(bytes: Vec<u8>) -> Expr<'a> {
        Expr::BStringLit(Rc::new(BStringData::from_dyn(bytes)))
    }

    #[must_use]
    pub fn create_bstring_from(storage: BStringData<'a>) -> Expr<'a> {
        Expr::BStringLit(Rc::new(storage))
    }

    #[must_use]
    pub fn create_list() -> ListBuilder<'a> {
        ListBuilder::new()
    }

    #[must_use]
    pub fn create_unit() -> Expr<'a> {
        Expr::UnitLit
    }

    #[must_use]
    pub fn create_object() -> ObjectBuilder<'a> {
        ObjectBuilder::new()
    }

    #[must_use]
    pub fn create_unary_expr() -> UnaryExprBuilder<'a> {
        UnaryExprBuilder::new()
    }

    #[must_use]
    pub fn create_binexpr() -> BinExprBuilder<'a> {
        BinExprBuilder::new()
    }

    #[must_use]
    pub fn create_statement() -> StatementBuilder<'a> {
        StatementBuilder::new()
    }

    #[must_use]
    pub fn create_block() -> BlockBuilder<'a> {
        BlockBuilder::new()
    }

    #[must_use]
    pub fn create_function() -> FunctionBuilder<'a> {
        FunctionBuilder::new()
    }

    #[must_use]
    pub fn create_variable() -> VariableBuilder<'a> {
        VariableBuilder::new()
    }

    #[must_use]
    pub fn create_identifier(name: &'a str) -> Expr<'a> {
        Expr::Identifier(Rc::new(name))
    }

    #[must_use]
    pub fn create_scope() -> ScopeBuilder<'a> {
        ScopeBuilder::new()
    }

    #[must_use]
    pub fn create_let() -> VariableBuilder<'a> {
        Self::create_variable().with_kind(VariableKind::Let)
    }

    #[must_use]
    pub fn create_var() -> VariableBuilder<'a> {
        Self::create_variable().with_kind(VariableKind::Var)
    }

    #[must_use]
    pub fn create_if() -> IfBuilder<'a> {
        IfBuilder::new()
    }

    #[must_use]
    pub fn create_while_loop() -> WhileLoopBuilder<'a> {
        WhileLoopBuilder::new()
    }

    #[must_use]
    pub fn create_do_while_loop() -> DoWhileLoopBuilder<'a> {
        DoWhileLoopBuilder::new()
    }

    #[must_use]
    pub fn create_switch() -> SwitchBuilder<'a> {
        SwitchBuilder::new()
    }

    #[must_use]
    pub fn create_break() -> BreakBuilder<'a> {
        BreakBuilder::new()
    }

    #[must_use]
    pub fn create_continue() -> ContinueBuilder<'a> {
        ContinueBuilder::new()
    }

    #[must_use]
    pub fn create_return() -> ReturnBuilder<'a> {
        ReturnBuilder::new()
    }

    #[must_use]
    pub fn create_for_each() -> ForEachBuilder<'a> {
        ForEachBuilder::new()
    }

    #[must_use]
    pub fn create_await() -> AwaitBuilder<'a> {
        AwaitBuilder::new()
    }

    #[must_use]
    pub fn create_assert() -> AssertBuilder<'a> {
        AssertBuilder::new()
    }

    #[must_use]
    pub fn create_direct_call() -> DirectCallBuilder<'a> {
        DirectCallBuilder::new()
    }

    #[must_use]
    pub fn create_indirect_call() -> IndirectCallBuilder<'a> {
        IndirectCallBuilder::new()
    }

    #[must_use]
    pub fn get_bool() -> Type<'a> {
        Type::Bool
    }

    #[must_use]
    pub fn get_u8() -> Type<'a> {
        Type::UInt8
    }

    #[must_use]
    pub fn get_u16() -> Type<'a> {
        Type::UInt16
    }

    #[must_use]
    pub fn get_u32() -> Type<'a> {
        Type::UInt32
    }

    #[must_use]
    pub fn get_u64() -> Type<'a> {
        Type::UInt64
    }

    #[must_use]
    pub fn get_u128() -> Type<'a> {
        Type::UInt128
    }

    #[must_use]
    pub fn get_i8() -> Type<'a> {
        Type::Int8
    }

    #[must_use]
    pub fn get_i16() -> Type<'a> {
        Type::Int16
    }

    #[must_use]
    pub fn get_i32() -> Type<'a> {
        Type::Int32
    }

    #[must_use]
    pub fn get_i64() -> Type<'a> {
        Type::Int64
    }

    #[must_use]
    pub fn get_i128() -> Type<'a> {
        Type::Int128
    }

    #[must_use]
    pub fn get_f8() -> Type<'a> {
        Type::Float8
    }

    #[must_use]
    pub fn get_f16() -> Type<'a> {
        Type::Float16
    }

    #[must_use]
    pub fn get_f32() -> Type<'a> {
        Type::Float32
    }

    #[must_use]
    pub fn get_f64() -> Type<'a> {
        Type::Float64
    }

    #[must_use]
    pub fn get_f128() -> Type<'a> {
        Type::Float128
    }

    #[must_use]
    pub fn get_infer_type() -> Type<'a> {
        Type::InferType
    }

    #[must_use]
    pub fn get_unit_type() -> Type<'a> {
        Type::TupleType(Rc::new(TupleType::new(vec![])))
    }

    #[must_use]
    pub fn create_type_name(name: &'a str) -> Type<'a> {
        Type::TypeName(name)
    }

    #[must_use]
    pub fn create_refinement_type() -> RefinementTypeBuilder<'a> {
        RefinementTypeBuilder::new()
    }

    #[must_use]
    pub fn create_tuple_type() -> TupleTypeBuilder<'a> {
        TupleTypeBuilder::new()
    }

    #[must_use]
    pub fn create_array_type() -> ArrayTypeBuilder<'a> {
        ArrayTypeBuilder::new()
    }

    #[must_use]
    pub fn create_map_type() -> MapTypeBuilder<'a> {
        MapTypeBuilder::new()
    }

    #[must_use]
    pub fn create_slice_type() -> SliceTypeBuilder<'a> {
        SliceTypeBuilder::new()
    }

    #[must_use]
    pub fn create_function_type() -> FunctionTypeBuilder<'a> {
        FunctionTypeBuilder::new()
    }

    #[must_use]
    pub fn create_managed_type() -> ManagedRefTypeBuilder<'a> {
        ManagedRefTypeBuilder::new()
    }

    #[must_use]
    pub fn create_unmanaged_type() -> UnmanagedRefTypeBuilder<'a> {
        UnmanagedRefTypeBuilder::new()
    }

    #[must_use]
    pub fn create_generic_type() -> GenericTypeBuilder<'a> {
        GenericTypeBuilder::new()
    }

    #[must_use]
    pub fn create_opaque_type(identity: StringData<'a>) -> Type<'a> {
        Type::OpaqueType(Rc::new(identity))
    }

    #[must_use]
    pub fn create_type_parentheses(inner: Type<'a>) -> Type<'a> {
        Type::HasParenthesesType(Rc::new(inner))
    }

    #[must_use]
    pub fn create_parentheses(inner: Expr<'a>) -> Expr<'a> {
        Expr::HasParentheses(Rc::new(inner))
    }
}
