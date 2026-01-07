use crate::{context::Ast2HirCtx, diagnosis::HirErr, expr::lower_expr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_evaluate::HirEvalCtx;
use nitrate_nstring::NString;
use nitrate_tree::ast::{self as ast, SymbolKind};
use std::{collections::BTreeSet, ops::Deref};

fn lower_infer_type(ctx: &mut Ast2HirCtx) -> Type {
    ctx.create_inference_placeholder()
}

fn lower_type_path(type_path: ast::TypePath, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    // TODO: Validate implementation

    if type_path.segments.iter().any(|seg| seg.type_arguments.is_some()) {
        log.report(&HirErr::UnimplementedFeature(
            "generic type arguments in type paths".into(),
        ));
    }

    match type_path.resolved_path {
        Some(resolved_path) => match ctx.ast_symbol_map.get(&resolved_path) {
            Some(SymbolKind::Struct) => Ok(Type::Struct {
                def: ctx.tab.get_struct_or_insert_placeholder(&resolved_path).clone(),
            }),

            Some(SymbolKind::Enum) => Ok(Type::Enum {
                def: ctx.tab.get_enum_or_insert_placeholder(&resolved_path).clone(),
            }),

            Some(SymbolKind::TypeAlias) => Ok(Type::TypeAlias {
                def: ctx.tab.get_type_alias_or_insert_placeholder(&resolved_path).clone(),
            }),

            _ => {
                log.report(&HirErr::UnresolvedSymbol);
                Err(())
            }
        },

        None => {
            log.report(&HirErr::UnresolvedTypePath);
            Err(())
        }
    }
}

fn lower_refinement_type(
    _refinement_type: ast::RefinementType,
    _ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    log.report(&HirErr::UnimplementedFeature("refinement types".into()));
    Err(())
}

fn lower_tuple_type(tuple_type: ast::TupleType, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    // TODO: Validate implementation

    if tuple_type.element_types.is_empty() {
        return Ok(Type::Unit);
    }

    let mut elements = Vec::with_capacity(tuple_type.element_types.len());

    for ast_elem_ty in tuple_type.element_types.into_iter() {
        let hir_elem_ty = lower_type(ast_elem_ty, ctx, log)?.into();
        elements.push(hir_elem_ty);
    }

    Ok(Type::Tuple {
        element_types: elements.into(),
    })
}

fn lower_array_type(array_type: ast::ArrayType, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    // TODO: Validate implementation

    let element_type = lower_type(array_type.element_type, ctx, log)?.into();

    let hir_length = Value::Cast {
        value: lower_expr(array_type.len, ctx, log)?.into(),
        target_type: Type::USize.into(),
    };

    let mut eval = HirEvalCtx::new(log, ctx.ptr_size);
    let len = match eval.evaluate_to_literal(&hir_length) {
        Ok(Lit::USize32(val)) => {
            if ctx.ptr_size != PtrSize::U32 {
                log.report(&HirErr::FoundUSize32InNon32BitTarget);
                return Err(());
            }

            val
        }

        Ok(Lit::USize64(val)) => {
            if ctx.ptr_size != PtrSize::U64 {
                log.report(&HirErr::FoundUSize64InNon64BitTarget);
                return Err(());
            }

            val as u32
        }

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

fn lower_function_type(function_type: ast::FunctionType, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    // TODO: Validate implementation

    let ast_attributes = function_type.attributes.unwrap_or_default();

    let attributes = BTreeSet::new();
    for _attr in ast_attributes {
        log.report(&HirErr::UnrecognizedFunctionAttribute);
    }

    let mut parameters = Vec::with_capacity(function_type.parameters.len());
    for param in function_type.parameters {
        // let attributes = BTreeSet::new();
        if let Some(ast_attributes) = &param.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedFunctionParameterAttribute);
            }
        }

        let name = NString::from(param.name.deref());
        let ty = lower_type(param.ty, ctx, log)?.into();

        parameters.push((name, ty));
    }

    let return_type = match function_type.return_type {
        Some(ret_ty) => lower_type(ret_ty, ctx, log)?,
        None => Type::Unit,
    };

    let function_type = FunctionType {
        attributes,
        params: parameters.into(),
        return_type: return_type.into(),
    };

    Ok(Type::Function {
        function_type: function_type.into(),
    })
}

fn lower_reference_type(
    reference_type: ast::ReferenceType,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Type, ()> {
    // TODO: Validate implementation

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

fn lower_slice_type(_slice_type: ast::SliceType, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    log.report(&HirErr::SliceTypesCannotExistOutsideReferencesOrPointers);
    Err(())
}

fn lower_pointer_type(pointer_type: ast::PointerType, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    // TODO: Validate implementation

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
        let _element_type: TypeId = lower_type(slice.element_type, ctx, log)?.into();

        log.report(&HirErr::UnimplementedFeature("slice pointers".into()));
        Err(())
    } else {
        let to = lower_type(pointer_type.to, ctx, log)?.into();

        Ok(Type::Pointer { exclusive, mutable, to })
    }
}

fn lower_latent_type(_latent_type: ast::LatentType, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    log.report(&HirErr::UnimplementedFeature("latent types".into()));
    Err(())
}

fn lower_lifetime(_lifetime: ast::Lifetime, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    log.report(&HirErr::UnimplementedFeature("lifetimes".into()));
    Err(())
}

pub fn lower_type(ty: ast::Type, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Type, ()> {
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
        ast::Type::InferType(_) => Ok(lower_infer_type(ctx)),
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
