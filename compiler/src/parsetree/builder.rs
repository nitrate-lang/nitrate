use apint::UInt;

use crate::parsetree::{BinaryOperator, InnerExpr, OriginTag};

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

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct BinaryExprBuilderHelper<'a> {
    outer: Builder<'a>,
    left: Option<Box<Expr<'a>>>,
    right: Option<Box<Expr<'a>>>,
    operator: Option<BinaryOperator>,
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

    pub fn with_right(mut self, right: Box<Expr<'a>>) -> Self {
        self.right = Some(right);
        self
    }

    pub fn with_operator(mut self, operator: BinaryOperator) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let left = self.left.expect("Left expression must be provided");
        let right = self.right.expect("Right expression must be provided");
        let operator = self.operator.expect("Binary operator must be provided");

        let binary_expr = InnerExpr::BinaryOp(BinaryExpr::new(left, operator, right));

        Expr::new(binary_expr, self.outer.metadata)
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

    pub fn array_type(self) -> ArrayTypeBuilderHelper<'a> {
        ArrayTypeBuilderHelper::new(self)
    }

    pub fn binary_expr(self) -> BinaryExprBuilderHelper<'a> {
        BinaryExprBuilderHelper::new(self)
    }
}

fn test() {
    let element_ty = Rc::new(Type::new(InnerType::UInt8, false));
    let count = Box::new(Expr::new(
        InnerExpr::Float(FloatLit::new(5.0)),
        Metadata::default(),
    ));

    let e = Builder::default()
        .array_type()
        .with_element_ty(element_ty)
        .with_count(count)
        .build();
}
