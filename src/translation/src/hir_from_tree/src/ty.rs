use crate::{context::Ast2HirCtx, diagnosis::HirErr, expr::lower_expr, helpers};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_evaluate::Evaluator;
use nitrate_token::IntegerKind;
use nitrate_tree::ByteSpan;
use nitrate_tree::ast::{self as ast, SymbolKind};
use std::collections::BTreeSet;
use std::ops::Deref;

// ═══════════════════════════════════════════════════════════════════════════
// Primitive Type Lowering (used by `lower_type` dispatch)
// ═══════════════════════════════════════════════════════════════════════════

fn store_lit(lit: Lit) -> LiteralId {
    get_storage(|store| store.store_literal(lit))
}

fn min_lit_for_type(ty: &Type) -> Lit {
    match ty {
        Type::U8 { .. } => Lit::U8(0),
        Type::U16 { .. } => Lit::U16(0),
        Type::U32 { .. } => Lit::U32(0),
        Type::U64 { .. } => Lit::U64(0),
        Type::U128 { .. } => Lit::U128(0),
        Type::USize { .. } => Lit::USize(32, 0),
        Type::I8 { .. } => Lit::I8(0),
        Type::I16 { .. } => Lit::I16(0),
        Type::I32 { .. } => Lit::I32(0),
        Type::I64 { .. } => Lit::I64(0),
        Type::I128 { .. } => Lit::I128(0),
        _ => Lit::U64(0),
    }
}

fn max_lit_for_type(ty: &Type) -> Lit {
    match ty {
        Type::U8 { .. } => Lit::U8(u8::MAX),
        Type::U16 { .. } => Lit::U16(u16::MAX),
        Type::U32 { .. } => Lit::U32(u32::MAX),
        Type::U64 { .. } => Lit::U64(u64::MAX),
        Type::U128 { .. } => Lit::U128(u128::MAX),
        Type::USize { .. } => Lit::USize(64, u64::MAX),
        Type::I8 { .. } => Lit::I8(i8::MAX),
        Type::I16 { .. } => Lit::I16(i16::MAX),
        Type::I32 { .. } => Lit::I32(i32::MAX),
        Type::I64 { .. } => Lit::I64(i64::MAX),
        Type::I128 { .. } => Lit::I128(i128::MAX),
        _ => Lit::U64(u64::MAX),
    }
}

fn lit_to_u128(lit: &Lit) -> Option<u128> {
    match lit {
        Lit::U8(w) => Some(*w as u128),
        Lit::U16(w) => Some(*w as u128),
        Lit::U32(w) => Some(*w as u128),
        Lit::U64(w) => Some(*w as u128),
        Lit::U128(w) => Some(*w),
        Lit::USize(_bits, w) => Some(*w as u128),
        Lit::I8(w) if *w >= 0 => Some(*w as u128),
        Lit::I16(w) if *w >= 0 => Some(*w as u128),
        Lit::I32(w) if *w >= 0 => Some(*w as u128),
        Lit::I64(w) if *w >= 0 => Some(*w as u128),
        Lit::I128(w) if *w >= 0 => Some(*w as u128),
        _ => None,
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Type Path Lowering
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_type_path(type_path: ast::TypePath, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    let span = type_path.span;

    let has_intermediate_generics = type_path.segments[..type_path.segments.len().saturating_sub(1)]
        .iter()
        .any(|seg| seg.type_arguments.is_some());

    if has_intermediate_generics {
        let path_str = type_path
            .segments
            .iter()
            .map(|s| s.name.to_string())
            .collect::<Vec<_>>()
            .join("::");
        log.report(&HirErr::IntermediateGenericArgsNotSupported { span, path: path_str });
        return Err(());
    }

    let type_args: Vec<TypeId> = type_path
        .segments
        .last()
        .map(|seg| helpers::extract_type_args(&seg.type_arguments, ctx, log))
        .unwrap_or_default();

    match type_path.resolved_path {
        Some(ref resolved_path) => {
            if (resolved_path.deref() == "Self" || resolved_path.deref() == "self")
                && let Some(self_type) = &ctx.current_self_type
            {
                return Ok((**self_type).clone());
            }

            let base_type = match ctx.ast_symbol_map.get(resolved_path) {
                Some(SymbolKind::Struct) => Type::Struct {
                    span,
                    def: ctx.tab.get_struct_or_insert_placeholder(resolved_path).clone(),
                },
                Some(SymbolKind::Enum) => Type::Enum {
                    span,
                    def: ctx.tab.get_enum_or_insert_placeholder(resolved_path).clone(),
                },
                Some(SymbolKind::TypeAlias) => Type::TypeAlias {
                    span,
                    def: ctx.tab.get_type_alias_or_insert_placeholder(resolved_path).clone(),
                },
                Some(SymbolKind::GenericParameter) => {
                    return Ok(ctx.create_generic_placeholder(resolved_path.clone()));
                }
                _ => {
                    log.report(&HirErr::UnresolvedSymbol {
                        span,
                        name: resolved_path.to_string(),
                    });
                    return Err(());
                }
            };

            if !type_args.is_empty() {
                let base_id: TypeId = base_type.into();
                Ok(Type::Parameterized {
                    span,
                    base: base_id,
                    args: Arguments {
                        positional: type_args.into(),
                        named: thin_vec::ThinVec::new(),
                    },
                })
            } else {
                Ok(base_type)
            }
        }
        None => {
            let path_str = type_path
                .segments
                .iter()
                .map(|s| s.name.to_string())
                .collect::<Vec<_>>()
                .join("::");
            log.report(&HirErr::UnresolvedTypePath { span, name: path_str });
            Err(())
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Refinement Type Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_refinement_bound(
    bound_expr: ast::Expr,
    target_type: Option<&Type>,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<LiteralId, ()> {
    let hir_value = lower_expr(bound_expr, ctx, log)?;

    let cast_value = match target_type {
        Some(ty) => Value::Cast {
            span: ByteSpan::default(),
            value: hir_value.into(),
            target_type: ty.clone().into(),
        },
        None => hir_value,
    };

    match Evaluator::new(log, ctx.ptr_size).evaluate_to_literal(&cast_value) {
        Ok(lit) => Ok(store_lit(lit)),
        Err(_) => {
            log.report(&HirErr::RefinementBoundNotConstant {
                span: ByteSpan::default(),
            });
            Err(())
        }
    }
}

pub(crate) fn lower_refinement_type(
    refinement_type: ast::RefinementType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let r_span = refinement_type.span;
    let basis_type = lower_type(refinement_type.basis_type, ctx, log)?;

    let is_integer = matches!(
        &basis_type,
        Type::U8 { .. }
            | Type::U16 { .. }
            | Type::U32 { .. }
            | Type::U64 { .. }
            | Type::U128 { .. }
            | Type::USize { .. }
            | Type::I8 { .. }
            | Type::I16 { .. }
            | Type::I32 { .. }
            | Type::I64 { .. }
            | Type::I128 { .. }
    );

    if !is_integer {
        let base_desc = format!("{:?}", basis_type);
        log.report(&HirErr::RefinementTypeOnNonInteger {
            span: r_span,
            base_type: base_desc,
        });
        return Err(());
    }

    let (min_lit, max_lit): (LiteralId, LiteralId) =
        match (refinement_type.width, refinement_type.minimum, refinement_type.maximum) {
            (Some(width_expr), None, None) => {
                let w_id = lower_refinement_bound(width_expr, None, ctx, log)?;
                let w_lit: Lit = get_storage(|store| store[&w_id]);
                let width_val = lit_to_u128(&w_lit).filter(|&v| v != 0 && v <= 128).ok_or_else(|| {
                    let width_str = format!("{:?}", w_lit);
                    log.report(&HirErr::RefinementWidthOutOfRange {
                        span: r_span,
                        width: width_str,
                    });
                })?;

                let max_val = (1u128 << width_val) - 1;
                let min = store_lit(Lit::U128(0));
                let max = store_lit(Lit::U128(max_val));
                (min, max)
            }

            (None, Some(min_expr), Some(max_expr)) => {
                let min = lower_refinement_bound(min_expr, Some(&basis_type), ctx, log)?;
                let max = lower_refinement_bound(max_expr, Some(&basis_type), ctx, log)?;
                (min, max)
            }

            (Some(width_expr), Some(min_expr), Some(max_expr)) => {
                let _w_id = lower_refinement_bound(width_expr, None, ctx, log)?;
                let min = lower_refinement_bound(min_expr, Some(&basis_type), ctx, log)?;
                let max = lower_refinement_bound(max_expr, Some(&basis_type), ctx, log)?;
                (min, max)
            }

            (None, Some(min_expr), None) => {
                let min = lower_refinement_bound(min_expr, Some(&basis_type), ctx, log)?;
                let max = store_lit(max_lit_for_type(&basis_type));
                (min, max)
            }

            (None, None, Some(max_expr)) => {
                let min = store_lit(min_lit_for_type(&basis_type));
                let max = lower_refinement_bound(max_expr, Some(&basis_type), ctx, log)?;
                (min, max)
            }

            (Some(width_expr), Some(min_expr), None) => {
                let _w_id = lower_refinement_bound(width_expr, None, ctx, log)?;
                let min = lower_refinement_bound(min_expr, Some(&basis_type), ctx, log)?;
                let max = store_lit(max_lit_for_type(&basis_type));
                (min, max)
            }

            (Some(width_expr), None, Some(max_expr)) => {
                let _w_id = lower_refinement_bound(width_expr, None, ctx, log)?;
                let min = store_lit(min_lit_for_type(&basis_type));
                let max = lower_refinement_bound(max_expr, Some(&basis_type), ctx, log)?;
                (min, max)
            }

            (None, None, None) => {
                log.report(&HirErr::RefinementTypeEmpty { span: r_span });
                return Err(());
            }
        };

    Ok(Type::Refine {
        span: r_span,
        base: basis_type.into(),
        min: min_lit,
        max: max_lit,
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Compound Type Lowering
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_tuple_type(
    tuple_type: ast::TupleType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    if tuple_type.element_types.is_empty() {
        return Ok(Type::Unit {
            span: ByteSpan::default(),
        });
    }

    let mut element_types = Vec::with_capacity(tuple_type.element_types.len());
    for ast_element_type in tuple_type.element_types.into_iter() {
        let hir_elem_ty: TypeId = lower_type(ast_element_type, ctx, log)?.into();
        element_types.push(hir_elem_ty);
    }

    Ok(Type::Tuple {
        span: tuple_type.span,
        element_types: element_types.into(),
    })
}

pub(crate) fn lower_array_type(
    array_type: ast::ArrayType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let element_type: TypeId = lower_type(array_type.element_type, ctx, log)?.into();
    let a_span = array_type.span;

    let array_length_expr = Value::Cast {
        span: ByteSpan::default(),
        value: lower_expr(array_type.len, ctx, log)?.into(),
        target_type: Type::USize {
            span: ByteSpan::default(),
        }
        .into(),
    };

    let len = match Evaluator::new(log, ctx.ptr_size).evaluate_to_literal(&array_length_expr) {
        Ok(Lit::USize(_bits, val)) => u32::try_from(val).map_err(|_| {
            log.report(&HirErr::ArrayLengthExpectedUSize { span: a_span });
        })?,
        Ok(_) => {
            log.report(&HirErr::ArrayLengthExpectedUSize { span: a_span });
            return Err(());
        }
        Err(err) => {
            log.report(&HirErr::ArrayTypeLengthEvalError { span: a_span, err });
            return Err(());
        }
    };

    Ok(Type::Array {
        span: a_span,
        element_type,
        len,
    })
}

pub(crate) fn lower_function_type(
    func_type_ast: ast::FunctionType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let span = func_type_ast.span;

    if let Some(attrs) = &func_type_ast.attributes {
        for attr in attrs {
            let name = match attr {
                ast::Expr::Path(p) => p
                    .segments
                    .iter()
                    .map(|s| s.name.to_string())
                    .collect::<Vec<_>>()
                    .join("::"),
                _ => "#[<unknown>]".to_string(),
            };
            log.report(&HirErr::UnrecognizedFunctionAttribute {
                span: attr.span(),
                name,
            });
        }
    }

    let mut parameters = Vec::with_capacity(func_type_ast.parameters.len());
    for param in func_type_ast.parameters {
        if let Some(attrs) = &param.attributes {
            for attr in attrs {
                let name = match attr {
                    ast::Expr::Path(p) => p
                        .segments
                        .iter()
                        .map(|s| s.name.to_string())
                        .collect::<Vec<_>>()
                        .join("::"),
                    _ => "#[<unknown>]".to_string(),
                };
                log.report(&HirErr::UnrecognizedFunctionParamAttribute {
                    span: attr.span(),
                    name,
                });
            }
        }

        let ty: TypeId = lower_type(param.ty, ctx, log)?.into();
        parameters.push((param.name, ty));
    }

    let return_type: TypeId = match func_type_ast.return_type {
        Some(ret_ty) => lower_type(ret_ty, ctx, log)?.into(),
        None => Type::Unit {
            span: ByteSpan::default(),
        }
        .into(),
    };

    let func_type = FunctionType {
        attributes: BTreeSet::new(),
        params: parameters.into(),
        return_type,
    };

    Ok(Type::Function {
        span,
        function_type: func_type.into(),
    })
}

// ═══════════════════════════════════════════════════════════════════════════
// Reference and Pointer Type Lowering
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_reference_type(
    reference_type: ast::ReferenceType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let lifetime = helpers::lower_lifetime(reference_type.lifetime, log)?;
    let mutable = helpers::is_mutable(reference_type.mutability);
    let exclusive = helpers::lower_exclusivity(reference_type.exclusivity, mutable);

    if let ast::Type::SliceType(slice) = reference_type.to {
        let element_type = lower_type(slice.element_type, ctx, log)?.into();
        Ok(Type::SliceRef {
            span: reference_type.span,
            lifetime,
            exclusive,
            mutable,
            element_type,
        })
    } else {
        let to = lower_type(reference_type.to, ctx, log)?.into();
        Ok(Type::Reference {
            span: reference_type.span,
            lifetime,
            exclusive,
            mutable,
            to,
        })
    }
}

pub(crate) fn lower_pointer_type(
    pointer_type: ast::PointerType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let mutable = helpers::is_mutable(pointer_type.mutability);
    let exclusive = helpers::lower_exclusivity(pointer_type.exclusivity, mutable);

    if let ast::Type::SliceType(slice) = pointer_type.to {
        let element_type: TypeId = lower_type(slice.element_type, ctx, log)?.into();
        Ok(Type::SlicePtr {
            span: pointer_type.span,
            lifetime: Lifetime::Inferred,
            exclusive,
            mutable,
            element_type,
        })
    } else {
        let to = lower_type(pointer_type.to, ctx, log)?.into();
        Ok(Type::Pointer {
            span: pointer_type.span,
            lifetime: Lifetime::Inferred,
            exclusive,
            mutable,
            to,
        })
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Error-Stub Type Lowering
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_slice_type(
    slice_type: ast::SliceType,
    _ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    log.report(&HirErr::SliceTypesMustBeInRefOrPtr { span: slice_type.span });
    Err(())
}

pub(crate) fn lower_type_potential(
    type_potential: ast::TypePotential,
    _ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    log.report(&HirErr::TypePotentialNotImplemented {
        span: type_potential.span,
    });
    Err(())
}

pub(crate) fn lower_lifetime_type(
    lifetime: ast::Lifetime,
    _ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    log.report(&HirErr::LifetimeTypeNotImplemented { span: lifetime.span });
    Err(())
}

// ═══════════════════════════════════════════════════════════════════════════
// Main Type Dispatch
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_type(ty: ast::Type, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    match ty {
        ast::Type::SyntaxError(_) => Err(()),
        ast::Type::Bool(t) => Ok(Type::Bool { span: t.span }),
        ast::Type::UInt8(t) => Ok(Type::U8 { span: t.span }),
        ast::Type::UInt16(t) => Ok(Type::U16 { span: t.span }),
        ast::Type::UInt32(t) => Ok(Type::U32 { span: t.span }),
        ast::Type::UInt64(t) => Ok(Type::U64 { span: t.span }),
        ast::Type::UInt128(t) => Ok(Type::U128 { span: t.span }),
        ast::Type::USize(t) => Ok(Type::USize { span: t.span }),
        ast::Type::Int8(t) => Ok(Type::I8 { span: t.span }),
        ast::Type::Int16(t) => Ok(Type::I16 { span: t.span }),
        ast::Type::Int32(t) => Ok(Type::I32 { span: t.span }),
        ast::Type::Int64(t) => Ok(Type::I64 { span: t.span }),
        ast::Type::Int128(t) => Ok(Type::I128 { span: t.span }),
        ast::Type::Float32(t) => Ok(Type::F32 { span: t.span }),
        ast::Type::Float64(t) => Ok(Type::F64 { span: t.span }),
        ast::Type::InferType(t) => Ok(ctx.create_inference_placeholder()),
        ast::Type::TypePath(t) => lower_type_path(*t, ctx, log),
        ast::Type::RefinementType(t) => lower_refinement_type(*t, ctx, log),
        ast::Type::TupleType(t) => lower_tuple_type(*t, ctx, log),
        ast::Type::ArrayType(t) => lower_array_type(*t, ctx, log),
        ast::Type::SliceType(t) => lower_slice_type(*t, ctx, log),
        ast::Type::FunctionType(t) => lower_function_type(*t, ctx, log),
        ast::Type::ReferenceType(t) => lower_reference_type(*t, ctx, log),
        ast::Type::PointerType(t) => lower_pointer_type(*t, ctx, log),
        ast::Type::TypePotential(t) => lower_type_potential(*t, ctx, log),
        ast::Type::Lifetime(t) => lower_lifetime_type(*t, ctx, log),
        ast::Type::Parentheses(t) => lower_type(t.inner, ctx, log),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::context::Ast2HirCtx;
    use nitrate_hir::Store;
    use nitrate_nstring::NString;
    use nitrate_tree::ast;
    use nitrate_tree_resolve::ImportContext;
    use std::collections::BTreeSet;

    fn ctx_and_log() -> (Ast2HirCtx, CompilerLog) {
        let log = CompilerLog::default();
        let import_ctx = ImportContext::new("test".into(), "test.nit".into());
        let ctx = Ast2HirCtx::new(PtrSize::U64, import_ctx);
        (ctx, log)
    }

    fn run<R>(f: impl FnOnce(&mut Ast2HirCtx, &CompilerLog) -> R) -> R {
        let store = Store::new();
        using_storage(&store, || {
            let (mut ctx, log) = ctx_and_log();
            f(&mut ctx, &log)
        })
    }

    #[test]
    fn lower_type_bool() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::Bool(ast::Bool {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::Bool {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_u8() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::UInt8(ast::UInt8 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::U8 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_u16() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::UInt16(ast::UInt16 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::U16 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_u32() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::UInt32(ast::UInt32 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::U32 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_u64() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::UInt64(ast::UInt64 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::U64 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_i8() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::Int8(ast::Int8 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::I8 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_i32() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::Int32(ast::Int32 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::I32 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_i64() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::Int64(ast::Int64 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::I64 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_f32() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::Float32(ast::Float32 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::F32 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_f64() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::Float64(ast::Float64 {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::F64 {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_infer() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::InferType(ast::InferType {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        )
        .unwrap();
        assert!(r.is_inferred());
    }

    #[test]
    fn lower_type_syntax_error() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::SyntaxError(ast::TypeSyntaxError {
                span: ByteSpan::default(),
            }),
            &mut ctx,
            &log,
        );
        assert!(r.is_err());
    }

    #[test]
    fn lower_type_parentheses() {
        let (mut ctx, log) = ctx_and_log();
        let r = lower_type(
            ast::Type::Parentheses(Box::new(ast::TypeParentheses {
                span: ByteSpan::default(),
                inner: ast::Type::Bool(ast::Bool {
                    span: ByteSpan::default(),
                }),
            })),
            &mut ctx,
            &log,
        );
        assert_eq!(
            r,
            Ok(Type::Bool {
                span: ByteSpan::default()
            })
        );
    }

    #[test]
    fn lower_type_path_struct_resolved() {
        run(|ctx, log| {
            ctx.ast_symbol_map.insert("Foo".into(), SymbolKind::Struct);
            ctx.tab.get_struct_or_insert_placeholder(&"Foo".into());
            let tp = ast::TypePath {
                span: ByteSpan::default(),
                segments: vec![ast::TypePathSegment {
                    span: ByteSpan::default(),
                    name: "Foo".into(),
                    type_arguments: None,
                }],
                resolved_path: Some("Foo".into()),
            };
            let r = lower_type_path(tp, ctx, log).unwrap();
            assert!(matches!(r, Type::Struct { .. }));
        })
    }

    #[test]
    fn lower_type_path_enum_resolved() {
        run(|ctx, log| {
            ctx.ast_symbol_map.insert("Color".into(), SymbolKind::Enum);
            ctx.tab.get_enum_or_insert_placeholder(&"Color".into());
            let tp = ast::TypePath {
                span: ByteSpan::default(),
                segments: vec![ast::TypePathSegment {
                    span: ByteSpan::default(),
                    name: "Color".into(),
                    type_arguments: None,
                }],
                resolved_path: Some("Color".into()),
            };
            let r = lower_type_path(tp, ctx, log).unwrap();
            assert!(matches!(r, Type::Enum { .. }));
        })
    }

    #[test]
    fn lower_type_path_type_alias_resolved() {
        run(|ctx, log| {
            ctx.ast_symbol_map.insert("MyInt".into(), SymbolKind::TypeAlias);
            ctx.tab.get_type_alias_or_insert_placeholder(&"MyInt".into());
            let tp = ast::TypePath {
                span: ByteSpan::default(),
                segments: vec![ast::TypePathSegment {
                    span: ByteSpan::default(),
                    name: "MyInt".into(),
                    type_arguments: None,
                }],
                resolved_path: Some("MyInt".into()),
            };
            let r = lower_type_path(tp, ctx, log).unwrap();
            assert!(matches!(r, Type::TypeAlias { .. }));
        })
    }

    #[test]
    fn lower_type_path_generic_param() {
        let (mut ctx, log) = ctx_and_log();
        ctx.ast_symbol_map.insert("T".into(), SymbolKind::GenericParameter);
        let tp = ast::TypePath {
            span: ByteSpan::default(),
            segments: vec![ast::TypePathSegment {
                span: ByteSpan::default(),
                name: "T".into(),
                type_arguments: None,
            }],
            resolved_path: Some("T".into()),
        };
        let r = lower_type_path(tp, &mut ctx, &log).unwrap();
        assert!(matches!(r, Type::GenericParam { .. }));
    }

    #[test]
    fn lower_type_path_unresolved_symbol() {
        let (mut ctx, log) = ctx_and_log();
        ctx.ast_symbol_map.insert("X".into(), SymbolKind::Function);
        let tp = ast::TypePath {
            span: ByteSpan::default(),
            segments: vec![ast::TypePathSegment {
                span: ByteSpan::default(),
                name: "X".into(),
                type_arguments: None,
            }],
            resolved_path: Some("X".into()),
        };
        assert!(lower_type_path(tp, &mut ctx, &log).is_err());
    }

    #[test]
    fn lower_type_path_unresolved_path() {
        let (mut ctx, log) = ctx_and_log();
        let tp = ast::TypePath {
            span: ByteSpan::default(),
            segments: vec![ast::TypePathSegment {
                span: ByteSpan::default(),
                name: "X".into(),
                type_arguments: None,
            }],
            resolved_path: None,
        };
        assert!(lower_type_path(tp, &mut ctx, &log).is_err());
    }

    #[test]
    fn lower_type_path_intermediate_generics() {
        let (mut ctx, log) = ctx_and_log();
        ctx.ast_symbol_map.insert("Foo".into(), SymbolKind::Struct);
        ctx.ast_symbol_map.insert("Bar".into(), SymbolKind::Struct);
        let tp = ast::TypePath {
            span: ByteSpan::default(),
            segments: vec![
                ast::TypePathSegment {
                    span: ByteSpan::default(),
                    name: "Foo".into(),
                    type_arguments: Some(vec![ast::TypeArgument {
                        span: ByteSpan::default(),
                        name: None,
                        value: ast::Type::Int32(ast::Int32 {
                            span: ByteSpan::default(),
                        }),
                    }]),
                },
                ast::TypePathSegment {
                    span: ByteSpan::default(),
                    name: "Bar".into(),
                    type_arguments: None,
                },
            ],
            resolved_path: Some("Foo::Bar".into()),
        };
        assert!(lower_type_path(tp, &mut ctx, &log).is_err());
    }

    #[test]
    fn lower_type_path_parameterized() {
        run(|ctx, log| {
            ctx.ast_symbol_map.insert("Vec".into(), SymbolKind::Struct);
            ctx.tab.get_struct_or_insert_placeholder(&"Vec".into());
            let tp = ast::TypePath {
                span: ByteSpan::default(),
                segments: vec![ast::TypePathSegment {
                    span: ByteSpan::default(),
                    name: "Vec".into(),
                    type_arguments: Some(vec![ast::TypeArgument {
                        span: ByteSpan::default(),
                        name: None,
                        value: ast::Type::Int32(ast::Int32 {
                            span: ByteSpan::default(),
                        }),
                    }]),
                }],
                resolved_path: Some("Vec".into()),
            };
            let r = lower_type_path(tp, ctx, log).unwrap();
            assert!(matches!(r, Type::Parameterized { .. }));
        })
    }

    #[test]
    fn lower_tuple_empty_is_unit() {
        let (mut ctx, log) = ctx_and_log();
        let t = ast::TupleType {
            span: ByteSpan::default(),
            element_types: vec![],
        };
        assert_eq!(
            lower_tuple_type(t, &mut ctx, &log).unwrap(),
            Type::Unit {
                span: ByteSpan::default()
            }
        );
    }

    #[test]
    fn lower_tuple_single() {
        run(|ctx, log| {
            let t = ast::TupleType {
                span: ByteSpan::default(),
                element_types: vec![ast::Type::Bool(ast::Bool {
                    span: ByteSpan::default(),
                })],
            };
            let r = lower_tuple_type(t, ctx, log).unwrap();
            assert!(matches!(r, Type::Tuple { element_types, .. } if element_types.len() == 1));
        })
    }

    #[test]
    fn lower_tuple_multiple() {
        run(|ctx, log| {
            let t = ast::TupleType {
                span: ByteSpan::default(),
                element_types: vec![
                    ast::Type::Bool(ast::Bool {
                        span: ByteSpan::default(),
                    }),
                    ast::Type::Int32(ast::Int32 {
                        span: ByteSpan::default(),
                    }),
                    ast::Type::Float64(ast::Float64 {
                        span: ByteSpan::default(),
                    }),
                ],
            };
            let r = lower_tuple_type(t, ctx, log).unwrap();
            assert!(matches!(r, Type::Tuple { element_types, .. } if element_types.len() == 3));
        })
    }

    #[test]
    fn lower_fn_type_empty() {
        run(|ctx, log| {
            let ft = ast::FunctionType {
                span: ByteSpan::default(),
                attributes: None,
                parameters: vec![],
                return_type: None,
            };
            let r = lower_function_type(ft, ctx, log).unwrap();
            assert!(matches!(r, Type::Function { .. }));
        })
    }

    #[test]
    fn lower_fn_type_with_params() {
        run(|ctx, log| {
            let ft = ast::FunctionType {
                span: ByteSpan::default(),
                attributes: None,
                parameters: vec![
                    ast::FuncTypeParam {
                        span: ByteSpan::default(),
                        attributes: None,
                        name: "a".into(),
                        ty: ast::Type::Int32(ast::Int32 {
                            span: ByteSpan::default(),
                        }),
                    },
                    ast::FuncTypeParam {
                        span: ByteSpan::default(),
                        attributes: None,
                        name: "b".into(),
                        ty: ast::Type::Bool(ast::Bool {
                            span: ByteSpan::default(),
                        }),
                    },
                ],
                return_type: Some(ast::Type::Float64(ast::Float64 {
                    span: ByteSpan::default(),
                })),
            };
            let r = lower_function_type(ft, ctx, log).unwrap();
            assert!(matches!(r, Type::Function { .. }));
        })
    }

    #[test]
    fn lower_fn_type_attr_error() {
        run(|ctx, log| {
            let ft = ast::FunctionType {
                span: ByteSpan::default(),
                attributes: Some(vec![ast::Expr::Path(Box::new(ast::ExprPath {
                    span: ByteSpan::default(),
                    segments: vec![ast::ExprPathSegment {
                        span: ByteSpan::default(),
                        name: "bad".into(),
                        type_arguments: None,
                    }],
                    resolved_path: None,
                }))]),
                parameters: vec![],
                return_type: None,
            };
            assert!(lower_function_type(ft, ctx, log).is_ok());
            assert!(log.error_bit());
        })
    }

    #[test]
    fn lower_ref_default() {
        run(|ctx, log| {
            let rt = ast::ReferenceType {
                span: ByteSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32 {
                    span: ByteSpan::default(),
                }),
            };
            let r = lower_reference_type(rt, ctx, log).unwrap();
            assert!(matches!(
                r,
                Type::Reference {
                    lifetime: Lifetime::Inferred,
                    mutable: false,
                    exclusive: false,
                    ..
                }
            ));
        })
    }

    #[test]
    fn lower_ref_mut() {
        run(|ctx, log| {
            let rt = ast::ReferenceType {
                span: ByteSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::Bool(ast::Bool {
                    span: ByteSpan::default(),
                }),
            };
            let r = lower_reference_type(rt, ctx, log).unwrap();
            assert!(matches!(
                r,
                Type::Reference {
                    mutable: true,
                    exclusive: true,
                    ..
                }
            ));
        })
    }

    #[test]
    fn lower_ref_lifetime_static() {
        run(|ctx, log| {
            let rt = ast::ReferenceType {
                span: ByteSpan::default(),
                lifetime: Some(ast::Lifetime {
                    span: ByteSpan::default(),
                    name: "static".into(),
                }),
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32 {
                    span: ByteSpan::default(),
                }),
            };
            let r = lower_reference_type(rt, ctx, log).unwrap();
            assert!(matches!(
                r,
                Type::Reference {
                    lifetime: Lifetime::Static,
                    ..
                }
            ));
        })
    }

    #[test]
    fn lower_ref_to_slice() {
        run(|ctx, log| {
            let rt = ast::ReferenceType {
                span: ByteSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    span: ByteSpan::default(),
                    element_type: ast::Type::Int32(ast::Int32 {
                        span: ByteSpan::default(),
                    }),
                })),
            };
            let r = lower_reference_type(rt, ctx, log).unwrap();
            assert!(matches!(r, Type::SliceRef { .. }));
        })
    }

    #[test]
    fn lower_ref_bad_lifetime() {
        let (mut ctx, log) = ctx_and_log();
        let rt = ast::ReferenceType {
            span: ByteSpan::default(),
            lifetime: Some(ast::Lifetime {
                span: ByteSpan::default(),
                name: "invalid".into(),
            }),
            exclusivity: None,
            mutability: None,
            to: ast::Type::Int32(ast::Int32 {
                span: ByteSpan::default(),
            }),
        };
        assert!(lower_reference_type(rt, &mut ctx, &log).is_err());
    }

    #[test]
    fn lower_ptr_default() {
        run(|ctx, log| {
            let pt = ast::PointerType {
                span: ByteSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::Int32(ast::Int32 {
                    span: ByteSpan::default(),
                }),
            };
            let r = lower_pointer_type(pt, ctx, log).unwrap();
            assert!(matches!(
                r,
                Type::Pointer {
                    mutable: false,
                    exclusive: false,
                    ..
                }
            ));
        })
    }

    #[test]
    fn lower_ptr_mut() {
        run(|ctx, log| {
            let pt = ast::PointerType {
                span: ByteSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: Some(ast::Mutability::Mut),
                to: ast::Type::Bool(ast::Bool {
                    span: ByteSpan::default(),
                }),
            };
            let r = lower_pointer_type(pt, ctx, log).unwrap();
            assert!(matches!(
                r,
                Type::Pointer {
                    mutable: true,
                    exclusive: true,
                    ..
                }
            ));
        })
    }

    #[test]
    fn lower_ptr_to_slice() {
        run(|ctx, log| {
            let pt = ast::PointerType {
                span: ByteSpan::default(),
                lifetime: None,
                exclusivity: None,
                mutability: None,
                to: ast::Type::SliceType(Box::new(ast::SliceType {
                    span: ByteSpan::default(),
                    element_type: ast::Type::Int32(ast::Int32 {
                        span: ByteSpan::default(),
                    }),
                })),
            };
            let r = lower_pointer_type(pt, ctx, log).unwrap();
            assert!(matches!(r, Type::SlicePtr { .. }));
        })
    }

    #[test]
    fn lower_slice_outside_ref_ptr_fails() {
        let (mut ctx, log) = ctx_and_log();
        assert!(
            lower_slice_type(
                ast::SliceType {
                    span: ByteSpan::default(),
                    element_type: ast::Type::Int32(ast::Int32 {
                        span: ByteSpan::default()
                    })
                },
                &mut ctx,
                &log,
            )
            .is_err()
        );
    }

    #[test]
    fn lower_refinement_type_fails() {
        let (mut ctx, log) = ctx_and_log();
        assert!(
            lower_refinement_type(
                ast::RefinementType {
                    span: ByteSpan::default(),
                    basis_type: ast::Type::Int32(ast::Int32 {
                        span: ByteSpan::default()
                    }),
                    width: None,
                    minimum: None,
                    maximum: None
                },
                &mut ctx,
                &log,
            )
            .is_err()
        );
    }

    #[test]
    fn lower_type_potential_fails() {
        let (mut ctx, log) = ctx_and_log();
        assert!(
            lower_type_potential(
                ast::TypePotential {
                    span: ByteSpan::default(),
                    body: ast::Block {
                        span: ByteSpan::default(),
                        safety: None,
                        elements: vec![]
                    }
                },
                &mut ctx,
                &log,
            )
            .is_err()
        );
    }

    #[test]
    fn lower_array_i32_5() {
        run(|ctx, log| {
            let a = ast::ArrayType {
                span: ByteSpan::default(),
                element_type: ast::Type::Int32(ast::Int32 {
                    span: ByteSpan::default(),
                }),
                len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                    span: ByteSpan::default(),
                    value: 5,
                    kind: IntegerKind::Dec,
                })),
            };
            let r = lower_array_type(a, ctx, log).unwrap();
            assert!(matches!(r, Type::Array { len: 5, .. }));
        })
    }

    #[test]
    fn lower_array_string_len_fails() {
        run(|ctx, log| {
            let a = ast::ArrayType {
                span: ByteSpan::default(),
                element_type: ast::Type::Int32(ast::Int32 {
                    span: ByteSpan::default(),
                }),
                len: ast::Expr::String(ast::StringLit {
                    span: ByteSpan::default(),
                    value: "bad".into(),
                }),
            };
            assert!(lower_array_type(a, ctx, log).is_err());
        })
    }
}
