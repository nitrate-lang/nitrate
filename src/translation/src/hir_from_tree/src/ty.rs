use crate::{context::Ast2HirCtx, diagnosis::HirErr, expr::lower_expr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_evaluate::HirEvalCtx;
use nitrate_token::IntegerKind;
use nitrate_tree::ast::{self as ast, SymbolKind};
use nitrate_tree_resolve::ImportContext;
use std::{collections::BTreeSet, ops::Deref, ops::Index};

pub(crate) fn lower_type_path(type_path: ast::TypePath, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    // Check for generic args in intermediate segments (e.g., Foo<i32>::Bar)
    if type_path.segments[..type_path.segments.len().saturating_sub(1)]
        .iter()
        .any(|seg| seg.type_arguments.is_some())
    {
        log.report(&HirErr::UnimplementedFeature(
            "can not lower type paths with generic arguments in intermediate segments".into(),
        ));
    }

    // Check if the last segment has type arguments
    let type_args: Option<Vec<TypeId>> = type_path.segments.last().and_then(|seg| {
        seg.type_arguments.as_ref().map(|type_args| {
            type_args
                .iter()
                .filter_map(|type_arg| {
                    lower_type(type_arg.value.clone(), ctx, log).ok().map(|t| {
                        let hir_type_arg: TypeId = t.into();
                        hir_type_arg
                    })
                })
                .collect()
        })
    });

    match type_path.resolved_path {
        Some(ref resolved_path) => {
            // Handle `Self` keyword: resolve to the current impl type
            if resolved_path.deref() == "Self" || resolved_path.deref() == "self" {
                if let Some(self_type) = &ctx.current_self_type {
                    return Ok((&**self_type).clone());
                }
            }

            let base_type = match ctx.ast_symbol_map.get(&resolved_path) {
                Some(SymbolKind::Struct) => Type::Struct {
                    def: ctx.tab.get_struct_or_insert_placeholder(&resolved_path).clone(),
                },

                Some(SymbolKind::Enum) => Type::Enum {
                    def: ctx.tab.get_enum_or_insert_placeholder(&resolved_path).clone(),
                },

                Some(SymbolKind::TypeAlias) => Type::TypeAlias {
                    def: ctx.tab.get_type_alias_or_insert_placeholder(&resolved_path).clone(),
                },

                Some(SymbolKind::GenericParameter) => return Ok(ctx.create_generic_placeholder(resolved_path.clone())),

                _ => {
                    log.report(&HirErr::UnresolvedSymbol);
                    return Err(());
                }
            };

            // If there are type arguments, wrap in Parameterized
            if let Some(args) = type_args {
                let base_id: TypeId = base_type.into();
                Ok(Type::Parameterized {
                    base: base_id,
                    args: Arguments {
                        positional: args.into(),
                        named: thin_vec::ThinVec::new(),
                    },
                })
            } else {
                Ok(base_type)
            }
        }

        None => {
            log.report(&HirErr::UnresolvedTypePath);
            Err(())
        }
    }
}

/// Stores a literal value into the global store and returns its ID.
fn store_lit(lit: Lit) -> LiteralId {
    get_storage(|store| store.store_literal(lit))
}

/// Returns the minimum literal value for a given integer type.
fn min_lit_for_type(ty: &Type) -> Lit {
    match ty {
        Type::U8 => Lit::U8(0),
        Type::U16 => Lit::U16(0),
        Type::U32 => Lit::U32(0),
        Type::U64 => Lit::U64(0),
        Type::U128 => Lit::U128(0),
        Type::USize => Lit::USize32(0),
        Type::I8 => Lit::I8(0),
        Type::I16 => Lit::I16(0),
        Type::I32 => Lit::I32(0),
        Type::I64 => Lit::I64(0),
        Type::I128 => Lit::I128(0),
        _ => Lit::U64(0),
    }
}

/// Returns the maximum literal value for a given integer type.
fn max_lit_for_type(ty: &Type) -> Lit {
    match ty {
        Type::U8 => Lit::U8(u8::MAX),
        Type::U16 => Lit::U16(u16::MAX),
        Type::U32 => Lit::U32(u32::MAX),
        Type::U64 => Lit::U64(u64::MAX),
        Type::U128 => Lit::U128(u128::MAX),
        Type::USize => Lit::USize64(u64::MAX),
        Type::I8 => Lit::I8(i8::MAX),
        Type::I16 => Lit::I16(i16::MAX),
        Type::I32 => Lit::I32(i32::MAX),
        Type::I64 => Lit::I64(i64::MAX),
        Type::I128 => Lit::I128(i128::MAX),
        _ => Lit::U64(u64::MAX),
    }
}

/// Evaluate an AST expression to a constant literal value.
fn lower_refinement_bound(
    bound_expr: ast::Expr,
    target_type: Option<&Type>,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<LiteralId, ()> {
    let hir_value = lower_expr(bound_expr, ctx, log)?;

    let cast_value = match target_type {
        Some(ty) => Value::Cast {
            value: hir_value.into(),
            target_type: ty.clone().into(),
        },
        None => hir_value,
    };

    match HirEvalCtx::new(log, ctx.ptr_size).evaluate_to_literal(&cast_value) {
        Ok(lit) => Ok(store_lit(lit)),
        Err(_) => {
            log.report(&HirErr::RefinementBoundNotConstant);
            Err(())
        }
    }
}

/// Get the Lit::U128 value from a Lit (or fail if not a compatible numeric lit).
fn lit_to_u128(lit: &Lit) -> Option<u128> {
    match lit {
        Lit::U8(w) => Some(*w as u128),
        Lit::U16(w) => Some(*w as u128),
        Lit::U32(w) => Some(*w as u128),
        Lit::U64(w) => Some(*w as u128),
        Lit::U128(w) => Some(*w),
        Lit::USize32(w) => Some(*w as u128),
        Lit::USize64(w) => Some(*w as u128),
        Lit::I8(w) if *w >= 0 => Some(*w as u128),
        Lit::I16(w) if *w >= 0 => Some(*w as u128),
        Lit::I32(w) if *w >= 0 => Some(*w as u128),
        Lit::I64(w) if *w >= 0 => Some(*w as u128),
        Lit::I128(w) if *w >= 0 => Some(*w as u128),
        _ => None,
    }
}

pub(crate) fn lower_refinement_type(
    refinement_type: ast::RefinementType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let basis_type = lower_type(refinement_type.basis_type, ctx, log)?;

    // Ensure the basis type is an integer type that can be refined
    match &basis_type {
        Type::U8
        | Type::U16
        | Type::U32
        | Type::U64
        | Type::U128
        | Type::USize
        | Type::I8
        | Type::I16
        | Type::I32
        | Type::I64
        | Type::I128 => {}
        _ => {
            log.report(&HirErr::RefinementTypeOnNonInteger);
            return Err(());
        }
    }

    let (min_lit, max_lit): (LiteralId, LiteralId) =
        match (refinement_type.width, refinement_type.minimum, refinement_type.maximum) {
            // Just width: u8: 6  =>  [0: 2^width - 1]
            (Some(width_expr), None, None) => {
                let w_id = lower_refinement_bound(width_expr, None, ctx, log)?;
                let w_lit: Lit = get_storage(|store| store[&w_id].clone());
                let width_val = lit_to_u128(&w_lit).filter(|&v| v != 0 && v <= 128).ok_or_else(|| {
                    log.report(&HirErr::RefinementWidthOutOfRange);
                })?;

                let max_val = (1u128 << width_val) - 1;
                let min = store_lit(Lit::U128(0));
                let max = store_lit(Lit::U128(max_val));
                (min, max)
            }

            // Just explicit range: u8: [0:10]
            (None, Some(min_expr), Some(max_expr)) => {
                let min = lower_refinement_bound(min_expr, Some(&basis_type), ctx, log)?;
                let max = lower_refinement_bound(max_expr, Some(&basis_type), ctx, log)?;
                (min, max)
            }

            // Both width and range: u8: 6: [0:10]
            (Some(width_expr), Some(min_expr), Some(max_expr)) => {
                let _w_id = lower_refinement_bound(width_expr, None, ctx, log)?;
                let min = lower_refinement_bound(min_expr, Some(&basis_type), ctx, log)?;
                let max = lower_refinement_bound(max_expr, Some(&basis_type), ctx, log)?;
                (min, max)
            }

            // Just min: u8: [0:]
            (None, Some(min_expr), None) => {
                let min = lower_refinement_bound(min_expr, Some(&basis_type), ctx, log)?;
                let max = store_lit(max_lit_for_type(&basis_type));
                (min, max)
            }

            // Just max: u8: [:10]
            (None, None, Some(max_expr)) => {
                let min = store_lit(min_lit_for_type(&basis_type));
                let max = lower_refinement_bound(max_expr, Some(&basis_type), ctx, log)?;
                (min, max)
            }

            // width + min only: u8: 6: [0:]
            (Some(width_expr), Some(min_expr), None) => {
                let _w_id = lower_refinement_bound(width_expr, None, ctx, log)?;
                let min = lower_refinement_bound(min_expr, Some(&basis_type), ctx, log)?;
                let max = store_lit(max_lit_for_type(&basis_type));
                (min, max)
            }

            // width + max only: u8: 6: [:10]
            (Some(width_expr), None, Some(max_expr)) => {
                let _w_id = lower_refinement_bound(width_expr, None, ctx, log)?;
                let min = store_lit(min_lit_for_type(&basis_type));
                let max = lower_refinement_bound(max_expr, Some(&basis_type), ctx, log)?;
                (min, max)
            }

            // No bounds at all (shouldn't reach here since parser only creates RefinementType when bounds present)
            (None, None, None) => {
                log.report(&HirErr::RefinementTypeEmpty);
                return Err(());
            }
        };

    Ok(Type::Refine {
        base: basis_type.into(),
        min: min_lit,
        max: max_lit,
    })
}
pub(crate) fn lower_tuple_type(
    tuple_type: ast::TupleType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    if tuple_type.element_types.is_empty() {
        return Ok(Type::Unit);
    }

    let mut element_types = Vec::with_capacity(tuple_type.element_types.len());
    for ast_element_type in tuple_type.element_types.into_iter() {
        let hir_elem_ty: TypeId = lower_type(ast_element_type, ctx, log)?.into();
        element_types.push(hir_elem_ty);
    }

    Ok(Type::Tuple {
        element_types: element_types.into(),
    })
}

pub(crate) fn lower_array_type(
    array_type: ast::ArrayType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let element_type: TypeId = lower_type(array_type.element_type, ctx, log)?.into();

    let array_length_expr = Value::Cast {
        value: lower_expr(array_type.len, ctx, log)?.into(),
        target_type: Type::USize.into(),
    };

    let len = match HirEvalCtx::new(log, ctx.ptr_size).evaluate_to_literal(&array_length_expr) {
        Ok(Lit::USize32(val)) => val,
        Ok(Lit::USize64(val)) => val as u32,

        Ok(_) => {
            log.report(&HirErr::ArrayLengthExpectedUSize);
            return Err(());
        }

        Err(_) => {
            log.report(&HirErr::ArrayTypeLengthEvalError);
            return Err(());
        }
    };

    Ok(Type::Array { element_type, len })
}

pub(crate) fn lower_function_type(
    function_type: ast::FunctionType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let ast_attributes = function_type.attributes.unwrap_or_default();
    let function_attributes = BTreeSet::new();
    for _attr in ast_attributes {
        log.report(&HirErr::UnrecognizedFunctionAttribute);
    }

    let mut parameters = Vec::with_capacity(function_type.parameters.len());
    for param in function_type.parameters {
        if let Some(ast_attributes) = &param.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedFunctionParameterAttribute);
            }
        }

        let ty: TypeId = lower_type(param.ty, ctx, log)?.into();
        parameters.push((param.name, ty));
    }

    let return_type: TypeId = match function_type.return_type {
        Some(ret_ty) => lower_type(ret_ty, ctx, log)?.into(),
        None => Type::Unit.into(),
    };

    let function_type = FunctionType {
        attributes: function_attributes,
        params: parameters.into(),
        return_type,
    };

    Ok(Type::Function {
        function_type: function_type.into(),
    })
}

pub(crate) fn lower_reference_type(
    reference_type: ast::ReferenceType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let lifetime = match reference_type.lifetime {
        None => Lifetime::Inferred,
        Some(ast::Lifetime { name }) => match name.deref() {
            "static" => Lifetime::Static,
            "gc" => Lifetime::Gc,
            "thread" => Lifetime::ThreadLocal,
            "task" => Lifetime::TaskLocal,
            "_" => Lifetime::Inferred,
            _ => {
                log.report(&HirErr::UnrecognizedLifetime);
                return Err(());
            }
        },
    };

    let mutable = match reference_type.mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    };

    let exclusive = match reference_type.exclusivity {
        Some(ast::Exclusivity::Iso) => true,
        Some(ast::Exclusivity::Poly) => false,
        None => mutable,
    };

    if let ast::Type::SliceType(slice) = reference_type.to {
        let element_type = lower_type(slice.element_type, ctx, log)?.into();

        Ok(Type::SliceRef {
            lifetime,
            exclusive,
            mutable,
            element_type,
        })
    } else {
        let to = lower_type(reference_type.to, ctx, log)?.into();

        Ok(Type::Reference {
            lifetime,
            exclusive,
            mutable,
            to,
        })
    }
}

pub(crate) fn lower_slice_type(
    _slice_type: ast::SliceType,
    _ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    log.report(&HirErr::SliceTypesCannotExistOutsideReferencesOrPointers);
    Err(())
}

pub(crate) fn lower_pointer_type(
    pointer_type: ast::PointerType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    let mutable = match pointer_type.mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    };

    let exclusive = match pointer_type.exclusivity {
        Some(ast::Exclusivity::Iso) => true,
        Some(ast::Exclusivity::Poly) => false,
        None => mutable,
    };

    if let ast::Type::SliceType(slice) = pointer_type.to {
        let element_type: TypeId = lower_type(slice.element_type, ctx, log)?.into();

        Ok(Type::SlicePtr {
            lifetime: Lifetime::Inferred,
            exclusive,
            mutable,
            element_type,
        })
    } else {
        let to = lower_type(pointer_type.to, ctx, log)?.into();

        Ok(Type::Pointer {
            lifetime: Lifetime::Inferred,
            exclusive,
            mutable,
            to,
        })
    }
}

pub(crate) fn lower_latent_type(
    _latent_type: ast::LatentType,
    _ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    log.report(&HirErr::UnimplementedFeature("latent types".into()));
    Err(())
}

pub(crate) fn lower_lifetime(_lifetime: ast::Lifetime, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    log.report(&HirErr::UnimplementedFeature("lifetimes".into()));
    Err(())
}

pub(crate) fn lower_type(ty: ast::Type, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    match ty {
        ast::Type::SyntaxError(_) => Err(()),
        ast::Type::Bool(_) => Ok(Type::Bool),
        ast::Type::UInt8(_) => Ok(Type::U8),
        ast::Type::UInt16(_) => Ok(Type::U16),
        ast::Type::UInt32(_) => Ok(Type::U32),
        ast::Type::UInt64(_) => Ok(Type::U64),
        ast::Type::UInt128(_) => Ok(Type::U128),
        ast::Type::USize(_) => Ok(Type::USize),
        ast::Type::Int8(_) => Ok(Type::I8),
        ast::Type::Int16(_) => Ok(Type::I16),
        ast::Type::Int32(_) => Ok(Type::I32),
        ast::Type::Int64(_) => Ok(Type::I64),
        ast::Type::Int128(_) => Ok(Type::I128),
        ast::Type::Float32(_) => Ok(Type::F32),
        ast::Type::Float64(_) => Ok(Type::F64),
        ast::Type::InferType(_) => Ok(ctx.create_inference_placeholder()),
        ast::Type::TypePath(t) => lower_type_path(*t, ctx, log),
        ast::Type::RefinementType(t) => lower_refinement_type(*t, ctx, log),
        ast::Type::TupleType(t) => lower_tuple_type(*t, ctx, log),
        ast::Type::ArrayType(t) => lower_array_type(*t, ctx, log),
        ast::Type::SliceType(t) => lower_slice_type(*t, ctx, log),
        ast::Type::FunctionType(t) => lower_function_type(*t, ctx, log),
        ast::Type::ReferenceType(t) => lower_reference_type(*t, ctx, log),
        ast::Type::PointerType(t) => lower_pointer_type(*t, ctx, log),
        ast::Type::LatentType(t) => lower_latent_type(*t, ctx, log),
        ast::Type::Lifetime(t) => lower_lifetime(*t, ctx, log),
        ast::Type::Parentheses(t) => lower_type(t.inner, ctx, log),
    }
}

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

fn get_type(t: &TypeId) -> &Type {
    t.deref()
}

// ===== lower_type (primitive types) =====

#[test]
fn lower_type_bool() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::Bool(ast::Bool), &mut ctx, &log);
    assert_eq!(r, Ok(Type::Bool));
}

#[test]
fn lower_type_u8() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::UInt8(ast::UInt8), &mut ctx, &log);
    assert_eq!(r, Ok(Type::U8));
}

#[test]
fn lower_type_u16() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::UInt16(ast::UInt16), &mut ctx, &log);
    assert_eq!(r, Ok(Type::U16));
}

#[test]
fn lower_type_u32() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::UInt32(ast::UInt32), &mut ctx, &log);
    assert_eq!(r, Ok(Type::U32));
}

#[test]
fn lower_type_u64() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::UInt64(ast::UInt64), &mut ctx, &log);
    assert_eq!(r, Ok(Type::U64));
}

#[test]
fn lower_type_u128() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::UInt128(ast::UInt128), &mut ctx, &log);
    assert_eq!(r, Ok(Type::U128));
}

#[test]
fn lower_type_usize() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::USize(ast::USize), &mut ctx, &log);
    assert_eq!(r, Ok(Type::USize));
}

#[test]
fn lower_type_i8() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::Int8(ast::Int8), &mut ctx, &log);
    assert_eq!(r, Ok(Type::I8));
}

#[test]
fn lower_type_i16() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::Int16(ast::Int16), &mut ctx, &log);
    assert_eq!(r, Ok(Type::I16));
}

#[test]
fn lower_type_i32() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::Int32(ast::Int32), &mut ctx, &log);
    assert_eq!(r, Ok(Type::I32));
}

#[test]
fn lower_type_i64() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::Int64(ast::Int64), &mut ctx, &log);
    assert_eq!(r, Ok(Type::I64));
}

#[test]
fn lower_type_i128() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::Int128(ast::Int128), &mut ctx, &log);
    assert_eq!(r, Ok(Type::I128));
}

#[test]
fn lower_type_f32() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::Float32(ast::Float32), &mut ctx, &log);
    assert_eq!(r, Ok(Type::F32));
}

#[test]
fn lower_type_f64() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::Float64(ast::Float64), &mut ctx, &log);
    assert_eq!(r, Ok(Type::F64));
}

#[test]
fn lower_type_infer() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::InferType(ast::InferType), &mut ctx, &log).unwrap();
    assert!(r.is_inferred());
}

#[test]
fn lower_type_syntax_error() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(ast::Type::SyntaxError(ast::TypeSyntaxError), &mut ctx, &log);
    assert!(r.is_err());
}

#[test]
fn lower_type_parentheses() {
    let (mut ctx, log) = ctx_and_log();
    let r = lower_type(
        ast::Type::Parentheses(Box::new(ast::TypeParentheses {
            inner: ast::Type::Bool(ast::Bool),
        })),
        &mut ctx,
        &log,
    );
    assert_eq!(r, Ok(Type::Bool));
}

// ===== lower_type_path =====

#[test]
fn lower_type_path_struct_resolved() {
    run(|ctx, log| {
        ctx.ast_symbol_map.insert("Foo".into(), SymbolKind::Struct);
        ctx.tab.get_struct_or_insert_placeholder(&"Foo".into());
        let tp = ast::TypePath {
            segments: vec![ast::TypePathSegment {
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
            segments: vec![ast::TypePathSegment {
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
            segments: vec![ast::TypePathSegment {
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
        segments: vec![ast::TypePathSegment {
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
        segments: vec![ast::TypePathSegment {
            name: "X".into(),
            type_arguments: None,
        }],
        resolved_path: Some("X".into()),
    };
    let r = lower_type_path(tp, &mut ctx, &log);
    assert!(r.is_err());
}

#[test]
fn lower_type_path_unresolved_path() {
    let (mut ctx, log) = ctx_and_log();
    let tp = ast::TypePath {
        segments: vec![ast::TypePathSegment {
            name: "X".into(),
            type_arguments: None,
        }],
        resolved_path: None,
    };
    let r = lower_type_path(tp, &mut ctx, &log);
    assert!(r.is_err());
}

#[test]
fn lower_type_path_intermediate_generics() {
    let (mut ctx, log) = ctx_and_log();
    ctx.ast_symbol_map.insert("Foo".into(), SymbolKind::Struct);
    ctx.ast_symbol_map.insert("Bar".into(), SymbolKind::Struct);
    let tp = ast::TypePath {
        segments: vec![
            ast::TypePathSegment {
                name: "Foo".into(),
                type_arguments: Some(vec![ast::TypeArgument {
                    name: None,
                    value: ast::Type::Int32(ast::Int32),
                }]),
            },
            ast::TypePathSegment {
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
            segments: vec![ast::TypePathSegment {
                name: "Vec".into(),
                type_arguments: Some(vec![ast::TypeArgument {
                    name: None,
                    value: ast::Type::Int32(ast::Int32),
                }]),
            }],
            resolved_path: Some("Vec".into()),
        };
        let r = lower_type_path(tp, ctx, log).unwrap();
        assert!(matches!(r, Type::Parameterized { .. }));
    })
}

#[test]
fn lower_type_path_unresolved_type_arg_skipped() {
    run(|ctx, log| {
        ctx.ast_symbol_map.insert("Map".into(), SymbolKind::Struct);
        ctx.tab.get_struct_or_insert_placeholder(&"Map".into());
        let tp = ast::TypePath {
            segments: vec![ast::TypePathSegment {
                name: "Map".into(),
                type_arguments: Some(vec![ast::TypeArgument {
                    name: None,
                    value: ast::Type::TypePath(Box::new(ast::TypePath {
                        segments: vec![ast::TypePathSegment {
                            name: "Missing".into(),
                            type_arguments: None,
                        }],
                        resolved_path: None,
                    })),
                }]),
            }],
            resolved_path: Some("Map".into()),
        };
        // Should not panic - should gracefully skip unresolved type args
        let r = lower_type_path(tp, ctx, log);
        // filtered_map removes failed items, so there are 0 valid type args
        assert!(r.is_ok());
    })
}

// ===== lower_tuple_type =====

#[test]
fn lower_tuple_empty_is_unit() {
    let (mut ctx, log) = ctx_and_log();
    let t = ast::TupleType { element_types: vec![] };
    assert_eq!(lower_tuple_type(t, &mut ctx, &log).unwrap(), Type::Unit);
}

#[test]
fn lower_tuple_single() {
    run(|ctx, log| {
        let t = ast::TupleType {
            element_types: vec![ast::Type::Bool(ast::Bool)],
        };
        let r = lower_tuple_type(t, ctx, log).unwrap();
        assert!(matches!(r, Type::Tuple { element_types } if element_types.len() == 1));
    })
}

#[test]
fn lower_tuple_multiple() {
    run(|ctx, log| {
        let t = ast::TupleType {
            element_types: vec![
                ast::Type::Bool(ast::Bool),
                ast::Type::Int32(ast::Int32),
                ast::Type::Float64(ast::Float64),
            ],
        };
        let r = lower_tuple_type(t, ctx, log).unwrap();
        match r {
            Type::Tuple { element_types } => {
                assert_eq!(element_types.len(), 3);
            }
            _ => panic!("expected tuple"),
        }
    })
}

#[test]
fn lower_tuple_nested() {
    run(|ctx, log| {
        let t = ast::TupleType {
            element_types: vec![
                ast::Type::Int32(ast::Int32),
                ast::Type::TupleType(Box::new(ast::TupleType {
                    element_types: vec![ast::Type::Float64(ast::Float64)],
                })),
            ],
        };
        let r = lower_tuple_type(t, ctx, log).unwrap();
        match r {
            Type::Tuple { element_types } => {
                assert_eq!(element_types.len(), 2);
                assert!(element_types[1].deref().is_tuple());
            }
            _ => panic!("expected tuple"),
        }
    })
}

// ===== lower_function_type =====

#[test]
fn lower_fn_type_empty() {
    run(|ctx, log| {
        let ft = ast::FunctionType {
            attributes: None,
            parameters: vec![],
            return_type: None,
        };
        let r = lower_function_type(ft, ctx, log).unwrap();
        match r {
            Type::Function { function_type } => {
                assert!(function_type.params.is_empty());
                assert!(matches!(function_type.return_type.deref(), Type::Unit));
            }
            _ => panic!("expected function type"),
        }
    })
}

#[test]
fn lower_fn_type_with_params() {
    run(|ctx, log| {
        let ft = ast::FunctionType {
            attributes: None,
            parameters: vec![
                ast::FuncTypeParam {
                    attributes: None,
                    name: "a".into(),
                    ty: ast::Type::Int32(ast::Int32),
                },
                ast::FuncTypeParam {
                    attributes: None,
                    name: "b".into(),
                    ty: ast::Type::Bool(ast::Bool),
                },
            ],
            return_type: Some(ast::Type::Float64(ast::Float64)),
        };
        let r = lower_function_type(ft, ctx, log).unwrap();
        match r {
            Type::Function { function_type } => {
                assert_eq!(function_type.params.len(), 2);
                assert_eq!(function_type.params[0].0.deref(), "a");
                assert!(function_type.params[1].1.deref().is_bool());
                assert!(function_type.return_type.deref().is_float_primitive());
            }
            _ => panic!("expected function type"),
        }
    })
}

#[test]
fn lower_fn_type_attr_error() {
    run(|ctx, log| {
        let ft = ast::FunctionType {
            attributes: Some(vec![ast::Expr::Path(Box::new(ast::ExprPath {
                segments: vec![ast::ExprPathSegment {
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
fn lower_fn_type_param_attr_error() {
    run(|ctx, log| {
        let ft = ast::FunctionType {
            attributes: None,
            parameters: vec![ast::FuncTypeParam {
                attributes: Some(vec![ast::Expr::Boolean(ast::BooleanLit { value: true })]),
                name: "x".into(),
                ty: ast::Type::Int32(ast::Int32),
            }],
            return_type: None,
        };
        assert!(lower_function_type(ft, ctx, log).is_ok());
        assert!(log.error_bit());
    })
}

// ===== lower_reference_type =====

#[test]
fn lower_ref_default() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: None,
            exclusivity: None,
            mutability: None,
            to: ast::Type::Int32(ast::Int32),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        match r {
            Type::Reference {
                lifetime: Lifetime::Inferred,
                mutable: false,
                exclusive: false,
                to,
            } => {
                assert!(matches!(to.deref(), Type::I32));
            }
            _ => panic!("expected &i32"),
        }
    })
}

#[test]
fn lower_ref_mut() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: None,
            exclusivity: None,
            mutability: Some(ast::Mutability::Mut),
            to: ast::Type::Bool(ast::Bool),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        match r {
            Type::Reference { mutable, exclusive, .. } => {
                assert!(mutable);
                assert!(exclusive);
            }
            _ => panic!("expected &mut bool"),
        }
    })
}

#[test]
fn lower_ref_iso() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: None,
            exclusivity: Some(ast::Exclusivity::Iso),
            mutability: None,
            to: ast::Type::Float64(ast::Float64),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        match r {
            Type::Reference { exclusive, .. } => assert!(exclusive),
            _ => panic!("expected iso &"),
        }
    })
}

#[test]
fn lower_ref_poly() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: None,
            exclusivity: Some(ast::Exclusivity::Poly),
            mutability: Some(ast::Mutability::Mut),
            to: ast::Type::Int32(ast::Int32),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        match r {
            Type::Reference { exclusive, mutable, .. } => {
                assert!(!exclusive);
                assert!(mutable);
            }
            _ => panic!("expected poly &mut"),
        }
    })
}

#[test]
fn lower_ref_lifetime_static() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: Some(ast::Lifetime { name: "static".into() }),
            exclusivity: None,
            mutability: None,
            to: ast::Type::Int32(ast::Int32),
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
fn lower_ref_lifetime_gc() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: Some(ast::Lifetime { name: "gc".into() }),
            exclusivity: None,
            mutability: None,
            to: ast::Type::Int32(ast::Int32),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::Gc,
                ..
            }
        ));
    })
}

#[test]
fn lower_ref_lifetime_thread() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: Some(ast::Lifetime { name: "thread".into() }),
            exclusivity: None,
            mutability: None,
            to: ast::Type::Int32(ast::Int32),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::ThreadLocal,
                ..
            }
        ));
    })
}

#[test]
fn lower_ref_lifetime_task() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: Some(ast::Lifetime { name: "task".into() }),
            exclusivity: None,
            mutability: None,
            to: ast::Type::Int32(ast::Int32),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::TaskLocal,
                ..
            }
        ));
    })
}

#[test]
fn lower_ref_lifetime_underscore() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: Some(ast::Lifetime { name: "_".into() }),
            exclusivity: None,
            mutability: None,
            to: ast::Type::Int32(ast::Int32),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        assert!(matches!(
            r,
            Type::Reference {
                lifetime: Lifetime::Inferred,
                ..
            }
        ));
    })
}

#[test]
fn lower_ref_bad_lifetime() {
    let (mut ctx, log) = ctx_and_log();
    let rt = ast::ReferenceType {
        lifetime: Some(ast::Lifetime { name: "invalid".into() }),
        exclusivity: None,
        mutability: None,
        to: ast::Type::Int32(ast::Int32),
    };
    assert!(lower_reference_type(rt, &mut ctx, &log).is_err());
}

#[test]
fn lower_ref_to_slice() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: Some(ast::Lifetime { name: "static".into() }),
            exclusivity: None,
            mutability: None,
            to: ast::Type::SliceType(Box::new(ast::SliceType {
                element_type: ast::Type::Int32(ast::Int32),
            })),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        assert!(matches!(
            r,
            Type::SliceRef {
                lifetime: Lifetime::Static,
                ..
            }
        ));
    })
}

#[test]
fn lower_ref_to_slice_mut() {
    run(|ctx, log| {
        let rt = ast::ReferenceType {
            lifetime: None,
            exclusivity: None,
            mutability: Some(ast::Mutability::Mut),
            to: ast::Type::SliceType(Box::new(ast::SliceType {
                element_type: ast::Type::UInt8(ast::UInt8),
            })),
        };
        let r = lower_reference_type(rt, ctx, log).unwrap();
        assert!(matches!(
            r,
            Type::SliceRef {
                mutable: true,
                exclusive: true,
                ..
            }
        ));
    })
}

// ===== lower_pointer_type =====

#[test]
fn lower_ptr_default() {
    run(|ctx, log| {
        let pt = ast::PointerType {
            lifetime: None,
            exclusivity: None,
            mutability: None,
            to: ast::Type::Int32(ast::Int32),
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
            lifetime: None,
            exclusivity: None,
            mutability: Some(ast::Mutability::Mut),
            to: ast::Type::Bool(ast::Bool),
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
fn lower_ptr_iso() {
    run(|ctx, log| {
        let pt = ast::PointerType {
            lifetime: None,
            exclusivity: Some(ast::Exclusivity::Iso),
            mutability: None,
            to: ast::Type::UInt64(ast::UInt64),
        };
        let r = lower_pointer_type(pt, ctx, log).unwrap();
        assert!(matches!(r, Type::Pointer { exclusive: true, .. }));
    })
}

#[test]
fn lower_ptr_poly() {
    run(|ctx, log| {
        let pt = ast::PointerType {
            lifetime: None,
            exclusivity: Some(ast::Exclusivity::Poly),
            mutability: None,
            to: ast::Type::Float32(ast::Float32),
        };
        let r = lower_pointer_type(pt, ctx, log).unwrap();
        assert!(matches!(r, Type::Pointer { exclusive: false, .. }));
    })
}

#[test]
fn lower_ptr_poly_mut() {
    run(|ctx, log| {
        let pt = ast::PointerType {
            lifetime: None,
            exclusivity: Some(ast::Exclusivity::Poly),
            mutability: Some(ast::Mutability::Mut),
            to: ast::Type::UInt8(ast::UInt8),
        };
        let r = lower_pointer_type(pt, ctx, log).unwrap();
        assert!(matches!(
            r,
            Type::Pointer {
                mutable: true,
                exclusive: false,
                ..
            }
        ));
    })
}

#[test]
fn lower_ptr_to_slice() {
    run(|ctx, log| {
        let pt = ast::PointerType {
            lifetime: None,
            exclusivity: None,
            mutability: None,
            to: ast::Type::SliceType(Box::new(ast::SliceType {
                element_type: ast::Type::Int32(ast::Int32),
            })),
        };
        let r = lower_pointer_type(pt, ctx, log).unwrap();
        assert!(matches!(r, Type::SlicePtr { .. }));
    })
}

// ===== Error-returning type functions =====

#[test]
fn lower_slice_outside_ref_ptr_fails() {
    let (mut ctx, log) = ctx_and_log();
    assert!(
        lower_slice_type(
            ast::SliceType {
                element_type: ast::Type::Int32(ast::Int32)
            },
            &mut ctx,
            &log
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
                basis_type: ast::Type::Int32(ast::Int32),
                width: None,
                minimum: None,
                maximum: None,
            },
            &mut ctx,
            &log
        )
        .is_err()
    );
}

#[test]
fn lower_latent_type_fails() {
    let (mut ctx, log) = ctx_and_log();
    assert!(
        lower_latent_type(
            ast::LatentType {
                body: ast::Block {
                    safety: None,
                    elements: vec![]
                }
            },
            &mut ctx,
            &log
        )
        .is_err()
    );
}

#[test]
fn lower_lifetime_as_type_fails() {
    let (mut ctx, log) = ctx_and_log();
    assert!(lower_lifetime(ast::Lifetime { name: "static".into() }, &mut ctx, &log).is_err());
}

// ===== lower_array_type =====

#[test]
fn lower_array_i32_5() {
    run(|ctx, log| {
        let a = ast::ArrayType {
            element_type: ast::Type::Int32(ast::Int32),
            len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 5,
                kind: IntegerKind::Dec,
            })),
        };
        let r = lower_array_type(a, ctx, log).unwrap();
        match r {
            Type::Array { element_type, len } => {
                assert!(matches!(element_type.deref(), Type::I32));
                assert_eq!(len, 5);
            }
            _ => panic!("expected array"),
        }
    })
}

#[test]
fn lower_array_zero() {
    run(|ctx, log| {
        let a = ast::ArrayType {
            element_type: ast::Type::UInt8(ast::UInt8),
            len: ast::Expr::Integer(Box::new(ast::IntegerLit {
                value: 0,
                kind: IntegerKind::Dec,
            })),
        };
        let r = lower_array_type(a, ctx, log).unwrap();
        assert_eq!(
            r,
            Type::Array {
                element_type: Type::U8.into(),
                len: 0
            }
        );
    })
}

#[test]
fn lower_array_string_len_fails() {
    run(|ctx, log| {
        let a = ast::ArrayType {
            element_type: ast::Type::Int32(ast::Int32),
            len: ast::Expr::String(ast::StringLit { value: "bad".into() }),
        };
        assert!(lower_array_type(a, ctx, log).is_err());
    })
}
