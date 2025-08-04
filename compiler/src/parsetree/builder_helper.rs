use super::array_type::ArrayType;
use super::binary_op::{BinaryOp, BinaryOperator};
use super::block::Block;
use super::builder::Builder;
use super::character::CharLit;
use super::expression::{ExprOwned, ExprRef, TypeOwned, TypeRef};
use super::function::{Function, FunctionParameter};
use super::function_type::FunctionType;
use super::list::ListLit;
use super::number::{FloatLit, IntegerLit};
use super::object::ObjectLit;
use super::returns::Return;
use super::statement::Statement;
use super::storage::{ExprKey, Storage, TypeKey};
use super::string::StringLit;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use super::unary_op::{UnaryOp, UnaryOperator};
use super::variable::{Variable, VariableKind};
use crate::lexer::IntegerKind;
use apint::UInt;
use std::collections::BTreeMap;

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

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage.add_expr(ExprOwned::IntegerLit(IntegerLit::new(
            self.value.expect("Integer value must be provided"),
            self.kind.unwrap_or(IntegerKind::Decimal),
        )?))
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

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage.add_expr(ExprOwned::FloatLit(FloatLit::new(
            self.value.expect("Float value must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct StringBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<&'a [u8]>,
}

impl<'storage, 'a> StringBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        StringBuilder {
            storage,
            value: None,
        }
    }

    pub fn with_utf8string(mut self, value: &'a str) -> Self {
        self.value = Some(value.as_bytes());
        self
    }

    pub fn with_raw_bytes(mut self, value: &'a [u8]) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage.add_expr(ExprOwned::StringLit(StringLit::new(
            self.value.expect("String value must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct CharBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<char>,
}

impl<'storage, 'a> CharBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        CharBuilder {
            storage,
            value: None,
        }
    }

    pub fn with_char(mut self, value: char) -> Self {
        self.value = Some(value);
        self
    }

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage.add_expr(ExprOwned::CharLit(CharLit::new(
            self.value.expect("Char value must be provided"),
        )))
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

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage
            .add_expr(ExprOwned::ListLit(ListLit::new(self.elements)))
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

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage
            .add_expr(ExprOwned::ObjectLit(ObjectLit::new(self.fields)))
    }
}

#[derive(Debug)]
pub struct UnaryOpBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    operator: Option<UnaryOperator>,
    operand: Option<ExprKey<'a>>,
    is_postfix: Option<bool>,
}

impl<'storage, 'a> UnaryOpBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        UnaryOpBuilder {
            storage,
            operator: None,
            operand: None,
            is_postfix: None,
        }
    }

    pub fn with_operator(mut self, operator: UnaryOperator) -> Self {
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

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage.add_expr(ExprOwned::UnaryOp(UnaryOp::new(
            self.operand.expect("Operand must be provided"),
            self.operator.expect("Unary operator must be provided"),
            self.is_postfix.expect("Postfix flag must be provided"),
        )))
    }
}

#[derive(Debug)]
pub struct BinaryOpBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    left: Option<ExprKey<'a>>,
    operator: Option<BinaryOperator>,
    right: Option<ExprKey<'a>>,
}

impl<'storage, 'a> BinaryOpBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        BinaryOpBuilder {
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

    pub fn with_operator(mut self, operator: BinaryOperator) -> Self {
        self.operator = Some(operator);
        self
    }

    pub fn with_right(mut self, right: ExprKey<'a>) -> Self {
        self.right = Some(right);
        self
    }

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage.add_expr(ExprOwned::BinaryOp(BinaryOp::new(
            self.left.expect("Left expression must be provided"),
            self.operator.expect("Binary operator must be provided"),
            self.right.expect("Right expression must be provided"),
        )))
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

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage.add_expr(ExprOwned::Statement(Statement::new(
            self.expression.expect("Expression must be provided"),
        )))
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

    pub fn add_statement(mut self, expression: ExprKey<'a>) -> Option<Self> {
        let statement = Builder::new(self.storage)
            .create_statement()
            .with_expression(expression)
            .build()?;

        self.elements.push(statement);
        Some(self)
    }

    pub fn add_statements<I>(mut self, elements: I) -> Option<Self>
    where
        I: IntoIterator<Item = ExprKey<'a>>,
    {
        for expression in elements {
            let statement = Builder::new(self.storage)
                .create_statement()
                .with_expression(expression)
                .build()?;

            self.elements.push(statement);
        }

        Some(self)
    }

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage
            .add_expr(ExprOwned::Block(Block::new(self.elements)))
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
        ty: Option<TypeKey<'a>>,
        default_value: Option<ExprKey<'a>>,
    ) -> Self {
        self.parameters.push((name, ty, default_value));
        self
    }

    pub fn with_parameters<I>(mut self, parameters: I) -> Self
    where
        I: IntoIterator<Item = (&'a str, Option<TypeKey<'a>>, Option<ExprKey<'a>>)>,
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

    pub fn build(self) -> Option<ExprKey<'a>> {
        let definition = self.definition.map(|d| match d.get(self.storage) {
            ExprRef::Block(block) => block.to_owned(),
            _ => panic!("Function definition must be a block expression"),
        });

        self.storage.add_expr(ExprOwned::Function(Function::new(
            self.name,
            self.parameters,
            self.return_type,
            self.attributes,
            definition,
        )))
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

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage.add_expr(ExprOwned::Variable(Variable::new(
            self.kind.expect("Variable kind must be provided"),
            self.name,
            self.ty,
            self.value,
        )))
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

    pub fn build(self) -> Option<ExprKey<'a>> {
        self.storage
            .add_expr(ExprOwned::Return(Return::new(self.value)))
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

    pub fn build(self) -> Option<TypeKey<'a>> {
        self.storage
            .add_type(TypeOwned::TupleType(TupleType::new(self.elements)))
    }
}

#[derive(Debug)]
pub struct ArrayTypeBuilder<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    element_ty: Option<TypeKey<'a>>,
    count: Option<ExprKey<'a>>,
}

impl<'storage, 'a> ArrayTypeBuilder<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ArrayTypeBuilder {
            storage,
            element_ty: None,
            count: None,
        }
    }

    pub fn with_element_ty(mut self, element_ty: TypeKey<'a>) -> Self {
        self.element_ty = Some(element_ty);
        self
    }

    pub fn with_count(mut self, count: ExprKey<'a>) -> Self {
        self.count = Some(count);
        self
    }

    pub fn build(self) -> Option<TypeKey<'a>> {
        self.storage.add_type(TypeOwned::ArrayType(ArrayType::new(
            self.element_ty.expect("Element type must be provided"),
            self.count.expect("Count must be provided"),
        )))
    }
}

// #[derive(Debug)]
// pub struct StructTypeBuilder<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     name: Option<&'a str>,
//     attributes: Vec<ExprKey<'a>>,
//     fields: BTreeMap<&'a str, TypeKey<'a>>,
// }

// impl<'storage, 'a> StructTypeBuilder<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         StructTypeBuilder {
//             outer,
//             name: None,
//             attributes: Vec::new(),
//             fields: BTreeMap::new(),
//         }
//     }

//     pub fn with_name(mut self, name: &'a str) -> Self {
//         self.name = Some(name);
//         self
//     }

//     pub fn add_attribute(mut self, attribute: ExprKey<'a>) -> Self {
//         self.attributes.push(attribute);
//         self
//     }

//     pub fn add_attributes<I>(mut self, attributes: I) -> Self
//     where
//         I: IntoIterator<Item = ExprKey<'a>>,
//     {
//         self.attributes.extend(attributes);
//         self
//     }

//     pub fn add_field(mut self, name: &'a str, ty: TypeKey<'a>) -> Self {
//         self.fields.insert(name, ty);
//         self
//     }

//     pub fn add_fields<I>(mut self, fields: I) -> Self
//     where
//         I: IntoIterator<Item = (&'a str, TypeKey<'a>)>,
//     {
//         self.fields.extend(fields);
//         self
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         let struct_type = StructType::new(self.name, self.attributes, self.fields);

//         Box::new(Expr::StructType(struct_type))
//     }
// }

// #[derive(Debug)]
// pub struct FunctionTypeBuilder<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     parameters: Vec<FunctionParameter<'a>>,
//     return_type: Option<TypeKey<'a>>,
//     attributes: Vec<ExprKey<'a>>,
// }

// impl<'storage, 'a> FunctionTypeBuilder<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         FunctionTypeBuilder {
//             outer,
//             parameters: Vec::new(),
//             return_type: None,
//             attributes: Vec::new(),
//         }
//     }

//     pub fn add_parameter(
//         mut self,
//         name: &'a str,
//         ty: Option<TypeKey<'a>>,
//         default_value: Option<ExprKey<'a>>,
//     ) -> Self {
//         self.parameters.push((name, ty, default_value));
//         self
//     }

//     pub fn add_parameters<I>(mut self, parameters: I) -> Self
//     where
//         I: IntoIterator<Item = (&'a str, Option<TypeKey<'a>>, Option<ExprKey<'a>>)>,
//     {
//         self.parameters.extend(parameters);
//         self
//     }

//     pub fn with_return_type(mut self, ty: TypeKey<'a>) -> Self {
//         self.return_type = Some(ty);
//         self
//     }

//     pub fn add_attribute(mut self, attribute: ExprKey<'a>) -> Self {
//         self.attributes.push(attribute);
//         self
//     }

//     pub fn add_attributes<I>(mut self, attributes: I) -> Self
//     where
//         I: IntoIterator<Item = ExprKey<'a>>,
//     {
//         self.attributes.extend(attributes);
//         self
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         let function_type = FunctionType::new(self.parameters, self.return_type, self.attributes);

//         Box::new(Expr::FunctionType(function_type))
//     }
// }
