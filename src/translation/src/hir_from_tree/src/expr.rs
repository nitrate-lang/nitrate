use crate::context::Ast2HirCtx;
use crate::diagnosis::HirErr;
use crate::lower::Ast2Hir;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{SymbolTab, prelude::*};
use nitrate_hir_get_type::HirGetType;
use nitrate_nstring::NString;
use nitrate_token::escape_string;
use nitrate_token_lexer::{Lexer, LexerError};
use nitrate_tree::ast::{self as ast, SymbolKind, UnaryExprOp};
use nitrate_tree_parse::Parser;
use ordered_float::OrderedFloat;
use std::collections::BTreeSet;

pub(crate) enum EncodeErr {}

fn metatype_encode(_ctx: &mut Ast2HirCtx, _from: Type) -> Result<Value, EncodeErr> {
    // TODO: Serialize the type metaprogrammatically

    unimplemented!()
}

impl Ast2Hir for ast::ExprSyntaxError {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Err(())
    }
}

impl Ast2Hir for ast::ExprParentheses {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        self.inner.ast2hir(ctx, log)
    }
}

impl Ast2Hir for ast::BooleanLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        match self.value {
            true => Ok(Value::Bool(true)),
            false => Ok(Value::Bool(false)),
        }
    }
}

impl Ast2Hir for ast::IntegerLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::InferredInteger(Box::new(self.value)))
    }
}

impl Ast2Hir for ast::FloatLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::InferredFloat(OrderedFloat::from(*self.value)))
    }
}

impl Ast2Hir for ast::StringLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::StringLit(self.value.to_string().into()))
    }
}

impl Ast2Hir for ast::BStringLit {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::BStringLit(self.value.into()))
    }
}

impl Ast2Hir for ast::TypeInfo {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
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

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
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

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
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

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        if self
            .path
            .segments
            .iter()
            .any(|seg| seg.type_arguments.is_some())
        {
            // TODO: Support generic type arguments
            log.report(&HirErr::UnimplementedFeature(
                "generic type arguments in type paths".into(),
            ));
        }

        let mut fields = Vec::with_capacity(self.fields.len());
        for field in self.fields {
            let field_name = NString::from(field.0.to_string());
            let field_value = field.1.ast2hir(ctx, log)?.into_id(&ctx.store);

            fields.push((field_name, field_value));
        }

        if let Some(resolved_path) = self.path.resolved_path {
            return Ok(Value::StructObject {
                struct_path: NString::from(resolved_path),
                fields: fields.into(),
            });
        }

        log.report(&HirErr::UnresolvedTypePath);
        Err(())
    }
}

impl Ast2Hir for ast::UnaryExpr {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let operand = self.operand.ast2hir(ctx, log)?;

        match self.operator {
            UnaryExprOp::Add => Ok(Value::Unary {
                op: UnaryOp::Add,
                operand: operand.into_id(&ctx.store),
            }),

            UnaryExprOp::Sub => Ok(Value::Unary {
                op: UnaryOp::Sub,
                operand: operand.into_id(&ctx.store),
            }),

            UnaryExprOp::Not => Ok(Value::Unary {
                op: UnaryOp::Not,
                operand: operand.into_id(&ctx.store),
            }),

            UnaryExprOp::Deref => Ok(Value::Deref {
                place: operand.into_id(&ctx.store),
            }),

            UnaryExprOp::Borrow => Ok(Value::Borrow {
                exclusive: false,
                mutable: false,
                place: operand.into_id(&ctx.store),
            }),

            UnaryExprOp::Typeof => match operand.get_type(&ctx.store, &ctx.tab) {
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

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let left = self.left.ast2hir(ctx, log)?.into_id(&ctx.store);
        let right = self.right.ast2hir(ctx, log)?.into_id(&ctx.store);

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
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetMinus => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Sub,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetTimes => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Mul,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetSlash => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Div,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetPercent => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Mod,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetBitAnd => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::And,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetBitOr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Or,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetBitXor => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Xor,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetBitShl => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Shl,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetBitShr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Shr,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetBitRotl => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Rol,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetBitRotr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Ror,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetLogicAnd => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::And,
                    right,
                }
                .into_id(&ctx.store),
            }),

            ast::BinExprOp::SetLogicOr => Ok(Value::Assign {
                place: left.clone(),
                value: Value::Binary {
                    left,
                    op: BinaryOp::Or,
                    right,
                }
                .into_id(&ctx.store),
            }),

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

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
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

            (Value::InferredInteger(value), Type::USize) => match ctx.ptr_size {
                PtrSize::U32 => match u32::try_from(*value) {
                    Ok(v) => Ok(Value::USize32(v)),
                    Err(_) => failed_to_cast(log),
                },

                PtrSize::U64 => match u64::try_from(*value) {
                    Ok(v) => Ok(Value::USize64(v)),
                    Err(_) => failed_to_cast(log),
                },
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
                value: expr.into_id(&ctx.store),
                target_type: to.into_id(&ctx.store),
            }),
        }
    }
}

fn ast_localvar2hir(
    var: &ast::LocalVariable,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<LocalVariableId, ()> {
    let kind = match var.kind {
        ast::LocalVariableKind::Let => LocalVariableKind::Stack,
        ast::LocalVariableKind::Var => LocalVariableKind::Dynamic,
    };

    let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &var.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedLocalVariableAttribute);
        }
    }

    let is_mutable = match var.mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    };

    let name = ctx.qualify_name(&var.name).into();

    let ty = match var.ty.to_owned() {
        None => ctx.create_inference_placeholder().into_id(&ctx.store),
        Some(t) => {
            let ty_hir = t.ast2hir(ctx, log)?.into_id(&ctx.store);
            ty_hir
        }
    };

    let initializer = match var.initializer.to_owned() {
        Some(expr) => Some(expr.ast2hir(ctx, log)?.into_id(&ctx.store)),
        None => None,
    };

    let localvar_id = LocalVariable {
        kind,
        attributes,
        is_mutable,
        name,
        ty,
        init: initializer,
    }
    .into_id(&ctx.store);

    let symbol = SymbolId::LocalVariable(localvar_id.clone());
    ctx.tab.add_symbol(symbol, &ctx.store);

    Ok(localvar_id)
}

impl Ast2Hir for ast::Block {
    type Hir = Block;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let elements_len = self.elements.len();
        let mut elements = Vec::with_capacity(elements_len);

        for (i, element) in self.elements.into_iter().enumerate() {
            match element {
                ast::BlockItem::Expr(e) => {
                    let hir_element = e.ast2hir(ctx, log)?.into_id(&ctx.store);
                    elements.push(BlockElement::Expr(hir_element));
                }

                ast::BlockItem::Stmt(s) => {
                    let hir_element = s.ast2hir(ctx, log)?.into_id(&ctx.store);
                    elements.push(BlockElement::Expr(hir_element));
                    if i == elements_len - 1 {
                        elements.push(BlockElement::Expr(Value::Unit.into_id(&ctx.store)));
                    }
                }

                ast::BlockItem::Variable(var) => {
                    let var_hir = ast_localvar2hir(&var, ctx, log)?;
                    elements.push(BlockElement::Local(var_hir));
                }
            }
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

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Closure to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Closure".into()));
        Err(())
    }
}

impl Ast2Hir for ast::ExprPath {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        if self.segments.iter().any(|seg| seg.type_arguments.is_some()) {
            // TODO: Support generic type arguments
            log.report(&HirErr::UnimplementedFeature(
                "generic type arguments in expr paths".into(),
            ));
        }

        match self.resolved_path {
            Some(resolved_path) => match ctx.ast_symbol_map.get(&resolved_path) {
                Some(SymbolKind::Function) => match ctx.tab.get_function(&resolved_path) {
                    Some(existing_function_id) => {
                        println!("Resolved function path: {}", resolved_path);
                        return Ok(Value::FunctionSymbol {
                            id: existing_function_id.clone(),
                        });
                    }

                    None => {
                        // TODO: Create function placeholder
                        unimplemented!()
                    }
                },

                Some(SymbolKind::GlobalVariable) => {
                    match ctx.tab.get_global_variable(&resolved_path) {
                        Some(existing_variable_id) => {
                            return Ok(Value::GlobalVariableSymbol {
                                id: existing_variable_id.clone(),
                            });
                        }

                        None => {
                            // TODO: Create global variable placeholder
                            unimplemented!()
                        }
                    }
                }

                Some(SymbolKind::LocalVariable) => {
                    match ctx.tab.get_local_variable(&resolved_path) {
                        Some(existing_local_variable_id) => {
                            return Ok(Value::LocalVariableSymbol {
                                id: existing_local_variable_id.clone(),
                            });
                        }

                        None => {
                            // TODO: Create local variable placeholder
                            unimplemented!()
                        }
                    }
                }

                Some(SymbolKind::Parameter) => {
                    match ctx.tab.get_parameter(&resolved_path) {
                        Some(existing_parameter_id) => {
                            return Ok(Value::ParameterSymbol {
                                id: existing_parameter_id.clone(),
                            });
                        }

                        None => {
                            // TODO: Create parameter placeholder
                            unimplemented!()
                        }
                    }
                }

                _ => {
                    println!("Unresolved symbol: {}", resolved_path);
                    log.report(&HirErr::UnresolvedSymbol);
                    Err(())
                }
            },

            None => {
                println!("Unresolved path in expr: {:?}", self.segments);
                log.report(&HirErr::UnresolvedSymbol);
                Err(())
            }
        }
    }
}

impl Ast2Hir for ast::IndexAccess {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let collection = self.collection.ast2hir(ctx, log)?.into_id(&ctx.store);
        let index = self.index.ast2hir(ctx, log)?.into_id(&ctx.store);
        Ok(Value::IndexAccess { collection, index })
    }
}

impl Ast2Hir for ast::FieldAccess {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let object = self.object.ast2hir(ctx, log)?.into_id(&ctx.store);
        let field = self.field.to_string().into();

        Ok(Value::FieldAccess {
            expr: object,
            field_name: field,
        })
    }
}

impl Ast2Hir for ast::If {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let condition = self.condition.ast2hir(ctx, log)?.into_id(&ctx.store);

        let true_branch = self.true_branch.ast2hir(ctx, log)?.into_id(&ctx.store);

        let false_branch = match self.false_branch {
            Some(ast::ElseIf::If(else_if)) => {
                let else_if_value = else_if.ast2hir(ctx, log)?;
                let block = Block {
                    safety: BlockSafety::Safe,
                    elements: vec![BlockElement::Expr(else_if_value.into_id(&ctx.store))],
                }
                .into_id(&ctx.store);
                Some(block)
            }
            Some(ast::ElseIf::Block(block)) => {
                let block = block.ast2hir(ctx, log)?.into_id(&ctx.store);
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

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let condition = match self.condition {
            Some(cond) => cond.ast2hir(ctx, log)?.into_id(&ctx.store),
            None => Value::Bool(true).into_id(&ctx.store),
        };

        let body = self.body.ast2hir(ctx, log)?.into_id(&ctx.store);

        Ok(Value::While { condition, body })
    }
}

impl Ast2Hir for ast::Match {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Match to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Match".into()));
        Err(())
    }
}

impl Ast2Hir for ast::Break {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::Break {
            label: self.label.map(|l| l.to_string().into()),
        })
    }
}

impl Ast2Hir for ast::Continue {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, _log: &CompilerLog) -> Result<Self::Hir, ()> {
        Ok(Value::Continue {
            label: self.label.map(|l| l.to_string().into()),
        })
    }
}

impl Ast2Hir for ast::Return {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let value = match self.value {
            Some(v) => v.ast2hir(ctx, log)?.into_id(&ctx.store),
            None => Value::Unit.into_id(&ctx.store),
        };

        Ok(Value::Return { value })
    }
}

impl Ast2Hir for ast::ForEach {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::ForEach to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::erFor".into()));
        Err(())
    }
}

impl Ast2Hir for ast::Await {
    type Hir = Value;

    fn ast2hir(self, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        // TODO: lower ast::Await to HIR
        log.report(&HirErr::UnimplementedFeature("ast::Expr::Await".into()));
        Err(())
    }
}

impl Ast2Hir for ast::FunctionCall {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let callee = self.callee.ast2hir(ctx, log)?;

        let mut positional = Vec::with_capacity(self.positional.len());
        let mut named = Vec::with_capacity(self.named.len());

        for arg in self.positional {
            let value = arg.ast2hir(ctx, log)?.into_id(&ctx.store);
            positional.push(value);
        }

        for (name, arg) in self.named {
            let name = NString::from(name.to_string());
            let value = arg.ast2hir(ctx, log)?.into_id(&ctx.store);
            named.push((name, value));
        }

        Ok(Value::Call {
            callee: callee.into_id(&ctx.store),
            positional: positional.into(),
            named: named.into(),
        })
    }
}

impl Ast2Hir for ast::MethodCall {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let object = self.object.ast2hir(ctx, log)?.into_id(&ctx.store);
        let method = NString::from(self.method_name);

        let mut positional = Vec::with_capacity(self.positional.len());
        let mut named = Vec::with_capacity(self.named.len());

        for arg in self.positional {
            let value = arg.ast2hir(ctx, log)?.into_id(&ctx.store);
            positional.push(value);
        }

        for (name, arg) in self.named {
            let name = NString::from(name.to_string());
            let value = arg.ast2hir(ctx, log)?.into_id(&ctx.store);
            named.push((name, value));
        }

        Ok(Value::MethodCall {
            object,
            method_name: method,
            positional: positional.into(),
            named: named.into(),
        })
    }
}

impl Ast2Hir for ast::Expr {
    type Hir = Value;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
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
                block: e.ast2hir(ctx, log)?.into_id(&ctx.store),
            }),
            ast::Expr::Closure(e) => e.ast2hir(ctx, log),
            ast::Expr::Path(e) => e.ast2hir(ctx, log),
            ast::Expr::IndexAccess(e) => e.ast2hir(ctx, log),
            ast::Expr::FieldAccess(e) => e.ast2hir(ctx, log),
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
