use std::ops::Deref;

use crate::expression::Expr;
use crate::expression::VariableKind;
use crate::kind::Block;
use crate::types::{TupleType, Type};
use interned_string::IString;
use nitrate_tokenize::{IntegerKind, NotNan};

use crate::builder_helper::{
    ArrayTypeBuilder, AwaitBuilder, BinExprBuilder, BlockBuilder, BreakBuilder, CallBuilder,
    ContinueBuilder, DoWhileLoopBuilder, ForEachBuilder, FunctionBuilder, FunctionTypeBuilder,
    GenericTypeBuilder, IfBuilder, IndexAccessBuilder, IntegerBuilder, ListBuilder,
    ManagedRefTypeBuilder, MapTypeBuilder, ObjectBuilder, RefinementTypeBuilder, ReturnBuilder,
    ScopeBuilder, SliceTypeBuilder, StructTypeBuilder, SwitchBuilder, TupleTypeBuilder,
    UnaryExprBuilder, UnmanagedRefTypeBuilder, VariableBuilder, WhileLoopBuilder,
};

#[derive(Debug, Default)]
pub struct Builder {}

impl Builder {
    #[must_use]
    pub fn new() -> Self {
        Builder {}
    }

    #[must_use]
    pub fn get_discard() -> Expr {
        Expr::Discard
    }

    #[must_use]
    pub fn create_boolean(value: bool) -> Expr {
        Expr::Boolean(value)
    }

    #[must_use]
    pub fn create_integer(x: u128, kind: IntegerKind) -> Expr {
        IntegerBuilder::new().with_u128(x).with_kind(kind).build()
    }

    #[must_use]
    pub fn create_float(x: NotNan<f64>) -> Expr {
        Expr::Float(x)
    }

    #[must_use]
    pub fn create_string(str: IString) -> Expr {
        Expr::String(str)
    }

    #[must_use]
    pub fn create_bstring(bytes: Vec<u8>) -> Expr {
        Expr::BString(Box::new(bytes))
    }

    #[must_use]
    pub fn create_type_info(inner: Type) -> Expr {
        Expr::TypeInfo(inner)
    }

    #[must_use]
    pub fn create_list() -> ListBuilder {
        ListBuilder::new()
    }

    #[must_use]
    pub fn create_unit() -> Expr {
        Expr::Unit
    }

    #[must_use]
    pub fn create_object() -> ObjectBuilder {
        ObjectBuilder::new()
    }

    #[must_use]
    pub fn create_unary_expr() -> UnaryExprBuilder {
        UnaryExprBuilder::new()
    }

    #[must_use]
    pub fn create_binexpr() -> BinExprBuilder {
        BinExprBuilder::new()
    }

    #[must_use]
    pub fn create_block() -> BlockBuilder {
        BlockBuilder::new()
    }

    #[must_use]
    pub fn create_function() -> FunctionBuilder {
        FunctionBuilder::new()
    }

    #[must_use]
    pub fn create_variable() -> VariableBuilder {
        VariableBuilder::new()
    }

    #[must_use]
    pub fn create_identifier(path: Vec<IString>) -> Expr {
        let joined = path
            .iter()
            .map(Deref::deref)
            .collect::<Vec<&str>>()
            .join("::");

        Expr::Identifier(joined.into())
    }

    #[must_use]
    pub fn create_index_access() -> IndexAccessBuilder {
        IndexAccessBuilder::new()
    }

    #[must_use]
    pub fn create_scope() -> ScopeBuilder {
        ScopeBuilder::new()
    }

    #[must_use]
    pub fn create_let() -> VariableBuilder {
        Self::create_variable().with_kind(VariableKind::Let)
    }

    #[must_use]
    pub fn create_var() -> VariableBuilder {
        Self::create_variable().with_kind(VariableKind::Var)
    }

    #[must_use]
    pub fn create_if() -> IfBuilder {
        IfBuilder::new()
    }

    #[must_use]
    pub fn create_while_loop() -> WhileLoopBuilder {
        WhileLoopBuilder::new()
    }

    #[must_use]
    pub fn create_do_while_loop() -> DoWhileLoopBuilder {
        DoWhileLoopBuilder::new()
    }

    #[must_use]
    pub fn create_switch() -> SwitchBuilder {
        SwitchBuilder::new()
    }

    #[must_use]
    pub fn create_break() -> BreakBuilder {
        BreakBuilder::new()
    }

    #[must_use]
    pub fn create_continue() -> ContinueBuilder {
        ContinueBuilder::new()
    }

    #[must_use]
    pub fn create_return() -> ReturnBuilder {
        ReturnBuilder::new()
    }

    #[must_use]
    pub fn create_for_each() -> ForEachBuilder {
        ForEachBuilder::new()
    }

    #[must_use]
    pub fn create_await() -> AwaitBuilder {
        AwaitBuilder::new()
    }

    #[must_use]
    pub fn create_call() -> CallBuilder {
        CallBuilder::new()
    }

    #[must_use]
    pub fn get_bool() -> Type {
        Type::Bool
    }

    #[must_use]
    pub fn get_u8() -> Type {
        Type::UInt8
    }

    #[must_use]
    pub fn get_u16() -> Type {
        Type::UInt16
    }

    #[must_use]
    pub fn get_u32() -> Type {
        Type::UInt32
    }

    #[must_use]
    pub fn get_u64() -> Type {
        Type::UInt64
    }

    #[must_use]
    pub fn get_u128() -> Type {
        Type::UInt128
    }

    #[must_use]
    pub fn get_i8() -> Type {
        Type::Int8
    }

    #[must_use]
    pub fn get_i16() -> Type {
        Type::Int16
    }

    #[must_use]
    pub fn get_i32() -> Type {
        Type::Int32
    }

    #[must_use]
    pub fn get_i64() -> Type {
        Type::Int64
    }

    #[must_use]
    pub fn get_i128() -> Type {
        Type::Int128
    }

    #[must_use]
    pub fn get_f8() -> Type {
        Type::Float8
    }

    #[must_use]
    pub fn get_f16() -> Type {
        Type::Float16
    }

    #[must_use]
    pub fn get_f32() -> Type {
        Type::Float32
    }

    #[must_use]
    pub fn get_f64() -> Type {
        Type::Float64
    }

    #[must_use]
    pub fn get_f128() -> Type {
        Type::Float128
    }

    #[must_use]
    pub fn get_infer_type() -> Type {
        Type::InferType
    }

    #[must_use]
    pub fn get_unit_type() -> Type {
        Type::TupleType(Box::new(TupleType::new(vec![])))
    }

    #[must_use]
    pub fn create_type_name(path: Vec<IString>) -> Type {
        let joined = path
            .iter()
            .map(Deref::deref)
            .collect::<Vec<&str>>()
            .join("::");

        Type::TypeName(joined.into())
    }

    #[must_use]
    pub fn create_refinement_type() -> RefinementTypeBuilder {
        RefinementTypeBuilder::new()
    }

    #[must_use]
    pub fn create_tuple_type() -> TupleTypeBuilder {
        TupleTypeBuilder::new()
    }

    #[must_use]
    pub fn create_array_type() -> ArrayTypeBuilder {
        ArrayTypeBuilder::new()
    }

    #[must_use]
    pub fn create_map_type() -> MapTypeBuilder {
        MapTypeBuilder::new()
    }

    #[must_use]
    pub fn create_slice_type() -> SliceTypeBuilder {
        SliceTypeBuilder::new()
    }

    #[must_use]
    pub fn create_function_type() -> FunctionTypeBuilder {
        FunctionTypeBuilder::new()
    }

    #[must_use]
    pub fn create_managed_type() -> ManagedRefTypeBuilder {
        ManagedRefTypeBuilder::new()
    }

    #[must_use]
    pub fn create_unmanaged_type() -> UnmanagedRefTypeBuilder {
        UnmanagedRefTypeBuilder::new()
    }

    #[must_use]
    pub fn create_generic_type() -> GenericTypeBuilder {
        GenericTypeBuilder::new()
    }

    #[must_use]
    pub fn create_opaque_type(identity: IString) -> Type {
        Type::OpaqueType(identity)
    }

    #[must_use]
    pub fn create_struct_type() -> StructTypeBuilder {
        StructTypeBuilder::new()
    }

    #[must_use]
    pub fn create_latent_type(block: Box<Block>) -> Type {
        Type::LatentType(block)
    }

    #[must_use]
    pub fn create_type_parentheses(inner: Type) -> Type {
        Type::HasParenthesesType(Box::new(inner))
    }

    #[must_use]
    pub fn create_parentheses(inner: Expr) -> Expr {
        Expr::HasParentheses(Box::new(inner))
    }
}
