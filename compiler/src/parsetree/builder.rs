use crate::parsetree::{BinaryOperator, InnerExpr, OriginTag, UnaryOperator};

use super::array_type::ArrayType;
use super::binary_op::BinaryExpr;
use super::block::Block;
use super::character::CharLit;
use super::expression::{Expr, Metadata};
use super::function::Function;
use super::function_type::FunctionType;
use super::list::List;
use super::number::{FloatLit, IntegerLit};
use super::object::Object;
use super::returns::Return;
use super::statement::Statement;
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::types::{InnerType, Type};
use super::unary_op::UnaryExpr;
use super::variable::Variable;
use crate::lexer::{Float, Integer, IntegerKind, Token};
use std::rc::Rc;

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
    element_ty: Option<Rc<Type<'a>>>,
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

    pub fn with_element_ty(mut self, element_ty: Rc<Type<'a>>) -> Self {
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
    pub fn set_parenthesis(mut self, has_parenthesis: bool) -> Self {
        self.metadata.set_has_parenthesis(has_parenthesis);

        self
    }

    pub fn set_origin(mut self, origin: OriginTag) -> Self {
        self.metadata.set_origin(origin);
        self
    }

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

    pub fn array_type(self) -> ArrayTypeBuilderHelper<'a> {
        ArrayTypeBuilderHelper::new(self)
    }
}

fn test() {
    let element_ty = Rc::new(Type::new(InnerType::UInt8, false));
    let count = Box::new(Expr::new(
        InnerExpr::Float(FloatLit::new(5.0)),
        Metadata::default(),
    ));

    let e = Builder::default()
        .block()
        .add_expression(*count.clone())
        .add_expression(*count)
        .build();
}
