use super::builder_helper::{
    ArrayTypeBuilder, AssertBuilder, AwaitBuilder, BStringBuilder, BinExprBuilder, BlockBuilder,
    BreakBuilder, ContinueBuilder, DoWhileLoopBuilder, FloatBuilder, ForEachBuilder,
    FunctionBuilder, FunctionTypeBuilder, GenericTypeBuilder, IfBuilder, IntegerBuilder,
    ListBuilder, ManagedRefTypeBuilder, MapTypeBuilder, ObjectBuilder, RefinementTypeBuilder,
    ReturnBuilder, SliceTypeBuilder, StatementBuilder, StringBuilder, SwitchBuilder,
    TupleTypeBuilder, UnaryExprBuilder, UnmanagedRefTypeBuilder, VariableBuilder, WhileLoopBuilder,
};
use super::expression::{Expr, Type};
use super::tuple_type::TupleType;
use super::variable::VariableKind;
use crate::lexical::StringData;
use crate::parsetree::builder_helper::ScopeBuilder;
use std::rc::Rc;
use std::sync::Arc;

#[derive(Debug)]
pub struct Builder {}

impl<'a> Builder {
    pub fn new() -> Self {
        Builder {}
    }

    #[must_use]
    pub fn get_discard(self) -> Arc<Expr<'a>> {
        Arc::new(Expr::Discard)
    }

    #[must_use]
    pub fn create_boolean(self, value: bool) -> Arc<Expr<'a>> {
        Arc::new(Expr::BooleanLit(value))
    }

    #[must_use]
    pub fn create_integer(self) -> IntegerBuilder {
        IntegerBuilder::new()
    }

    #[must_use]
    pub fn create_float(self) -> FloatBuilder {
        FloatBuilder::new()
    }

    #[must_use]
    pub fn create_string(self) -> StringBuilder<'a> {
        StringBuilder::new()
    }

    #[must_use]
    pub fn create_bstring(self) -> BStringBuilder<'a> {
        BStringBuilder::new()
    }

    #[must_use]
    pub fn create_list(self) -> ListBuilder<'a> {
        ListBuilder::new()
    }

    #[must_use]
    pub fn create_object(self) -> ObjectBuilder<'a> {
        ObjectBuilder::new()
    }

    #[must_use]
    pub fn create_unary_expr(self) -> UnaryExprBuilder<'a> {
        UnaryExprBuilder::new()
    }

    #[must_use]
    pub fn create_binexpr(self) -> BinExprBuilder<'a> {
        BinExprBuilder::new()
    }

    #[must_use]
    pub fn create_statement(self) -> StatementBuilder<'a> {
        StatementBuilder::new()
    }

    #[must_use]
    pub fn create_block(self) -> BlockBuilder<'a> {
        BlockBuilder::new()
    }

    #[must_use]
    pub fn create_function(self) -> FunctionBuilder<'a> {
        FunctionBuilder::new()
    }

    #[must_use]
    pub fn create_variable(self) -> VariableBuilder<'a> {
        VariableBuilder::new()
    }

    #[must_use]
    pub fn create_identifier(self, name: &'a str) -> Arc<Expr<'a>> {
        Arc::new(Expr::Identifier(name))
    }

    #[must_use]
    pub fn create_scope(self) -> ScopeBuilder<'a> {
        ScopeBuilder::new()
    }

    #[must_use]
    pub fn create_let(self) -> VariableBuilder<'a> {
        self.create_variable().with_kind(VariableKind::Let)
    }

    #[must_use]
    pub fn create_var(self) -> VariableBuilder<'a> {
        self.create_variable().with_kind(VariableKind::Var)
    }

    #[must_use]
    pub fn create_if(self) -> IfBuilder<'a> {
        IfBuilder::new()
    }

    #[must_use]
    pub fn create_while_loop(self) -> WhileLoopBuilder<'a> {
        WhileLoopBuilder::new()
    }

    #[must_use]
    pub fn create_do_while_loop(self) -> DoWhileLoopBuilder<'a> {
        DoWhileLoopBuilder::new()
    }

    #[must_use]
    pub fn create_switch(self) -> SwitchBuilder<'a> {
        SwitchBuilder::new()
    }

    #[must_use]
    pub fn create_break(self) -> BreakBuilder<'a> {
        BreakBuilder::new()
    }

    #[must_use]
    pub fn create_continue(self) -> ContinueBuilder<'a> {
        ContinueBuilder::new()
    }

    #[must_use]
    pub fn create_return(self) -> ReturnBuilder<'a> {
        ReturnBuilder::new()
    }

    #[must_use]
    pub fn create_for_each(self) -> ForEachBuilder<'a> {
        ForEachBuilder::new()
    }

    #[must_use]
    pub fn create_await(self) -> AwaitBuilder<'a> {
        AwaitBuilder::new()
    }

    #[must_use]
    pub fn create_assert(self) -> AssertBuilder<'a> {
        AssertBuilder::new()
    }

    #[must_use]
    pub fn get_bool(self) -> Rc<Type<'a>> {
        Rc::new(Type::Bool)
    }

    #[must_use]
    pub fn get_u8(self) -> Rc<Type<'a>> {
        Rc::new(Type::UInt8)
    }

    #[must_use]
    pub fn get_u16(self) -> Rc<Type<'a>> {
        Rc::new(Type::UInt16)
    }

    #[must_use]
    pub fn get_u32(self) -> Rc<Type<'a>> {
        Rc::new(Type::UInt32)
    }

    #[must_use]
    pub fn get_u64(self) -> Rc<Type<'a>> {
        Rc::new(Type::UInt64)
    }

    #[must_use]
    pub fn get_u128(self) -> Rc<Type<'a>> {
        Rc::new(Type::UInt128)
    }

    #[must_use]
    pub fn get_i8(self) -> Rc<Type<'a>> {
        Rc::new(Type::Int8)
    }

    #[must_use]
    pub fn get_i16(self) -> Rc<Type<'a>> {
        Rc::new(Type::Int16)
    }

    #[must_use]
    pub fn get_i32(self) -> Rc<Type<'a>> {
        Rc::new(Type::Int32)
    }

    #[must_use]
    pub fn get_i64(self) -> Rc<Type<'a>> {
        Rc::new(Type::Int64)
    }

    #[must_use]
    pub fn get_i128(self) -> Rc<Type<'a>> {
        Rc::new(Type::Int128)
    }

    #[must_use]
    pub fn get_f8(self) -> Rc<Type<'a>> {
        Rc::new(Type::Float8)
    }

    #[must_use]
    pub fn get_f16(self) -> Rc<Type<'a>> {
        Rc::new(Type::Float16)
    }

    #[must_use]
    pub fn get_f32(self) -> Rc<Type<'a>> {
        Rc::new(Type::Float32)
    }

    #[must_use]
    pub fn get_f64(self) -> Rc<Type<'a>> {
        Rc::new(Type::Float64)
    }

    #[must_use]
    pub fn get_f128(self) -> Rc<Type<'a>> {
        Rc::new(Type::Float128)
    }

    #[must_use]
    pub fn get_infer_type(self) -> Rc<Type<'a>> {
        Rc::new(Type::InferType)
    }

    #[must_use]
    pub fn get_unit(self) -> Rc<Type<'a>> {
        Rc::new(Type::TupleType(TupleType::new(vec![])))
    }

    #[must_use]
    pub fn create_type_name(self, name: &'a str) -> Rc<Type<'a>> {
        Rc::new(Type::TypeName(name))
    }

    #[must_use]
    pub fn create_refinement_type(self) -> RefinementTypeBuilder<'a> {
        RefinementTypeBuilder::new()
    }

    #[must_use]
    pub fn create_tuple_type(self) -> TupleTypeBuilder<'a> {
        TupleTypeBuilder::new()
    }

    #[must_use]
    pub fn create_array_type(self) -> ArrayTypeBuilder<'a> {
        ArrayTypeBuilder::new()
    }

    #[must_use]
    pub fn create_map_type(self) -> MapTypeBuilder<'a> {
        MapTypeBuilder::new()
    }

    #[must_use]
    pub fn create_slice_type(self) -> SliceTypeBuilder<'a> {
        SliceTypeBuilder::new()
    }

    #[must_use]
    pub fn create_function_type(self) -> FunctionTypeBuilder<'a> {
        FunctionTypeBuilder::new()
    }

    #[must_use]
    pub fn create_managed_type(self) -> ManagedRefTypeBuilder<'a> {
        ManagedRefTypeBuilder::new()
    }

    #[must_use]
    pub fn create_unmanaged_type(self) -> UnmanagedRefTypeBuilder<'a> {
        UnmanagedRefTypeBuilder::new()
    }

    #[must_use]
    pub fn create_generic_type(self) -> GenericTypeBuilder<'a> {
        GenericTypeBuilder::new()
    }

    #[must_use]
    pub fn create_opaque_type(self, identity: StringData<'a>) -> Rc<Type<'a>> {
        Rc::new(Type::OpaqueType(identity))
    }
}
