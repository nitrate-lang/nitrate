use super::builder_helper::*;
use super::expression::{ExprOwned, TypeOwned};
use super::storage::{ExprKey, Storage, TypeKey};
use super::tuple_type::TupleType;

pub use super::binary_op::BinaryOperator;
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

    pub fn get_discard(&mut self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::Discard)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Literal Expression Builders
    pub fn create_integer(&mut self) -> IntegerBuilder<'_, 'a> {
        IntegerBuilder::new(self.storage)
    }

    pub fn create_float(&mut self) -> FloatBuilder<'_, 'a> {
        FloatBuilder::new(self.storage)
    }

    pub fn create_string(&mut self) -> StringBuilder<'_, 'a> {
        StringBuilder::new(self.storage)
    }

    pub fn create_char(&mut self) -> CharBuilder<'_, 'a> {
        CharBuilder::new(self.storage)
    }

    pub fn create_list(&mut self) -> ListBuilder<'_, 'a> {
        ListBuilder::new(self.storage)
    }

    pub fn create_object(&mut self) -> ObjectBuilder<'_, 'a> {
        ObjectBuilder::new(self.storage)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Compound Expression Builders
    pub fn create_unary_expr(&mut self) -> UnaryOpBuilder<'_, 'a> {
        UnaryOpBuilder::new(self.storage)
    }

    pub fn create_binary_expr(&mut self) -> BinaryOpBuilder<'_, 'a> {
        BinaryOpBuilder::new(self.storage)
    }

    pub fn create_statement(&mut self) -> StatementBuilder<'_, 'a> {
        StatementBuilder::new(self.storage)
    }

    pub fn create_block(&mut self) -> BlockBuilder<'_, 'a> {
        BlockBuilder::new(self.storage)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Definition Builders
    pub fn create_function(&mut self) -> FunctionBuilder<'_, 'a> {
        FunctionBuilder::new(self.storage)
    }

    pub fn create_variable(&mut self) -> VariableBuilder<'_, 'a> {
        VariableBuilder::new(self.storage)
    }

    pub fn create_let(&mut self) -> VariableBuilder<'_, 'a> {
        self.create_variable().with_kind(VariableKind::Let)
    }

    pub fn create_var(&mut self) -> VariableBuilder<'_, 'a> {
        self.create_variable().with_kind(VariableKind::Var)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Control Flow Builders
    pub fn create_return(&mut self) -> ReturnBuilder<'_, 'a> {
        ReturnBuilder::new(self.storage)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Primitive Type Builders
    pub fn get_bool(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Bool)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u8(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u16(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u32(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u64(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u128(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UInt128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i8(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i16(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i32(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i64(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i128(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Int128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f8(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f16(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f32(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f64(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f128(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::Float128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_infer_type(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::InferType)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_unit(&mut self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::TupleType(TupleType::new(vec![])))
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Compound Type Builders
    pub fn create_tuple_type(&mut self) -> TupleTypeBuilder<'_, 'a> {
        TupleTypeBuilder::new(self.storage)
    }

    pub fn create_array_type(&mut self) -> ArrayTypeBuilder<'_, 'a> {
        ArrayTypeBuilder::new(self.storage)
    }

    pub fn create_slice_type(&mut self) -> SliceTypeBuilder<'_, 'a> {
        SliceTypeBuilder::new(self.storage)
    }

    pub fn create_struct_type(&mut self) -> StructTypeBuilder<'_, 'a> {
        StructTypeBuilder::new(self.storage)
    }

    pub fn create_function_type(&mut self) -> FunctionTypeBuilder<'_, 'a> {
        FunctionTypeBuilder::new(self.storage)
    }
}
