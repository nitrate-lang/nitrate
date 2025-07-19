use super::array_type::ArrayType;
use super::binary_op::{BinaryExpr, BinaryOperator};
use super::block::Block;
use super::builder::Builder;
use super::character::CharLit;
use super::expression::{Expr, InnerExpr};
use super::function::{Function, FunctionParameter};
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
use super::unary_op::{UnaryExpr, UnaryOperator};
use super::variable::{Variable, VariableKind};
use crate::lexer::IntegerKind;
use apint::UInt;
use std::collections::BTreeMap;
use std::sync::Arc;

#[derive(Debug)]
pub struct TypeFactory<'a> {
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
    unit: Arc<Type<'a>>,
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
            unit: Arc::new(Type::new(
                InnerType::TupleType(Box::new(TupleType::new(Vec::new()))),
                false,
            )),
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

    pub fn get_unit(&self) -> Arc<Type<'a>> {
        self.unit.clone()
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct IntegerBuilderHelper<'a> {
    outer: Builder<'a>,
    value: Option<UInt>,
    kind: Option<IntegerKind>,
}

impl<'a> IntegerBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        IntegerBuilderHelper {
            outer,
            value: None,
            kind: None,
        }
    }

    pub fn with_u8(mut self, value: u8) -> Self {
        self.value = Some(UInt::from_u8(value));
        self
    }

    pub fn with_u16(mut self, value: u16) -> Self {
        self.value = Some(UInt::from_u16(value));
        self
    }

    pub fn with_u32(mut self, value: u32) -> Self {
        self.value = Some(UInt::from_u32(value));
        self
    }

    pub fn with_u64(mut self, value: u64) -> Self {
        self.value = Some(UInt::from_u64(value));
        self
    }

    pub fn with_u128(mut self, value: u128) -> Self {
        self.value = Some(UInt::from_u128(value));
        self
    }

    pub fn with_kind(mut self, kind: IntegerKind) -> Self {
        self.kind = Some(kind);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let value = self.value.expect("Integer value must be provided");
        let kind = self.kind.unwrap_or(IntegerKind::Decimal);

        let integer_lit = IntegerLit::new(value, kind).expect("Failed to create IntegerLit");

        Expr::new(
            InnerExpr::Integer(Box::new(integer_lit)),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct FloatBuilderHelper<'a> {
    outer: Builder<'a>,
    value: Option<f64>,
}

impl<'a> FloatBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        FloatBuilderHelper { outer, value: None }
    }

    pub fn with_value(mut self, value: f64) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let value = self.value.expect("Float value must be provided");

        Expr::new(
            InnerExpr::Float(FloatLit::new(value)),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StringBuilderHelper<'a> {
    outer: Builder<'a>,
    value: Option<&'a [u8]>,
}

impl<'a> StringBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        StringBuilderHelper { outer, value: None }
    }

    pub fn with_string(mut self, value: &'a str) -> Self {
        self.value = Some(value.as_bytes());
        self
    }

    pub fn with_bytes(mut self, value: &'a [u8]) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let value = self.value.expect("String value must be provided");

        Expr::new(
            InnerExpr::String(Box::new(StringLit::new(value))),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct CharBuilderHelper<'a> {
    outer: Builder<'a>,
    value: Option<char>,
}

impl<'a> CharBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        CharBuilderHelper { outer, value: None }
    }

    pub fn with_char(mut self, value: char) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let value = self.value.expect("Char value must be provided");

        Expr::new(
            InnerExpr::Char(CharLit::new(value)),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct ListBuilderHelper<'a> {
    outer: Builder<'a>,
    elements: Vec<Expr<'a>>,
}

impl<'a> ListBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        ListBuilderHelper {
            outer,
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, element: Expr<'a>) -> Self {
        self.elements.push(element);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Expr<'a>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::new(
            InnerExpr::List(Box::new(List::new(self.elements))),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct ObjectBuilderHelper<'a> {
    outer: Builder<'a>,
    fields: BTreeMap<&'a str, Expr<'a>>,
}

impl<'a> ObjectBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        ObjectBuilderHelper {
            outer,
            fields: BTreeMap::new(),
        }
    }

    pub fn add_field(mut self, key: &'a str, value: Expr<'a>) -> Self {
        self.fields.insert(key, value);
        self
    }

    pub fn add_fields<I>(mut self, fields: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Expr<'a>)>,
    {
        self.fields.extend(fields);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::new(
            InnerExpr::Object(Box::new(Object::new(self.fields))),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct UnaryExprBuilderHelper<'a> {
    outer: Builder<'a>,
    operator: Option<UnaryOperator>,
    operand: Option<Expr<'a>>,
    is_postfix: Option<bool>,
}

impl<'a> UnaryExprBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
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

    pub fn with_operand(mut self, operand: Expr<'a>) -> Self {
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

        let unary = UnaryExpr::new(operand, operator, is_postfix);

        Expr::new(
            InnerExpr::UnaryOp(Box::new(unary)),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct BinaryExprBuilderHelper<'a> {
    outer: Builder<'a>,
    left: Option<Expr<'a>>,
    operator: Option<BinaryOperator>,
    right: Option<Expr<'a>>,
}

impl<'a> BinaryExprBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        BinaryExprBuilderHelper {
            outer,
            left: None,
            right: None,
            operator: None,
        }
    }

    pub fn with_left(mut self, left: Expr<'a>) -> Self {
        self.left = Some(left);
        self
    }

    pub fn with_operator(mut self, operator: BinaryOperator) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_right(mut self, right: Expr<'a>) -> Self {
        self.right = Some(right);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let left = self.left.expect("Left expression must be provided");
        let operator = self.operator.expect("Binary operator must be provided");
        let right = self.right.expect("Right expression must be provided");

        let binary_expr = BinaryExpr::new(left, operator, right);

        Expr::new(
            InnerExpr::BinaryOp(Box::new(binary_expr)),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StatementBuilderHelper<'a> {
    outer: Builder<'a>,
    expression: Option<Expr<'a>>,
}

impl<'a> StatementBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        StatementBuilderHelper {
            outer,
            expression: None,
        }
    }

    pub fn with_expression(mut self, expression: Expr<'a>) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expression = self.expression.expect("Expression must be provided");

        Expr::new(
            InnerExpr::Statement(Box::new(Statement::new(expression))),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct BlockBuilderHelper<'a> {
    outer: Builder<'a>,
    elements: Vec<Expr<'a>>,
}

impl<'a> BlockBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        BlockBuilderHelper {
            outer,
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, expr: Expr<'a>) -> Self {
        self.elements.push(expr);
        self
    }

    pub fn add_expressions<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Expr<'a>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn add_statement(mut self, expression: Expr<'a>) -> Self {
        let statement = Builder::get_statement().with_expression(expression).build();
        self.elements.push(statement);
        self
    }

    pub fn add_statements<I>(mut self, statements: I) -> Self
    where
        I: IntoIterator<Item = Expr<'a>>,
    {
        for statement in statements {
            let statement = Builder::get_statement().with_expression(statement).build();
            self.elements.push(statement);
        }
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::new(
            InnerExpr::Block(Box::new(Block::new(self.elements))),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct FunctionBuilderHelper<'a> {
    outer: Builder<'a>,
    name: &'a str,
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Arc<Type<'a>>>,
    attributes: Vec<Expr<'a>>,
    definition: Option<Expr<'a>>,
}

impl<'a> FunctionBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        FunctionBuilderHelper {
            outer,
            name: "",
            parameters: Vec::new(),
            return_type: None,
            attributes: Vec::new(),
            definition: None,
        }
    }

    pub fn with_name(mut self, name: &'a str) -> Self {
        self.name = name;
        self
    }

    pub fn with_parameter(
        mut self,
        name: &'a str,
        ty: Option<Arc<Type<'a>>>,
        default_value: Option<Expr<'a>>,
    ) -> Self {
        self.parameters
            .push((name, ty, default_value.map(|v| v.into())));
        self
    }

    pub fn with_parameters<I>(mut self, parameters: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Option<Arc<Type<'a>>>, Option<Expr<'a>>)>,
    {
        self.parameters.extend(parameters);
        self
    }

    pub fn with_return_type(mut self, ty: Arc<Type<'a>>) -> Self {
        self.return_type = Some(ty);
        self
    }

    pub fn with_attribute(mut self, attribute: Expr<'a>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn with_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Expr<'a>>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn with_definition(mut self, definition: Expr<'a>) -> Self {
        match definition.get() {
            InnerExpr::Block(_) => {
                self.definition = Some(definition);
                self
            }

            _ => panic!("Function definition must be a block expression"),
        }
    }

    pub fn build(self) -> Expr<'a> {
        let definition = self.definition.map(|d| match d.into_inner() {
            InnerExpr::Block(block) => Block::new(block.into_inner()),
            _ => panic!("Function definition must be a block expression"),
        });

        let function = Function::new(
            self.name,
            self.parameters,
            self.return_type,
            self.attributes,
            definition,
        );

        Expr::new(
            InnerExpr::Function(Box::new(function)),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct VariableBuilderHelper<'a> {
    outer: Builder<'a>,
    kind: Option<VariableKind>,
    name: &'a str,
    ty: Option<Arc<Type<'a>>>,
    value: Option<Expr<'a>>,
}

impl<'a> VariableBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        VariableBuilderHelper {
            outer,
            kind: None,
            name: "",
            ty: None,
            value: None,
        }
    }

    pub fn with_kind(mut self, kind: VariableKind) -> Self {
        self.kind = Some(kind);
        self
    }

    pub fn with_name(mut self, name: &'a str) -> Self {
        self.name = name;
        self
    }

    pub fn with_type(mut self, ty: Arc<Type<'a>>) -> Self {
        self.ty = Some(ty);
        self
    }

    pub fn with_value(mut self, value: Expr<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let kind = self.kind.expect("Variable kind must be provided");

        Expr::new(
            InnerExpr::Variable(Box::new(Variable::new(
                kind, self.name, self.ty, self.value,
            ))),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct ReturnBuilderHelper<'a> {
    outer: Builder<'a>,
    value: Option<Expr<'a>>,
}

impl<'a> ReturnBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        ReturnBuilderHelper { outer, value: None }
    }

    pub fn with_value(mut self, value: Expr<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::new(
            InnerExpr::Return(Box::new(Return::new(self.value))),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct TupleTypeBuilderHelper<'a> {
    outer: Builder<'a>,
    elements: Vec<Arc<Type<'a>>>,
}

impl<'a> TupleTypeBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        TupleTypeBuilderHelper {
            outer,
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, ty: Arc<Type<'a>>) -> Self {
        self.elements.push(ty);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Arc<Type<'a>>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::new(
            InnerExpr::TupleType(Box::new(TupleType::new(self.elements))),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct ArrayTypeBuilderHelper<'a> {
    outer: Builder<'a>,
    element_ty: Option<Arc<Type<'a>>>,
    count: Option<Expr<'a>>,
}

impl<'a> ArrayTypeBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
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

    pub fn with_count(mut self, count: Expr<'a>) -> Self {
        self.count = Some(count);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let element_ty = self.element_ty.expect("Element type must be provided");
        let count = self.count.expect("Count must be provided");

        let array_type = ArrayType::new(element_ty, count);

        Expr::new(
            InnerExpr::ArrayType(Box::new(array_type)),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StructTypeBuilderHelper<'a> {
    outer: Builder<'a>,
    name: Option<&'a str>,
    attributes: Vec<Expr<'a>>,
    fields: BTreeMap<&'a str, Arc<Type<'a>>>,
}

impl<'a> StructTypeBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        StructTypeBuilderHelper {
            outer,
            name: None,
            attributes: Vec::new(),
            fields: BTreeMap::new(),
        }
    }

    pub fn with_name(mut self, name: &'a str) -> Self {
        self.name = Some(name);
        self
    }

    pub fn add_attribute(mut self, attribute: Expr<'a>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn add_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Expr<'a>>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn add_field(mut self, name: &'a str, ty: Arc<Type<'a>>) -> Self {
        self.fields.insert(name, ty);
        self
    }

    pub fn add_fields<I>(mut self, fields: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Arc<Type<'a>>)>,
    {
        self.fields.extend(fields);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let struct_type = StructType::new(self.name, self.attributes, self.fields);

        Expr::new(
            InnerExpr::StructType(Box::new(struct_type)),
            self.outer.get_metadata(),
        )
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct FunctionTypeBuilderHelper<'a> {
    outer: Builder<'a>,
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Arc<Type<'a>>>,
    attributes: Vec<Expr<'a>>,
}

impl<'a> FunctionTypeBuilderHelper<'a> {
    pub fn new(outer: Builder<'a>) -> Self {
        FunctionTypeBuilderHelper {
            outer,
            parameters: Vec::new(),
            return_type: None,
            attributes: Vec::new(),
        }
    }

    pub fn add_parameter(
        mut self,
        name: &'a str,
        ty: Option<Arc<Type<'a>>>,
        default_value: Option<Expr<'a>>,
    ) -> Self {
        self.parameters
            .push((name, ty, default_value.map(|v| v.into())));
        self
    }

    pub fn add_parameters<I>(mut self, parameters: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Option<Arc<Type<'a>>>, Option<Expr<'a>>)>,
    {
        self.parameters.extend(parameters);
        self
    }

    pub fn with_return_type(mut self, ty: Arc<Type<'a>>) -> Self {
        self.return_type = Some(ty);
        self
    }

    pub fn add_attribute(mut self, attribute: Expr<'a>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn add_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Expr<'a>>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let function_type = FunctionType::new(self.parameters, self.return_type, self.attributes);

        Expr::new(
            InnerExpr::FunctionType(Box::new(function_type)),
            self.outer.get_metadata(),
        )
    }
}
