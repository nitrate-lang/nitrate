use crate::expression::VariableKind;
use crate::expression::{Expr, Identifier};
use crate::types::{TupleType, Type};
use interned_string::IString;
use nitrate_tokenize::NotNan;
use std::sync::Arc;

use crate::builder_helper::{
    ArrayTypeBuilder, AssertBuilder, AwaitBuilder, BinExprBuilder, BlockBuilder, BreakBuilder,
    CallBuilder, ContinueBuilder, DoWhileLoopBuilder, ForEachBuilder, FunctionBuilder,
    FunctionTypeBuilder, GenericTypeBuilder, IfBuilder, IndexAccessBuilder, IntegerBuilder,
    ListBuilder, ManagedRefTypeBuilder, MapTypeBuilder, ObjectBuilder, RefinementTypeBuilder,
    ReturnBuilder, ScopeBuilder, SliceTypeBuilder, StatementBuilder, StructTypeBuilder,
    SwitchBuilder, TupleTypeBuilder, UnaryExprBuilder, UnmanagedRefTypeBuilder, VariableBuilder,
    WhileLoopBuilder,
};

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
        Expr::Boolean(value)
    }

    #[must_use]
    pub fn create_u8(x: u8) -> Expr<'a> {
        IntegerBuilder::new().with_u8(x).build()
    }

    #[must_use]
    pub fn create_u16(x: u16) -> Expr<'a> {
        IntegerBuilder::new().with_u16(x).build()
    }

    #[must_use]
    pub fn create_u32(x: u32) -> Expr<'a> {
        IntegerBuilder::new().with_u32(x).build()
    }

    #[must_use]
    pub fn create_u64(x: u64) -> Expr<'a> {
        IntegerBuilder::new().with_u64(x).build()
    }

    #[must_use]
    pub fn create_u128(x: u128) -> Expr<'a> {
        IntegerBuilder::new().with_u128(x).build()
    }

    #[must_use]
    pub fn create_integer_with_kind() -> IntegerBuilder {
        IntegerBuilder::new()
    }

    #[must_use]
    pub fn create_float(x: NotNan<f64>) -> Expr<'a> {
        Expr::Float(x)
    }

    #[must_use]
    pub fn create_string(str: IString) -> Expr<'a> {
        Expr::String(str)
    }

    #[must_use]
    pub fn create_bstring(bytes: Vec<u8>) -> Expr<'a> {
        Expr::BString(Arc::new(bytes))
    }

    #[must_use]
    pub fn create_type_envelop(inner: Type<'a>) -> Expr<'a> {
        Expr::TypeEnvelop(Arc::new(inner))
    }

    #[must_use]
    pub fn create_list() -> ListBuilder<'a> {
        ListBuilder::new()
    }

    #[must_use]
    pub fn create_unit() -> Expr<'a> {
        Expr::Unit
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
    pub fn create_qualified_identifier(segments: Vec<IString>) -> Expr<'a> {
        Expr::Identifier(Arc::new(Identifier::new(segments)))
    }

    #[must_use]
    pub fn create_identifier(name: IString) -> Expr<'a> {
        let parts = name
            .to_string()
            .split("::")
            .map(IString::from)
            .collect::<Vec<IString>>();

        Self::create_qualified_identifier(parts)
    }

    #[must_use]
    pub fn create_index_access() -> IndexAccessBuilder<'a> {
        IndexAccessBuilder::new()
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
    pub fn create_break() -> BreakBuilder {
        BreakBuilder::new()
    }

    #[must_use]
    pub fn create_continue() -> ContinueBuilder {
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
    pub fn create_call() -> CallBuilder<'a> {
        CallBuilder::new()
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
        Type::TupleType(Arc::new(TupleType::new(vec![])))
    }

    #[must_use]
    pub fn create_qualified_type_name(segments: Vec<IString>) -> Type<'a> {
        Type::TypeName(Arc::new(Identifier::new(segments)))
    }

    #[must_use]
    pub fn create_type_name(name: IString) -> Type<'a> {
        let parts = name
            .to_string()
            .split("::")
            .map(IString::from)
            .collect::<Vec<IString>>();

        Self::create_qualified_type_name(parts)
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
    pub fn create_opaque_type(identity: IString) -> Type<'a> {
        Type::OpaqueType(identity)
    }

    #[must_use]
    pub fn create_struct_type() -> StructTypeBuilder<'a> {
        StructTypeBuilder::new()
    }

    #[must_use]
    pub fn create_latent_type(expr: Expr<'a>) -> Type<'a> {
        Type::LatentType(Arc::new(expr))
    }

    #[must_use]
    pub fn create_type_parentheses(inner: Type<'a>) -> Type<'a> {
        Type::HasParenthesesType(Arc::new(inner))
    }

    #[must_use]
    pub fn create_parentheses(inner: Expr<'a>) -> Expr<'a> {
        Expr::HasParentheses(Arc::new(inner))
    }
}
