use crate::Builder;
use apint::UInt;
use interned_string::IString;
use nitrate_tokenize::IntegerKind;
use std::sync::Arc;
use std::{collections::BTreeMap, sync::RwLock};

use crate::kind::{
    ArrayType, Await, BinExpr, BinExprOp, Block, Break, Call, CallArguments, Continue, DoWhileLoop,
    Expr, ForEach, Function, FunctionParameter, FunctionType, GenericType, If, IndexAccess,
    Integer, List, ManagedRefType, MapType, Object, RefinementType, Return, Scope, SliceType,
    StructField, StructType, Switch, TupleType, Type, UnaryExpr, UnaryExprOp, UnmanagedRefType,
    Variable, VariableKind, WhileLoop,
};

#[derive(Debug)]
pub struct RefinementTypeBuilder {
    base: Option<Type>,
    width: Option<Expr>,
    minimum: Option<Expr>,
    maximum: Option<Expr>,
}

impl RefinementTypeBuilder {
    pub(crate) fn new() -> Self {
        RefinementTypeBuilder {
            base: None,
            width: None,
            minimum: None,
            maximum: None,
        }
    }

    pub fn with_base(mut self, base: Type) -> Self {
        self.base = Some(base);
        self
    }

    pub fn with_width(mut self, width: Option<Expr>) -> Self {
        self.width = width;
        self
    }

    pub fn with_minimum(mut self, minimum: Option<Expr>) -> Self {
        self.minimum = minimum;
        self
    }

    pub fn with_maximum(mut self, maximum: Option<Expr>) -> Self {
        self.maximum = maximum;
        self
    }

    pub fn build(self) -> Type {
        Type::RefinementType(Arc::new(RefinementType::new(
            self.base.expect("Principal type must be provided"),
            self.width,
            self.minimum,
            self.maximum,
        )))
    }
}

#[derive(Debug)]
pub struct TupleTypeBuilder {
    elements: Vec<Type>,
}

impl TupleTypeBuilder {
    pub(crate) fn new() -> Self {
        TupleTypeBuilder {
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, ty: Type) -> Self {
        self.elements.push(ty);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Type>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> Type {
        if self.elements.is_empty() {
            Type::UnitType
        } else {
            Type::TupleType(Arc::new(TupleType::new(self.elements)))
        }
    }
}

#[derive(Debug)]
pub struct ArrayTypeBuilder {
    element: Option<Type>,
    count: Option<Expr>,
}

impl ArrayTypeBuilder {
    pub(crate) fn new() -> Self {
        ArrayTypeBuilder {
            element: None,
            count: None,
        }
    }

    pub fn with_element(mut self, element: Type) -> Self {
        self.element = Some(element);
        self
    }

    pub fn with_count(mut self, count: Expr) -> Self {
        self.count = Some(count);
        self
    }

    pub fn build(self) -> Type {
        Type::ArrayType(Arc::new(ArrayType::new(
            self.element.expect("Element type must be provided"),
            self.count.expect("Array length must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct MapTypeBuilder {
    key: Option<Type>,
    value: Option<Type>,
}

impl MapTypeBuilder {
    pub(crate) fn new() -> Self {
        MapTypeBuilder {
            key: None,
            value: None,
        }
    }

    pub fn with_key(mut self, key: Type) -> Self {
        self.key = Some(key);
        self
    }

    pub fn with_value(mut self, value: Type) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Type {
        Type::MapType(Arc::new(MapType::new(
            self.key.expect("Key type must be provided"),
            self.value.expect("Value type must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct SliceTypeBuilder {
    element: Option<Type>,
}

impl SliceTypeBuilder {
    pub(crate) fn new() -> Self {
        SliceTypeBuilder { element: None }
    }

    pub fn with_element(mut self, element: Type) -> Self {
        self.element = Some(element);
        self
    }

    pub fn build(self) -> Type {
        Type::SliceType(Arc::new(SliceType::new(
            self.element.expect("Element type must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct FunctionTypeBuilder {
    parameters: Vec<FunctionParameter>,
    return_type: Option<Type>,
    attributes: Vec<Expr>,
}

impl FunctionTypeBuilder {
    pub(crate) fn new() -> Self {
        FunctionTypeBuilder {
            parameters: Vec::new(),
            return_type: None,
            attributes: Vec::new(),
        }
    }

    pub fn add_parameter(mut self, name: IString, ty: Type, default_value: Option<Expr>) -> Self {
        self.parameters
            .push(FunctionParameter::new(name, ty, default_value));
        self
    }

    pub fn add_parameters<I>(mut self, parameters: I) -> Self
    where
        I: IntoIterator<Item = FunctionParameter>,
    {
        self.parameters.extend(parameters);
        self
    }

    pub fn with_return_type(mut self, ty: Type) -> Self {
        self.return_type = Some(ty);
        self
    }

    pub fn add_attribute(mut self, attribute: Expr) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn add_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Expr>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn build(self) -> Type {
        Type::FunctionType(Arc::new(FunctionType::new(
            self.parameters,
            self.return_type.expect("Return type must be provided"),
            self.attributes,
        )))
    }
}

#[derive(Debug)]
pub struct ManagedRefTypeBuilder {
    target: Option<Type>,
    is_mutable: bool,
}

impl ManagedRefTypeBuilder {
    pub(crate) fn new() -> Self {
        ManagedRefTypeBuilder {
            target: None,
            is_mutable: false,
        }
    }

    pub fn with_target(mut self, target: Type) -> Self {
        self.target = Some(target);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn build(self) -> Type {
        Type::ManagedRefType(Arc::new(ManagedRefType::new(
            self.target.expect("Target type must be provided"),
            self.is_mutable,
        )))
    }
}

#[derive(Debug)]
pub struct UnmanagedRefTypeBuilder {
    target: Option<Type>,
    is_mutable: bool,
}

impl UnmanagedRefTypeBuilder {
    pub(crate) fn new() -> Self {
        UnmanagedRefTypeBuilder {
            target: None,
            is_mutable: false,
        }
    }

    pub fn with_target(mut self, target: Type) -> Self {
        self.target = Some(target);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn build(self) -> Type {
        Type::UnmanagedRefType(Arc::new(UnmanagedRefType::new(
            self.target.expect("Target type must be provided"),
            self.is_mutable,
        )))
    }
}

#[derive(Debug)]
pub struct GenericTypeBuilder {
    base: Option<Type>,
    arguments: Vec<(IString, Expr)>,
}

impl GenericTypeBuilder {
    pub(crate) fn new() -> Self {
        GenericTypeBuilder {
            base: None,
            arguments: Vec::new(),
        }
    }

    pub fn with_base(mut self, base: Type) -> Self {
        self.base = Some(base);
        self
    }

    pub fn add_argument(mut self, name: IString, value: Expr) -> Self {
        self.arguments.push((name, value));
        self
    }

    pub fn add_arguments<I>(mut self, arguments: I) -> Self
    where
        I: IntoIterator<Item = (IString, Expr)>,
    {
        self.arguments.extend(arguments);
        self
    }

    pub fn build(self) -> Type {
        Type::GenericType(Arc::new(GenericType::new(
            self.base.expect("Principal type must be provided"),
            self.arguments,
        )))
    }
}

#[derive(Debug)]
pub struct StructTypeBuilder {
    fields: Vec<StructField>,
}

impl StructTypeBuilder {
    pub(crate) fn new() -> Self {
        StructTypeBuilder { fields: Vec::new() }
    }

    pub fn add_field(mut self, name: IString, ty: Type, default_value: Option<Expr>) -> Self {
        self.fields.push((name, ty, default_value));
        self
    }

    pub fn add_fields<I>(mut self, fields: I) -> Self
    where
        I: IntoIterator<Item = StructField>,
    {
        self.fields.extend(fields);
        self
    }

    pub fn build(self) -> Type {
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

    pub fn build(self) -> Expr {
        let lit = Integer::new(
            self.value.expect("Integer value must be provided"),
            self.kind.unwrap_or(IntegerKind::Dec),
        )
        .expect("Invalid integer value");

        Expr::Integer(Arc::new(lit))
    }
}

#[derive(Debug)]
pub struct ListBuilder {
    elements: Vec<Expr>,
}

impl ListBuilder {
    pub(crate) fn new() -> Self {
        ListBuilder {
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, element: Expr) -> Self {
        self.elements.push(element);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Expr>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn prepend_element(mut self, element: Expr) -> Self {
        self.elements.insert(0, element);
        self
    }

    pub fn build(self) -> Expr {
        Expr::List(Arc::new(List::new(self.elements)))
    }
}

#[derive(Debug)]
pub struct ObjectBuilder {
    fields: BTreeMap<IString, Expr>,
}

impl ObjectBuilder {
    pub(crate) fn new() -> Self {
        ObjectBuilder {
            fields: BTreeMap::new(),
        }
    }

    pub fn add_field(mut self, key: IString, value: Expr) -> Self {
        self.fields.insert(key, value);
        self
    }

    pub fn add_fields<I>(mut self, fields: I) -> Self
    where
        I: IntoIterator<Item = (IString, Expr)>,
    {
        self.fields.extend(fields);
        self
    }

    pub fn build(self) -> Expr {
        Expr::Object(Arc::new(Object::new(self.fields)))
    }
}

#[derive(Debug)]
pub struct UnaryExprBuilder {
    operator: Option<UnaryExprOp>,
    operand: Option<Expr>,
    is_postfix: Option<bool>,
}

impl UnaryExprBuilder {
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

    pub fn with_operand(mut self, operand: Expr) -> Self {
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

    pub fn build(self) -> Expr {
        Expr::UnaryExpr(Arc::new(UnaryExpr::new(
            self.operand.expect("Operand must be provided"),
            self.operator.expect("Unary operator must be provided"),
            self.is_postfix.expect("Postfix flag must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct BinExprBuilder {
    left: Option<Expr>,
    operator: Option<BinExprOp>,
    right: Option<Expr>,
}

impl BinExprBuilder {
    pub(crate) fn new() -> Self {
        BinExprBuilder {
            left: None,
            operator: None,
            right: None,
        }
    }

    pub fn with_left(mut self, left: Expr) -> Self {
        self.left = Some(left);
        self
    }

    pub fn with_operator(mut self, operator: BinExprOp) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_right(mut self, right: Expr) -> Self {
        self.right = Some(right);
        self
    }

    pub fn build(self) -> Expr {
        Expr::BinExpr(Arc::new(BinExpr::new(
            self.left.expect("Left expression must be provided"),
            self.operator.expect("BinExpr operator must be provided"),
            self.right.expect("Right expression must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct BlockBuilder {
    elements: Vec<Expr>,
}

impl BlockBuilder {
    pub(crate) fn new() -> Self {
        BlockBuilder {
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, expr: Expr) -> Self {
        self.elements.push(expr);
        self
    }

    pub fn add_expressions<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Expr>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> Expr {
        Expr::Block(Arc::new(Block::new(self.elements)))
    }
}

#[derive(Debug)]
pub struct FunctionBuilder {
    parameters: Vec<FunctionParameter>,
    return_type: Option<Type>,
    attributes: Vec<Expr>,
    definition: Option<Expr>,
}

impl FunctionBuilder {
    pub(crate) fn new() -> Self {
        FunctionBuilder {
            parameters: Vec::new(),
            return_type: None,
            attributes: Vec::new(),
            definition: None,
        }
    }

    pub fn with_parameter(mut self, name: IString, ty: Type, default_value: Option<Expr>) -> Self {
        self.parameters
            .push(FunctionParameter::new(name, ty, default_value));
        self
    }

    pub fn with_parameters<I>(mut self, parameters: I) -> Self
    where
        I: IntoIterator<Item = FunctionParameter>,
    {
        self.parameters.extend(parameters);
        self
    }

    pub fn with_return_type(mut self, ty: Type) -> Self {
        self.return_type = Some(ty);
        self
    }

    pub fn with_attribute(mut self, attribute: Expr) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn with_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Expr>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn with_definition(mut self, definition: Option<Expr>) -> Self {
        self.definition = definition;
        self
    }

    pub fn build(self) -> Expr {
        Expr::Function(Arc::new(RwLock::new(Function::new(
            self.parameters,
            self.return_type.expect("Return type must be provided"),
            self.attributes,
            self.definition,
        ))))
    }
}

#[derive(Debug)]
pub struct VariableBuilder {
    kind: Option<VariableKind>,
    is_mutable: bool,
    attributes: Vec<Expr>,
    name: IString,
    ty: Option<Type>,
    value: Option<Expr>,
}

impl VariableBuilder {
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

    pub fn add_attribute(mut self, attribute: Expr) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn with_attributes(mut self, attributes: Vec<Expr>) -> Self {
        self.attributes = attributes;
        self
    }

    pub fn with_name(mut self, name: IString) -> Self {
        self.name = name;
        self
    }

    pub fn with_type(mut self, ty: Type) -> Self {
        self.ty = Some(ty);
        self
    }

    pub fn with_value(mut self, value: Option<Expr>) -> Self {
        self.value = value;
        self
    }

    pub fn build(self) -> Expr {
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
pub struct IndexAccessBuilder {
    collection: Option<Expr>,
    index: Option<Expr>,
}

impl IndexAccessBuilder {
    pub(crate) fn new() -> Self {
        IndexAccessBuilder {
            collection: None,
            index: None,
        }
    }

    pub fn with_collection(mut self, collection: Expr) -> Self {
        self.collection = Some(collection);
        self
    }

    pub fn with_index(mut self, index: Expr) -> Self {
        self.index = Some(index);
        self
    }

    pub fn build(self) -> Expr {
        Expr::IndexAccess(Arc::new(IndexAccess::new(
            self.collection
                .expect("Collection expression must be provided"),
            self.index.expect("Index expression must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct ScopeBuilder {
    name: Option<IString>,
    attributes: Vec<Expr>,
    elements: Vec<Expr>,
}

impl ScopeBuilder {
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

    pub fn add_attribute(mut self, attribute: Expr) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn add_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Expr>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn add_element(mut self, element: Expr) -> Self {
        self.elements.push(element);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Expr>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> Expr {
        Expr::Scope(Arc::new(Scope::new(
            self.name.expect("Name must be provided"),
            self.attributes,
            self.elements,
        )))
    }
}

#[derive(Debug)]
pub struct IfBuilder {
    condition: Option<Expr>,
    then_branch: Option<Expr>,
    else_branch: Option<Expr>,
}

impl IfBuilder {
    pub(crate) fn new() -> Self {
        IfBuilder {
            condition: None,
            then_branch: None,
            else_branch: None,
        }
    }

    pub fn with_condition(mut self, condition: Expr) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_then_branch(mut self, then_branch: Expr) -> Self {
        self.then_branch = Some(then_branch);
        self
    }

    pub fn with_else_branch(mut self, else_branch: Option<Expr>) -> Self {
        self.else_branch = else_branch;
        self
    }

    pub fn build(self) -> Expr {
        let expr = If::new(
            self.condition.expect("Condition must be provided"),
            self.then_branch.expect("Then branch must be provided"),
            self.else_branch,
        );

        Expr::If(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct WhileLoopBuilder {
    condition: Option<Expr>,
    body: Option<Expr>,
}

impl WhileLoopBuilder {
    pub(crate) fn new() -> Self {
        WhileLoopBuilder {
            condition: None,
            body: None,
        }
    }

    pub fn with_condition(mut self, condition: Expr) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_body(mut self, body: Expr) -> Self {
        self.body = Some(body);
        self
    }

    pub fn build(self) -> Expr {
        let expr = WhileLoop::new(
            self.condition.expect("Condition must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Expr::WhileLoop(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct DoWhileLoopBuilder {
    body: Option<Expr>,
    condition: Option<Expr>,
}

impl DoWhileLoopBuilder {
    pub(crate) fn new() -> Self {
        DoWhileLoopBuilder {
            body: None,
            condition: None,
        }
    }

    pub fn with_body(mut self, body: Expr) -> Self {
        self.body = Some(body);
        self
    }

    pub fn with_condition(mut self, condition: Expr) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn build(self) -> Expr {
        let expr = DoWhileLoop::new(
            self.condition.expect("Condition must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Expr::DoWhileLoop(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct SwitchBuilder {
    condition: Option<Expr>,
    cases: Vec<(Expr, Expr)>,
    default: Option<Expr>,
}

impl SwitchBuilder {
    pub(crate) fn new() -> Self {
        SwitchBuilder {
            condition: None,
            cases: Vec::new(),
            default: None,
        }
    }

    pub fn with_condition(mut self, condition: Expr) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn add_case(mut self, case: Expr, body: Expr) -> Self {
        self.cases.push((case, body));
        self
    }

    pub fn add_cases<I>(mut self, cases: I) -> Self
    where
        I: IntoIterator<Item = (Expr, Expr)>,
    {
        self.cases.extend(cases);
        self
    }

    pub fn with_default(mut self, default: Option<Expr>) -> Self {
        self.default = default;
        self
    }

    pub fn build(self) -> Expr {
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

    pub fn build(self) -> Expr {
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

    pub fn build(self) -> Expr {
        let expr = Continue::new(self.label);

        Expr::Continue(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct ReturnBuilder {
    value: Option<Expr>,
}

impl ReturnBuilder {
    pub(crate) fn new() -> Self {
        ReturnBuilder { value: None }
    }

    pub fn with_value(mut self, value: Option<Expr>) -> Self {
        self.value = value;
        self
    }

    pub fn build(self) -> Expr {
        let expr = Return::new(self.value);

        Expr::Return(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct ForEachBuilder {
    bindings: Vec<(IString, Option<Type>)>,
    iterable: Option<Expr>,
    body: Option<Expr>,
}

impl ForEachBuilder {
    pub(crate) fn new() -> Self {
        ForEachBuilder {
            bindings: Vec::new(),
            iterable: None,
            body: None,
        }
    }

    pub fn add_binding(mut self, name: IString, ty: Option<Type>) -> Self {
        self.bindings.push((name, ty));
        self
    }

    pub fn add_bindings<I>(mut self, bindings: I) -> Self
    where
        I: IntoIterator<Item = (IString, Option<Type>)>,
    {
        self.bindings.extend(bindings);
        self
    }

    pub fn with_iterable(mut self, iterable: Expr) -> Self {
        self.iterable = Some(iterable);
        self
    }

    pub fn with_body(mut self, body: Expr) -> Self {
        self.body = Some(body);
        self
    }

    pub fn build(self) -> Expr {
        let expr = ForEach::new(
            self.bindings,
            self.iterable.expect("Iterable expression must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Expr::ForEach(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct AwaitBuilder {
    expression: Option<Expr>,
}

impl AwaitBuilder {
    pub(crate) fn new() -> Self {
        AwaitBuilder { expression: None }
    }

    pub fn with_expression(mut self, expression: Expr) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> Expr {
        let expr = Await::new(self.expression.expect("Expression must be provided"));

        Expr::Await(Arc::new(expr))
    }
}

#[derive(Debug)]
pub struct CallBuilder {
    callee: Option<Expr>,
    arguments: CallArguments,
}

impl CallBuilder {
    pub(crate) fn new() -> Self {
        CallBuilder {
            callee: None,
            arguments: Vec::new(),
        }
    }

    pub fn with_callee(mut self, callee: Expr) -> Self {
        self.callee = Some(callee);
        self
    }

    pub fn with_callee_name(mut self, callee: IString) -> Self {
        self.callee = Some(Builder::create_identifier(callee));
        self
    }

    pub fn add_argument(mut self, name: Option<IString>, value: Expr) -> Self {
        self.arguments.push((name, value));
        self
    }

    pub fn add_arguments<I>(mut self, arguments: I) -> Self
    where
        I: IntoIterator<Item = (Option<IString>, Expr)>,
    {
        self.arguments.extend(arguments);
        self
    }

    pub fn build(self) -> Expr {
        let expr = Call::new(
            self.callee.expect("Callee must be provided"),
            self.arguments,
        );

        Expr::Call(Arc::new(expr))
    }
}
