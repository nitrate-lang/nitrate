use super::builder_helper::{
    ArrayTypeBuilder, AssertBuilder, AwaitBuilder, BStringBuilder, BinExprBuilder, BlockBuilder,
    BreakBuilder, ContinueBuilder, DoWhileLoopBuilder, FloatBuilder, ForEachBuilder,
    FunctionBuilder, FunctionTypeBuilder, GenericTypeBuilder, IfBuilder, IntegerBuilder,
    ListBuilder, ManagedRefTypeBuilder, MapTypeBuilder, ObjectBuilder, RefinementTypeBuilder,
    ReturnBuilder, SliceTypeBuilder, StatementBuilder, StringBuilder, SwitchBuilder,
    TupleTypeBuilder, UnaryExprBuilder, UnmanagedRefTypeBuilder, VariableBuilder, WhileLoopBuilder,
};
use super::expression::{ExprOwned, TypeOwned};
use super::storage::{ExprKey, Storage, TypeKey};
use super::tuple_type::TupleType;
use super::variable::VariableKind;
use crate::lexer::StringData;
use crate::parsetree::builder_helper::ScopeBuilder;

#[derive(Debug)]
pub struct Builder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
}

impl<'storage, 'a> Builder<'storage, 'a> {
    pub fn new(storage: &'storage mut Storage<'a>) -> Self {
        Builder { storage }
    }

    pub fn get_storage(&mut self) -> &mut Storage<'a> {
        self.storage
    }

    #[must_use]
    pub fn get_discard(self) -> ExprKey<'a> {
        self.storage.add_expr(ExprOwned::Discard)
    }

    #[must_use]
    pub fn create_boolean(self, value: bool) -> ExprKey<'a> {
        self.storage.add_expr(ExprOwned::BooleanLit(value))
    }

    #[must_use]
    pub fn create_integer(self) -> IntegerBuilder<'storage, 'a> {
        IntegerBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_float(self) -> FloatBuilder<'storage, 'a> {
        FloatBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_string(self) -> StringBuilder<'storage, 'a> {
        StringBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_bstring(self) -> BStringBuilder<'storage, 'a> {
        BStringBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_list(self) -> ListBuilder<'storage, 'a> {
        ListBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_object(self) -> ObjectBuilder<'storage, 'a> {
        ObjectBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_unary_expr(self) -> UnaryExprBuilder<'storage, 'a> {
        UnaryExprBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_binexpr(self) -> BinExprBuilder<'storage, 'a> {
        BinExprBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_statement(self) -> StatementBuilder<'storage, 'a> {
        StatementBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_block(self) -> BlockBuilder<'storage, 'a> {
        BlockBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_function(self) -> FunctionBuilder<'storage, 'a> {
        FunctionBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_variable(self) -> VariableBuilder<'storage, 'a> {
        VariableBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_identifier(self, name: &'a str) -> ExprKey<'a> {
        self.storage.add_expr(ExprOwned::Identifier(name))
    }

    #[must_use]
    pub fn create_scope(self) -> ScopeBuilder<'storage, 'a> {
        ScopeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_let(self) -> VariableBuilder<'storage, 'a> {
        self.create_variable().with_kind(VariableKind::Let)
    }

    #[must_use]
    pub fn create_var(self) -> VariableBuilder<'storage, 'a> {
        self.create_variable().with_kind(VariableKind::Var)
    }

    #[must_use]
    pub fn create_if(self) -> IfBuilder<'storage, 'a> {
        IfBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_while_loop(self) -> WhileLoopBuilder<'storage, 'a> {
        WhileLoopBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_do_while_loop(self) -> DoWhileLoopBuilder<'storage, 'a> {
        DoWhileLoopBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_switch(self) -> SwitchBuilder<'storage, 'a> {
        SwitchBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_break(self) -> BreakBuilder<'storage, 'a> {
        BreakBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_continue(self) -> ContinueBuilder<'storage, 'a> {
        ContinueBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_return(self) -> ReturnBuilder<'storage, 'a> {
        ReturnBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_for_each(self) -> ForEachBuilder<'storage, 'a> {
        ForEachBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_await(self) -> AwaitBuilder<'storage, 'a> {
        AwaitBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_assert(self) -> AssertBuilder<'storage, 'a> {
        AssertBuilder::new(self.storage)
    }

    #[must_use]
    pub fn get_bool(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Bool)
    }

    #[must_use]
    pub fn get_u8(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::UInt8)
    }

    #[must_use]
    pub fn get_u16(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::UInt16)
    }

    #[must_use]
    pub fn get_u32(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::UInt32)
    }

    #[must_use]
    pub fn get_u64(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::UInt64)
    }

    #[must_use]
    pub fn get_u128(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::UInt128)
    }

    #[must_use]
    pub fn get_i8(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Int8)
    }

    #[must_use]
    pub fn get_i16(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Int16)
    }

    #[must_use]
    pub fn get_i32(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Int32)
    }

    #[must_use]
    pub fn get_i64(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Int64)
    }

    #[must_use]
    pub fn get_i128(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Int128)
    }

    #[must_use]
    pub fn get_f8(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Float8)
    }

    #[must_use]
    pub fn get_f16(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Float16)
    }

    #[must_use]
    pub fn get_f32(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Float32)
    }

    #[must_use]
    pub fn get_f64(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Float64)
    }

    #[must_use]
    pub fn get_f128(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::Float128)
    }

    #[must_use]
    pub fn get_infer_type(self) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::InferType)
    }

    #[must_use]
    pub fn get_unit(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::TupleType(TupleType::new(vec![])))
    }

    #[must_use]
    pub fn create_type_name(self, name: &'a str) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::TypeName(name))
    }

    #[must_use]
    pub fn create_refinement_type(self) -> RefinementTypeBuilder<'storage, 'a> {
        RefinementTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_tuple_type(self) -> TupleTypeBuilder<'storage, 'a> {
        TupleTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_array_type(self) -> ArrayTypeBuilder<'storage, 'a> {
        ArrayTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_map_type(self) -> MapTypeBuilder<'storage, 'a> {
        MapTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_slice_type(self) -> SliceTypeBuilder<'storage, 'a> {
        SliceTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_function_type(self) -> FunctionTypeBuilder<'storage, 'a> {
        FunctionTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_managed_type(self) -> ManagedRefTypeBuilder<'storage, 'a> {
        ManagedRefTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_unmanaged_type(self) -> UnmanagedRefTypeBuilder<'storage, 'a> {
        UnmanagedRefTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_generic_type(self) -> GenericTypeBuilder<'storage, 'a> {
        GenericTypeBuilder::new(self.storage)
    }

    #[must_use]
    pub fn create_opaque_type(self, identity: StringData<'a>) -> TypeKey<'a> {
        self.storage.add_type(TypeOwned::OpaqueType(identity))
    }
}
