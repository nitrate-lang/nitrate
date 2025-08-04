use super::builder_helper::*;
use super::expression::TypeOwned;
use super::storage::{Storage, TypeKey};
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

    /////////////////////////////////////////////////////////////////
    // BEGIN: Literal Expression Builders
    pub fn create_integer(&mut self) -> IntegerBuilderHelper<'_, 'a> {
        IntegerBuilderHelper::new(self.storage)
    }

    pub fn create_float(&mut self) -> FloatBuilderHelper<'_, 'a> {
        FloatBuilderHelper::new(self.storage)
    }

    pub fn create_string(&mut self) -> StringBuilderHelper<'_, 'a> {
        StringBuilderHelper::new(self.storage)
    }

    pub fn create_char(&mut self) -> CharBuilderHelper<'_, 'a> {
        CharBuilderHelper::new(self.storage)
    }

    pub fn create_list(&mut self) -> ListBuilderHelper<'_, 'a> {
        ListBuilderHelper::new(self.storage)
    }

    pub fn create_object(&mut self) -> ObjectBuilderHelper<'_, 'a> {
        ObjectBuilderHelper::new(self.storage)
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Compound Expression Builders
    pub fn create_unary_expr(&mut self) -> UnaryOpBuilderHelper<'_, 'a> {
        UnaryOpBuilderHelper::new(self.storage)
    }

    pub fn create_binary_expr(&mut self) -> BinaryOpBuilderHelper<'_, 'a> {
        BinaryOpBuilderHelper::new(self.storage)
    }

    pub fn create_statement(&mut self) -> StatementBuilderHelper<'_, 'a> {
        StatementBuilderHelper::new(self.storage)
    }

    // pub fn block(self) -> BlockBuilderHelper<'a> {
    //     BlockBuilderHelper::new(self)
    // }

    // /////////////////////////////////////////////////////////////////
    // BEGIN: Definition Builders
    // pub fn function(self) -> FunctionBuilderHelper<'a> {
    //     FunctionBuilderHelper::new(self)
    // }

    // pub fn variable(self) -> VariableBuilderHelper<'a> {
    //     VariableBuilderHelper::new(self)
    // }

    // /////////////////////////////////////////////////////////////////
    // // BEGIN: Control Flow Builders
    // pub fn return_(self) -> ReturnBuilderHelper<'a> {
    //     ReturnBuilderHelper::new(self)
    // }

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

    // /////////////////////////////////////////////////////////////////
    // // BEGIN: Compound Type Builders
    // pub fn tuple_type(self) -> TupleTypeBuilderHelper<'a> {
    //     TupleTypeBuilderHelper::new(self)
    // }

    // pub fn array_type(self) -> ArrayTypeBuilderHelper<'a> {
    //     ArrayTypeBuilderHelper::new(self)
    // }

    // pub fn struct_type(self) -> StructTypeBuilderHelper<'a> {
    //     StructTypeBuilderHelper::new(self)
    // }

    // pub fn function_type(self) -> FunctionTypeBuilderHelper<'a> {
    //     FunctionTypeBuilderHelper::new(self)
    // }
}
