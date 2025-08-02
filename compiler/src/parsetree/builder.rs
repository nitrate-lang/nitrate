// use super::builder_helper::*;
// use super::expression::{Expr, Type};
// pub use super::variable::VariableKind;
// use std::sync::LazyLock;

// static TYPE_FACTORY: LazyLock<TypeFactory> = LazyLock::new(|| TypeFactory::new());

// #[derive(Debug, Clone, Default)]
// pub struct Builder<'a> {
//     inner: Option<Expr<'a>>,
// }

// impl<'a> Builder<'a> {
//     pub fn init() {
//         TYPE_FACTORY.get_unit();
//     }

//     /////////////////////////////////////////////////////////////////
//     /// BEGIN: Primitive Expression Builders
//     pub fn integer(self) -> IntegerBuilderHelper<'a> {
//         IntegerBuilderHelper::new(self)
//     }

//     pub fn get_integer() -> IntegerBuilderHelper<'a> {
//         IntegerBuilderHelper::new(Builder::default())
//     }

//     pub fn float(self) -> FloatBuilderHelper<'a> {
//         FloatBuilderHelper::new(self)
//     }

//     pub fn get_float() -> FloatBuilderHelper<'a> {
//         FloatBuilderHelper::new(Builder::default())
//     }

//     pub fn string(self) -> StringBuilderHelper<'a> {
//         StringBuilderHelper::new(self)
//     }

//     pub fn get_string() -> StringBuilderHelper<'a> {
//         StringBuilderHelper::new(Builder::default())
//     }

//     pub fn char(self) -> CharBuilderHelper<'a> {
//         CharBuilderHelper::new(self)
//     }

//     pub fn get_char() -> CharBuilderHelper<'a> {
//         CharBuilderHelper::new(Builder::default())
//     }

//     pub fn list(self) -> ListBuilderHelper<'a> {
//         ListBuilderHelper::new(self)
//     }

//     pub fn get_list() -> ListBuilderHelper<'a> {
//         ListBuilderHelper::new(Builder::default())
//     }

//     pub fn object(self) -> ObjectBuilderHelper<'a> {
//         ObjectBuilderHelper::new(self)
//     }

//     pub fn get_object() -> ObjectBuilderHelper<'a> {
//         ObjectBuilderHelper::new(Builder::default())
//     }

//     /////////////////////////////////////////////////////////////////
//     // BEGIN: Compound Expression Builders
//     pub fn unary_expr(self) -> UnaryExprBuilderHelper<'a> {
//         UnaryExprBuilderHelper::new(self)
//     }

//     pub fn get_unary_expr() -> UnaryExprBuilderHelper<'a> {
//         UnaryExprBuilderHelper::new(Builder::default())
//     }

//     pub fn binary_expr(self) -> BinaryExprBuilderHelper<'a> {
//         BinaryExprBuilderHelper::new(self)
//     }

//     pub fn get_binary_expr() -> BinaryExprBuilderHelper<'a> {
//         BinaryExprBuilderHelper::new(Builder::default())
//     }

//     pub fn statement(self) -> StatementBuilderHelper<'a> {
//         StatementBuilderHelper::new(self)
//     }

//     pub fn get_statement() -> StatementBuilderHelper<'a> {
//         StatementBuilderHelper::new(Builder::default())
//     }

//     pub fn block(self) -> BlockBuilderHelper<'a> {
//         BlockBuilderHelper::new(self)
//     }

//     pub fn get_block() -> BlockBuilderHelper<'a> {
//         BlockBuilderHelper::new(Builder::default())
//     }

//     /////////////////////////////////////////////////////////////////
//     /// BEGIN: Definition Builders
//     pub fn function(self) -> FunctionBuilderHelper<'a> {
//         FunctionBuilderHelper::new(self)
//     }

//     pub fn get_function() -> FunctionBuilderHelper<'a> {
//         FunctionBuilderHelper::new(Builder::default())
//     }

//     pub fn variable(self) -> VariableBuilderHelper<'a> {
//         VariableBuilderHelper::new(self)
//     }

//     pub fn get_variable() -> VariableBuilderHelper<'a> {
//         VariableBuilderHelper::new(Builder::default())
//     }

//     /////////////////////////////////////////////////////////////////
//     /// BEGIN: Control Flow Builders
//     pub fn return_(self) -> ReturnBuilderHelper<'a> {
//         ReturnBuilderHelper::new(self)
//     }

//     pub fn get_return() -> ReturnBuilderHelper<'a> {
//         ReturnBuilderHelper::new(Builder::default())
//     }

//     /////////////////////////////////////////////////////////////////
//     // BEGIN: Primitive Type Builders
//     pub fn get_bool() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_bool()
//     }

//     pub fn get_u8() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_u8()
//     }

//     pub fn get_u16() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_u16()
//     }

//     pub fn get_u32() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_u32()
//     }

//     pub fn get_u64() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_u64()
//     }

//     pub fn get_u128() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_u128()
//     }

//     pub fn get_i8() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_i8()
//     }

//     pub fn get_i16() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_i16()
//     }

//     pub fn get_i32() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_i32()
//     }

//     pub fn get_i64() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_i64()
//     }

//     pub fn get_i128() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_i128()
//     }

//     pub fn get_f8() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_f8()
//     }

//     pub fn get_f16() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_f16()
//     }

//     pub fn get_f32() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_f32()
//     }

//     pub fn get_f64() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_f64()
//     }

//     pub fn get_f128() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_f128()
//     }

//     pub fn get_infer_type() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_infer_type()
//     }

//     pub fn get_unit() -> Box<Type<'a>> {
//         TYPE_FACTORY.get_unit()
//     }

//     /////////////////////////////////////////////////////////////////
//     // BEGIN: Compound Type Builders
//     pub fn tuple_type(self) -> TupleTypeBuilderHelper<'a> {
//         TupleTypeBuilderHelper::new(self)
//     }

//     pub fn get_tuple_type() -> TupleTypeBuilderHelper<'a> {
//         TupleTypeBuilderHelper::new(Builder::default())
//     }

//     pub fn array_type(self) -> ArrayTypeBuilderHelper<'a> {
//         ArrayTypeBuilderHelper::new(self)
//     }

//     pub fn get_array_type() -> ArrayTypeBuilderHelper<'a> {
//         ArrayTypeBuilderHelper::new(Builder::default())
//     }

//     pub fn struct_type(self) -> StructTypeBuilderHelper<'a> {
//         StructTypeBuilderHelper::new(self)
//     }

//     pub fn get_struct_type() -> StructTypeBuilderHelper<'a> {
//         StructTypeBuilderHelper::new(Builder::default())
//     }

//     pub fn function_type(self) -> FunctionTypeBuilderHelper<'a> {
//         FunctionTypeBuilderHelper::new(self)
//     }

//     pub fn get_function_type() -> FunctionTypeBuilderHelper<'a> {
//         FunctionTypeBuilderHelper::new(Builder::default())
//     }
// }
