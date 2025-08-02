use super::builder_helper::*;
use super::expression::{OwnedExpr, OwnedType};
use super::storage::{Storage, TypeRef};
use super::tuple_type::TupleType;
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

    // pub fn get_integer() -> IntegerBuilderHelper<'a> {
    //     IntegerBuilderHelper::new(Builder::default())
    // }

    // pub fn float(self) -> FloatBuilderHelper<'a> {
    //     FloatBuilderHelper::new(self)
    // }

    // pub fn get_float() -> FloatBuilderHelper<'a> {
    //     FloatBuilderHelper::new(Builder::default())
    // }

    // pub fn string(self) -> StringBuilderHelper<'a> {
    //     StringBuilderHelper::new(self)
    // }

    // pub fn get_string() -> StringBuilderHelper<'a> {
    //     StringBuilderHelper::new(Builder::default())
    // }

    // pub fn char(self) -> CharBuilderHelper<'a> {
    //     CharBuilderHelper::new(self)
    // }

    // pub fn get_char() -> CharBuilderHelper<'a> {
    //     CharBuilderHelper::new(Builder::default())
    // }

    // pub fn list(self) -> ListBuilderHelper<'a> {
    //     ListBuilderHelper::new(self)
    // }

    // pub fn get_list() -> ListBuilderHelper<'a> {
    //     ListBuilderHelper::new(Builder::default())
    // }

    // pub fn object(self) -> ObjectBuilderHelper<'a> {
    //     ObjectBuilderHelper::new(self)
    // }

    // pub fn get_object() -> ObjectBuilderHelper<'a> {
    //     ObjectBuilderHelper::new(Builder::default())
    // }

    // /////////////////////////////////////////////////////////////////
    // BEGIN: Compound Expression Builders
    // pub fn unary_expr(self) -> UnaryExprBuilderHelper<'a> {
    //     UnaryExprBuilderHelper::new(self)
    // }

    // pub fn get_unary_expr() -> UnaryExprBuilderHelper<'a> {
    //     UnaryExprBuilderHelper::new(Builder::default())
    // }

    // pub fn binary_expr(self) -> BinaryExprBuilderHelper<'a> {
    //     BinaryExprBuilderHelper::new(self)
    // }

    // pub fn get_binary_expr() -> BinaryExprBuilderHelper<'a> {
    //     BinaryExprBuilderHelper::new(Builder::default())
    // }

    // pub fn statement(self) -> StatementBuilderHelper<'a> {
    //     StatementBuilderHelper::new(self)
    // }

    // pub fn get_statement() -> StatementBuilderHelper<'a> {
    //     StatementBuilderHelper::new(Builder::default())
    // }

    // pub fn block(self) -> BlockBuilderHelper<'a> {
    //     BlockBuilderHelper::new(self)
    // }

    // pub fn get_block() -> BlockBuilderHelper<'a> {
    //     BlockBuilderHelper::new(Builder::default())
    // }

    // /////////////////////////////////////////////////////////////////
    // BEGIN: Definition Builders
    // pub fn function(self) -> FunctionBuilderHelper<'a> {
    //     FunctionBuilderHelper::new(self)
    // }

    // pub fn get_function() -> FunctionBuilderHelper<'a> {
    //     FunctionBuilderHelper::new(Builder::default())
    // }

    // pub fn variable(self) -> VariableBuilderHelper<'a> {
    //     VariableBuilderHelper::new(self)
    // }

    // pub fn get_variable() -> VariableBuilderHelper<'a> {
    //     VariableBuilderHelper::new(Builder::default())
    // }

    // /////////////////////////////////////////////////////////////////
    // // BEGIN: Control Flow Builders
    // pub fn return_(self) -> ReturnBuilderHelper<'a> {
    //     ReturnBuilderHelper::new(self)
    // }

    // pub fn get_return() -> ReturnBuilderHelper<'a> {
    //     ReturnBuilderHelper::new(Builder::default())
    // }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Primitive Type Builders
    pub fn get_bool(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Bool)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u8(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::UInt8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u16(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::UInt16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u32(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::UInt32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u64(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::UInt64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_u128(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::UInt128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i8(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Int8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i16(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Int16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i32(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Int32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i64(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Int64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_i128(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Int128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f8(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Float8)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f16(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Float16)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f32(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Float32)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f64(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Float64)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_f128(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::Float128)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_infer_type(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::InferType)
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    pub fn get_unit(&mut self) -> TypeRef<'a> {
        self.storage
            .add_type(OwnedType::TupleType(TupleType::new(vec![])))
            .expect(ADD_TYPE_EXPECT_REASON)
    }

    // /////////////////////////////////////////////////////////////////
    // // BEGIN: Compound Type Builders
    // pub fn tuple_type(self) -> TupleTypeBuilderHelper<'a> {
    //     TupleTypeBuilderHelper::new(self)
    // }

    // pub fn get_tuple_type() -> TupleTypeBuilderHelper<'a> {
    //     TupleTypeBuilderHelper::new(Builder::default())
    // }

    // pub fn array_type(self) -> ArrayTypeBuilderHelper<'a> {
    //     ArrayTypeBuilderHelper::new(self)
    // }

    // pub fn get_array_type() -> ArrayTypeBuilderHelper<'a> {
    //     ArrayTypeBuilderHelper::new(Builder::default())
    // }

    // pub fn struct_type(self) -> StructTypeBuilderHelper<'a> {
    //     StructTypeBuilderHelper::new(self)
    // }

    // pub fn get_struct_type() -> StructTypeBuilderHelper<'a> {
    //     StructTypeBuilderHelper::new(Builder::default())
    // }

    // pub fn function_type(self) -> FunctionTypeBuilderHelper<'a> {
    //     FunctionTypeBuilderHelper::new(self)
    // }

    // pub fn get_function_type() -> FunctionTypeBuilderHelper<'a> {
    //     FunctionTypeBuilderHelper::new(Builder::default())
    // }
}
