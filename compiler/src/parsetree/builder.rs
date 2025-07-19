use super::array_type::ArrayType;
use super::binary_op::BinaryExpr;
use super::block::Block;
use super::expression::{Expr, Metadata};
use super::number::FloatLit;
use super::statement::Statement;
use super::types::{InnerType, Type};
use super::unary_op::UnaryExpr;
use crate::parsetree::{BinaryOperator, InnerExpr, OriginTag, UnaryOperator};
use std::sync::{Arc, LazyLock};

#[derive(Debug)]
struct TypeFactory<'a> {
    bool: Arc<Type<'a>>,
    u8: Arc<Type<'a>>,
    u16: Arc<Type<'a>>,
    u32: Arc<Type<'a>>,
    u64: Arc<Type<'a>>,
    u128: Arc<Type<'a>>,
    i8: Arc<Type<'a>>,
    i16: Arc<Type<'a>>,
    i32: Arc<Type<'a>>,
    i64: Arc<Type<'a>>,
    i128: Arc<Type<'a>>,
    f8: Arc<Type<'a>>,
    f16: Arc<Type<'a>>,
    f32: Arc<Type<'a>>,
    f64: Arc<Type<'a>>,
    f128: Arc<Type<'a>>,
    infer_type: Arc<Type<'a>>,
}

impl<'a> TypeFactory<'a> {
    pub fn new() -> Self {
        TypeFactory {
            bool: Arc::new(Type::new(InnerType::Bool, false)),
            u8: Arc::new(Type::new(InnerType::UInt8, false)),
            u16: Arc::new(Type::new(InnerType::UInt16, false)),
            u32: Arc::new(Type::new(InnerType::UInt32, false)),
            u64: Arc::new(Type::new(InnerType::UInt64, false)),
            u128: Arc::new(Type::new(InnerType::UInt128, false)),
            i8: Arc::new(Type::new(InnerType::Int8, false)),
            i16: Arc::new(Type::new(InnerType::Int16, false)),
            i32: Arc::new(Type::new(InnerType::Int32, false)),
            i64: Arc::new(Type::new(InnerType::Int64, false)),
            i128: Arc::new(Type::new(InnerType::Int128, false)),
            f8: Arc::new(Type::new(InnerType::Float8, false)),
            f16: Arc::new(Type::new(InnerType::Float16, false)),
            f32: Arc::new(Type::new(InnerType::Float32, false)),
            f64: Arc::new(Type::new(InnerType::Float64, false)),
            f128: Arc::new(Type::new(InnerType::Float128, false)),
            infer_type: Arc::new(Type::new(InnerType::InferType, false)),
        }
    }

    pub fn get_bool(&self) -> Arc<Type<'a>> {
        self.bool.clone()
    }

    pub fn get_u8(&self) -> Arc<Type<'a>> {
        self.u8.clone()
    }

    pub fn get_u16(&self) -> Arc<Type<'a>> {
        self.u16.clone()
    }

    pub fn get_u32(&self) -> Arc<Type<'a>> {
        self.u32.clone()
    }

    pub fn get_u64(&self) -> Arc<Type<'a>> {
        self.u64.clone()
    }

    pub fn get_u128(&self) -> Arc<Type<'a>> {
        self.u128.clone()
    }

    pub fn get_i8(&self) -> Arc<Type<'a>> {
        self.i8.clone()
    }

    pub fn get_i16(&self) -> Arc<Type<'a>> {
        self.i16.clone()
    }

    pub fn get_i32(&self) -> Arc<Type<'a>> {
        self.i32.clone()
    }

    pub fn get_i64(&self) -> Arc<Type<'a>> {
        self.i64.clone()
    }

    pub fn get_i128(&self) -> Arc<Type<'a>> {
        self.i128.clone()
    }

    pub fn get_f8(&self) -> Arc<Type<'a>> {
        self.f8.clone()
    }

    pub fn get_f16(&self) -> Arc<Type<'a>> {
        self.f16.clone()
    }

    pub fn get_f32(&self) -> Arc<Type<'a>> {
        self.f32.clone()
    }

    pub fn get_f64(&self) -> Arc<Type<'a>> {
        self.f64.clone()
    }

    pub fn get_f128(&self) -> Arc<Type<'a>> {
        self.f128.clone()
    }

    pub fn get_infer_type(&self) -> Arc<Type<'a>> {
        self.infer_type.clone()
    }
}

static TYPE_FACTORY: LazyLock<TypeFactory> = LazyLock::new(|| TypeFactory::new());

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct UnaryExprBuilderHelper<'a> {
    outer: Builder<'a>,
    operator: Option<UnaryOperator>,
    operand: Option<Box<Expr<'a>>>,
    is_postfix: Option<bool>,
}

impl<'a> UnaryExprBuilderHelper<'a> {
    fn new(outer: Builder<'a>) -> Self {
        UnaryExprBuilderHelper {
            outer,
            operator: None,
            operand: None,
            is_postfix: None,
        }
    }

    pub fn with_operator(mut self, operator: UnaryOperator) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_operand(mut self, operand: Box<Expr<'a>>) -> Self {
        self.operand = Some(operand);
        self
    }

    pub fn set_postfix(mut self, is_postfix: bool) -> Self {
        self.is_postfix = Some(is_postfix);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let operator = self.operator.expect("Unary operator must be provided");
        let operand = self.operand.expect("Operand must be provided");
        let is_postfix = self.is_postfix.expect("is_postfix flag must be provided");

        let unary_expr = InnerExpr::UnaryOp(UnaryExpr::new(operand, operator, is_postfix));

        Expr::new(unary_expr, self.outer.metadata)
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct BinaryExprBuilderHelper<'a> {
    outer: Builder<'a>,
    left: Option<Box<Expr<'a>>>,
    operator: Option<BinaryOperator>,
    right: Option<Box<Expr<'a>>>,
}

impl<'a> BinaryExprBuilderHelper<'a> {
    fn new(outer: Builder<'a>) -> Self {
        BinaryExprBuilderHelper {
            outer,
            left: None,
            right: None,
            operator: None,
        }
    }

    pub fn with_left(mut self, left: Box<Expr<'a>>) -> Self {
        self.left = Some(left);
        self
    }

    pub fn with_operator(mut self, operator: BinaryOperator) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_right(mut self, right: Box<Expr<'a>>) -> Self {
        self.right = Some(right);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let left = self.left.expect("Left expression must be provided");
        let operator = self.operator.expect("Binary operator must be provided");
        let right = self.right.expect("Right expression must be provided");

        let binary_expr = InnerExpr::BinaryOp(BinaryExpr::new(left, operator, right));

        Expr::new(binary_expr, self.outer.metadata)
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StatementBuilderHelper<'a> {
    outer: Builder<'a>,
    expression: Option<Box<Expr<'a>>>,
}

impl<'a> StatementBuilderHelper<'a> {
    fn new(outer: Builder<'a>) -> Self {
        StatementBuilderHelper {
            outer,
            expression: None,
        }
    }

    pub fn with_expression(mut self, expression: Box<Expr<'a>>) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expression = self.expression.expect("Expression must be provided");
        let statement = InnerExpr::Statement(Statement::new(expression));

        Expr::new(statement, self.outer.metadata)
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct BlockBuilderHelper<'a> {
    outer: Builder<'a>,
    expressions: Vec<Expr<'a>>,
}

impl<'a> BlockBuilderHelper<'a> {
    fn new(outer: Builder<'a>) -> Self {
        BlockBuilderHelper {
            outer,
            expressions: Vec::new(),
        }
    }

    pub fn add_expression(mut self, expr: Expr<'a>) -> Self {
        self.expressions.push(expr);
        self
    }

    pub fn add_expressions<I>(mut self, expressions: I) -> Self
    where
        I: IntoIterator<Item = Expr<'a>>,
    {
        self.expressions.extend(expressions);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let block = InnerExpr::Block(Block::new(self.expressions));

        Expr::new(block, self.outer.metadata)
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct ArrayTypeBuilderHelper<'a> {
    outer: Builder<'a>,
    element_ty: Option<Arc<Type<'a>>>,
    count: Option<Box<Expr<'a>>>,
}

impl<'a> ArrayTypeBuilderHelper<'a> {
    fn new(outer: Builder<'a>) -> Self {
        ArrayTypeBuilderHelper {
            outer,
            element_ty: None,
            count: None,
        }
    }

    pub fn with_element_ty(mut self, element_ty: Arc<Type<'a>>) -> Self {
        self.element_ty = Some(element_ty);
        self
    }

    pub fn with_count(mut self, count: Box<Expr<'a>>) -> Self {
        self.count = Some(count);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let array_type = InnerExpr::ArrayType(ArrayType::new(
            self.element_ty.unwrap(),
            self.count.unwrap(),
        ));

        Expr::new(array_type, self.outer.metadata)
    }
}

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

    /////////////////////////////////////////////////////////////////
    // BEGIN: Compound Expression Builders

    pub fn unary_expr(self) -> UnaryExprBuilderHelper<'a> {
        UnaryExprBuilderHelper::new(self)
    }

    pub fn binary_expr(self) -> BinaryExprBuilderHelper<'a> {
        BinaryExprBuilderHelper::new(self)
    }

    pub fn statement(self) -> StatementBuilderHelper<'a> {
        StatementBuilderHelper::new(self)
    }

    pub fn block(self) -> BlockBuilderHelper<'a> {
        BlockBuilderHelper::new(self)
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
}

fn test() {
    let element_ty = Arc::new(Type::new(InnerType::UInt8, false));
    let count = Box::new(Expr::new(
        InnerExpr::Float(FloatLit::new(5.0)),
        Metadata::default(),
    ));

    let e = Builder::get_u8();
}
