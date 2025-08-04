use super::array_type::ArrayType;
use super::binary_op::{BinaryOp, BinaryOperator};
use super::block::Block;
use super::builder::Builder;
use super::character::CharLit;
use super::expression::ExprOwned;
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
pub struct IntegerBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<UInt>,
    kind: Option<IntegerKind>,
}

impl<'storage, 'a> IntegerBuilderHelper<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        IntegerBuilderHelper {
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
pub struct FloatBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<f64>,
}

impl<'storage, 'a> FloatBuilderHelper<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        FloatBuilderHelper {
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
pub struct StringBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<&'a [u8]>,
}

impl<'storage, 'a> StringBuilderHelper<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        StringBuilderHelper {
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
pub struct CharBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<char>,
}

impl<'storage, 'a> CharBuilderHelper<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        CharBuilderHelper {
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
pub struct ListBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    elements: Vec<ExprKey<'a>>,
}

impl<'storage, 'a> ListBuilderHelper<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ListBuilderHelper {
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
pub struct ObjectBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    fields: BTreeMap<&'a str, ExprKey<'a>>,
}

impl<'storage, 'a> ObjectBuilderHelper<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        ObjectBuilderHelper {
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
pub struct UnaryOpBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    operator: Option<UnaryOperator>,
    operand: Option<ExprKey<'a>>,
    is_postfix: Option<bool>,
}

impl<'storage, 'a> UnaryOpBuilderHelper<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        UnaryOpBuilderHelper {
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
pub struct BinaryOpBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    left: Option<ExprKey<'a>>,
    operator: Option<BinaryOperator>,
    right: Option<ExprKey<'a>>,
}

impl<'storage, 'a> BinaryOpBuilderHelper<'storage, 'a> {
    pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
        BinaryOpBuilderHelper {
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

// #[derive(Debug)]
// pub struct StatementBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     expression: Option<ExprKey<'a>>,
// }

// impl<'storage, 'a> StatementBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         StatementBuilderHelper {
//             outer,
//             expression: None,
//         }
//     }

//     pub fn with_expression(mut self, expression: ExprKey<'a>) -> Self {
//         self.expression = Some(expression);
//         self
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         let expression = self.expression.expect("Expression must be provided");

//         Box::new(Expr::Statement(Statement::new(expression)))
//     }
// }

// #[derive(Debug)]
// pub struct BlockBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     elements: Vec<ExprKey<'a>>,
// }

// impl<'storage, 'a> BlockBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         BlockBuilderHelper {
//             outer,
//             elements: Vec::new(),
//         }
//     }

//     pub fn add_element(mut self, expr: ExprKey<'a>) -> Self {
//         self.elements.push(expr);
//         self
//     }

//     pub fn add_expressions<I>(mut self, elements: I) -> Self
//     where
//         I: IntoIterator<Item = ExprKey<'a>>,
//     {
//         self.elements.extend(elements);
//         self
//     }

//     pub fn add_statement(mut self, expression: ExprKey<'a>) -> Self {
//         let statement = Builder::get_statement().with_expression(expression).build();
//         self.elements.push(statement);
//         self
//     }

//     pub fn add_statements<I>(mut self, statements: I) -> Self
//     where
//         I: IntoIterator<Item = ExprKey<'a>>,
//     {
//         for statement in statements {
//             let statement = Builder::get_statement().with_expression(statement).build();
//             self.elements.push(statement);
//         }
//         self
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         Box::new(Expr::Block(Block::new(self.elements)))
//     }
// }

// #[derive(Debug)]
// pub struct FunctionBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     name: &'a str,
//     parameters: Vec<FunctionParameter<'a>>,
//     return_type: Option<Box<Type<'a>>>,
//     attributes: Vec<ExprKey<'a>>,
//     definition: Option<ExprKey<'a>>,
// }

// impl<'storage, 'a> FunctionBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         FunctionBuilderHelper {
//             outer,
//             name: "",
//             parameters: Vec::new(),
//             return_type: None,
//             attributes: Vec::new(),
//             definition: None,
//         }
//     }

//     pub fn with_name(mut self, name: &'a str) -> Self {
//         self.name = name;
//         self
//     }

//     pub fn with_parameter(
//         mut self,
//         name: &'a str,
//         ty: Option<Box<Type<'a>>>,
//         default_value: Option<ExprKey<'a>>,
//     ) -> Self {
//         self.parameters.push((name, ty, default_value));
//         self
//     }

//     pub fn with_parameters<I>(mut self, parameters: I) -> Self
//     where
//         I: IntoIterator<Item = (&'a str, Option<Box<Type<'a>>>, Option<ExprKey<'a>>)>,
//     {
//         self.parameters.extend(parameters);
//         self
//     }

//     pub fn with_return_type(mut self, ty: Box<Type<'a>>) -> Self {
//         self.return_type = Some(ty);
//         self
//     }

//     pub fn with_attribute(mut self, attribute: ExprKey<'a>) -> Self {
//         self.attributes.push(attribute);
//         self
//     }

//     pub fn with_attributes<I>(mut self, attributes: I) -> Self
//     where
//         I: IntoIterator<Item = ExprKey<'a>>,
//     {
//         self.attributes.extend(attributes);
//         self
//     }

//     pub fn with_definition(mut self, definition: ExprKey<'a>) -> Self {
//         match *definition {
//             Expr::Block(_) => {
//                 self.definition = Some(definition);
//                 self
//             }

//             _ => panic!("Function definition must be a block expression"),
//         }
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         let definition = self.definition.map(|d| match *d {
//             Expr::Block(block) => Block::new(block.into_inner()),
//             _ => panic!("Function definition must be a block expression"),
//         });

//         let function = Function::new(
//             self.name,
//             self.parameters,
//             self.return_type,
//             self.attributes,
//             definition,
//         );

//         Box::new(Expr::Function(function))
//     }
// }

// #[derive(Debug)]
// pub struct VariableBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     kind: Option<VariableKind>,
//     name: &'a str,
//     ty: Option<Box<Type<'a>>>,
//     value: Option<ExprKey<'a>>,
// }

// impl<'storage, 'a> VariableBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         VariableBuilderHelper {
//             outer,
//             kind: None,
//             name: "",
//             ty: None,
//             value: None,
//         }
//     }

//     pub fn with_kind(mut self, kind: VariableKind) -> Self {
//         self.kind = Some(kind);
//         self
//     }

//     pub fn with_name(mut self, name: &'a str) -> Self {
//         self.name = name;
//         self
//     }

//     pub fn with_type(mut self, ty: Box<Type<'a>>) -> Self {
//         self.ty = Some(ty);
//         self
//     }

//     pub fn with_value(mut self, value: ExprKey<'a>) -> Self {
//         self.value = Some(value);
//         self
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         let kind = self.kind.expect("Variable kind must be provided");

//         Box::new(Expr::Variable(Variable::new(
//             kind, self.name, self.ty, self.value,
//         )))
//     }
// }

// #[derive(Debug)]
// pub struct ReturnBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     value: Option<ExprKey<'a>>,
// }

// impl<'storage, 'a> ReturnBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         ReturnBuilderHelper { outer, value: None }
//     }

//     pub fn with_value(mut self, value: ExprKey<'a>) -> Self {
//         self.value = Some(value);
//         self
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         Box::new(Expr::Return(Return::new(self.value)))
//     }
// }

// #[derive(Debug)]
// pub struct TupleTypeBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     elements: Vec<Box<Type<'a>>>,
// }

// impl<'storage, 'a> TupleTypeBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         TupleTypeBuilderHelper {
//             outer,
//             elements: Vec::new(),
//         }
//     }

//     pub fn add_element(mut self, ty: Box<Type<'a>>) -> Self {
//         self.elements.push(ty);
//         self
//     }

//     pub fn add_elements<I>(mut self, elements: I) -> Self
//     where
//         I: IntoIterator<Item = Box<Type<'a>>>,
//     {
//         self.elements.extend(elements);
//         self
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         Box::new(Expr::TupleType(TupleType::new(self.elements)))
//     }
// }

// #[derive(Debug)]
// pub struct ArrayTypeBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     element_ty: Option<Box<Type<'a>>>,
//     count: Option<ExprKey<'a>>,
// }

// impl<'storage, 'a> ArrayTypeBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         ArrayTypeBuilderHelper {
//             outer,
//             element_ty: None,
//             count: None,
//         }
//     }

//     pub fn with_element_ty(mut self, element_ty: Box<Type<'a>>) -> Self {
//         self.element_ty = Some(element_ty);
//         self
//     }

//     pub fn with_count(mut self, count: ExprKey<'a>) -> Self {
//         self.count = Some(count);
//         self
//     }

//     pub fn build(self) -> Option<ExprKey<'a>> {
//         let element_ty = self.element_ty.expect("Element type must be provided");
//         let count = self.count.expect("Count must be provided");

//         let array_type = ArrayType::new(element_ty, count);

//         Box::new(Expr::ArrayType(array_type))
//     }
// }

// #[derive(Debug)]
// pub struct StructTypeBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     name: Option<&'a str>,
//     attributes: Vec<ExprKey<'a>>,
//     fields: BTreeMap<&'a str, Box<Type<'a>>>,
// }

// impl<'storage, 'a> StructTypeBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         StructTypeBuilderHelper {
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

//     pub fn add_field(mut self, name: &'a str, ty: Box<Type<'a>>) -> Self {
//         self.fields.insert(name, ty);
//         self
//     }

//     pub fn add_fields<I>(mut self, fields: I) -> Self
//     where
//         I: IntoIterator<Item = (&'a str, Box<Type<'a>>)>,
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
// pub struct FunctionTypeBuilderHelper<'storage, 'a> {
//     storage: &'storage mut Storage<'a>,
//     parameters: Vec<FunctionParameter<'a>>,
//     return_type: Option<Box<Type<'a>>>,
//     attributes: Vec<ExprKey<'a>>,
// }

// impl<'storage, 'a> FunctionTypeBuilderHelper<'storage, 'a> {
//     pub(crate) fn new(storage: &'storage mut Storage<'a>) -> Self {
//         FunctionTypeBuilderHelper {
//             outer,
//             parameters: Vec::new(),
//             return_type: None,
//             attributes: Vec::new(),
//         }
//     }

//     pub fn add_parameter(
//         mut self,
//         name: &'a str,
//         ty: Option<Box<Type<'a>>>,
//         default_value: Option<ExprKey<'a>>,
//     ) -> Self {
//         self.parameters.push((name, ty, default_value));
//         self
//     }

//     pub fn add_parameters<I>(mut self, parameters: I) -> Self
//     where
//         I: IntoIterator<Item = (&'a str, Option<Box<Type<'a>>>, Option<ExprKey<'a>>)>,
//     {
//         self.parameters.extend(parameters);
//         self
//     }

//     pub fn with_return_type(mut self, ty: Box<Type<'a>>) -> Self {
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
