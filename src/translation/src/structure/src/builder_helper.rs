use crate::Builder;
use apint::UInt;
use interned_string::IString;
use nitrate_tokenize::IntegerKind;
use std::sync::Arc;
use std::{collections::BTreeMap, sync::RwLock};

use crate::kind::{
    ArrayType, Assert, Await, BinExpr, BinExprOp, Block, Break, Call, CallArguments, Continue,
    DoWhileLoop, Expr, ForEach, Function, FunctionParameter, FunctionType, GenericType, If,
    IndexAccess, Integer, List, ManagedRefType, MapType, Object, RefinementType, Return, Scope,
    SliceType, Statement, StructField, StructType, Switch, TupleType, Type, UnaryExpr, UnaryExprOp,
    UnmanagedRefType, Variable, VariableKind, WhileLoop,
};

#[derive(Debug)]
pub struct RefinementTypeBuilder<'a> {
    base: Option<Type<'a>>,
    width: Option<Expr<'a>>,
    minimum: Option<Expr<'a>>,
    maximum: Option<Expr<'a>>,
}

impl<'a> RefinementTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        RefinementTypeBuilder {
            base: None,
            width: None,
            minimum: None,
            maximum: None,
        }
    }

    pub fn with_base(mut self, base: Type<'a>) -> Self {
        self.base = Some(base);
        self
    }

    pub fn with_width(mut self, width: Option<Expr<'a>>) -> Self {
        self.width = width;
        self
    }

    pub fn with_minimum(mut self, minimum: Option<Expr<'a>>) -> Self {
        self.minimum = minimum;
        self
    }

    pub fn with_maximum(mut self, maximum: Option<Expr<'a>>) -> Self {
        self.maximum = maximum;
        self
    }

    pub fn build(self) -> Type<'a> {
        Type::RefinementType(Arc::new(RefinementType::new(
            self.base.expect("Principal type must be provided"),
            self.width,
            self.minimum,
            self.maximum,
        )))
    }
}

#[derive(Debug)]
pub struct TupleTypeBuilder<'a> {
    elements: Vec<Type<'a>>,
}

impl<'a> TupleTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        TupleTypeBuilder {
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, ty: Type<'a>) -> Self {
        self.elements.push(ty);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Type<'a>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> Type<'a> {
        if self.elements.is_empty() {
            Type::UnitType
        } else {
            Type::TupleType(Arc::new(TupleType::new(self.elements)))
        }
    }
}

#[derive(Debug)]
pub struct ArrayTypeBuilder<'a> {
    element: Option<Type<'a>>,
    count: Option<Expr<'a>>,
}

impl<'a> ArrayTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        ArrayTypeBuilder {
            element: None,
            count: None,
        }
    }

    pub fn with_element(mut self, element: Type<'a>) -> Self {
        self.element = Some(element);
        self
    }

    pub fn with_count(mut self, count: Expr<'a>) -> Self {
        self.count = Some(count);
        self
    }

    pub fn build(self) -> Type<'a> {
        Type::ArrayType(Arc::new(ArrayType::new(
            self.element.expect("Element type must be provided"),
            self.count.expect("Array length must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct MapTypeBuilder<'a> {
    key: Option<Type<'a>>,
    value: Option<Type<'a>>,
}

impl<'a> MapTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        MapTypeBuilder {
            key: None,
            value: None,
        }
    }

    pub fn with_key(mut self, key: Type<'a>) -> Self {
        self.key = Some(key);
        self
    }

    pub fn with_value(mut self, value: Type<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Type<'a> {
        Type::MapType(Arc::new(MapType::new(
            self.key.expect("Key type must be provided"),
            self.value.expect("Value type must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct SliceTypeBuilder<'a> {
    element: Option<Type<'a>>,
}

impl<'a> SliceTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        SliceTypeBuilder { element: None }
    }

    pub fn with_element(mut self, element: Type<'a>) -> Self {
        self.element = Some(element);
        self
    }

    pub fn build(self) -> Type<'a> {
        Type::SliceType(Arc::new(SliceType::new(
            self.element.expect("Element type must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct FunctionTypeBuilder<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Type<'a>>,
    attributes: Vec<Expr<'a>>,
}

impl<'a> FunctionTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        FunctionTypeBuilder {
            parameters: Vec::new(),
            return_type: None,
            attributes: Vec::new(),
        }
    }

    pub fn add_parameter(
        mut self,
        name: IString,
        ty: Type<'a>,
        default_value: Option<Expr<'a>>,
    ) -> Self {
        self.parameters
            .push(FunctionParameter::new(name, ty, default_value));
        self
    }

    pub fn add_parameters<I>(mut self, parameters: I) -> Self
    where
        I: IntoIterator<Item = FunctionParameter<'a>>,
    {
        self.parameters.extend(parameters);
        self
    }

    pub fn with_return_type(mut self, ty: Type<'a>) -> Self {
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

    pub fn build(self) -> Type<'a> {
        Type::FunctionType(Arc::new(FunctionType::new(
            self.parameters,
            self.return_type.expect("Return type must be provided"),
            self.attributes,
        )))
    }
}

#[derive(Debug)]
pub struct ManagedRefTypeBuilder<'a> {
    target: Option<Type<'a>>,
    is_mutable: bool,
}

impl<'a> ManagedRefTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        ManagedRefTypeBuilder {
            target: None,
            is_mutable: false,
        }
    }

    pub fn with_target(mut self, target: Type<'a>) -> Self {
        self.target = Some(target);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn build(self) -> Type<'a> {
        Type::ManagedRefType(Arc::new(ManagedRefType::new(
            self.target.expect("Target type must be provided"),
            self.is_mutable,
        )))
    }
}

#[derive(Debug)]
pub struct UnmanagedRefTypeBuilder<'a> {
    target: Option<Type<'a>>,
    is_mutable: bool,
}

impl<'a> UnmanagedRefTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        UnmanagedRefTypeBuilder {
            target: None,
            is_mutable: false,
        }
    }

    pub fn with_target(mut self, target: Type<'a>) -> Self {
        self.target = Some(target);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn build(self) -> Type<'a> {
        Type::UnmanagedRefType(Arc::new(UnmanagedRefType::new(
            self.target.expect("Target type must be provided"),
            self.is_mutable,
        )))
    }
}

#[derive(Debug)]
pub struct GenericTypeBuilder<'a> {
    base: Option<Type<'a>>,
    arguments: Vec<(IString, Expr<'a>)>,
}

impl<'a> GenericTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        GenericTypeBuilder {
            base: None,
            arguments: Vec::new(),
        }
    }

    pub fn with_base(mut self, base: Type<'a>) -> Self {
        self.base = Some(base);
        self
    }

    pub fn add_argument(mut self, name: IString, value: Expr<'a>) -> Self {
        self.arguments.push((name, value));
        self
    }

    pub fn add_arguments<I>(mut self, arguments: I) -> Self
    where
        I: IntoIterator<Item = (IString, Expr<'a>)>,
    {
        self.arguments.extend(arguments);
        self
    }

    pub fn build(self) -> Type<'a> {
        Type::GenericType(Arc::new(GenericType::new(
            self.base.expect("Principal type must be provided"),
            self.arguments,
        )))
    }
}

#[derive(Debug)]
pub struct StructTypeBuilder<'a> {
    fields: Vec<StructField<'a>>,
}

impl<'a> StructTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        StructTypeBuilder { fields: Vec::new() }
    }

    pub fn add_field(
        mut self,
        name: IString,
        ty: Type<'a>,
        default_value: Option<Expr<'a>>,
    ) -> Self {
        self.fields.push((name, ty, default_value));
        self
    }

    pub fn add_fields<I>(mut self, fields: I) -> Self
    where
        I: IntoIterator<Item = StructField<'a>>,
    {
        self.fields.extend(fields);
        self
    }

    pub fn build(self) -> Type<'a> {
        Type::StructType(Arc::new(StructType::new(self.fields)))
    }
}

#[derive(Debug)]
pub struct IntegerBuilder {
    value: Option<UInt>,
    kind: Option<IntegerKind>,
}

impl IntegerBuilder {
    pub(crate) fn new() -> Self {
        IntegerBuilder {
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

    pub fn build<'a>(self) -> Expr<'a> {
        let lit = Integer::new(
            self.value.expect("Integer value must be provided"),
            self.kind.unwrap_or(IntegerKind::Dec),
        )
        .expect("Invalid integer value");

        Expr::Integer(Arc::new(lit))
    }
}

#[derive(Debug)]
pub struct ListBuilder<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> ListBuilder<'a> {
    pub(crate) fn new() -> Self {
        ListBuilder {
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

    pub fn prepend_element(mut self, element: Expr<'a>) -> Self {
        self.elements.insert(0, element);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::List(Arc::new(List::new(self.elements)))
    }
}

#[derive(Debug)]
pub struct ObjectBuilder<'a> {
    fields: BTreeMap<IString, Expr<'a>>,
}

impl<'a> ObjectBuilder<'a> {
    pub(crate) fn new() -> Self {
        ObjectBuilder {
            fields: BTreeMap::new(),
        }
    }

    pub fn add_field(mut self, key: IString, value: Expr<'a>) -> Self {
        self.fields.insert(key, value);
        self
    }

    pub fn add_fields<I>(mut self, fields: I) -> Self
    where
        I: IntoIterator<Item = (IString, Expr<'a>)>,
    {
        self.fields.extend(fields);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::Object(Arc::new(Object::new(self.fields)))
    }
}

#[derive(Debug)]
pub struct UnaryExprBuilder<'a> {
    operator: Option<UnaryExprOp>,
    operand: Option<Expr<'a>>,
    is_postfix: Option<bool>,
}

impl<'a> UnaryExprBuilder<'a> {
    pub(crate) fn new() -> Self {
        UnaryExprBuilder {
            operator: None,
            operand: None,
            is_postfix: None,
        }
    }

    pub fn with_operator(mut self, operator: UnaryExprOp) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_operand(mut self, operand: Expr<'a>) -> Self {
        self.operand = Some(operand);
        self
    }

    pub fn with_prefix(mut self) -> Self {
        self.is_postfix = Some(false);
        self
    }

    pub fn with_postfix(mut self) -> Self {
        self.is_postfix = Some(true);
        self
    }

    pub fn with_postfix_flag(mut self, is_postfix: bool) -> Self {
        self.is_postfix = Some(is_postfix);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::UnaryExpr(Arc::new(UnaryExpr::new(
            self.operand.expect("Operand must be provided"),
            self.operator.expect("Unary operator must be provided"),
            self.is_postfix.expect("Postfix flag must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct BinExprBuilder<'a> {
    left: Option<Expr<'a>>,
    operator: Option<BinExprOp>,
    right: Option<Expr<'a>>,
}

impl<'a> BinExprBuilder<'a> {
    pub(crate) fn new() -> Self {
        BinExprBuilder {
            left: None,
            operator: None,
            right: None,
        }
    }

    pub fn with_left(mut self, left: Expr<'a>) -> Self {
        self.left = Some(left);
        self
    }

    pub fn with_operator(mut self, operator: BinExprOp) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_right(mut self, right: Expr<'a>) -> Self {
        self.right = Some(right);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::BinExpr(Arc::new(BinExpr::new(
            self.left.expect("Left expression must be provided"),
            self.operator.expect("BinExpr operator must be provided"),
            self.right.expect("Right expression must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct StatementBuilder<'a> {
    expression: Option<Expr<'a>>,
}

impl<'a> StatementBuilder<'a> {
    pub(crate) fn new() -> Self {
        StatementBuilder { expression: None }
    }

    pub fn with_expression(mut self, expression: Expr<'a>) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::Statement(Arc::new(Statement::new(
            self.expression.expect("Expression must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct BlockBuilder<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> BlockBuilder<'a> {
    pub(crate) fn new() -> Self {
        BlockBuilder {
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
        let statement = Builder::create_statement()
            .with_expression(expression)
            .build();

        self.elements.push(statement);
        self
    }

    pub fn add_statements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Expr<'a>>,
    {
        for expression in elements {
            let statement = Builder::create_statement()
                .with_expression(expression)
                .build();

            self.elements.push(statement);
        }

        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::Block(Arc::new(Block::new(self.elements)))
    }
}

#[derive(Debug)]
pub struct FunctionBuilder<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Type<'a>>,
    attributes: Vec<Expr<'a>>,
    definition: Option<Expr<'a>>,
}

impl<'a> FunctionBuilder<'a> {
    pub(crate) fn new() -> Self {
        FunctionBuilder {
            parameters: Vec::new(),
            return_type: None,
            attributes: Vec::new(),
            definition: None,
        }
    }

    pub fn with_parameter(
        mut self,
        name: IString,
        ty: Type<'a>,
        default_value: Option<Expr<'a>>,
    ) -> Self {
        self.parameters
            .push(FunctionParameter::new(name, ty, default_value));
        self
    }

    pub fn with_parameters<I>(mut self, parameters: I) -> Self
    where
        I: IntoIterator<Item = FunctionParameter<'a>>,
    {
        self.parameters.extend(parameters);
        self
    }

    pub fn with_return_type(mut self, ty: Type<'a>) -> Self {
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

    pub fn with_definition(mut self, definition: Option<Expr<'a>>) -> Self {
        self.definition = definition;
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::Function(Arc::new(RwLock::new(Function::new(
            self.parameters,
            self.return_type.expect("Return type must be provided"),
            self.attributes,
            self.definition,
        ))))
    }
}

#[derive(Debug)]
pub struct VariableBuilder<'a> {
    kind: Option<VariableKind>,
    is_mutable: bool,
    attributes: Vec<Expr<'a>>,
    name: IString,
    ty: Option<Type<'a>>,
    value: Option<Expr<'a>>,
}

impl<'a> VariableBuilder<'a> {
    pub(crate) fn new() -> Self {
        VariableBuilder {
            kind: None,
            is_mutable: false,
            attributes: Vec::new(),
            name: IString::from(""),
            ty: None,
            value: None,
        }
    }

    pub fn with_kind(mut self, kind: VariableKind) -> Self {
        self.kind = Some(kind);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn add_attribute(mut self, attribute: Expr<'a>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn with_attributes(mut self, attributes: Vec<Expr<'a>>) -> Self {
        self.attributes = attributes;
        self
    }

    pub fn with_name(mut self, name: IString) -> Self {
        self.name = name;
        self
    }

    pub fn with_type(mut self, ty: Type<'a>) -> Self {
        self.ty = Some(ty);
        self
    }

    pub fn with_value(mut self, value: Option<Expr<'a>>) -> Self {
        self.value = value;
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::Variable(Arc::new(Variable::new(
            self.kind.expect("Variable kind must be provided"),
            self.is_mutable,
            self.attributes,
            self.name,
            self.ty.expect("Variable type must be provided"),
            self.value,
        )))
    }
}

#[derive(Debug)]
pub struct IndexAccessBuilder<'a> {
    collection: Option<Expr<'a>>,
    index: Option<Expr<'a>>,
}

impl<'a> IndexAccessBuilder<'a> {
    pub(crate) fn new() -> Self {
        IndexAccessBuilder {
            collection: None,
            index: None,
        }
    }

    pub fn with_collection(mut self, collection: Expr<'a>) -> Self {
        self.collection = Some(collection);
        self
    }

    pub fn with_index(mut self, index: Expr<'a>) -> Self {
        self.index = Some(index);
        self
    }

    pub fn build(self) -> Expr<'a> {
        Expr::IndexAccess(Arc::new(IndexAccess::new(
            self.collection
                .expect("Collection expression must be provided"),
            self.index.expect("Index expression must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct ScopeBuilder<'a> {
    name: Option<IString>,
    attributes: Vec<Expr<'a>>,
    elements: Vec<Expr<'a>>,
}

impl<'a> ScopeBuilder<'a> {
    pub(crate) fn new() -> Self {
        ScopeBuilder {
            name: None,
            attributes: Vec::new(),
            elements: Vec::new(),
        }
    }

    pub fn with_name(mut self, name: IString) -> Self {
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
        Expr::Scope(Arc::new(Scope::new(
            self.name.expect("Name must be provided"),
            self.attributes,
            self.elements,
        )))
    }
}

#[derive(Debug)]
pub struct IfBuilder<'a> {
    condition: Option<Expr<'a>>,
    then_branch: Option<Expr<'a>>,
    else_branch: Option<Expr<'a>>,
}

impl<'a> IfBuilder<'a> {
    pub(crate) fn new() -> Self {
        IfBuilder {
            condition: None,
            then_branch: None,
            else_branch: None,
        }
    }

    pub fn with_condition(mut self, condition: Expr<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_then_branch(mut self, then_branch: Expr<'a>) -> Self {
        self.then_branch = Some(then_branch);
        self
    }

    pub fn with_else_branch(mut self, else_branch: Option<Expr<'a>>) -> Self {
        self.else_branch = else_branch;
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = If::new(
            self.condition.expect("Condition must be provided"),
            self.then_branch.expect("Then branch must be provided"),
            self.else_branch,
        );

        Expr::If(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct WhileLoopBuilder<'a> {
    condition: Option<Expr<'a>>,
    body: Option<Expr<'a>>,
}

impl<'a> WhileLoopBuilder<'a> {
    pub(crate) fn new() -> Self {
        WhileLoopBuilder {
            condition: None,
            body: None,
        }
    }

    pub fn with_condition(mut self, condition: Expr<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_body(mut self, body: Expr<'a>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = WhileLoop::new(
            self.condition.expect("Condition must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Expr::WhileLoop(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct DoWhileLoopBuilder<'a> {
    body: Option<Expr<'a>>,
    condition: Option<Expr<'a>>,
}

impl<'a> DoWhileLoopBuilder<'a> {
    pub(crate) fn new() -> Self {
        DoWhileLoopBuilder {
            body: None,
            condition: None,
        }
    }

    pub fn with_body(mut self, body: Expr<'a>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn with_condition(mut self, condition: Expr<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = DoWhileLoop::new(
            self.condition.expect("Condition must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Expr::DoWhileLoop(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct SwitchBuilder<'a> {
    condition: Option<Expr<'a>>,
    cases: Vec<(Expr<'a>, Expr<'a>)>,
    default: Option<Expr<'a>>,
}

impl<'a> SwitchBuilder<'a> {
    pub(crate) fn new() -> Self {
        SwitchBuilder {
            condition: None,
            cases: Vec::new(),
            default: None,
        }
    }

    pub fn with_condition(mut self, condition: Expr<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn add_case(mut self, case: Expr<'a>, body: Expr<'a>) -> Self {
        self.cases.push((case, body));
        self
    }

    pub fn add_cases<I>(mut self, cases: I) -> Self
    where
        I: IntoIterator<Item = (Expr<'a>, Expr<'a>)>,
    {
        self.cases.extend(cases);
        self
    }

    pub fn with_default(mut self, default: Option<Expr<'a>>) -> Self {
        self.default = default;
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = Switch::new(
            self.condition.expect("Condition must be provided"),
            self.cases,
            self.default,
        );

        Expr::Switch(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct BreakBuilder {
    label: Option<IString>,
}

impl BreakBuilder {
    pub(crate) fn new() -> Self {
        BreakBuilder { label: None }
    }

    pub fn with_label(mut self, label: Option<IString>) -> Self {
        self.label = label;
        self
    }

    pub fn build<'a>(self) -> Expr<'a> {
        let expr = Break::new(self.label);

        Expr::Break(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct ContinueBuilder {
    label: Option<IString>,
}

impl ContinueBuilder {
    pub(crate) fn new() -> Self {
        ContinueBuilder { label: None }
    }

    pub fn with_label(mut self, label: Option<IString>) -> Self {
        self.label = label;
        self
    }

    pub fn build<'a>(self) -> Expr<'a> {
        let expr = Continue::new(self.label);

        Expr::Continue(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct ReturnBuilder<'a> {
    value: Option<Expr<'a>>,
}

impl<'a> ReturnBuilder<'a> {
    pub(crate) fn new() -> Self {
        ReturnBuilder { value: None }
    }

    pub fn with_value(mut self, value: Option<Expr<'a>>) -> Self {
        self.value = value;
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = Return::new(self.value);

        Expr::Return(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct ForEachBuilder<'a> {
    bindings: Vec<(IString, Option<Type<'a>>)>,
    iterable: Option<Expr<'a>>,
    body: Option<Expr<'a>>,
}

impl<'a> ForEachBuilder<'a> {
    pub(crate) fn new() -> Self {
        ForEachBuilder {
            bindings: Vec::new(),
            iterable: None,
            body: None,
        }
    }

    pub fn add_binding(mut self, name: IString, ty: Option<Type<'a>>) -> Self {
        self.bindings.push((name, ty));
        self
    }

    pub fn add_bindings<I>(mut self, bindings: I) -> Self
    where
        I: IntoIterator<Item = (IString, Option<Type<'a>>)>,
    {
        self.bindings.extend(bindings);
        self
    }

    pub fn with_iterable(mut self, iterable: Expr<'a>) -> Self {
        self.iterable = Some(iterable);
        self
    }

    pub fn with_body(mut self, body: Expr<'a>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = ForEach::new(
            self.bindings,
            self.iterable.expect("Iterable expression must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Expr::ForEach(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct AwaitBuilder<'a> {
    expression: Option<Expr<'a>>,
}

impl<'a> AwaitBuilder<'a> {
    pub(crate) fn new() -> Self {
        AwaitBuilder { expression: None }
    }

    pub fn with_expression(mut self, expression: Expr<'a>) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = Await::new(self.expression.expect("Expression must be provided"));

        Expr::Await(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct AssertBuilder<'a> {
    condition: Option<Expr<'a>>,
    message: Option<Expr<'a>>,
}

impl<'a> AssertBuilder<'a> {
    pub(crate) fn new() -> Self {
        AssertBuilder {
            condition: None,
            message: None,
        }
    }

    pub fn with_condition(mut self, condition: Expr<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_message(mut self, message: Expr<'a>) -> Self {
        self.message = Some(message);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = Assert::new(
            self.condition.expect("Condition must be provided"),
            self.message.expect("Message must be provided"),
        );

        Expr::Assert(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct CallBuilder<'a> {
    callee: Option<Expr<'a>>,
    arguments: CallArguments<'a>,
}

impl<'a> CallBuilder<'a> {
    pub(crate) fn new() -> Self {
        CallBuilder {
            callee: None,
            arguments: Vec::new(),
        }
    }

    pub fn with_callee(mut self, callee: Expr<'a>) -> Self {
        self.callee = Some(callee);
        self
    }

    pub fn with_callee_name(mut self, callee: IString) -> Self {
        self.callee = Some(Builder::create_identifier(callee));
        self
    }

    pub fn add_argument(mut self, name: Option<IString>, value: Expr<'a>) -> Self {
        self.arguments.push((name, value));
        self
    }

    pub fn add_arguments<I>(mut self, arguments: I) -> Self
    where
        I: IntoIterator<Item = (Option<IString>, Expr<'a>)>,
    {
        self.arguments.extend(arguments);
        self
    }

    pub fn build(self) -> Expr<'a> {
        let expr = Call::new(
            self.callee.expect("Callee must be provided"),
            self.arguments,
        );

        Expr::Call(Arc::new(expr))
    }
}
