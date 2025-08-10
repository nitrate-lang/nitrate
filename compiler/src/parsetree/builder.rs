use super::builder_helper::*;
use super::expression::{ExprOwned, TypeOwned};
use super::opaque_type::OpaqueType;
use super::storage::{ExprKey, Storage, TypeKey};
use super::tuple_type::TupleType;
use crate::lexer::StringData;

pub use super::binary_op::BinaryOperator;
pub use super::function::FunctionParameter;
pub use super::variable::VariableKind;

#[derive(Debug)]
pub struct Builder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
}

const ADD_TYPE_EXPECT_REASON: &str = "`Type` object construction failed because of incorrect object deduplication, thereby causing redundant, exorbitant object construction.";

impl<'storage, 'a> Builder<'storage, 'a> {
    pub fn new(storage: &'storage mut Storage<'a>) -> Self {
        Builder { storage }
    }

    pub fn get_storage(&mut self) -> &mut Storage<'a> {
        self.storage
    }

    pub fn get_discard(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::Discard)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Literal Expression Builders
    pub fn create_integer(self) -> IntegerBuilder<'storage, 'a> {
        IntegerBuilder::new(self.storage)
    }

    pub fn create_float(self) -> FloatBuilder<'storage, 'a> {
        FloatBuilder::new(self.storage)
    }

    pub fn create_string(self) -> StringBuilder<'storage, 'a> {
        StringBuilder::new(self.storage)
    }

    pub fn create_binary(self) -> BinaryBuilder<'storage, 'a> {
        BinaryBuilder::new(self.storage)
    }

    pub fn create_char(self, char: char) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::CharLit(char))
            .expect("Failed to create char literal")
    }

    pub fn create_list(self) -> ListBuilder<'storage, 'a> {
        ListBuilder::new(self.storage)
    }

    pub fn create_object(self) -> ObjectBuilder<'storage, 'a> {
        ObjectBuilder::new(self.storage)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Compound Expression Builders
    pub fn create_unary_expr(self) -> UnaryOpBuilder<'storage, 'a> {
        UnaryOpBuilder::new(self.storage)
    }

    pub fn create_binary_expr(self) -> BinaryOpBuilder<'storage, 'a> {
        BinaryOpBuilder::new(self.storage)
    }

    pub fn create_statement(self) -> StatementBuilder<'storage, 'a> {
        StatementBuilder::new(self.storage)
    }

    pub fn create_block(self) -> BlockBuilder<'storage, 'a> {
        BlockBuilder::new(self.storage)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Definition Builders
    pub fn create_function(self) -> FunctionBuilder<'storage, 'a> {
        FunctionBuilder::new(self.storage)
    }

    pub fn create_variable(self) -> VariableBuilder<'storage, 'a> {
        VariableBuilder::new(self.storage)
    }

    pub fn create_let(self) -> VariableBuilder<'storage, 'a> {
        self.create_variable().with_kind(VariableKind::Let)
    }

    pub fn create_var(self) -> VariableBuilder<'storage, 'a> {
        self.create_variable().with_kind(VariableKind::Var)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Control Flow Builders
    pub fn create_return(self) -> ReturnBuilder<'storage, 'a> {
        ReturnBuilder::new(self.storage)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Primitive Type Builders
    pub fn get_bool(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Bool)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u8(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u16(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u32(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u64(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u128(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i8(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i16(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i32(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i64(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i128(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f8(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f16(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f32(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f64(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f128(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_infer_type(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::InferType)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_unit(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::TupleType(TupleType::new(vec![])))
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Complex Type Builders
    pub fn create_type_name(self, name: &'a str) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::TypeName(name))
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn create_refinement_type(self) -> RefinementTypeBuilder<'storage, 'a> {
        RefinementTypeBuilder::new(self.storage)
    }

    pub fn create_tuple_type(self) -> TupleTypeBuilder<'storage, 'a> {
        TupleTypeBuilder::new(self.storage)
    }

    pub fn create_array_type(self) -> ArrayTypeBuilder<'storage, 'a> {
        ArrayTypeBuilder::new(self.storage)
    }

    pub fn create_map_type(self) -> MapTypeBuilder<'storage, 'a> {
        MapTypeBuilder::new(self.storage)
    }

    pub fn create_slice_type(self) -> SliceTypeBuilder<'storage, 'a> {
        SliceTypeBuilder::new(self.storage)
    }

    pub fn create_function_type(self) -> FunctionTypeBuilder<'storage, 'a> {
        FunctionTypeBuilder::new(self.storage)
    }

    pub fn create_managed_type(self) -> ManagedRefTypeBuilder<'storage, 'a> {
        ManagedRefTypeBuilder::new(self.storage)
    }

    pub fn create_unmanaged_type(self) -> UnmanagedRefTypeBuilder<'storage, 'a> {
        UnmanagedRefTypeBuilder::new(self.storage)
    }

    pub fn create_generic_type(self) -> GenericTypeBuilder<'storage, 'a> {
        GenericTypeBuilder::new(self.storage)
    }

    pub fn create_opaque_type(self, identity: StringData<'a>) -> Option<TypeKey<'a>> {
        self.storage
            .add_type(TypeOwned::OpaqueType(OpaqueType::new(identity)))
    }
}
