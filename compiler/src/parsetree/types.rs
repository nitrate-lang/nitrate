use super::array_type::ArrayType;
use super::expression::{CodeFormat, Expr, InnerExpr, Metadata, ToCode};
use super::function_type::FunctionType;
use super::struct_type::StructType;
use super::tuple_type::TupleType;
use crate::lexer::{Identifier, Punctuation, Token};
use crate::parsetree::{ArrayTypeBuilder, OriginTag, StructTypeBuilder, TupleTypeBuilder};
use hashbrown::HashSet;
use std::collections::BTreeMap;
use std::rc::Rc;
use std::sync::Mutex;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub enum InnerType<'a> {
    /* Primitive Types */
    Bool,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Int8,
    Int16,
    Int32,
    Int64,
    Int128,
    Float8,
    Float16,
    Float32,
    Float64,
    Float128,

    /* Compound Types */
    InferType,
    TupleType(TupleType<'a>),
    ArrayType(ArrayType<'a>),
    StructType(StructType<'a>),
    FunctionType(FunctionType<'a>),
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Type<'a> {
    expr: InnerType<'a>,
    has_parenthesis: bool,
}

impl<'a> Type<'a> {
    pub fn new(expr: InnerType<'a>, has_parenthesis: bool) -> Self {
        Type {
            expr,
            has_parenthesis,
        }
    }

    pub fn into_inner(self) -> InnerType<'a> {
        self.expr
    }

    pub fn get(&self) -> &InnerType<'a> {
        &self.expr
    }

    pub fn has_parenthesis(&self) -> bool {
        self.has_parenthesis
    }

    pub fn into_expr(self) -> Expr<'a> {
        let has_parenthesis = self.has_parenthesis();

        let expr = match self.expr {
            InnerType::Bool => InnerExpr::Bool,
            InnerType::UInt8 => InnerExpr::UInt8,
            InnerType::UInt16 => InnerExpr::UInt16,
            InnerType::UInt32 => InnerExpr::UInt32,
            InnerType::UInt64 => InnerExpr::UInt64,
            InnerType::UInt128 => InnerExpr::UInt128,
            InnerType::Int8 => InnerExpr::Int8,
            InnerType::Int16 => InnerExpr::Int16,
            InnerType::Int32 => InnerExpr::Int32,
            InnerType::Int64 => InnerExpr::Int64,
            InnerType::Int128 => InnerExpr::Int128,
            InnerType::Float8 => InnerExpr::Float8,
            InnerType::Float16 => InnerExpr::Float16,
            InnerType::Float32 => InnerExpr::Float32,
            InnerType::Float64 => InnerExpr::Float64,
            InnerType::Float128 => InnerExpr::Float128,

            InnerType::InferType => InnerExpr::InferType,
            InnerType::TupleType(tuple) => InnerExpr::TupleType(tuple),
            InnerType::ArrayType(array) => InnerExpr::ArrayType(array),
            InnerType::StructType(struct_type) => InnerExpr::StructType(struct_type),
            InnerType::FunctionType(function) => InnerExpr::FunctionType(function),
        };

        Expr::new(
            expr,
            Metadata::new(OriginTag::default(), has_parenthesis, None),
        )
    }

    pub fn is_lit(&self) -> bool {
        match &self.expr {
            InnerType::Bool => true,
            InnerType::UInt8 => true,
            InnerType::UInt16 => true,
            InnerType::UInt32 => true,
            InnerType::UInt64 => true,
            InnerType::UInt128 => true,
            InnerType::Int8 => true,
            InnerType::Int16 => true,
            InnerType::Int32 => true,
            InnerType::Int64 => true,
            InnerType::Int128 => true,
            InnerType::Float8 => true,
            InnerType::Float16 => true,
            InnerType::Float32 => true,
            InnerType::Float64 => true,
            InnerType::Float128 => true,

            InnerType::InferType => false,
            InnerType::TupleType(tuple) => tuple.elements().iter().all(|item| item.is_lit()),
            InnerType::ArrayType(array) => array.element_ty().is_lit() && array.count().is_lit(),
            InnerType::StructType(_struct) => _struct
                .fields()
                .iter()
                .all(|(_, field_ty)| field_ty.is_lit()),
            InnerType::FunctionType(function) => {
                function.parameters().iter().all(|(_, ty, default)| {
                    ty.is_lit() && default.as_ref().map_or(true, |d| d.is_lit())
                }) && function.return_type().map_or(true, |ty| ty.is_lit())
                    && function.attributes().iter().all(|attr| attr.is_lit())
            }
        }
    }
}

impl<'a> ToCode<'a> for Type<'a> {
    fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
        if self.has_parenthesis() {
            tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
        }

        match &self.expr {
            InnerType::Bool => tokens.push(Token::Identifier(Identifier::new("bool"))),
            InnerType::UInt8 => tokens.push(Token::Identifier(Identifier::new("u8"))),
            InnerType::UInt16 => tokens.push(Token::Identifier(Identifier::new("u16"))),
            InnerType::UInt32 => tokens.push(Token::Identifier(Identifier::new("u32"))),
            InnerType::UInt64 => tokens.push(Token::Identifier(Identifier::new("u64"))),
            InnerType::UInt128 => tokens.push(Token::Identifier(Identifier::new("u128"))),
            InnerType::Int8 => tokens.push(Token::Identifier(Identifier::new("i8"))),
            InnerType::Int16 => tokens.push(Token::Identifier(Identifier::new("i16"))),
            InnerType::Int32 => tokens.push(Token::Identifier(Identifier::new("i32"))),
            InnerType::Int64 => tokens.push(Token::Identifier(Identifier::new("i64"))),
            InnerType::Int128 => tokens.push(Token::Identifier(Identifier::new("i128"))),
            InnerType::Float8 => tokens.push(Token::Identifier(Identifier::new("f8"))),
            InnerType::Float16 => tokens.push(Token::Identifier(Identifier::new("f16"))),
            InnerType::Float32 => tokens.push(Token::Identifier(Identifier::new("f32"))),
            InnerType::Float64 => tokens.push(Token::Identifier(Identifier::new("f64"))),
            InnerType::Float128 => tokens.push(Token::Identifier(Identifier::new("f128"))),

            InnerType::InferType => tokens.push(Token::Identifier(Identifier::new("_"))),
            InnerType::TupleType(e) => e.to_code(tokens, options),
            InnerType::ArrayType(e) => e.to_code(tokens, options),
            InnerType::StructType(e) => e.to_code(tokens, options),
            InnerType::FunctionType(e) => e.to_code(tokens, options),
        }

        if self.has_parenthesis() {
            tokens.push(Token::Punctuation(Punctuation::RightParenthesis));
        }
    }
}

impl<'a> std::ops::Deref for Type<'a> {
    type Target = InnerType<'a>;

    fn deref(&self) -> &Self::Target {
        &self.expr
    }
}

#[derive(Debug)]
pub struct TypeFactory<'a> {
    bool: Rc<Type<'a>>,
    u8: Rc<Type<'a>>,
    u16: Rc<Type<'a>>,
    u32: Rc<Type<'a>>,
    u64: Rc<Type<'a>>,
    u128: Rc<Type<'a>>,
    i8: Rc<Type<'a>>,
    i16: Rc<Type<'a>>,
    i32: Rc<Type<'a>>,
    i64: Rc<Type<'a>>,
    i128: Rc<Type<'a>>,
    f8: Rc<Type<'a>>,
    f16: Rc<Type<'a>>,
    f32: Rc<Type<'a>>,
    f64: Rc<Type<'a>>,
    f128: Rc<Type<'a>>,
    infer_type: Rc<Type<'a>>,

    compound_types: Mutex<HashSet<Rc<Type<'a>>>>,
}

impl<'a> TypeFactory<'a> {
    pub fn new() -> Self {
        TypeFactory {
            bool: Rc::new(Type::new(InnerType::Bool, false)),
            u8: Rc::new(Type::new(InnerType::UInt8, false)),
            u16: Rc::new(Type::new(InnerType::UInt16, false)),
            u32: Rc::new(Type::new(InnerType::UInt32, false)),
            u64: Rc::new(Type::new(InnerType::UInt64, false)),
            u128: Rc::new(Type::new(InnerType::UInt128, false)),
            i8: Rc::new(Type::new(InnerType::Int8, false)),
            i16: Rc::new(Type::new(InnerType::Int16, false)),
            i32: Rc::new(Type::new(InnerType::Int32, false)),
            i64: Rc::new(Type::new(InnerType::Int64, false)),
            i128: Rc::new(Type::new(InnerType::Int128, false)),
            f8: Rc::new(Type::new(InnerType::Float8, false)),
            f16: Rc::new(Type::new(InnerType::Float16, false)),
            f32: Rc::new(Type::new(InnerType::Float32, false)),
            f64: Rc::new(Type::new(InnerType::Float64, false)),
            f128: Rc::new(Type::new(InnerType::Float128, false)),
            infer_type: Rc::new(Type::new(InnerType::InferType, false)),
            compound_types: Mutex::new(HashSet::new()),
        }
    }

    pub fn get_bool(&self) -> Rc<Type<'a>> {
        self.bool.clone()
    }

    pub fn get_u8(&self) -> Rc<Type<'a>> {
        self.u8.clone()
    }

    pub fn get_u16(&self) -> Rc<Type<'a>> {
        self.u16.clone()
    }

    pub fn get_u32(&self) -> Rc<Type<'a>> {
        self.u32.clone()
    }

    pub fn get_u64(&self) -> Rc<Type<'a>> {
        self.u64.clone()
    }

    pub fn get_u128(&self) -> Rc<Type<'a>> {
        self.u128.clone()
    }

    pub fn get_i8(&self) -> Rc<Type<'a>> {
        self.i8.clone()
    }

    pub fn get_i16(&self) -> Rc<Type<'a>> {
        self.i16.clone()
    }

    pub fn get_i32(&self) -> Rc<Type<'a>> {
        self.i32.clone()
    }

    pub fn get_i64(&self) -> Rc<Type<'a>> {
        self.i64.clone()
    }

    pub fn get_i128(&self) -> Rc<Type<'a>> {
        self.i128.clone()
    }

    pub fn get_f8(&self) -> Rc<Type<'a>> {
        self.f8.clone()
    }

    pub fn get_f16(&self) -> Rc<Type<'a>> {
        self.f16.clone()
    }

    pub fn get_f32(&self) -> Rc<Type<'a>> {
        self.f32.clone()
    }

    pub fn get_f64(&self) -> Rc<Type<'a>> {
        self.f64.clone()
    }

    pub fn get_f128(&self) -> Rc<Type<'a>> {
        self.f128.clone()
    }

    pub fn get_infer_type(&self) -> Rc<Type<'a>> {
        self.infer_type.clone()
    }

    pub fn get_tuple_type(&self, elements: Vec<Rc<Type<'a>>>) -> Rc<Type<'a>> {
        let mut pool = self.compound_types.lock().unwrap();

        let object = Rc::new(Type::new(
            InnerType::TupleType(TupleTypeBuilder::default().with_elements(elements).build()),
            false,
        ));

        pool.get_or_insert(object).clone()
    }

    pub fn get_array_type(&self, element: Rc<Type<'a>>, size: Box<Expr<'a>>) -> Rc<Type<'a>> {
        let mut pool = self.compound_types.lock().unwrap();

        let object = Rc::new(Type::new(
            InnerType::ArrayType(
                ArrayTypeBuilder::default()
                    .with_element_ty(element)
                    .with_count(size)
                    .build(),
            ),
            false,
        ));

        pool.get_or_insert(object).clone()
    }

    pub fn get_struct_type(
        &self,
        name: Option<&'a str>,
        attributes: Vec<Expr<'a>>,
        fields: BTreeMap<&'a str, Rc<Type<'a>>>,
    ) -> Rc<Type<'a>> {
        let mut pool = self.compound_types.lock().unwrap();

        let object = Rc::new(Type::new(
            InnerType::StructType(
                StructTypeBuilder::default()
                    .with_name(name)
                    .with_attributes(attributes)
                    .with_fields(fields)
                    .build(),
            ),
            false,
        ));

        pool.get_or_insert(object).clone()
    }

    pub fn get_function_type(
        &self,
        parameters: Vec<(&'a str, Rc<Type<'a>>, Option<Expr<'a>>)>,
        return_type: Option<Rc<Type<'a>>>,
        attributes: Vec<Expr<'a>>,
        parentheses: bool,
    ) -> Rc<Type<'a>> {
        let mut pool = self.compound_types.lock().unwrap();

        let object = Rc::new(Type::new(
            InnerType::FunctionType(FunctionType::new(parameters, return_type, attributes)),
            parentheses,
        ));

        pool.get_or_insert(object).clone()
    }
}
