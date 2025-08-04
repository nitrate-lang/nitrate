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
            self.value?,
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
        self.storage
            .add_expr(ExprOwned::FloatLit(FloatLit::new(self.value?)))
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
        self.storage
            .add_expr(ExprOwned::StringLit(StringLit::new(self.value?)))
    }
}

#[derive(Debug)]
pub struct CharBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    value: Option<char>,
}

impl<'storage, 'a> CharBuilderHelper<'storage, 'a> {
    pub fn new(storage: &'storage mut Storage<'a>) -> Self {
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
        self.storage
            .add_expr(ExprOwned::CharLit(CharLit::new(self.value?)))
    }
}

#[derive(Debug)]
pub struct ListBuilderHelper<'storage, 'a> {
    storage: &'storage mut Storage<'a>,
    elements: Vec<ExprKey<'a>>,
}

impl<'storage, 'a> ListBuilderHelper<'storage, 'a> {
    pub fn new(storage: &'storage mut Storage<'a>) -> Self {
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
    pub fn new(storage: &'storage mut Storage<'a>) -> Self {
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

// #[derive(Debug)]
// pub struct UnaryExprBuilderHelper<'a> {
//     outer: Builder<'a>,
//     operator: Option<UnaryOperator>,
//     operand: Option<ExprKey<'a>>,
//     is_postfix: Option<bool>,
// }

// impl<'a> UnaryExprBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
//         UnaryExprBuilderHelper {
//             outer,
//             operator: None,
//             operand: None,
//             is_postfix: None,
//         }
//     }

//     pub fn with_operator(mut self, operator: UnaryOperator) -> Self {
//         self.operator = Some(operator);
//         self
//     }

//     pub fn with_operand(mut self, operand: ExprKey<'a>) -> Self {
//         self.operand = Some(operand);
//         self
//     }

//     pub fn set_postfix(mut self, is_postfix: bool) -> Self {
//         self.is_postfix = Some(is_postfix);
//         self
//     }

//     pub fn build(self) -> ExprKey<'a> {
//         let operator = self.operator.expect("Unary operator must be provided");
//         let operand = self.operand.expect("Operand must be provided");
//         let is_postfix = self.is_postfix.expect("is_postfix flag must be provided");

//         let unary = UnaryOp::new(operand, operator, is_postfix);

//         Box::new(Expr::UnaryOp(unary))
//     }
// }

// #[derive(Debug)]
// pub struct BinaryExprBuilderHelper<'a> {
//     outer: Builder<'a>,
//     left: Option<ExprKey<'a>>,
//     operator: Option<BinaryOperator>,
//     right: Option<ExprKey<'a>>,
// }

// impl<'a> BinaryExprBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
//         BinaryExprBuilderHelper {
//             outer,
//             left: None,
//             right: None,
//             operator: None,
//         }
//     }

//     pub fn with_left(mut self, left: ExprKey<'a>) -> Self {
//         self.left = Some(left);
//         self
//     }

//     pub fn with_operator(mut self, operator: BinaryOperator) -> Self {
//         self.operator = Some(operator);
//         self
//     }

//     pub fn with_right(mut self, right: ExprKey<'a>) -> Self {
//         self.right = Some(right);
//         self
//     }

//     pub fn build(self) -> ExprKey<'a> {
//         let left = self.left.expect("Left expression must be provided");
//         let operator = self.operator.expect("Binary operator must be provided");
//         let right = self.right.expect("Right expression must be provided");

//         let binary_expr = BinaryExpr::new(left, operator, right);

//         Box::new(Expr::BinaryOp(binary_expr))
//     }
// }

// #[derive(Debug)]
// pub struct StatementBuilderHelper<'a> {
//     outer: Builder<'a>,
//     expression: Option<ExprKey<'a>>,
// }

// impl<'a> StatementBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
//         StatementBuilderHelper {
//             outer,
//             expression: None,
//         }
//     }

//     pub fn with_expression(mut self, expression: ExprKey<'a>) -> Self {
//         self.expression = Some(expression);
//         self
//     }

//     pub fn build(self) -> ExprKey<'a> {
//         let expression = self.expression.expect("Expression must be provided");

//         Box::new(Expr::Statement(Statement::new(expression)))
//     }
// }

// #[derive(Debug)]
// pub struct BlockBuilderHelper<'a> {
//     outer: Builder<'a>,
//     elements: Vec<ExprKey<'a>>,
// }

// impl<'a> BlockBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
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

//     pub fn build(self) -> ExprKey<'a> {
//         Box::new(Expr::Block(Block::new(self.elements)))
//     }
// }

// #[derive(Debug)]
// pub struct FunctionBuilderHelper<'a> {
//     outer: Builder<'a>,
//     name: &'a str,
//     parameters: Vec<FunctionParameter<'a>>,
//     return_type: Option<Box<Type<'a>>>,
//     attributes: Vec<ExprKey<'a>>,
//     definition: Option<ExprKey<'a>>,
// }

// impl<'a> FunctionBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
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

//     pub fn build(self) -> ExprKey<'a> {
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
// pub struct VariableBuilderHelper<'a> {
//     outer: Builder<'a>,
//     kind: Option<VariableKind>,
//     name: &'a str,
//     ty: Option<Box<Type<'a>>>,
//     value: Option<ExprKey<'a>>,
// }

// impl<'a> VariableBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
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

//     pub fn build(self) -> ExprKey<'a> {
//         let kind = self.kind.expect("Variable kind must be provided");

//         Box::new(Expr::Variable(Variable::new(
//             kind, self.name, self.ty, self.value,
//         )))
//     }
// }

// #[derive(Debug)]
// pub struct ReturnBuilderHelper<'a> {
//     outer: Builder<'a>,
//     value: Option<ExprKey<'a>>,
// }

// impl<'a> ReturnBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
//         ReturnBuilderHelper { outer, value: None }
//     }

//     pub fn with_value(mut self, value: ExprKey<'a>) -> Self {
//         self.value = Some(value);
//         self
//     }

//     pub fn build(self) -> ExprKey<'a> {
//         Box::new(Expr::Return(Return::new(self.value)))
//     }
// }

// #[derive(Debug)]
// pub struct TupleTypeBuilderHelper<'a> {
//     outer: Builder<'a>,
//     elements: Vec<Box<Type<'a>>>,
// }

// impl<'a> TupleTypeBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
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

//     pub fn build(self) -> ExprKey<'a> {
//         Box::new(Expr::TupleType(TupleType::new(self.elements)))
//     }
// }

// #[derive(Debug)]
// pub struct ArrayTypeBuilderHelper<'a> {
//     outer: Builder<'a>,
//     element_ty: Option<Box<Type<'a>>>,
//     count: Option<ExprKey<'a>>,
// }

// impl<'a> ArrayTypeBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
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

//     pub fn build(self) -> ExprKey<'a> {
//         let element_ty = self.element_ty.expect("Element type must be provided");
//         let count = self.count.expect("Count must be provided");

//         let array_type = ArrayType::new(element_ty, count);

//         Box::new(Expr::ArrayType(array_type))
//     }
// }

// #[derive(Debug)]
// pub struct StructTypeBuilderHelper<'a> {
//     outer: Builder<'a>,
//     name: Option<&'a str>,
//     attributes: Vec<ExprKey<'a>>,
//     fields: BTreeMap<&'a str, Box<Type<'a>>>,
// }

// impl<'a> StructTypeBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
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

//     pub fn build(self) -> ExprKey<'a> {
//         let struct_type = StructType::new(self.name, self.attributes, self.fields);

//         Box::new(Expr::StructType(struct_type))
//     }
// }

// #[derive(Debug)]
// pub struct FunctionTypeBuilderHelper<'a> {
//     outer: Builder<'a>,
//     parameters: Vec<FunctionParameter<'a>>,
//     return_type: Option<Box<Type<'a>>>,
//     attributes: Vec<ExprKey<'a>>,
// }

// impl<'a> FunctionTypeBuilderHelper<'a> {
//     pub fn new(outer: Builder<'a>) -> Self {
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

//     pub fn build(self) -> ExprKey<'a> {
//         let function_type = FunctionType::new(self.parameters, self.return_type, self.attributes);

//         Box::new(Expr::FunctionType(function_type))
//     }
// }
