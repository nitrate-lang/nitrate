use crate::lexer::{BStringData, IntegerKind, StringData};
use crate::parsetree::{node::*, *};
use apint::UInt;
use std::collections::BTreeMap;

#[derive(Debug)]
pub struct RefinementTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    base: Option<TypeKey<'a>>,
    width: Option<ExprKey<'a>>,
    minimum: Option<ExprKey<'a>>,
    maximum: Option<ExprKey<'a>>,
}

impl<'storage, 'a> RefinementTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        RefinementTypeBuilder {
            storage,
            base: None,
            width: None,
            minimum: None,
            maximum: None,
        }
    }

    pub fn with_base(mut self, base: TypeKey<'a>) -> Self {
        self.base = Some(base);
        self
    }

    pub fn with_width(mut self, width: Option<ExprKey<'a>>) -> Self {
        self.width = width;
        self
    }

    pub fn with_minimum(mut self, minimum: Option<ExprKey<'a>>) -> Self {
        self.minimum = minimum;
        self
    }

    pub fn with_maximum(mut self, maximum: Option<ExprKey<'a>>) -> Self {
        self.maximum = maximum;
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::RefinementType(RefinementType::new(
                self.base.expect("Principal type must be provided"),
                self.width,
                self.minimum,
                self.maximum,
            )))
            .expect("Failed to create refinement type")
    }
}

#[derive(Debug)]
pub struct TupleTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    elements: Vec<TypeKey<'a>>,
}

impl<'storage, 'a> TupleTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        TupleTypeBuilder {
            storage,
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, ty: TypeKey<'a>) -> Self {
        self.elements.push(ty);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = TypeKey<'a>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::TupleType(TupleType::new(self.elements)))
            .expect("Failed to create tuple type")
    }
}

#[derive(Debug)]
pub struct ArrayTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    element: Option<TypeKey<'a>>,
    count: Option<ExprKey<'a>>,
}

impl<'storage, 'a> ArrayTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ArrayTypeBuilder {
            storage,
            element: None,
            count: None,
        }
    }

    pub fn with_element(mut self, element: TypeKey<'a>) -> Self {
        self.element = Some(element);
        self
    }

    pub fn with_count(mut self, count: ExprKey<'a>) -> Self {
        self.count = Some(count);
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::ArrayType(ArrayType::new(
                self.element.expect("Element type must be provided"),
                self.count.expect("Array length must be provided"),
            )))
            .expect("Failed to create array type")
    }
}

#[derive(Debug)]
pub struct MapTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    key: Option<TypeKey<'a>>,
    value: Option<TypeKey<'a>>,
}

impl<'storage, 'a> MapTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        MapTypeBuilder {
            storage,
            key: None,
            value: None,
        }
    }

    pub fn with_key(mut self, key: TypeKey<'a>) -> Self {
        self.key = Some(key);
        self
    }

    pub fn with_value(mut self, value: TypeKey<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::MapType(MapType::new(
                self.key.expect("Key type must be provided"),
                self.value.expect("Value type must be provided"),
            )))
            .expect("Failed to create map type")
    }
}

#[derive(Debug)]
pub struct SliceTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    element: Option<TypeKey<'a>>,
}

impl<'storage, 'a> SliceTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        SliceTypeBuilder {
            storage,
            element: None,
        }
    }

    pub fn with_element(mut self, element: TypeKey<'a>) -> Self {
        self.element = Some(element);
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::SliceType(SliceType::new(
                self.element.expect("Element type must be provided"),
            )))
            .expect("Failed to create slice type")
    }
}

#[derive(Debug)]
pub struct FunctionTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<TypeKey<'a>>,
    attributes: Vec<ExprKey<'a>>,
}

impl<'storage, 'a> FunctionTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        FunctionTypeBuilder {
            storage,
            parameters: Vec::new(),
            return_type: None,
            attributes: Vec::new(),
        }
    }

    pub fn add_parameter(
        mut self,
        name: &'a str,
        ty: TypeKey<'a>,
        default_value: Option<ExprKey<'a>>,
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

    pub fn with_return_type(mut self, ty: TypeKey<'a>) -> Self {
        self.return_type = Some(ty);
        self
    }

    pub fn add_attribute(mut self, attribute: ExprKey<'a>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn add_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = ExprKey<'a>>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::FunctionType(FunctionType::new(
                self.parameters,
                self.return_type.expect("Return type must be provided"),
                self.attributes,
            )))
            .expect("Failed to create function type")
    }
}

#[derive(Debug)]
pub struct ManagedRefTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    target: Option<TypeKey<'a>>,
    is_mutable: bool,
}

impl<'storage, 'a> ManagedRefTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ManagedRefTypeBuilder {
            storage,
            target: None,
            is_mutable: false,
        }
    }

    pub fn with_target(mut self, target: TypeKey<'a>) -> Self {
        self.target = Some(target);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::ManagedRefType(ManagedRefType::new(
                self.target.expect("Target type must be provided"),
                self.is_mutable,
            )))
            .expect("Failed to create managed reference type")
    }
}

#[derive(Debug)]
pub struct UnmanagedRefTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    target: Option<TypeKey<'a>>,
    is_mutable: bool,
}

impl<'storage, 'a> UnmanagedRefTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        UnmanagedRefTypeBuilder {
            storage,
            target: None,
            is_mutable: false,
        }
    }

    pub fn with_target(mut self, target: TypeKey<'a>) -> Self {
        self.target = Some(target);
        self
    }

    pub fn with_mutability(mut self, is_mutable: bool) -> Self {
        self.is_mutable = is_mutable;
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::UnmanagedRefType(UnmanagedRefType::new(
                self.target.expect("Target type must be provided"),
                self.is_mutable,
            )))
            .expect("Failed to create unmanaged reference type")
    }
}

#[derive(Debug)]
pub struct GenericTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    base: Option<TypeKey<'a>>,
    arguments: Vec<(&'a str, ExprKey<'a>)>,
}

impl<'storage, 'a> GenericTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        GenericTypeBuilder {
            storage,
            base: None,
            arguments: Vec::new(),
        }
    }

    pub fn with_base(mut self, base: TypeKey<'a>) -> Self {
        self.base = Some(base);
        self
    }

    pub fn add_argument(mut self, name: &'a str, value: ExprKey<'a>) -> Self {
        self.arguments.push((name, value));
        self
    }

    pub fn add_arguments<I>(mut self, arguments: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, ExprKey<'a>)>,
    {
        self.arguments.extend(arguments);
        self
    }

    pub fn build(self) -> TypeKey<'a> {
        self.storage
            .add_type(TypeOwned::GenericType(GenericType::new(
                self.base.expect("Principal type must be provided"),
                self.arguments,
            )))
            .expect("Failed to create generic type")
    }
}

#[derive(Debug)]
pub struct IntegerBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<UInt>,
    kind: Option<IntegerKind>,
}

impl<'storage, 'a> IntegerBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        IntegerBuilder {
            storage,
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

    pub fn build(self) -> ExprKey<'a> {
        let lit = IntegerLit::new(
            self.value.expect("Integer value must be provided"),
            self.kind.unwrap_or(IntegerKind::Dec),
        )
        .expect("Invalid integer value");

        self.storage
            .add_expr(ExprOwned::IntegerLit(lit))
            .expect("Failed to create integer literal")
    }
}

#[derive(Debug)]
pub struct FloatBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<f64>,
}

impl<'storage, 'a> FloatBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        FloatBuilder {
            storage,
            value: None,
        }
    }

    pub fn with_value(mut self, value: f64) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::FloatLit(
                self.value.expect("Float value must be provided"),
            ))
            .expect("Failed to create float literal")
    }
}

#[derive(Debug)]
pub struct StringBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<StringData<'a>>,
}

impl<'storage, 'a> StringBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        StringBuilder {
            storage,
            value: None,
        }
    }

    pub fn with_string(mut self, value: StringData<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::StringLit(
                self.value.expect("String value must be provided"),
            ))
            .expect("Failed to create string literal")
    }
}

#[derive(Debug)]
pub struct BStringBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<BStringData<'a>>,
}

impl<'storage, 'a> BStringBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        BStringBuilder {
            storage,
            value: None,
        }
    }

    pub fn with_value(mut self, value: BStringData<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::BStringLit(
                self.value.expect("BString value must be provided"),
            ))
            .expect("Failed to create BString literal")
    }
}

#[derive(Debug)]
pub struct ListBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    elements: Vec<ExprKey<'a>>,
}

impl<'storage, 'a> ListBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ListBuilder {
            storage,
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, element: ExprKey<'a>) -> Self {
        self.elements.push(element);
        self
    }

    pub fn add_elements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = ExprKey<'a>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn prepend_element(mut self, element: ExprKey<'a>) -> Self {
        self.elements.insert(0, element);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::ListLit(ListLit::new(self.elements)))
            .expect("Failed to create list literal")
    }
}

#[derive(Debug)]
pub struct ObjectBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    fields: BTreeMap<&'a str, ExprKey<'a>>,
}

impl<'storage, 'a> ObjectBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ObjectBuilder {
            storage,
            fields: BTreeMap::new(),
        }
    }

    pub fn add_field(mut self, key: &'a str, value: ExprKey<'a>) -> Self {
        self.fields.insert(key, value);
        self
    }

    pub fn add_fields<I>(mut self, fields: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, ExprKey<'a>)>,
    {
        self.fields.extend(fields);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::ObjectLit(ObjectLit::new(self.fields)))
            .expect("Failed to create object literal")
    }
}

#[derive(Debug)]
pub struct UnaryExprBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    operator: Option<UnaryExprOp>,
    operand: Option<ExprKey<'a>>,
    is_postfix: Option<bool>,
}

impl<'storage, 'a> UnaryExprBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        UnaryExprBuilder {
            storage,
            operator: None,
            operand: None,
            is_postfix: None,
        }
    }

    pub fn with_operator(mut self, operator: UnaryExprOp) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_operand(mut self, operand: ExprKey<'a>) -> Self {
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

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::UnaryExpr(UnaryExpr::new(
                self.operand.expect("Operand must be provided"),
                self.operator.expect("Unary operator must be provided"),
                self.is_postfix.expect("Postfix flag must be provided"),
            )))
            .expect("Failed to create unary operation")
    }
}

#[derive(Debug)]
pub struct BinExprBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    left: Option<ExprKey<'a>>,
    operator: Option<BinExprOp>,
    right: Option<ExprKey<'a>>,
}

impl<'storage, 'a> BinExprBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        BinExprBuilder {
            storage,
            left: None,
            operator: None,
            right: None,
        }
    }

    pub fn with_left(mut self, left: ExprKey<'a>) -> Self {
        self.left = Some(left);
        self
    }

    pub fn with_operator(mut self, operator: BinExprOp) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_right(mut self, right: ExprKey<'a>) -> Self {
        self.right = Some(right);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::BinExpr(BinExpr::new(
                self.left.expect("Left expression must be provided"),
                self.operator.expect("BinExpr operator must be provided"),
                self.right.expect("Right expression must be provided"),
            )))
            .expect("Failed to create BinExpr")
    }
}

#[derive(Debug)]
pub struct StatementBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    expression: Option<ExprKey<'a>>,
}

impl<'storage, 'a> StatementBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        StatementBuilder {
            storage,
            expression: None,
        }
    }

    pub fn with_expression(mut self, expression: ExprKey<'a>) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::Statement(Statement::new(
                self.expression.expect("Expression must be provided"),
            )))
            .expect("Failed to create statement")
    }
}

#[derive(Debug)]
pub struct BlockBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    elements: Vec<ExprKey<'a>>,
}

impl<'storage, 'a> BlockBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        BlockBuilder {
            storage,
            elements: Vec::new(),
        }
    }

    pub fn add_element(mut self, expr: ExprKey<'a>) -> Self {
        self.elements.push(expr);
        self
    }

    pub fn add_expressions<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = ExprKey<'a>>,
    {
        self.elements.extend(elements);
        self
    }

    pub fn add_statement(mut self, expression: ExprKey<'a>) -> Self {
        let statement = Builder::new(self.storage)
            .create_statement()
            .with_expression(expression)
            .build();

        self.elements.push(statement);
        self
    }

    pub fn add_statements<I>(mut self, elements: I) -> Self
    where
        I: IntoIterator<Item = ExprKey<'a>>,
    {
        for expression in elements {
            let statement = Builder::new(self.storage)
                .create_statement()
                .with_expression(expression)
                .build();

            self.elements.push(statement);
        }

        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::Block(Block::new(self.elements)))
            .expect("Failed to create block")
    }
}

#[derive(Debug)]
pub struct FunctionBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    name: &'a str,
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Option<TypeKey<'a>>,
    attributes: Vec<ExprKey<'a>>,
    definition: Option<ExprKey<'a>>,
}

impl<'storage, 'a> FunctionBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        FunctionBuilder {
            storage,
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
        ty: TypeKey<'a>,
        default_value: Option<ExprKey<'a>>,
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

    pub fn with_return_type(mut self, ty: TypeKey<'a>) -> Self {
        self.return_type = Some(ty);
        self
    }

    pub fn with_attribute(mut self, attribute: ExprKey<'a>) -> Self {
        self.attributes.push(attribute);
        self
    }

    pub fn with_attributes<I>(mut self, attributes: I) -> Self
    where
        I: IntoIterator<Item = ExprKey<'a>>,
    {
        self.attributes.extend(attributes);
        self
    }

    pub fn with_definition(mut self, definition: ExprKey<'a>) -> Option<Self> {
        match definition.get(self.storage) {
            ExprRef::Block(_) => {
                self.definition = Some(definition);
                Some(self)
            }

            _ => None,
        }
    }

    pub fn build(self) -> ExprKey<'a> {
        let definition = self.definition.map(|d| match d.get(self.storage) {
            ExprRef::Block(block) => block.to_owned(),
            _ => panic!("Function definition must be a block expression"),
        });

        self.storage
            .add_expr(ExprOwned::Function(Function::new(
                self.name,
                self.parameters,
                self.return_type,
                self.attributes,
                definition,
            )))
            .expect("Failed to create function")
    }
}

#[derive(Debug)]
pub struct VariableBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    kind: Option<VariableKind>,
    name: &'a str,
    ty: Option<TypeKey<'a>>,
    value: Option<ExprKey<'a>>,
}

impl<'storage, 'a> VariableBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        VariableBuilder {
            storage,
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

    pub fn with_type(mut self, ty: TypeKey<'a>) -> Self {
        self.ty = Some(ty);
        self
    }

    pub fn with_value(mut self, value: ExprKey<'a>) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        self.storage
            .add_expr(ExprOwned::Variable(Variable::new(
                self.kind.expect("Variable kind must be provided"),
                self.name,
                self.ty,
                self.value,
            )))
            .expect("Failed to create variable")
    }
}

#[derive(Debug)]
pub struct IfBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    condition: Option<ExprKey<'a>>,
    then_branch: Option<ExprKey<'a>>,
    else_branch: Option<ExprKey<'a>>,
}

impl<'storage, 'a> IfBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        IfBuilder {
            storage,
            condition: None,
            then_branch: None,
            else_branch: None,
        }
    }

    pub fn with_condition(mut self, condition: ExprKey<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_then_branch(mut self, then_branch: ExprKey<'a>) -> Self {
        self.then_branch = Some(then_branch);
        self
    }

    pub fn with_else_branch(mut self, else_branch: Option<ExprKey<'a>>) -> Self {
        self.else_branch = else_branch;
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = If::new(
            self.condition.expect("Condition must be provided"),
            self.then_branch.expect("Then branch must be provided"),
            self.else_branch,
        );

        self.storage
            .add_expr(ExprOwned::If(expr))
            .expect("Failed to create if expression")
    }
}

#[derive(Debug)]
pub struct WhileLoopBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    condition: Option<ExprKey<'a>>,
    body: Option<ExprKey<'a>>,
}

impl<'storage, 'a> WhileLoopBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        WhileLoopBuilder {
            storage,
            condition: None,
            body: None,
        }
    }

    pub fn with_condition(mut self, condition: ExprKey<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_body(mut self, body: ExprKey<'a>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = WhileLoop::new(
            self.condition.expect("Condition must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        self.storage
            .add_expr(ExprOwned::WhileLoop(expr))
            .expect("Failed to create while loop expression")
    }
}

#[derive(Debug)]
pub struct DoWhileLoopBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    body: Option<ExprKey<'a>>,
    condition: Option<ExprKey<'a>>,
}

impl<'storage, 'a> DoWhileLoopBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        DoWhileLoopBuilder {
            storage,
            body: None,
            condition: None,
        }
    }

    pub fn with_body(mut self, body: ExprKey<'a>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn with_condition(mut self, condition: ExprKey<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = DoWhileLoop::new(
            self.condition.expect("Condition must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        self.storage
            .add_expr(ExprOwned::DoWhileLoop(expr))
            .expect("Failed to create do-while loop expression")
    }
}

#[derive(Debug)]
pub struct SwitchBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    condition: Option<ExprKey<'a>>,
    cases: Vec<(ExprKey<'a>, ExprKey<'a>)>,
    default: Option<ExprKey<'a>>,
}

impl<'storage, 'a> SwitchBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        SwitchBuilder {
            storage,
            condition: None,
            cases: Vec::new(),
            default: None,
        }
    }

    pub fn with_condition(mut self, condition: ExprKey<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn add_case(mut self, case: ExprKey<'a>, body: ExprKey<'a>) -> Self {
        self.cases.push((case, body));
        self
    }

    pub fn add_cases<I>(mut self, cases: I) -> Self
    where
        I: IntoIterator<Item = (ExprKey<'a>, ExprKey<'a>)>,
    {
        self.cases.extend(cases);
        self
    }

    pub fn with_default(mut self, default: Option<ExprKey<'a>>) -> Self {
        self.default = default;
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = super::control_flow::Switch::new(
            self.condition.expect("Condition must be provided"),
            self.cases,
            self.default,
        );

        self.storage
            .add_expr(ExprOwned::Switch(expr))
            .expect("Failed to create switch expression")
    }
}

#[derive(Debug)]
pub struct BreakBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    label: Option<&'a str>,
}

impl<'storage, 'a> BreakBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        BreakBuilder {
            storage,
            label: None,
        }
    }

    pub fn with_label(mut self, label: Option<&'a str>) -> Self {
        self.label = label;
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = Break::new(self.label);

        self.storage
            .add_expr(ExprOwned::Break(expr))
            .expect("Failed to create break expression")
    }
}

#[derive(Debug)]
pub struct ContinueBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    label: Option<&'a str>,
}

impl<'storage, 'a> ContinueBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ContinueBuilder {
            storage,
            label: None,
        }
    }

    pub fn with_label(mut self, label: Option<&'a str>) -> Self {
        self.label = label;
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = Continue::new(self.label);

        self.storage
            .add_expr(ExprOwned::Continue(expr))
            .expect("Failed to create continue expression")
    }
}

#[derive(Debug)]
pub struct ReturnBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<ExprKey<'a>>,
}

impl<'storage, 'a> ReturnBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ReturnBuilder {
            storage,
            value: None,
        }
    }

    pub fn with_value(mut self, value: Option<ExprKey<'a>>) -> Self {
        self.value = value;
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = Return::new(self.value);

        self.storage
            .add_expr(ExprOwned::Return(expr))
            .expect("Failed to create return expression")
    }
}

#[derive(Debug)]
pub struct ForEachBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    bindings: Vec<(&'a str, Option<TypeKey<'a>>)>,
    iterable: Option<ExprKey<'a>>,
    body: Option<ExprKey<'a>>,
}

impl<'storage, 'a> ForEachBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ForEachBuilder {
            storage,
            bindings: Vec::new(),
            iterable: None,
            body: None,
        }
    }

    pub fn add_binding(mut self, name: &'a str, ty: Option<TypeKey<'a>>) -> Self {
        self.bindings.push((name, ty));
        self
    }

    pub fn add_bindings<I>(mut self, bindings: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Option<TypeKey<'a>>)>,
    {
        self.bindings.extend(bindings);
        self
    }

    pub fn with_iterable(mut self, iterable: ExprKey<'a>) -> Self {
        self.iterable = Some(iterable);
        self
    }

    pub fn with_body(mut self, body: ExprKey<'a>) -> Self {
        self.body = Some(body);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = ForEach::new(
            self.bindings,
            self.iterable.expect("Iterable expression must be provided"),
            self.body.expect("Body expression must be provided"),
        );

        self.storage
            .add_expr(ExprOwned::ForEach(expr))
            .expect("Failed to create for-each expression")
    }
}

#[derive(Debug)]
pub struct AwaitBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    expression: Option<ExprKey<'a>>,
}

impl<'storage, 'a> AwaitBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        AwaitBuilder {
            storage,
            expression: None,
        }
    }

    pub fn with_expression(mut self, expression: ExprKey<'a>) -> Self {
        self.expression = Some(expression);
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = Await::new(self.expression.expect("Expression must be provided"));

        self.storage
            .add_expr(ExprOwned::Await(expr))
            .expect("Failed to create await expression")
    }
}

#[derive(Debug)]
pub struct AssertBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    condition: Option<ExprKey<'a>>,
    message: Option<ExprKey<'a>>,
}

impl<'storage, 'a> AssertBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        AssertBuilder {
            storage,
            condition: None,
            message: None,
        }
    }

    pub fn with_condition(mut self, condition: ExprKey<'a>) -> Self {
        self.condition = Some(condition);
        self
    }

    pub fn with_message(mut self, message: Option<ExprKey<'a>>) -> Self {
        self.message = message;
        self
    }

    pub fn build(self) -> ExprKey<'a> {
        let expr = Assert::new(
            self.condition.expect("Condition must be provided"),
            self.message,
        );

        self.storage
            .add_expr(ExprOwned::Assert(expr))
            .expect("Failed to create assert expression")
    }
}
