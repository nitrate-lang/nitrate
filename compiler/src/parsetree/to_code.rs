// TODO: Reimplement this file

// use super::array_type::ArrayType;
// use super::binary_op::{BinaryExpr, BinaryOperator};
// use super::block::Block;
// use super::character::CharLit;
// use super::expression::{Expr, Type};
// use super::function::Function;
// use super::function_type::FunctionType;
// use super::list::List;
// use super::number::{FloatLit, IntegerLit};
// use super::object::Object;
// use super::returns::Return;
// use super::statement::Statement;
// use super::string::StringLit;
// use super::struct_type::StructType;
// use super::tuple_type::TupleType;
// use super::unary_op::{UnaryExpr, UnaryOperator};
// use super::variable::{Variable, VariableKind};
// use crate::lexer::{
//     Float, Identifier, Integer, Keyword, Operator, Punctuation, StringLit as StringLitToken, Token,
// };

// #[derive(Debug, Default, Clone, PartialEq, PartialOrd, Hash)]
// pub struct CodeFormat {}

// pub trait ToCode<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat);
// }

// impl<'a> ToCode<'a> for IntegerLit {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
//         let u128 = self
//             .try_to_u128()
//             .expect("IntegerLit apint::UInt value should fit in u128");

//         let number = Integer::new(u128, self.kind());
//         tokens.push(Token::Integer(number));
//     }
// }

// impl<'a> ToCode<'a> for FloatLit {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
//         let number = Float::new(self.get());
//         tokens.push(Token::Float(number));
//     }
// }

// impl<'a> ToCode<'a> for StringLit<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
//         let string_lit = StringLitToken::from_ref(self);
//         tokens.push(Token::String(string_lit));
//     }
// }

// impl<'a> ToCode<'a> for CharLit {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
//         tokens.push(Token::Char(self.get()));
//     }
// }

// impl<'a> ToCode<'a> for List<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Punctuation(Punctuation::LeftBracket));
//         for (i, expr) in self.elements().iter().enumerate() {
//             (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
//             expr.to_code(tokens, options);
//         }
//         tokens.push(Token::Punctuation(Punctuation::RightBracket));
//     }
// }

// impl<'a> ToCode<'a> for Object<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Punctuation(Punctuation::LeftBracket));
//         for (key, value) in self.get() {
//             tokens.push(Token::Identifier(Identifier::new(key)));
//             tokens.push(Token::Punctuation(Punctuation::Colon));

//             value.to_code(tokens, options);
//             tokens.push(Token::Punctuation(Punctuation::Comma));
//         }
//         tokens.push(Token::Punctuation(Punctuation::RightBracket));
//     }
// }

// impl<'a> ToCode<'a> for UnaryOperator {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
//         let operator = Token::Operator(match self {
//             UnaryOperator::Add => Operator::Add,
//             UnaryOperator::Sub => Operator::Sub,
//             UnaryOperator::Mul => Operator::Mul,
//             UnaryOperator::BitAnd => Operator::BitAnd,
//             UnaryOperator::BitNot => Operator::BitNot,
//             UnaryOperator::LogicNot => Operator::LogicNot,
//             UnaryOperator::Inc => Operator::Inc,
//             UnaryOperator::Dec => Operator::Dec,
//             UnaryOperator::Sizeof => Operator::Sizeof,
//             UnaryOperator::Alignof => Operator::Alignof,
//             UnaryOperator::Typeof => Operator::Typeof,
//             UnaryOperator::Question => Operator::Question,
//         });

//         tokens.push(operator);
//     }
// }

// impl<'a> ToCode<'a> for UnaryExpr<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         if self.is_postfix() {
//             self.operand().to_code(tokens, options);
//             self.operator().to_code(tokens, options);
//         } else {
//             self.operator().to_code(tokens, options);
//             self.operand().to_code(tokens, options);
//         }
//     }
// }

// impl<'a> ToCode<'a> for BinaryOperator {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, _options: &CodeFormat) {
//         let operator = Token::Operator(match self {
//             BinaryOperator::Add => Operator::Add,
//             BinaryOperator::Sub => Operator::Sub,
//             BinaryOperator::Mul => Operator::Mul,
//             BinaryOperator::Div => Operator::Div,
//             BinaryOperator::Mod => Operator::Mod,
//             BinaryOperator::BitAnd => Operator::BitAnd,
//             BinaryOperator::BitOr => Operator::BitOr,
//             BinaryOperator::BitXor => Operator::BitXor,
//             BinaryOperator::BitShl => Operator::BitShl,
//             BinaryOperator::BitShr => Operator::BitShr,
//             BinaryOperator::BitRotl => Operator::BitRotl,
//             BinaryOperator::BitRotr => Operator::BitRotr,
//             BinaryOperator::LogicAnd => Operator::LogicAnd,
//             BinaryOperator::LogicOr => Operator::LogicOr,
//             BinaryOperator::LogicXor => Operator::LogicXor,
//             BinaryOperator::LogicLt => Operator::LogicLt,
//             BinaryOperator::LogicGt => Operator::LogicGt,
//             BinaryOperator::LogicLe => Operator::LogicLe,
//             BinaryOperator::LogicGe => Operator::LogicGe,
//             BinaryOperator::LogicEq => Operator::LogicEq,
//             BinaryOperator::LogicNe => Operator::LogicNe,
//             BinaryOperator::Set => Operator::Set,
//             BinaryOperator::SetPlus => Operator::SetPlus,
//             BinaryOperator::SetMinus => Operator::SetMinus,
//             BinaryOperator::SetTimes => Operator::SetTimes,
//             BinaryOperator::SetSlash => Operator::SetSlash,
//             BinaryOperator::SetPercent => Operator::SetPercent,
//             BinaryOperator::SetBitAnd => Operator::SetBitAnd,
//             BinaryOperator::SetBitOr => Operator::SetBitOr,
//             BinaryOperator::SetBitXor => Operator::SetBitXor,
//             BinaryOperator::SetBitShl => Operator::SetBitShl,
//             BinaryOperator::SetBitShr => Operator::SetBitShr,
//             BinaryOperator::SetBitRotl => Operator::SetBitRotl,
//             BinaryOperator::SetBitRotr => Operator::SetBitRotr,
//             BinaryOperator::SetLogicAnd => Operator::SetLogicAnd,
//             BinaryOperator::SetLogicOr => Operator::SetLogicOr,
//             BinaryOperator::SetLogicXor => Operator::SetLogicXor,
//             BinaryOperator::As => Operator::As,
//             BinaryOperator::Dot => Operator::Dot,
//             BinaryOperator::Ellipsis => Operator::Ellipsis,
//             BinaryOperator::Scope => Operator::Scope,
//             BinaryOperator::Arrow => Operator::Arrow,
//             BinaryOperator::BlockArrow => Operator::BlockArrow,
//             BinaryOperator::Range => Operator::Range,
//             BinaryOperator::Question => Operator::Question,
//             BinaryOperator::Spaceship => Operator::Spaceship,
//         });

//         tokens.push(operator);
//     }
// }

// impl<'a> ToCode<'a> for BinaryExpr<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         self.left().to_code(tokens, options);
//         self.op().to_code(tokens, options);
//         self.right().to_code(tokens, options);
//     }
// }

// impl<'a> ToCode<'a> for Statement<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         self.get().to_code(tokens, options);
//         tokens.push(Token::Punctuation(Punctuation::Semicolon));
//     }
// }

// impl<'a> ToCode<'a> for Block<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Punctuation(Punctuation::LeftBrace));
//         for expr in self.elements() {
//             expr.to_code(tokens, options);
//         }
//         tokens.push(Token::Punctuation(Punctuation::RightBrace));
//     }
// }

// impl<'a> ToCode<'a> for Function<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Keyword(Keyword::Fn));

//         if !self.attributes().is_empty() {
//             tokens.push(Token::Punctuation(Punctuation::LeftBracket));
//             for (i, attr) in self.attributes().iter().enumerate() {
//                 (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
//                 attr.to_code(tokens, options);
//             }
//             tokens.push(Token::Punctuation(Punctuation::RightBracket));
//         }

//         if !self.name().is_empty() {
//             tokens.push(Token::Identifier(Identifier::new(self.name())));
//         }

//         tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
//         for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
//             (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));

//             tokens.push(Token::Identifier(Identifier::new(name)));

//             if let Some(ty) = ty {
//                 if **ty != Type::InferType {
//                     tokens.push(Token::Punctuation(Punctuation::Colon));
//                     ty.to_code(tokens, options);
//                 }
//             }

//             if let Some(default_expr) = default {
//                 tokens.push(Token::Operator(Operator::Set));
//                 default_expr.to_code(tokens, options);
//             }
//         }
//         tokens.push(Token::Punctuation(Punctuation::RightParenthesis));

//         if let Some(return_type) = self.return_type() {
//             if **return_type != Type::InferType {
//                 tokens.push(Token::Punctuation(Punctuation::Colon));
//                 return_type.to_code(tokens, options);
//             }
//         }

//         if let Some(definition) = self.definition() {
//             definition.to_code(tokens, options);
//         }
//     }
// }

// impl<'a> ToCode<'a> for Variable<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         match self.kind() {
//             VariableKind::Let => tokens.push(Token::Keyword(Keyword::Let)),
//             VariableKind::Var => tokens.push(Token::Keyword(Keyword::Var)),
//         }

//         tokens.push(Token::Identifier(Identifier::new(self.name())));

//         if let Some(var_type) = self.get_type() {
//             tokens.push(Token::Punctuation(Punctuation::Colon));
//             var_type.to_code(tokens, options);
//         }

//         if let Some(value) = self.value() {
//             tokens.push(Token::Operator(Operator::Set));
//             value.to_code(tokens, options);
//         }
//     }
// }

// impl<'a> ToCode<'a> for Return<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Keyword(Keyword::Ret));
//         if let Some(value) = self.value() {
//             value.to_code(tokens, options);
//         }
//     }
// }

// impl<'a> ToCode<'a> for TupleType<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Punctuation(Punctuation::LeftBracket));
//         for (i, ty) in self.elements().iter().enumerate() {
//             (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
//             ty.to_code(tokens, options);
//         }
//         tokens.push(Token::Punctuation(Punctuation::RightBracket));
//     }
// }

// impl<'a> ToCode<'a> for ArrayType<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Punctuation(Punctuation::LeftBracket));
//         self.element_ty().to_code(tokens, options);
//         tokens.push(Token::Punctuation(Punctuation::Semicolon));
//         self.count().to_code(tokens, options);
//         tokens.push(Token::Punctuation(Punctuation::RightBracket));
//     }
// }

// impl<'a> ToCode<'a> for StructType<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Keyword(Keyword::Struct));

//         if !self.attributes().is_empty() {
//             tokens.push(Token::Punctuation(Punctuation::LeftBracket));
//             for (i, attr) in self.attributes().iter().enumerate() {
//                 (i != 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
//                 attr.to_code(tokens, options);
//             }
//             tokens.push(Token::Punctuation(Punctuation::RightBracket));
//         }

//         if let Some(name) = self.name() {
//             tokens.push(Token::Identifier(Identifier::new(name)));
//         }

//         tokens.push(Token::Punctuation(Punctuation::LeftBrace));
//         for (field_name, field_ty) in self.fields() {
//             tokens.push(Token::Identifier(Identifier::new(field_name)));
//             tokens.push(Token::Punctuation(Punctuation::Colon));
//             field_ty.to_code(tokens, options);
//             tokens.push(Token::Punctuation(Punctuation::Comma));
//         }
//         tokens.push(Token::Punctuation(Punctuation::RightBrace));
//     }
// }

// impl<'a> ToCode<'a> for FunctionType<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         tokens.push(Token::Keyword(Keyword::Fn));

//         if !self.attributes().is_empty() {
//             tokens.push(Token::Punctuation(Punctuation::LeftBracket));
//             for (i, attr) in self.attributes().iter().enumerate() {
//                 (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));
//                 attr.to_code(tokens, options);
//             }
//             tokens.push(Token::Punctuation(Punctuation::RightBracket));
//         }

//         tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
//         for (i, (name, ty, default)) in self.parameters().iter().enumerate() {
//             (i > 0).then(|| tokens.push(Token::Punctuation(Punctuation::Comma)));

//             tokens.push(Token::Identifier(Identifier::new(name)));

//             if let Some(ty) = ty {
//                 if **ty != Type::InferType {
//                     tokens.push(Token::Punctuation(Punctuation::Colon));
//                     ty.to_code(tokens, options);
//                 }
//             }

//             if let Some(default_expr) = default {
//                 tokens.push(Token::Operator(Operator::Set));
//                 default_expr.to_code(tokens, options);
//             }
//         }
//         tokens.push(Token::Punctuation(Punctuation::RightParenthesis));

//         if let Some(return_type) = self.return_type() {
//             if **return_type != Type::InferType {
//                 tokens.push(Token::Punctuation(Punctuation::Colon));
//                 return_type.to_code(tokens, options);
//             }
//         }
//     }
// }

// impl<'a> ToCode<'a> for Expr<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         if self == &Expr::Discard {
//             return;
//         }

//         // if self.has_parenthesis() {
//         //     tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
//         // }
//         // TODO: Put parenthesis handling back if needed

//         match self {
//             Expr::Bool => tokens.push(Token::Identifier(Identifier::new("bool"))),
//             Expr::UInt8 => tokens.push(Token::Identifier(Identifier::new("u8"))),
//             Expr::UInt16 => tokens.push(Token::Identifier(Identifier::new("u16"))),
//             Expr::UInt32 => tokens.push(Token::Identifier(Identifier::new("u32"))),
//             Expr::UInt64 => tokens.push(Token::Identifier(Identifier::new("u64"))),
//             Expr::UInt128 => tokens.push(Token::Identifier(Identifier::new("u128"))),
//             Expr::Int8 => tokens.push(Token::Identifier(Identifier::new("i8"))),
//             Expr::Int16 => tokens.push(Token::Identifier(Identifier::new("i16"))),
//             Expr::Int32 => tokens.push(Token::Identifier(Identifier::new("i32"))),
//             Expr::Int64 => tokens.push(Token::Identifier(Identifier::new("i64"))),
//             Expr::Int128 => tokens.push(Token::Identifier(Identifier::new("i128"))),
//             Expr::Float8 => tokens.push(Token::Identifier(Identifier::new("f8"))),
//             Expr::Float16 => tokens.push(Token::Identifier(Identifier::new("f16"))),
//             Expr::Float32 => tokens.push(Token::Identifier(Identifier::new("f32"))),
//             Expr::Float64 => tokens.push(Token::Identifier(Identifier::new("f64"))),
//             Expr::Float128 => tokens.push(Token::Identifier(Identifier::new("f128"))),

//             Expr::InferType => tokens.push(Token::Identifier(Identifier::new("_"))),
//             Expr::TupleType(e) => e.to_code(tokens, options),
//             Expr::ArrayType(e) => e.to_code(tokens, options),
//             Expr::StructType(e) => e.to_code(tokens, options),
//             Expr::FunctionType(e) => e.to_code(tokens, options),

//             Expr::Discard => {}

//             Expr::Integer(e) => e.to_code(tokens, options),
//             Expr::Float(e) => e.to_code(tokens, options),
//             Expr::String(e) => e.to_code(tokens, options),
//             Expr::Char(e) => e.to_code(tokens, options),
//             Expr::List(e) => e.to_code(tokens, options),
//             Expr::Object(e) => e.to_code(tokens, options),

//             Expr::UnaryOp(e) => e.to_code(tokens, options),
//             Expr::BinaryOp(e) => e.to_code(tokens, options),
//             Expr::Statement(e) => e.to_code(tokens, options),
//             Expr::Block(e) => e.to_code(tokens, options),

//             Expr::Function(e) => e.to_code(tokens, options),
//             Expr::Variable(e) => e.to_code(tokens, options),

//             Expr::Return(e) => e.to_code(tokens, options),
//         }

//         // if self.has_parenthesis() {
//         //     tokens.push(Token::Punctuation(Punctuation::RightParenthesis));
//         // }
//     }
// }

// impl<'a> ToCode<'a> for Type<'a> {
//     fn to_code(&self, tokens: &mut Vec<Token<'a>>, options: &CodeFormat) {
//         // if self.has_parenthesis() {
//         //     tokens.push(Token::Punctuation(Punctuation::LeftParenthesis));
//         // }
//         // TODO: Put parenthesis handling back if needed

//         match self {
//             Type::Bool => tokens.push(Token::Identifier(Identifier::new("bool"))),
//             Type::UInt8 => tokens.push(Token::Identifier(Identifier::new("u8"))),
//             Type::UInt16 => tokens.push(Token::Identifier(Identifier::new("u16"))),
//             Type::UInt32 => tokens.push(Token::Identifier(Identifier::new("u32"))),
//             Type::UInt64 => tokens.push(Token::Identifier(Identifier::new("u64"))),
//             Type::UInt128 => tokens.push(Token::Identifier(Identifier::new("u128"))),
//             Type::Int8 => tokens.push(Token::Identifier(Identifier::new("i8"))),
//             Type::Int16 => tokens.push(Token::Identifier(Identifier::new("i16"))),
//             Type::Int32 => tokens.push(Token::Identifier(Identifier::new("i32"))),
//             Type::Int64 => tokens.push(Token::Identifier(Identifier::new("i64"))),
//             Type::Int128 => tokens.push(Token::Identifier(Identifier::new("i128"))),
//             Type::Float8 => tokens.push(Token::Identifier(Identifier::new("f8"))),
//             Type::Float16 => tokens.push(Token::Identifier(Identifier::new("f16"))),
//             Type::Float32 => tokens.push(Token::Identifier(Identifier::new("f32"))),
//             Type::Float64 => tokens.push(Token::Identifier(Identifier::new("f64"))),
//             Type::Float128 => tokens.push(Token::Identifier(Identifier::new("f128"))),

//             Type::InferType => tokens.push(Token::Identifier(Identifier::new("_"))),
//             Type::TupleType(e) => e.to_code(tokens, options),
//             Type::ArrayType(e) => e.to_code(tokens, options),
//             Type::StructType(e) => e.to_code(tokens, options),
//             Type::FunctionType(e) => e.to_code(tokens, options),
//         }

//         // if self.has_parenthesis() {
//         //     tokens.push(Token::Punctuation(Punctuation::RightParenthesis));
//         // }
//     }
// }
