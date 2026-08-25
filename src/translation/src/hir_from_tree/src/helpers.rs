//! Shared helper functions for HIR lowering patterns.
//!
//! This module consolidates common patterns used across the HIR lowering
//! pipeline (expression lowering, type lowering, and item lowering) to
//! eliminate code duplication and simplify the "god functions" that existed
//! previously.

use crate::context::Ast2HirCtx;
use crate::diagnosis::HirErr;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ast::{self, SymbolKind};
use nitrate_tree::{SrcPos, SrcSpan};
use std::collections::{BTreeMap, BTreeSet};
use std::ops::Deref;

// ═══════════════════════════════════════════════════════════════════════════
// Visibility Helpers
// ═══════════════════════════════════════════════════════════════════════════

/// Convert an AST visibility modifier into a HIR `Visibility`.
pub(crate) fn lower_visibility(visibility: Option<ast::Visibility>) -> Visibility {
    match visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Attribute Helpers
// ═══════════════════════════════════════════════════════════════════════════

/// Log an error for each unrecognized attribute on an item.
/// Used by item definitions that don't support custom attributes.
///
/// `err_ctor` is a function that takes `(name: String, span: SrcPos)` and
/// returns a `HirErr` variant with the span populated.
pub(crate) fn reject_all_attributes(
    ast_attributes: &Option<Vec<ast::Expr>>,
    err_ctor: fn(String, SrcPos) -> HirErr,
    log: &CompilerLog,
) {
    if let Some(attrs) = ast_attributes {
        for attr in attrs {
            let (attr_name, attr_span) = extract_attr_name_and_span(attr);
            log.report(&err_ctor(attr_name, attr_span));
        }
    }
}

/// Extract a display-friendly attribute name and its span from an AST expression.
fn extract_attr_name_and_span(attr: &ast::Expr) -> (String, SrcPos) {
    let name = match attr {
        ast::Expr::Path(p) => p
            .segments
            .iter()
            .map(|s| s.name.to_string())
            .collect::<Vec<_>>()
            .join("::"),
        ast::Expr::Boolean(_) => "#[true]".to_string(),
        ast::Expr::Integer(_) => "#[<integer>]".to_string(),
        ast::Expr::String(s) => format!("#[\"{}\"]", s.value),
        _ => "#[<unknown>]".to_string(),
    };
    (name, attr.span().start)
}

// We keep the old extract_attr_name for parse_function_attributes which doesn't need span
/// Extract a display-friendly attribute name from an AST expression.
fn extract_attr_name(attr: &ast::Expr) -> String {
    match attr {
        ast::Expr::Path(p) => p
            .segments
            .iter()
            .map(|s| s.name.to_string())
            .collect::<Vec<_>>()
            .join("::"),
        ast::Expr::Boolean(_) => "#[true]".to_string(),
        ast::Expr::Integer(_) => "#[<integer>]".to_string(),
        ast::Expr::String(s) => format!("#[\"{}\"]", s.value),
        _ => "#[<unknown>]".to_string(),
    }
}

/// Parse function attributes from a list of AST attribute expressions.
/// Recognizes `no_mangle`, `extern(abi)`, etc. Unknown attributes are reported as errors.
pub(crate) fn parse_function_attributes(
    ast_attributes: &Option<Vec<ast::Expr>>,
    log: &CompilerLog,
) -> BTreeSet<FunctionAttribute> {
    let mut attributes = BTreeSet::new();

    if let Some(attrs) = ast_attributes {
        for attr in attrs {
            let attr_span: SrcPos = attr.span().start;
            if let ast::Expr::Path(path) = attr {
                let ident = path
                    .segments
                    .iter()
                    .map(|seg| seg.name.to_string())
                    .collect::<Vec<_>>()
                    .join("::");

                match ident.as_str() {
                    "no_mangle" => {
                        attributes.insert(FunctionAttribute::NoMangle);
                        continue;
                    }
                    _ => {
                        log.report(&HirErr::UnrecognizedFunctionAttribute {
                            span: attr_span,
                            name: ident,
                        });
                        continue;
                    }
                }
            }

            log.report(&HirErr::UnrecognizedFunctionAttribute {
                span: attr_span,
                name: "#[<unknown>]".into(),
            });
        }
    }

    attributes
}

// ═══════════════════════════════════════════════════════════════════════════
// Duplicate Detection Helpers
// ═══════════════════════════════════════════════════════════════════════════

/// Check if an entity name has already been added in this scope.
/// Returns `Ok(())` if unique, or logs a `DuplicateEntity` error and returns `Err(())`.
pub(crate) fn check_duplicate(name: &NString, ctx: &Ast2HirCtx, log: &CompilerLog) -> Result<(), ()> {
    if ctx.entities_added.contains(name) {
        log.report(&HirErr::DuplicateEntity {
            span: SrcPos::default(),
            name: name.to_string(),
        });
        return Err(());
    }
    Ok(())
}

// ═══════════════════════════════════════════════════════════════════════════
// Mutability Helpers
// ═══════════════════════════════════════════════════════════════════════════

/// Convert an AST mutability to a boolean (true = mutable).
pub(crate) fn is_mutable(mutability: Option<ast::Mutability>) -> bool {
    match mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    }
}

/// Convert an AST exclusivity + mutability pair to boolean flags.
/// Defaults: if exclusivity is None, it defaults to mutable.
pub(crate) fn lower_exclusivity(exclusivity: Option<ast::Exclusivity>, mutable: bool) -> bool {
    match exclusivity {
        Some(ast::Exclusivity::Iso) => true,
        Some(ast::Exclusivity::Poly) => false,
        None => mutable,
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Generic Parameter Helpers
// ═══════════════════════════════════════════════════════════════════════════

/// Lower a list of generic parameter declarations from the AST into the
/// `BTreeMap<NString, Option<TypeId>>` format used by HIR definitions.
pub(crate) fn lower_generic_params(
    generic_params: Option<ast::Generics>,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<Option<BTreeMap<NString, Option<TypeId>>>, ()> {
    let Some(params) = generic_params else {
        return Ok(None);
    };

    let mut generics_map = BTreeMap::new();
    for (i, parameter) in params.params.iter().enumerate() {
        let generic_name: NString = parameter.name.to_string().into();
        let _generic_type: TypeId = Type::GenericParam {
            span: SrcPos::default(),
            index: i as u32,
            name: generic_name.clone(),
        }
        .into();

        let default_type = match &parameter.default_value {
            Some(ty) => Some(crate::ty::lower_type(ty.to_owned(), ctx, log)?.into()),
            None => None,
        };
        generics_map.insert(generic_name, default_type);
    }

    Ok(Some(generics_map))
}

// ═══════════════════════════════════════════════════════════════════════════
// Type Argument Extraction
// ═══════════════════════════════════════════════════════════════════════════

/// Extract type arguments from a path segment's type_arguments field.
/// Filters out any type arguments that fail to lower (silently skipping them).
pub(crate) fn extract_type_args(
    type_arguments: &Option<Vec<ast::TypeArgument>>,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Vec<TypeId> {
    let Some(args) = type_arguments else {
        return Vec::new();
    };

    args.iter()
        .filter_map(|type_arg| {
            crate::ty::lower_type(type_arg.value.clone(), ctx, log).ok().map(|t| {
                let hir_type_arg: TypeId = t.into();
                hir_type_arg
            })
        })
        .collect()
}

/// Extract type arguments from expression path segments.
pub(crate) fn extract_type_args_from_expr_path(
    segments: &[ast::ExprPathSegment],
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Option<Vec<TypeId>> {
    let has_type_args = segments.iter().any(|seg| seg.type_arguments.is_some());
    if !has_type_args {
        return None;
    }

    let args: Vec<TypeId> = segments
        .iter()
        .filter_map(|seg| {
            seg.type_arguments.as_ref().map(|type_args| {
                type_args
                    .iter()
                    .filter_map(|type_arg| {
                        crate::ty::lower_type(type_arg.value.clone(), ctx, log).ok().map(|t| {
                            let hir_type_arg: TypeId = t.into();
                            hir_type_arg
                        })
                    })
                    .collect::<Vec<_>>()
            })
        })
        .flatten()
        .collect();

    if args.is_empty() { None } else { Some(args) }
}

// ═══════════════════════════════════════════════════════════════════════════
// Lifetime Lowering
// ═══════════════════════════════════════════════════════════════════════════

/// Lower an AST lifetime into a HIR `Lifetime`.
/// Returns the lifetime, or logs an error and returns `Err(())` if unrecognized.
pub(crate) fn lower_lifetime(lifetime: Option<ast::Lifetime>, log: &CompilerLog) -> Result<Lifetime, ()> {
    match lifetime {
        None => Ok(Lifetime::Inferred),
        Some(ast::Lifetime { span: _, name }) => match name.deref() {
            "static" => Ok(Lifetime::Static),
            "gc" => Ok(Lifetime::Gc),
            "thread" => Ok(Lifetime::ThreadLocal),
            "task" => Ok(Lifetime::TaskLocal),
            "_" => Ok(Lifetime::Inferred),
            _ => {
                log.report(&HirErr::UnrecognizedLifetime {
                    span: SrcPos::default(),
                    name: name.to_string(),
                });
                Err(())
            }
        },
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Qualified Name Building
// ═══════════════════════════════════════════════════════════════════════════

/// Build a qualified name from the current scope and an item name.
/// E.g., `qualify(ctx, "foo")` might produce `"mymodule::foo"`.
pub(crate) fn qualify(name: &str, ctx: &Ast2HirCtx) -> NString {
    ctx.qualify_name(name).into()
}

// ═══════════════════════════════════════════════════════════════════════════
// Symbol Table Upsert Pattern
// ═══════════════════════════════════════════════════════════════════════════

/// Generic pattern for "get or insert, then update" in the symbol table.
/// Returns an entity ID. If the entity already exists, it's updated in place.
pub(crate) fn upsert_struct(struct_def: StructDef, ctx: &mut Ast2HirCtx) -> StructDefId {
    let name = struct_def.name.clone();
    ctx.entities_added.insert(name.clone());

    if let Some(existing_id) = ctx.tab.get_struct(&name) {
        let mut existing = existing_id.borrow_mut();
        *existing = struct_def;
        existing_id.clone()
    } else {
        let id: StructDefId = struct_def.into();
        ctx.tab.add_struct(id.clone());
        id
    }
}

pub(crate) fn upsert_enum(enum_def: EnumDef, variants: Vec<EnumVariant>, ctx: &mut Ast2HirCtx) -> EnumDefId {
    let name = enum_def.name.clone();
    ctx.entities_added.insert(name.clone());

    let enum_def_id = if let Some(existing_id) = ctx.tab.get_enum(&name) {
        let mut existing = existing_id.borrow_mut();
        *existing = enum_def;
        existing_id.clone()
    } else {
        let id: EnumDefId = enum_def.into();
        ctx.tab.add_enum(id.clone());
        id
    };

    // Register enum variants
    for variant in variants {
        let variant_name: NString = format!("{}::{}", name, variant.name).into();
        ctx.tab.add_enum_variant(variant_name, enum_def_id.clone());
    }

    enum_def_id
}

pub(crate) fn upsert_type_alias(type_alias: TypeAliasDef, ctx: &mut Ast2HirCtx) -> TypeAliasDefId {
    let name = type_alias.name.clone();
    ctx.entities_added.insert(name.clone());

    if let Some(existing_id) = ctx.tab.get_type_alias(&name) {
        let mut existing = existing_id.borrow_mut();
        *existing = type_alias;
        existing_id.clone()
    } else {
        let id: TypeAliasDefId = type_alias.into();
        ctx.tab.add_type_alias(id.clone());
        id
    }
}

pub(crate) fn upsert_function(function: Function, ctx: &mut Ast2HirCtx) -> FunctionId {
    let name = function.name.clone();
    ctx.entities_added.insert(name.clone());

    if let Some(existing_id) = ctx.tab.get_function(&name) {
        let mut existing = existing_id.borrow_mut();
        *existing = function;
        existing_id.clone()
    } else {
        let id: FunctionId = function.into();
        ctx.tab.add_function(id.clone());
        id
    }
}

pub(crate) fn upsert_global_variable(global: GlobalVariable, ctx: &mut Ast2HirCtx) -> GlobalVariableId {
    let name = global.name.clone();
    ctx.entities_added.insert(name.clone());

    if let Some(existing_id) = ctx.tab.get_global_variable(&name) {
        let mut existing = existing_id.borrow_mut();
        *existing = global;
        existing_id.clone()
    } else {
        let id: GlobalVariableId = global.into();
        ctx.tab.add_global_variable(id.clone());
        id
    }
}

pub(crate) fn upsert_trait(trait_: Trait, ctx: &mut Ast2HirCtx) -> TraitId {
    let name = trait_.name.clone();
    ctx.entities_added.insert(name.clone());

    if let Some(existing_id) = ctx.tab.get_trait(&name) {
        let mut existing = existing_id.borrow_mut();
        *existing = trait_;
        existing_id.clone()
    } else {
        let id: TraitId = trait_.into();
        ctx.tab.add_trait(id.clone());
        id
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Scope Management
// ═══════════════════════════════════════════════════════════════════════════

/// Temporarily push a scope name, execute a function, then pop the scope.
pub(crate) fn with_scope<T>(
    ctx: &mut Ast2HirCtx,
    scope_name: impl Into<NString>,
    f: impl FnOnce(&mut Ast2HirCtx) -> T,
) -> T {
    let name: NString = scope_name.into();
    ctx.current_scope.push(name.to_string().into());
    let result = f(ctx);
    ctx.current_scope.pop();
    result
}

/// Temporarily set `current_self_type`, execute a function, then restore it.
pub(crate) fn with_self_type<T>(
    ctx: &mut Ast2HirCtx,
    self_type: Option<TypeId>,
    f: impl FnOnce(&mut Ast2HirCtx) -> T,
) -> T {
    let prev = ctx.current_self_type.take();
    ctx.current_self_type = self_type;
    let result = f(ctx);
    ctx.current_self_type = prev;
    result
}
