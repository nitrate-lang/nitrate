use crate::lexical::{BStringData, IntegerKind, StringData};
use crate::parsetree::{node::*, *};
use crate::syntax::QualifiedScope;
use apint::UInt;
use std::collections::BTreeMap;
use std::rc::Rc;
use std::sync::Arc;

#[derive(Debug)]
pub struct RefinementTypeBuilder<'a> {
    base: Option<Rc<TypeOwned<'a>>>,
    width: Option<Arc<ExprOwned<'a>>>,
    minimum: Option<Arc<ExprOwned<'a>>>,
    maximum: Option<Arc<ExprOwned<'a>>>,
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

    pub fn with_base(mut self, base: Rc<TypeOwned<'a>>) -> Self {
        self.base = Some(base);
        self
    }

    pub fn with_width(mut self, width: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.width = width;
        self
    }

    pub fn with_minimum(mut self, minimum: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.minimum = minimum;
        self
    }

    pub fn with_maximum(mut self, maximum: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.maximum = maximum;
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::RefinementType(RefinementType::new(
            self.base.expect("Principal type must be provided"),
            self.width,
            self.minimum,
            self.maximum,
        )))
    }
}

#[derive(Debug)]
pub struct TupleTypeBuilder<'a> {
    elements: Vec<Rc<TypeOwned<'a>>>,
}

impl<'a> TupleTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        TupleTypeBuilder {
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, ty: Rc<TypeOwned<'a>>) -> Self {
        self.elements.push(ty);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Rc<TypeOwned<'a>>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::TupleType(TupleType::new(self.elements)))
    }
}

#[derive(Debug)]
pub struct ArrayTypeBuilder<'a> {
    element: Option<Rc<TypeOwned<'a>>>,
    count: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> ArrayTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        ArrayTypeBuilder {
            element: None,
            count: None,
        }
    }

    pub fn with_element(mut self, element: Rc<TypeOwned<'a>>) -> Self {
        self.element = Some(element);
        self
    }

    pub fn with_count(mut self, count: Arc<ExprOwned<'a>>) -> Self {
        self.count = Some(count);
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::ArrayType(ArrayType::new(
            self.element.expect("Element type must be provided"),
            self.count.expect("Array length must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct MapTypeBuilder<'a> {
    key: Option<Rc<TypeOwned<'a>>>,
    value: Option<Rc<TypeOwned<'a>>>,
}

impl<'a> MapTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        MapTypeBuilder {
            key: None,
            value: None,
        }
    }

    pub fn with_key(mut self, key: Rc<TypeOwned<'a>>) -> Self {
        self.key = Some(key);
        self
    }

    pub fn with_value(mut self, value: Rc<TypeOwned<'a>>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::MapType(MapType::new(
            self.key.expect("Key type must be provided"),
            self.value.expect("Value type must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct SliceTypeBuilder<'a> {
    element: Option<Rc<TypeOwned<'a>>>,
}

impl<'a> SliceTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        SliceTypeBuilder { element: None }
    }

    pub fn with_element(mut self, element: Rc<TypeOwned<'a>>) -> Self {
        self.element = Some(element);
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::SliceType(SliceType::new(
            self.element.expect("Element type must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct FunctionTypeBuilder<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Rc<TypeOwned<'a>>>,
    attributes: Vec<Arc<ExprOwned<'a>>>,
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
        name: &'a str,
        ty: Rc<TypeOwned<'a>>,
        default_value: Option<Arc<ExprOwned<'a>>>,
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

    pub fn with_return_type(mut self, ty: Rc<TypeOwned<'a>>) -> Self {
        self.return_type = Some(ty);
        self
    }

    pub fn add_attribute(mut self, attribute: Arc<ExprOwned<'a>>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn add_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Arc<ExprOwned<'a>>>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::FunctionType(FunctionType::new(
            self.parameters,
            self.return_type.expect("Return type must be provided"),
            self.attributes,
        )))
    }
}

#[derive(Debug)]
pub struct ManagedRefTypeBuilder<'a> {
    target: Option<Rc<TypeOwned<'a>>>,
    is_mutable: bool,
}

impl<'a> ManagedRefTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        ManagedRefTypeBuilder {
            target: None,
            is_mutable: false,
        }
    }

    pub fn with_target(mut self, target: Rc<TypeOwned<'a>>) -> Self {
        self.target = Some(target);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::ManagedRefType(ManagedRefType::new(
            self.target.expect("Target type must be provided"),
            self.is_mutable,
        )))
    }
}

#[derive(Debug)]
pub struct UnmanagedRefTypeBuilder<'a> {
    target: Option<Rc<TypeOwned<'a>>>,
    is_mutable: bool,
}

impl<'a> UnmanagedRefTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        UnmanagedRefTypeBuilder {
            target: None,
            is_mutable: false,
        }
    }

    pub fn with_target(mut self, target: Rc<TypeOwned<'a>>) -> Self {
        self.target = Some(target);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::UnmanagedRefType(UnmanagedRefType::new(
            self.target.expect("Target type must be provided"),
            self.is_mutable,
        )))
    }
}

#[derive(Debug)]
pub struct GenericTypeBuilder<'a> {
    base: Option<Rc<TypeOwned<'a>>>,
    arguments: Vec<(&'a str, Arc<ExprOwned<'a>>)>,
}

impl<'a> GenericTypeBuilder<'a> {
    pub(crate) fn new() -> Self {
        GenericTypeBuilder {
            base: None,
            arguments: Vec::new(),
        }
    }

    pub fn with_base(mut self, base: Rc<TypeOwned<'a>>) -> Self {
        self.base = Some(base);
        self
    }

    pub fn add_argument(mut self, name: &'a str, value: Arc<ExprOwned<'a>>) -> Self {
        self.arguments.push((name, value));
        self
    }

    pub fn add_arguments<I>(mut self, arguments: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Arc<ExprOwned<'a>>)>,
    {
        self.arguments.extend(arguments);
        self
    }

    pub fn build(self) -> Rc<TypeOwned<'a>> {
        Rc::new(TypeOwned::GenericType(GenericType::new(
            self.base.expect("Principal type must be provided"),
            self.arguments,
        )))
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

    pub fn build<'a>(self) -> Arc<ExprOwned<'a>> {
        let lit = IntegerLit::new(
            self.value.expect("Integer value must be provided"),
            self.kind.unwrap_or(IntegerKind::Dec),
        )
        .expect("Invalid integer value");

        Arc::new(ExprOwned::IntegerLit(lit))
    }
}

#[derive(Debug)]
pub struct FloatBuilder {
    value: Option<f64>,
}

impl FloatBuilder {
    pub(crate) fn new() -> Self {
        FloatBuilder { value: None }
    }

    pub fn with_value(mut self, value: f64) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build<'a>(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::FloatLit(
            self.value.expect("Float value must be provided"),
        ))
    }
}

#[derive(Debug)]
pub struct StringBuilder<'a> {
    value: Option<StringData<'a>>,
}

impl<'a> StringBuilder<'a> {
    pub(crate) fn new() -> Self {
        StringBuilder { value: None }
    }

    pub fn with_string(mut self, value: StringData<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::StringLit(
            self.value.expect("String value must be provided"),
        ))
    }
}

#[derive(Debug)]
pub struct BStringBuilder<'a> {
    value: Option<BStringData<'a>>,
}

impl<'a> BStringBuilder<'a> {
    pub(crate) fn new() -> Self {
        BStringBuilder { value: None }
    }

    pub fn with_value(mut self, value: BStringData<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::BStringLit(
            self.value.expect("BString value must be provided"),
        ))
    }
}

#[derive(Debug)]
pub struct ListBuilder<'a> {
    elements: Vec<Arc<ExprOwned<'a>>>,
}

impl<'a> ListBuilder<'a> {
    pub(crate) fn new() -> Self {
        ListBuilder {
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, element: Arc<ExprOwned<'a>>) -> Self {
        self.elements.push(element);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Arc<ExprOwned<'a>>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn prepend_element(mut self, element: Arc<ExprOwned<'a>>) -> Self {
        self.elements.insert(0, element);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::ListLit(ListLit::new(self.elements)))
    }
}

#[derive(Debug)]
pub struct ObjectBuilder<'a> {
    fields: BTreeMap<&'a str, Arc<ExprOwned<'a>>>,
}

impl<'a> ObjectBuilder<'a> {
    pub(crate) fn new() -> Self {
        ObjectBuilder {
            fields: BTreeMap::new(),
        }
    }

    pub fn add_field(mut self, key: &'a str, value: Arc<ExprOwned<'a>>) -> Self {
        self.fields.insert(key, value);
        self
    }

    pub fn add_fields<I>(mut self, fields: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Arc<ExprOwned<'a>>)>,
    {
        self.fields.extend(fields);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::ObjectLit(ObjectLit::new(self.fields)))
    }
}

#[derive(Debug)]
pub struct UnaryExprBuilder<'a> {
    operator: Option<UnaryExprOp>,
    operand: Option<Arc<ExprOwned<'a>>>,
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

    pub fn with_operand(mut self, operand: Arc<ExprOwned<'a>>) -> Self {
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

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::UnaryExpr(UnaryExpr::new(
            self.operand.expect("Operand must be provided"),
            self.operator.expect("Unary operator must be provided"),
            self.is_postfix.expect("Postfix flag must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct BinExprBuilder<'a> {
    left: Option<Arc<ExprOwned<'a>>>,
    operator: Option<BinExprOp>,
    right: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> BinExprBuilder<'a> {
    pub(crate) fn new() -> Self {
        BinExprBuilder {
            left: None,
            operator: None,
            right: None,
        }
    }

    pub fn with_left(mut self, left: Arc<ExprOwned<'a>>) -> Self {
        self.left = Some(left);
        self
    }

    pub fn with_operator(mut self, operator: BinExprOp) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_right(mut self, right: Arc<ExprOwned<'a>>) -> Self {
        self.right = Some(right);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::BinExpr(BinExpr::new(
            self.left.expect("Left expression must be provided"),
            self.operator.expect("BinExpr operator must be provided"),
            self.right.expect("Right expression must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct StatementBuilder<'a> {
    expression: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> StatementBuilder<'a> {
    pub(crate) fn new() -> Self {
        StatementBuilder { expression: None }
    }

    pub fn with_expression(mut self, expression: Arc<ExprOwned<'a>>) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::Statement(Statement::new(
            self.expression.expect("Expression must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct BlockBuilder<'a> {
    elements: Vec<Arc<ExprOwned<'a>>>,
}

impl<'a> BlockBuilder<'a> {
    pub(crate) fn new() -> Self {
        BlockBuilder {
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, expr: Arc<ExprOwned<'a>>) -> Self {
        self.elements.push(expr);
        self
    }

    pub fn add_expressions<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Arc<ExprOwned<'a>>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn add_statement(mut self, expression: Arc<ExprOwned<'a>>) -> Self {
        let statement = Builder::new()
            .create_statement()
            .with_expression(expression)
            .build();

        self.elements.push(statement);
        self
    }

    pub fn add_statements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = Arc<ExprOwned<'a>>>,
    {
        for expression in elements {
            let statement = Builder::new()
                .create_statement()
                .with_expression(expression)
                .build();

            self.elements.push(statement);
        }

        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::Block(Block::new(self.elements)))
    }
}

#[derive(Debug)]
pub struct FunctionBuilder<'a> {
    name: &'a str,
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<Rc<TypeOwned<'a>>>,
    attributes: Vec<Arc<ExprOwned<'a>>>,
    definition: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> FunctionBuilder<'a> {
    pub(crate) fn new() -> Self {
        FunctionBuilder {
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
        ty: Rc<TypeOwned<'a>>,
        default_value: Option<Arc<ExprOwned<'a>>>,
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

    pub fn with_return_type(mut self, ty: Rc<TypeOwned<'a>>) -> Self {
        self.return_type = Some(ty);
        self
    }

    pub fn with_attribute(mut self, attribute: Arc<ExprOwned<'a>>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn with_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Arc<ExprOwned<'a>>>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn with_definition(mut self, definition: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.definition = definition;
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::Function(Function::new(
            self.name,
            self.parameters,
            self.return_type.expect("Return type must be provided"),
            self.attributes,
            self.definition,
        )))
    }
}

#[derive(Debug)]
pub struct VariableBuilder<'a> {
    kind: Option<VariableKind>,
    is_mutable: bool,
    attributes: Vec<Arc<ExprOwned<'a>>>,
    name: &'a str,
    ty: Option<Rc<TypeOwned<'a>>>,
    value: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> VariableBuilder<'a> {
    pub(crate) fn new() -> Self {
        VariableBuilder {
            kind: None,
            is_mutable: false,
            attributes: Vec::new(),
            name: "",
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

    pub fn add_attribute(mut self, attribute: Arc<ExprOwned<'a>>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn with_attributes(mut self, attributes: Vec<Arc<ExprOwned<'a>>>) -> Self {
        self.attributes = attributes;
        self
    }

    pub fn with_name(mut self, name: &'a str) -> Self {
        self.name = name;
        self
    }

    pub fn with_type(mut self, ty: Rc<TypeOwned<'a>>) -> Self {
        self.ty = Some(ty);
        self
    }

    pub fn with_value(mut self, value: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.value = value;
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::Variable(Variable::new(
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
pub struct ScopeBuilder<'a> {
    scope: Option<QualifiedScope<'a>>,
    attributes: Vec<Arc<ExprOwned<'a>>>,
    block: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> ScopeBuilder<'a> {
    pub(crate) fn new() -> Self {
        ScopeBuilder {
            scope: None,
            attributes: Vec::new(),
            block: None,
        }
    }

    pub fn with_scope(mut self, scope: QualifiedScope<'a>) -> Self {
        self.scope = Some(scope);
        self
    }

    pub fn add_attribute(mut self, attribute: Arc<ExprOwned<'a>>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn add_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = Arc<ExprOwned<'a>>>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn with_block(mut self, block: Arc<ExprOwned<'a>>) -> Self {
        self.block = Some(block);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        Arc::new(ExprOwned::Scope(Scope::new(
            self.scope.expect("Scope must be provided"),
            self.attributes,
            self.block.expect("Block must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct IfBuilder<'a> {
    condition: Option<Arc<ExprOwned<'a>>>,
    then_branch: Option<Arc<ExprOwned<'a>>>,
    else_branch: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> IfBuilder<'a> {
    pub(crate) fn new() -> Self {
        IfBuilder {
            condition: None,
            then_branch: None,
            else_branch: None,
        }
    }

    pub fn with_condition(mut self, condition: Arc<ExprOwned<'a>>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_then_branch(mut self, then_branch: Arc<ExprOwned<'a>>) -> Self {
        self.then_branch = Some(then_branch);
        self
    }

    pub fn with_else_branch(mut self, else_branch: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.else_branch = else_branch;
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = If::new(
            self.condition.expect("Condition must be provided"),
            self.then_branch.expect("Then branch must be provided"),
            self.else_branch,
        );

        Arc::new(ExprOwned::If(expr))
    }
}

#[derive(Debug)]
pub struct WhileLoopBuilder<'a> {
    condition: Option<Arc<ExprOwned<'a>>>,
    body: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> WhileLoopBuilder<'a> {
    pub(crate) fn new() -> Self {
        WhileLoopBuilder {
            condition: None,
            body: None,
        }
    }

    pub fn with_condition(mut self, condition: Arc<ExprOwned<'a>>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_body(mut self, body: Arc<ExprOwned<'a>>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = WhileLoop::new(
            self.condition.expect("Condition must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Arc::new(ExprOwned::WhileLoop(expr))
    }
}

#[derive(Debug)]
pub struct DoWhileLoopBuilder<'a> {
    body: Option<Arc<ExprOwned<'a>>>,
    condition: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> DoWhileLoopBuilder<'a> {
    pub(crate) fn new() -> Self {
        DoWhileLoopBuilder {
            body: None,
            condition: None,
        }
    }

    pub fn with_body(mut self, body: Arc<ExprOwned<'a>>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn with_condition(mut self, condition: Arc<ExprOwned<'a>>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = DoWhileLoop::new(
            self.condition.expect("Condition must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Arc::new(ExprOwned::DoWhileLoop(expr))
    }
}

#[derive(Debug)]
pub struct SwitchBuilder<'a> {
    condition: Option<Arc<ExprOwned<'a>>>,
    cases: Vec<(Arc<ExprOwned<'a>>, Arc<ExprOwned<'a>>)>,
    default: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> SwitchBuilder<'a> {
    pub(crate) fn new() -> Self {
        SwitchBuilder {
            condition: None,
            cases: Vec::new(),
            default: None,
        }
    }

    pub fn with_condition(mut self, condition: Arc<ExprOwned<'a>>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn add_case(mut self, case: Arc<ExprOwned<'a>>, body: Arc<ExprOwned<'a>>) -> Self {
        self.cases.push((case, body));
        self
    }

    pub fn add_cases<I>(mut self, cases: I) -> Self
    where
        I: IntoIterator<Item = (Arc<ExprOwned<'a>>, Arc<ExprOwned<'a>>)>,
    {
        self.cases.extend(cases);
        self
    }

    pub fn with_default(mut self, default: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.default = default;
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = super::control_flow::Switch::new(
            self.condition.expect("Condition must be provided"),
            self.cases,
            self.default,
        );

        Arc::new(ExprOwned::Switch(expr))
    }
}

#[derive(Debug)]
pub struct BreakBuilder<'a> {
    label: Option<&'a str>,
}

impl<'a> BreakBuilder<'a> {
    pub(crate) fn new() -> Self {
        BreakBuilder { label: None }
    }

    pub fn with_label(mut self, label: Option<&'a str>) -> Self {
        self.label = label;
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = Break::new(self.label);

        Arc::new(ExprOwned::Break(expr))
    }
}

#[derive(Debug)]
pub struct ContinueBuilder<'a> {
    label: Option<&'a str>,
}

impl<'a> ContinueBuilder<'a> {
    pub(crate) fn new() -> Self {
        ContinueBuilder { label: None }
    }

    pub fn with_label(mut self, label: Option<&'a str>) -> Self {
        self.label = label;
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = Continue::new(self.label);

        Arc::new(ExprOwned::Continue(expr))
    }
}

#[derive(Debug)]
pub struct ReturnBuilder<'a> {
    value: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> ReturnBuilder<'a> {
    pub(crate) fn new() -> Self {
        ReturnBuilder { value: None }
    }

    pub fn with_value(mut self, value: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.value = value;
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = Return::new(self.value);

        Arc::new(ExprOwned::Return(expr))
    }
}

#[derive(Debug)]
pub struct ForEachBuilder<'a> {
    bindings: Vec<(&'a str, Option<Rc<TypeOwned<'a>>>)>,
    iterable: Option<Arc<ExprOwned<'a>>>,
    body: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> ForEachBuilder<'a> {
    pub(crate) fn new() -> Self {
        ForEachBuilder {
            bindings: Vec::new(),
            iterable: None,
            body: None,
        }
    }

    pub fn add_binding(mut self, name: &'a str, ty: Option<Rc<TypeOwned<'a>>>) -> Self {
        self.bindings.push((name, ty));
        self
    }

    pub fn add_bindings<I>(mut self, bindings: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Option<Rc<TypeOwned<'a>>>)>,
    {
        self.bindings.extend(bindings);
        self
    }

    pub fn with_iterable(mut self, iterable: Arc<ExprOwned<'a>>) -> Self {
        self.iterable = Some(iterable);
        self
    }

    pub fn with_body(mut self, body: Arc<ExprOwned<'a>>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = ForEach::new(
            self.bindings,
            self.iterable.expect("Iterable expression must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        Arc::new(ExprOwned::ForEach(expr))
    }
}

#[derive(Debug)]
pub struct AwaitBuilder<'a> {
    expression: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> AwaitBuilder<'a> {
    pub(crate) fn new() -> Self {
        AwaitBuilder { expression: None }
    }

    pub fn with_expression(mut self, expression: Arc<ExprOwned<'a>>) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = Await::new(self.expression.expect("Expression must be provided"));

        Arc::new(ExprOwned::Await(expr))
    }
}

#[derive(Debug)]
pub struct AssertBuilder<'a> {
    condition: Option<Arc<ExprOwned<'a>>>,
    message: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> AssertBuilder<'a> {
    pub(crate) fn new() -> Self {
        AssertBuilder {
            condition: None,
            message: None,
        }
    }

    pub fn with_condition(mut self, condition: Arc<ExprOwned<'a>>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_message(mut self, message: Option<Arc<ExprOwned<'a>>>) -> Self {
        self.message = message;
        self
    }

    pub fn build(self) -> Arc<ExprOwned<'a>> {
        let expr = Assert::new(
            self.condition.expect("Condition must be provided"),
            self.message,
        );

        Arc::new(ExprOwned::Assert(expr))
    }
}
