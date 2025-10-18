use crate::ast_expr2hir;
use crate::diagnosis::HirErr;
use crate::lower::lower::Ast2Hir;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_get_type::get_type;
use nitrate_parse::Parser;
use nitrate_parsetree::ast::{self as ast, CallArgument, UnaryExprOp};
use nitrate_tokenize::{Lexer, LexerError, escape_string};
use ordered_float::OrderedFloat;
use std::collections::HashMap;
use std::fmt::Write;
use std::ops::Deref;

fn from_nitrate_expression(ctx: &mut HirCtx, nitrate_expr: &str) -> Result<Value, ()> {
    let lexer = match Lexer::new(nitrate_expr.as_bytes(), None) {
        Ok(lexer) => lexer,
        Err(LexerError::SourceTooBig) => return Err(()),
    };

    let trash = CompilerLog::default();
    let mut parser = Parser::new(lexer, &trash);

    let expression = parser.parse_expression();
    if trash.error_bit() {
        return Err(());
    }

    let hir_value = ast_expr2hir(expression, ctx, &trash)?;
    if trash.error_bit() {
        return Err(());
    }

    Ok(hir_value)
}

pub(crate) enum EncodeErr {
    CannotEncodeInferredType,
    UnresolvedSymbol,
}

fn metatype_source_encode(store: &Store, from: &Type, o: &mut dyn Write) -> Result<(), EncodeErr> {
    // FIXME: std::meta::Type transcoding | code is stringy => perform manual testing
    // FIXME: Update implementation in accordance with the ratified definition of std::meta::Type within standard library.

    match from {
        Type::Never => {
            write!(o, "::std::meta::Type::Never").unwrap();
            Ok(())
        }

        Type::Unit => {
            write!(o, "::std::meta::Type::Unit").unwrap();
            Ok(())
        }

        Type::Bool => {
            write!(o, "::std::meta::Type::Bool").unwrap();
            Ok(())
        }

        Type::U8 => {
            write!(o, "::std::meta::Type::U8").unwrap();
            Ok(())
        }

        Type::U16 => {
            write!(o, "::std::meta::Type::U16").unwrap();
            Ok(())
        }

        Type::U32 => {
            write!(o, "::std::meta::Type::U32").unwrap();
            Ok(())
        }

        Type::U64 => {
            write!(o, "::std::meta::Type::U64").unwrap();
            Ok(())
        }

        Type::U128 => {
            write!(o, "::std::meta::Type::U128").unwrap();
            Ok(())
        }

        Type::I8 => {
            write!(o, "::std::meta::Type::I8").unwrap();
            Ok(())
        }

        Type::I16 => {
            write!(o, "::std::meta::Type::I16").unwrap();
            Ok(())
        }

        Type::I32 => {
            write!(o, "::std::meta::Type::I32").unwrap();
            Ok(())
        }

        Type::I64 => {
            write!(o, "::std::meta::Type::I64").unwrap();
            Ok(())
        }

        Type::I128 => {
            write!(o, "::std::meta::Type::I128").unwrap();
            Ok(())
        }

        Type::F32 => {
            write!(o, "::std::meta::Type::F32").unwrap();
            Ok(())
        }

        Type::F64 => {
            write!(o, "::std::meta::Type::F64").unwrap();
            Ok(())
        }

        Type::USize => {
            write!(o, "::std::meta::Type::USize").unwrap();
            Ok(())
        }

        Type::Opaque { name } => {
            write!(
                o,
                "::std::meta::Type::Opaque {{ name: String::from({}) }}",
                escape_string(name, true)
            )
            .unwrap();
            Ok(())
        }

        Type::Array { element_type, len } => {
            write!(o, "::std::meta::Type::Array {{ element_type: ").unwrap();
            metatype_source_encode(store, &store[element_type], o)?;
            write!(o, ", len: {} }}", len).unwrap();
            Ok(())
        }

        Type::Tuple { element_types } => {
            write!(o, "::std::meta::Type::Tuple {{ element_types: Vec::from([").unwrap();
            for elem in element_types {
                metatype_source_encode(store, elem, o)?;
                write!(o, ",").unwrap();
            }
            write!(o, "]) }}").unwrap();
            Ok(())
        }

        Type::Slice { element_type } => {
            write!(o, "::std::meta::Type::Slice {{ element_type: ").unwrap();
            metatype_source_encode(store, &store[element_type], o)?;
            write!(o, " }}").unwrap();
            Ok(())
        }

        Type::Struct { struct_type } => {
            let struct_type = &store[struct_type];
            write!(o, "::std::meta::Type::Struct {{ fields: Vec::from([").unwrap();
            for field in &struct_type.fields {
                write!(
                    o,
                    "::std::meta::StructField {{ name: String::from({}), ty: ",
                    escape_string(&field.0, true)
                )
                .unwrap();
                metatype_source_encode(store, &store[field.1], o)?;
                write!(o, " }},").unwrap();
            }
            write!(o, "]), attributes: Vec::from([").unwrap();
            for attr in &struct_type.attributes {
                match attr {
                    StructAttribute::Packed => {
                        write!(o, "::std::meta::StructAttribute::Packed,").unwrap()
                    }
                };
                write!(o, ",").unwrap();
            }
            write!(o, "]) }}").unwrap();
            Ok(())
        }

        Type::Enum { enum_type } => {
            let enum_type = &store[enum_type];
            write!(o, "::std::meta::Type::Enum {{ variants: Vec::from([").unwrap();
            for variant in &enum_type.variants {
                write!(
                    o,
                    "::std::meta::EnumVariant {{ name: String::from({}), ty: ",
                    escape_string(&variant.0, true)
                )
                .unwrap();
                metatype_source_encode(store, &store[variant.1], o)?;
                write!(o, " }},").unwrap();
            }
            write!(o, "]), attributes: Vec::from([").unwrap();
            for attr in &enum_type.attributes {
                match attr {
                    EnumAttribute::Placeholder => {
                        write!(o, "::std::meta::EnumAttribute::Placeholder,").unwrap()
                    }
                };
                write!(o, ",").unwrap();
            }
            write!(o, "]) }}").unwrap();
            Ok(())
        }

        Type::Refine { base, min, max } => {
            write!(o, "::std::meta::Type::Refine {{ base: ").unwrap();
            metatype_source_encode(store, &store[base], o)?;
            write!(o, ", min: {}, max: {} }}", &store[min], &store[max]).unwrap();
            Ok(())
        }

        Type::Bitfield { base, bits } => {
            write!(o, "::std::meta::Type::Bitfield {{ base: ").unwrap();
            metatype_source_encode(store, &store[base], o)?;
            write!(o, ", bits: {} }}", bits).unwrap();
            Ok(())
        }

        Type::Function { function_type } => {
            let function_type = &store[function_type];
            write!(o, "::std::meta::Type::Function {{ parameters: Vec::from([").unwrap();
            for param_id in &function_type.params {
                let param = &store[param_id].borrow();
                write!(
                    o,
                    "::std::meta::FunctionParameter {{ name: String::from({}) , ty: ",
                    escape_string(&param.name, true)
                )
                .unwrap();
                metatype_source_encode(store, &store[&param.ty], o)?;
                write!(o, " }},").unwrap();
            }
            write!(o, "]), return_type: ").unwrap();
            metatype_source_encode(store, &store[&function_type.return_type], o)?;
            write!(o, ", attributes: Vec::from([").unwrap();
            for attr in &function_type.attributes {
                match attr {
                    FunctionAttribute::Variadic => {
                        write!(o, "::std::meta::FunctionAttribute::Variadic,").unwrap()
                    }
                };
                write!(o, ",").unwrap();
            }
            write!(o, "]) }}").unwrap();
            Ok(())
        }

        Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
        } => {
            write!(o, "::std::meta::Type::Reference {{ lifetime: ").unwrap();
            match lifetime {
                Lifetime::Static => write!(o, "::std::meta::Lifetime::Static").unwrap(),
                Lifetime::Gc => write!(o, "::std::meta::Lifetime::Gc").unwrap(),
                Lifetime::ThreadLocal => write!(o, "::std::meta::Lifetime::ThreadLocal").unwrap(),
                Lifetime::TaskLocal => write!(o, "::std::meta::Lifetime::TaskLocal").unwrap(),
                Lifetime::Inferred => write!(o, "::std::meta::Lifetime::Inferred").unwrap(),
            };
            write!(o, ", exclusive: {}, mutable: {}, to: ", exclusive, mutable).unwrap();
            metatype_source_encode(store, &store[to], o)?;
            write!(o, " }}").unwrap();
            Ok(())
        }

        Type::Pointer {
            exclusive,
            mutable,
            to,
        } => {
            write!(
                o,
                "::std::meta::Type::Pointer {{ exclusive: {}, mutable: {}, to: ",
                exclusive, mutable
            )
            .unwrap();
            metatype_source_encode(store, &store[to], o)?;
            write!(o, " }}").unwrap();
            Ok(())
        }

        Type::Symbol { name: _, link } => match link {
            Some(type_id) => metatype_source_encode(store, &store[type_id], o),
            None => Err(EncodeErr::UnresolvedSymbol),
        },

        Type::InferredFloat => Err(EncodeErr::CannotEncodeInferredType),
        Type::InferredInteger => Err(EncodeErr::CannotEncodeInferredType),
        Type::Inferred { id: _ } => Err(EncodeErr::CannotEncodeInferredType),
    }
}

fn metatype_encode(ctx: &mut HirCtx, from: Type) -> Result<Value, EncodeErr> {
    let mut repr = String::new();
    metatype_source_encode(ctx.store(), &from, &mut repr)?;

    let hir_meta_object = from_nitrate_expression(ctx, &repr)
        .expect("failed to lower auto-generated std::meta::Type expression");

    Ok(hir_meta_object)
}

impl Ast2Hir for ast::ExprSyntaxError {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Err(())
    }
}

impl Ast2Hir for ast::ExprParentheses {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        self.inner.ast2hir(ctx, log)
    }
}

impl Ast2Hir for ast::BooleanLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        match self.value {
            true => Ok(Value::Bool(true)),
            false => Ok(Value::Bool(false)),
        }
    }
}

impl Ast2Hir for ast::IntegerLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::InferredInteger(Box::new(self.value)))
    }
}

impl Ast2Hir for ast::FloatLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::InferredFloat(OrderedFloat::from(*self.value)))
    }
}

impl Ast2Hir for ast::StringLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::StringLit(self.value.to_string().into()))
    }
}

impl Ast2Hir for ast::BStringLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::BStringLit(self.value.into()))
    }
}

impl Ast2Hir for ast::TypeInfo {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let hir_type = self.the.ast2hir(ctx, log)?;
        let encoded = match metatype_encode(ctx, hir_type) {
            Ok(v) => v,
            Err(_) => {
                log.report(&HirErr::TypeInferenceError);
                return Err(());
            }
        };

        Ok(encoded)
    }
}

impl Ast2Hir for ast::List {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let mut elements = Vec::with_capacity(self.elements.len());
        for element in self.elements {
            let hir_element = element.ast2hir(ctx, log)?;
            elements.push(hir_element);
        }

        Ok(Value::List {
            elements: elements.into(),
        })
    }
}

impl Ast2Hir for ast::Tuple {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let mut elements = Vec::with_capacity(self.elements.len());
        for element in self.elements {
            let hir_element = element.ast2hir(ctx, log)?;
            elements.push(hir_element);
        }

        Ok(Value::Tuple {
            elements: elements.into(),
        })
    }
}

impl Ast2Hir for ast::StructInit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::StructInit to HIR
        log.report(&HirErr::UnimplementedFeature(
            "ast::Expr::StructInit".into(),
        ));
        Err(())
    }
}

impl Ast2Hir for ast::UnaryExpr {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let operand = self.operand.ast2hir(ctx, log)?;

        match self.operator {
            UnaryExprOp::Add => Ok(Value::Unary {
                op: UnaryOp::Add,
                operand: operand.into_id(ctx.store()),
            }),

            UnaryExprOp::Sub => Ok(Value::Unary {
                op: UnaryOp::Sub,
                operand: operand.into_id(ctx.store()),
            }),

            UnaryExprOp::LogicNot => Ok(Value::Unary {
                op: UnaryOp::LogicNot,
                operand: operand.into_id(ctx.store()),
            }),

            UnaryExprOp::BitNot => Ok(Value::Unary {
                op: UnaryOp::BitNot,
                operand: operand.into_id(ctx.store()),
            }),

            UnaryExprOp::Deref => Ok(Value::Deref {
                place: operand.into_id(ctx.store()),
            }),

            UnaryExprOp::Borrow => Ok(Value::Borrow {
                mutable: false,
                place: operand.into_id(ctx.store()),
            }),

            UnaryExprOp::Typeof => match get_type(&operand, ctx.store()) {
                Ok(t) => {
                    let encoded = match metatype_encode(ctx, t) {
                        Ok(v) => v,
                        Err(_) => {
                            log.report(&HirErr::TypeInferenceError);
                            return Err(());
                        }
                    };

                    Ok(encoded)
                }
                Err(_) => {
                    log.report(&HirErr::TypeInferenceError);
                    Err(())
                }
            },
        }
    }
}

impl Ast2Hir for ast::BinExpr {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let left = self.left.ast2hir(ctx, log)?.into_id(ctx.store());
        let right = self.right.ast2hir(ctx, log)?.into_id(ctx.store());

        match self.operator {
            ast::BinExprOp::Add => Ok(Value::Binary {
                left,
                op: BinaryOp::Add,
                right,
            }),

            ast::BinExprOp::Sub => Ok(Value::Binary {
                left,
                op: BinaryOp::Sub,
                right,
            }),

            ast::BinExprOp::Mul => Ok(Value::Binary {
                left,
                op: BinaryOp::Mul,
                right,
            }),

            ast::BinExprOp::Div => Ok(Value::Binary {
                left,
                op: BinaryOp::Div,
                right,
            }),

            ast::BinExprOp::Mod => Ok(Value::Binary {
                left,
                op: BinaryOp::Mod,
                right,
            }),

            ast::BinExprOp::BitAnd => Ok(Value::Binary {
                left,
                op: BinaryOp::And,
                right,
            }),

            ast::BinExprOp::BitOr => Ok(Value::Binary {
                left,
                op: BinaryOp::Or,
                right,
            }),

            ast::BinExprOp::BitXor => Ok(Value::Binary {
                left,
                op: BinaryOp::Xor,
                right,
            }),

            ast::BinExprOp::BitShl => Ok(Value::Binary {
                left,
                op: BinaryOp::Shl,
                right,
            }),

            ast::BinExprOp::BitShr => Ok(Value::Binary {
                left,
                op: BinaryOp::Shr,
                right,
            }),

            ast::BinExprOp::BitRol => Ok(Value::Binary {
                left,
                op: BinaryOp::Rol,
                right,
            }),

            ast::BinExprOp::BitRor => Ok(Value::Binary {
                left,
                op: BinaryOp::Ror,
                right,
            }),

            ast::BinExprOp::LogicAnd => Ok(Value::Binary {
                left,
                op: BinaryOp::LogicAnd,
                right,
            }),

            ast::BinExprOp::LogicOr => Ok(Value::Binary {
                left,
                op: BinaryOp::LogicOr,
                right,
            }),

            ast::BinExprOp::LogicLt => Ok(Value::Binary {
                left,
                op: BinaryOp::Lt,
                right,
            }),

            ast::BinExprOp::LogicGt => Ok(Value::Binary {
                left,
                op: BinaryOp::Gt,
                right,
            }),

            ast::BinExprOp::LogicLe => Ok(Value::Binary {
                left,
                op: BinaryOp::Lte,
                right,
            }),

            ast::BinExprOp::LogicGe => Ok(Value::Binary {
                left,
                op: BinaryOp::Gte,
                right,
            }),

            ast::BinExprOp::LogicEq => Ok(Value::Binary {
                left,
                op: BinaryOp::Eq,
                right,
            }),

            ast::BinExprOp::LogicNe => Ok(Value::Binary {
                left,
                op: BinaryOp::Ne,
                right,
            }),

            ast::BinExprOp::Set => Ok(Value::Assign {
                place: left,
                value: right,
            }),

            ast::BinExprOp::SetPlus => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Add,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetMinus => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Sub,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetTimes => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Mul,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetSlash => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Div,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetPercent => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Mod,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitAnd => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::And,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitOr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Or,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitXor => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Xor,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitShl => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Shl,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitShr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Shr,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitRotl => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Rol,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetBitRotr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Ror,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetLogicAnd => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::And,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::SetLogicOr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Or,
                    right,
                }
                .into_id(ctx.store()),
            }),

            ast::BinExprOp::Dot => {
                // TODO: lower field access to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "field access with . operator".into(),
                ));
                Err(())
            }

            ast::BinExprOp::Arrow => {
                // TODO: lower field access to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "method call with -> operator".into(),
                ));
                Err(())
            }

            ast::BinExprOp::Range => {
                // TODO: lower range to HIR
                log.report(&HirErr::UnimplementedFeature(
                    "range with .. operator".into(),
                ));
                Err(())
            }
        }
    }
}

impl Ast2Hir for ast::Cast {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        fn failed_to_cast(log: &CompilerLog) -> Result<Value, ()> {
            log.report(&HirErr::IntegerCastOutOfRange);
            Err(())
        }

        let expr = self.value.ast2hir(ctx, log)?;
        let to = self.to.ast2hir(ctx, log)?;

        match (expr, to) {
            (Value::InferredInteger(value), Type::U8) => match u8::try_from(*value) {
                Ok(v) => Ok(Value::U8(v)),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::U16) => match u16::try_from(*value) {
                Ok(v) => Ok(Value::U16(v)),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::U32) => match u32::try_from(*value) {
                Ok(v) => Ok(Value::U32(v)),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::U64) => match u64::try_from(*value) {
                Ok(v) => Ok(Value::U64(v)),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::U128) => match u128::try_from(*value) {
                Ok(v) => Ok(Value::U128(Box::new(v))),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::I8) => match i8::try_from(*value) {
                Ok(v) => Ok(Value::I8(v)),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::I16) => match i16::try_from(*value) {
                Ok(v) => Ok(Value::I16(v)),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::I32) => match i32::try_from(*value) {
                Ok(v) => Ok(Value::I32(v)),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::I64) => match i64::try_from(*value) {
                Ok(v) => Ok(Value::I64(v)),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredInteger(value), Type::I128) => match i128::try_from(*value) {
                Ok(v) => Ok(Value::I128(Box::new(v))),
                Err(_) => failed_to_cast(log),
            },

            (Value::InferredFloat(v), Type::F32) => Ok(Value::F32(OrderedFloat::from(*v as f32))),
            (Value::InferredFloat(v), Type::F64) => Ok(Value::F64(OrderedFloat::from(*v as f64))),

            (expr, to) => Ok(Value::Cast {
                expr: expr.into_id(ctx.store()),
                to: to.into_id(ctx.store()),
            }),
        }
    }
}

impl Ast2Hir for ast::Block {
    type Hir = Block;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let mut elements = Vec::with_capacity(self.elements.len());
        let mut ends_with_unit = false;

        for element in self.elements {
            match element {
                ast::BlockItem::Expr(e) => {
                    let hir_element = e.ast2hir(ctx, log)?;
                    elements.push(hir_element);
                    ends_with_unit = false;
                }

                ast::BlockItem::Stmt(s) => {
                    let hir_element = s.ast2hir(ctx, log)?;
                    elements.push(hir_element);
                    ends_with_unit = true;
                }

                ast::BlockItem::Variable(_v) => {
                    // TODO: Handle variable declarations properly
                    log.report(&HirErr::UnimplementedFeature(
                        "variable declaration in block".into(),
                    ));

                    ends_with_unit = true;
                }
            }
        }

        if ends_with_unit {
            elements.push(Value::Unit);
        }

        let safety = match self.safety {
            Some(ast::Safety::Unsafe(None)) => BlockSafety::Unsafe,
            Some(ast::Safety::Safe) | None => BlockSafety::Safe,

            Some(ast::Safety::Unsafe(Some(_))) => {
                log.report(&HirErr::UnimplementedFeature(
                    "block safety unsafe expression".into(),
                ));
                return Err(());
            }
        };

        Ok(Block { safety, elements })
    }
}

impl Ast2Hir for ast::Closure {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Closure to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Closure".into()));
        Err(())
    }
}

impl Ast2Hir for ast::ExprPath {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::ExprPath to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Path".into()));
        Err(())
    }
}

impl Ast2Hir for ast::IndexAccess {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let collection = self.collection.ast2hir(ctx, log)?.into_id(ctx.store());
        let index = self.index.ast2hir(ctx, log)?.into_id(ctx.store());
        Ok(Value::IndexAccess { collection, index })
    }
}

impl Ast2Hir for ast::If {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let condition = self.condition.ast2hir(ctx, log)?.into_id(ctx.store());

        let true_branch = self.true_branch.ast2hir(ctx, log)?.into_id(ctx.store());

        let false_branch = match self.false_branch {
            Some(ast::ElseIf::If(else_if)) => {
                let else_if_value = else_if.ast2hir(ctx, log)?;
                let block = Block {
                    safety: BlockSafety::Safe,
                    elements: vec![else_if_value],
                }
                .into_id(ctx.store());
                Some(block)
            }
            Some(ast::ElseIf::Block(block)) => {
                let block = block.ast2hir(ctx, log)?.into_id(ctx.store());
                Some(block)
            }
            None => None,
        };

        Ok(Value::If {
            condition,
            true_branch,
            false_branch,
        })
    }
}

impl Ast2Hir for ast::WhileLoop {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let condition = match self.condition {
            Some(cond) => cond.ast2hir(_ctx, log)?.into_id(_ctx.store()),
            None => Value::Bool(true).into_id(_ctx.store()),
        };

        let body = self.body.ast2hir(_ctx, log)?.into_id(_ctx.store());

        Ok(Value::While { condition, body })
    }
}

impl Ast2Hir for ast::Match {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Match to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Match".into()));
        Err(())
    }
}

impl Ast2Hir for ast::Break {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::Break {
            label: self.label.map(|l| l.to_string().into()),
        })
    }
}

impl Ast2Hir for ast::Continue {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::Continue {
            label: self.label.map(|l| l.to_string().into()),
        })
    }
}

impl Ast2Hir for ast::Return {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let value = match self.value {
            Some(v) => v.ast2hir(_ctx, log)?.into_id(_ctx.store()),
            None => Value::Unit.into_id(_ctx.store()),
        };

        Ok(Value::Return { value })
    }
}

impl Ast2Hir for ast::ForEach {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::ForEach to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::erFor".into()));
        Err(())
    }
}

impl Ast2Hir for ast::Await {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Await to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Await".into()));
        Err(())
    }
}

fn construct_call(
    ctx: &mut HirCtx,
    log: &CompilerLog,
    function_type: &FunctionType,
    arguments: Vec<CallArgument>,
) -> Result<Vec<ValueId>, ()> {
    fn find_first_hole(arguments: &Vec<Option<ValueId>>) -> usize {
        if let Some((index, _)) = arguments.iter().enumerate().find(|x| x.1.is_none()) {
            return index;
        }

        // Must be a variadic argument
        arguments.len()
    }

    fn place_argument(
        position: usize,
        value: ValueId,
        log: &CompilerLog,
        call_arguments: &mut Vec<Option<ValueId>>,
    ) -> Result<(), ()> {
        // Push variadic argument
        if position == call_arguments.len() {
            call_arguments.push(Some(value));
            return Ok(());
        }

        if let Some(_) = call_arguments[position] {
            log.report(&HirErr::DuplicateFunctionArguments);
            return Err(());
        }

        call_arguments[position] = Some(value);
        Ok(())
    }

    let mut name_to_pos = HashMap::new();
    for (index, param) in function_type.params.iter().enumerate() {
        let param = ctx[param].borrow();
        name_to_pos.insert(param.name.to_string(), index);
    }

    let mut next_pos = 0;
    let mut call_arguments: Vec<Option<ValueId>> = Vec::new();
    call_arguments.resize(function_type.params.len(), None);

    for CallArgument { name, value } in arguments {
        match name {
            Some(name) => {
                if let Some(position) = name_to_pos.get(name.deref()) {
                    let value = value.ast2hir(ctx, log)?.into_id(ctx.store());
                    place_argument(position.to_owned(), value, log, &mut call_arguments)?;
                    next_pos = find_first_hole(&call_arguments);
                } else {
                    log.report(&HirErr::NoSuchParameter(name.to_string()));
                    return Err(());
                }
            }

            None => {
                let value = value.ast2hir(ctx, log)?.into_id(ctx.store());
                place_argument(next_pos, value, log, &mut call_arguments)?;
                next_pos = find_first_hole(&call_arguments);
            }
        };
    }

    for (i, arg) in call_arguments.iter_mut().enumerate() {
        if arg.is_none() {
            if let Some(parameter) = function_type.params.get(i) {
                let parameter = ctx[parameter].borrow();
                if let Some(default_value) = &parameter.default_value {
                    if arg.is_none() {
                        *arg = Some(default_value.clone());
                        continue;
                    }
                }
            }

            log.report(&HirErr::MissingFunctionArguments);
            return Err(());
        }
    }

    if !function_type
        .attributes
        .contains(&FunctionAttribute::Variadic)
    {
        if call_arguments.len() > function_type.params.len() {
            log.report(&HirErr::TooManyFunctionArguments);
            return Err(());
        }
    }

    Ok(call_arguments
        .into_iter()
        .map(|v| v.unwrap())
        .collect::<Vec<ValueId>>())
}

impl Ast2Hir for ast::FunctionCall {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let callee = self.callee.ast2hir(ctx, log)?;
        let callee_type = get_type(&callee, ctx.store()).map_err(|_| {
            log.report(&HirErr::TypeInferenceError);
        })?;

        if !callee_type.is_function() {
            log.report(&HirErr::CalleeIsNotFunctionType);
            return Err(());
        }

        let function_type = match callee_type {
            Type::Function { function_type } => ctx[&function_type].to_owned(),
            _ => unreachable!(),
        };

        let call_arguments = construct_call(ctx, log, &function_type, self.arguments)?;

        Ok(Value::Call {
            callee: callee.into_id(ctx.store()),
            arguments: call_arguments.into(),
        })
    }
}

impl Ast2Hir for ast::MethodCall {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let object = self.object.ast2hir(ctx, log)?;
        let _object_type = get_type(&object, ctx.store()).map_err(|_| {
            log.report(&HirErr::TypeInferenceError);
        })?;

        todo!()

        // let call_arguments = construct_call(ctx, log, &function_type, self.arguments)?;

        // Ok(Value::Call {
        //     callee: callee.into_id(ctx.store()),
        //     arguments: call_arguments.into(),
        // })
    }
}

impl Ast2Hir for ast::Expr {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        match self {
            ast::Expr::SyntaxError(e) => e.ast2hir(ctx, log),
            ast::Expr::Parentheses(e) => e.ast2hir(ctx, log),
            ast::Expr::Boolean(e) => e.ast2hir(ctx, log),
            ast::Expr::Integer(e) => e.ast2hir(ctx, log),
            ast::Expr::Float(e) => e.ast2hir(ctx, log),
            ast::Expr::String(e) => e.ast2hir(ctx, log),
            ast::Expr::BString(e) => e.ast2hir(ctx, log),
            ast::Expr::TypeInfo(e) => e.ast2hir(ctx, log),
            ast::Expr::List(e) => e.ast2hir(ctx, log),
            ast::Expr::Tuple(e) => e.ast2hir(ctx, log),
            ast::Expr::StructInit(e) => e.ast2hir(ctx, log),
            ast::Expr::UnaryExpr(e) => e.ast2hir(ctx, log),
            ast::Expr::BinExpr(e) => e.ast2hir(ctx, log),
            ast::Expr::Cast(e) => e.ast2hir(ctx, log),
            ast::Expr::Block(e) => Ok(Value::Block {
                block: e.ast2hir(ctx, log)?.into_id(ctx.store()),
            }),
            ast::Expr::Closure(e) => e.ast2hir(ctx, log),
            ast::Expr::Path(e) => e.ast2hir(ctx, log),
            ast::Expr::IndexAccess(e) => e.ast2hir(ctx, log),
            ast::Expr::If(e) => e.ast2hir(ctx, log),
            ast::Expr::While(e) => e.ast2hir(ctx, log),
            ast::Expr::Match(e) => e.ast2hir(ctx, log),
            ast::Expr::Break(e) => e.ast2hir(ctx, log),
            ast::Expr::Continue(e) => e.ast2hir(ctx, log),
            ast::Expr::Return(e) => e.ast2hir(ctx, log),
            ast::Expr::For(e) => e.ast2hir(ctx, log),
            ast::Expr::Await(e) => e.ast2hir(ctx, log),
            ast::Expr::FunctionCall(e) => e.ast2hir(ctx, log),
            ast::Expr::MethodCall(e) => e.ast2hir(ctx, log),
        }
    }
}
