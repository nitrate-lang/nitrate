use super::builder_helper::*;
use super::expression::{InnerExpr, Metadata, OriginTag};
use super::types::Type;
use std::sync::{Arc, LazyLock};

static TYPE_FACTORY: LazyLock<TypeFactory> = LazyLock::new(|| TypeFactory::new());

#[derive(Debug, Clone, Default, PartialEq, Eq, PartialOrd, Hash)]
pub struct Builder<'a> {
    inner: Option<InnerExpr<'a>>,
    metadata: Metadata<'a>,
}

impl<'a> Builder<'a> {
    /////////////////////////////////////////////////////////////////
    // BEGIN: Metadata Setters

    pub fn set_parenthesis(mut self, has_parenthesis: bool) -> Self {
        self.metadata.set_has_parenthesis(has_parenthesis);

        self
    }

    pub fn set_origin(mut self, origin: OriginTag) -> Self {
        self.metadata.set_origin(origin);
        self
    }

    pub fn get_metadata(self) -> Metadata<'a> {
        self.metadata
    }

    /////////////////////////////////////////////////////////////////
    /// BEGIN: Primitive Expression Builders
    pub fn integer(self) -> IntegerBuilderHelper<'a> {
        IntegerBuilderHelper::new(self)
    }

    pub fn get_integer() -> IntegerBuilderHelper<'a> {
        IntegerBuilderHelper::new(Builder::default())
    }

    pub fn float(self) -> FloatBuilderHelper<'a> {
        FloatBuilderHelper::new(self)
    }

    pub fn get_float() -> FloatBuilderHelper<'a> {
        FloatBuilderHelper::new(Builder::default())
    }

    pub fn string(self) -> StringBuilderHelper<'a> {
        StringBuilderHelper::new(self)
    }

    pub fn get_string() -> StringBuilderHelper<'a> {
        StringBuilderHelper::new(Builder::default())
    }

    pub fn char(self) -> CharBuilderHelper<'a> {
        CharBuilderHelper::new(self)
    }

    pub fn get_char() -> CharBuilderHelper<'a> {
        CharBuilderHelper::new(Builder::default())
    }

    pub fn list(self) -> ListBuilderHelper<'a> {
        ListBuilderHelper::new(self)
    }

    pub fn get_list() -> ListBuilderHelper<'a> {
        ListBuilderHelper::new(Builder::default())
    }

    pub fn object(self) -> ObjectBuilderHelper<'a> {
        ObjectBuilderHelper::new(self)
    }

    pub fn get_object() -> ObjectBuilderHelper<'a> {
        ObjectBuilderHelper::new(Builder::default())
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Compound Expression Builders

    pub fn unary_expr(self) -> UnaryExprBuilderHelper<'a> {
        UnaryExprBuilderHelper::new(self)
    }

    pub fn get_unary_expr() -> UnaryExprBuilderHelper<'a> {
        UnaryExprBuilderHelper::new(Builder::default())
    }

    pub fn binary_expr(self) -> BinaryExprBuilderHelper<'a> {
        BinaryExprBuilderHelper::new(self)
    }

    pub fn get_binary_expr() -> BinaryExprBuilderHelper<'a> {
        BinaryExprBuilderHelper::new(Builder::default())
    }

    pub fn statement(self) -> StatementBuilderHelper<'a> {
        StatementBuilderHelper::new(self)
    }

    pub fn get_statement() -> StatementBuilderHelper<'a> {
        StatementBuilderHelper::new(Builder::default())
    }

    pub fn block(self) -> BlockBuilderHelper<'a> {
        BlockBuilderHelper::new(self)
    }

    pub fn get_block() -> BlockBuilderHelper<'a> {
        BlockBuilderHelper::new(Builder::default())
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Primitive Type Builders
    pub fn get_bool() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_bool()
    }

    pub fn get_u8() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_u8()
    }

    pub fn get_u16() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_u16()
    }

    pub fn get_u32() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_u32()
    }

    pub fn get_u64() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_u64()
    }

    pub fn get_u128() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_u128()
    }

    pub fn get_i8() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_i8()
    }

    pub fn get_i16() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_i16()
    }

    pub fn get_i32() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_i32()
    }

    pub fn get_i64() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_i64()
    }

    pub fn get_i128() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_i128()
    }

    pub fn get_f8() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_f8()
    }

    pub fn get_f16() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_f16()
    }

    pub fn get_f32() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_f32()
    }

    pub fn get_f64() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_f64()
    }

    pub fn get_f128() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_f128()
    }

    pub fn get_infer_type() -> Arc<Type<'a>> {
        TYPE_FACTORY.get_infer_type()
    }

    /////////////////////////////////////////////////////////////////
    // BEGIN: Compound Type Builders

    pub fn array_type(self) -> ArrayTypeBuilderHelper<'a> {
        ArrayTypeBuilderHelper::new(self)
    }

    pub fn get_array_type() -> ArrayTypeBuilderHelper<'a> {
        ArrayTypeBuilderHelper::new(Builder::default())
    }
}

pub fn test_builder() {
    let _e = Builder::get_list()
        .add_element(Builder::get_integer().with_u32(42).build())
        .add_element(Builder::get_string().with_string("Hello").build())
        .build();
}
