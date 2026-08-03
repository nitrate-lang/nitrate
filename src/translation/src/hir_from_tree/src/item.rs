use crate::{
    context::Ast2HirCtx,
    convert_ast_to_hir,
    diagnosis::HirErr,
    expr::{lower_block, lower_expr},
    helpers,
    ty::lower_type,
};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::SrcPos;
use nitrate_tree::SrcSpan;
use nitrate_tree::ast::{self};
use std::collections::{BTreeMap, BTreeSet};

// Helper closures for reject_all_attributes — signature: fn(String, SrcPos) -> HirErr
const ATTR_MODULE: fn(String, SrcPos) -> HirErr = |name, span| HirErr::UnrecognizedModuleAttribute { span, name };
const ATTR_GLOBAL_VAR: fn(String, SrcPos) -> HirErr =
    |name, span| HirErr::UnrecognizedGlobalVarAttribute { span, name };
const ATTR_FUNCTION: fn(String, SrcPos) -> HirErr = |name, span| HirErr::UnrecognizedFunctionAttribute { span, name };
const ATTR_FUNC_PARAM: fn(String, SrcPos) -> HirErr =
    |name, span| HirErr::UnrecognizedFunctionParamAttribute { span, name };
const ATTR_TYPE_ALIAS: fn(String, SrcPos) -> HirErr =
    |name, span| HirErr::UnrecognizedTypeAliasAttribute { span, name };
const ATTR_STRUCT: fn(String, SrcPos) -> HirErr = |name, span| HirErr::UnrecognizedStructAttribute { span, name };
const ATTR_STRUCT_FIELD: fn(String, SrcPos) -> HirErr =
    |name, span| HirErr::UnrecognizedStructFieldAttribute { span, name };
const ATTR_ENUM: fn(String, SrcPos) -> HirErr = |name, span| HirErr::UnrecognizedEnumAttribute { span, name };
const ATTR_ENUM_VARIANT: fn(String, SrcPos) -> HirErr =
    |name, span| HirErr::UnrecognizedEnumVariantAttribute { span, name };
const ATTR_LOCAL_VAR: fn(String, SrcPos) -> HirErr = |name, span| HirErr::UnrecognizedLocalVarAttribute { span, name };
const ATTR_TRAIT: fn(String, SrcPos) -> HirErr = |name, span| HirErr::UnrecognizedTraitAttribute { span, name };

// ═══════════════════════════════════════════════════════════════════════════
// Type Alias Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_type_alias(type_alias: ast::TypeAlias, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<TypeAliasDefId, ()> {
    let span: SrcPos = type_alias.span.start;
    let visibility = helpers::lower_visibility(type_alias.visibility);
    helpers::reject_all_attributes(&type_alias.attributes, ATTR_TYPE_ALIAS, log);

    let name = helpers::qualify(&type_alias.name, ctx);
    helpers::check_duplicate(&name, ctx, log)?;

    let generics = helpers::lower_generic_params(type_alias.generics, ctx, log)?;

    let type_id = match type_alias.alias_type {
        Some(ty) => lower_type(ty, ctx, log)?.into(),
        None => {
            log.report(&HirErr::TypeAliasMustHaveType {
                span,
                name: name.to_string(),
            });
            return Err(());
        }
    };

    let type_alias_def = TypeAliasDef {
        span,
        visibility,
        name,
        generics,
        type_id,
    };

    Ok(helpers::upsert_type_alias(type_alias_def, ctx))
}

// ═══════════════════════════════════════════════════════════════════════════
// Struct Definition Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_struct_definition(
    struct_def: ast::Struct,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<StructDefId, ()> {
    let span: SrcPos = struct_def.span.start;
    let visibility = helpers::lower_visibility(struct_def.visibility);
    helpers::reject_all_attributes(&struct_def.attributes, ATTR_STRUCT, log);

    let name = helpers::qualify(&struct_def.name, ctx);
    helpers::check_duplicate(&name, ctx, log)?;

    let generics = helpers::lower_generic_params(struct_def.generics, ctx, log)?;

    let mut fields = BTreeMap::new();
    let mut layout = StructLayout::new();

    for field in &struct_def.fields {
        let field_visibility = helpers::lower_visibility(field.visibility);
        helpers::reject_all_attributes(&field.attributes, ATTR_STRUCT_FIELD, log);

        let field_name: NString = field.name.to_string().into();
        let field_type = lower_type(field.ty.to_owned(), ctx, log)?.into();

        let default_value = match field.default_value.to_owned() {
            Some(expr) => Some(lower_expr(expr, ctx, log)?.into()),
            None => None,
        };

        let struct_field = StructField {
            span: field.span.start,
            visibility: field_visibility,
            attributes: BTreeSet::new(),
            name: field_name,
            ty: field_type,
            default_value,
        };

        let field_name = struct_field.name.clone();
        fields.insert(field_name.clone(), struct_field);
        layout.push(StructMemoryLayoutCell::Field { field_name });
    }

    let struct_def = StructDef {
        span,
        visibility,
        name,
        attributes: BTreeSet::new(),
        generics,
        fields,
        layout,
    };

    Ok(helpers::upsert_struct(struct_def, ctx))
}

// ═══════════════════════════════════════════════════════════════════════════
// Enum Definition Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_enum_definition(enum_def: ast::Enum, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<EnumDefId, ()> {
    let span: SrcPos = enum_def.span.start;
    let visibility = helpers::lower_visibility(enum_def.visibility);
    helpers::reject_all_attributes(&enum_def.attributes, ATTR_ENUM, log);

    let name: NString = helpers::qualify(&enum_def.name, ctx);
    helpers::check_duplicate(&name, ctx, log)?;

    let generics = helpers::lower_generic_params(enum_def.generics, ctx, log)?;

    let mut variants = Vec::new();

    for variant in &enum_def.variants {
        helpers::reject_all_attributes(&variant.attributes, ATTR_ENUM_VARIANT, log);

        let variant_name: NString = variant.name.to_string().into();

        let variant_type = match variant.ty.to_owned() {
            Some(ty) => lower_type(ty, ctx, log)?.into(),
            None => Type::Unit {
                span: SrcPos::default(),
            }
            .into(),
        };

        let field_default = match variant.default_value.to_owned() {
            Some(expr) => Some(lower_expr(expr, ctx, log)?.into()),
            None => None,
        };

        let variant = EnumVariant {
            span: variant.span.start,
            attributes: BTreeSet::new(),
            name: variant_name,
            ty: variant_type,
            default_value: field_default,
        };

        variants.push(variant);
    }

    let enum_def = EnumDef {
        span,
        visibility,
        name: name.clone(),
        attributes: BTreeSet::new(),
        generics,
        variants: variants.clone().into(),
    };

    Ok(helpers::upsert_enum(enum_def, variants, ctx))
}

// ═══════════════════════════════════════════════════════════════════════════
// Trait Definition Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_trait_definition(trait_: &ast::Trait, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<TraitId, ()> {
    let span: SrcPos = trait_.span.start;
    let visibility = helpers::lower_visibility(trait_.visibility);
    helpers::reject_all_attributes(&trait_.attributes, ATTR_TRAIT, log);

    let name: NString = helpers::qualify(&trait_.name, ctx);
    helpers::check_duplicate(&name, ctx, log)?;

    // Push trait name as scope so methods get properly qualified names
    ctx.current_scope.push(trait_.name.clone());

    // Set current_self_type to a generic Self placeholder
    let prev_self = ctx.current_self_type.take();
    ctx.current_self_type = Some(
        Type::GenericParam {
            span: SrcPos::default(),
            index: u32::MAX,
            name: "Self".into(),
        }
        .into(),
    );

    let generics = helpers::lower_generic_params(trait_.generics.clone(), ctx, log)?;

    let mut methods = Vec::new();
    let mut associated_types: Vec<NString> = Vec::new();
    let mut associated_constants: Vec<NString> = Vec::new();

    for method in &trait_.items {
        match method {
            ast::AssociatedItem::Method(func) => {
                let func_id: FunctionId = lower_function(func.to_owned(), ctx, log)?;
                methods.push(func_id);
            }
            ast::AssociatedItem::TypeAlias(type_alias) => {
                let type_name: NString = helpers::qualify(&type_alias.name, ctx);
                associated_types.push(type_name);
            }
            ast::AssociatedItem::ConstantItem(const_var) => {
                let const_name: NString = helpers::qualify(&const_var.name, ctx);
                associated_constants.push(const_name);
            }
            ast::AssociatedItem::SyntaxError(_) => return Err(()),
        }
    }

    // Restore the previous self type
    ctx.current_self_type = prev_self;
    ctx.current_scope.pop();

    let trait_ = Trait {
        span,
        visibility,
        name: name.clone(),
        generics,
        supertraits: Vec::new(),
        where_clause: None,
        methods,
        associated_types,
        associated_constants,
    };

    Ok(helpers::upsert_trait(trait_, ctx))
}

// ═══════════════════════════════════════════════════════════════════════════
// Implementation Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_implementation(impl_: ast::Impl, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<(), ()> {
    // Extract the impl type name before moving impl_.for_type
    let impl_type_name: Option<NString> = match &impl_.for_type {
        nitrate_tree::ast::Type::TypePath(type_path) => {
            let name = type_path
                .segments
                .iter()
                .map(|seg| seg.name.clone())
                .collect::<Vec<_>>()
                .join("::");
            Some(name.into())
        }
        _ => None,
    };

    let for_type: TypeId = lower_type(impl_.for_type, ctx, log)?.into();

    // Push the impl type name onto the current scope
    if let Some(ref name) = impl_type_name {
        ctx.current_scope.push(name.clone());
    }

    // Save and set the current self type
    let prev_self = ctx.current_self_type.take();
    ctx.current_self_type = Some(for_type);

    let result = match impl_.trait_path {
        Some(trait_path) => {
            let trait_name = trait_path
                .segments
                .iter()
                .map(|seg| seg.name.to_string())
                .collect::<Vec<_>>()
                .join("::");

            let trait_id = ctx.tab.get_trait_or_insert_placeholder(&trait_name.into());
            ctx.tab.add_impl_trait(for_type, trait_id.clone());

            for associated_item in impl_.items {
                match associated_item {
                    ast::AssociatedItem::Method(method) => {
                        let name = method.name.clone();
                        let func_id = lower_function(method, ctx, log)?;
                        ctx.tab.add_trait_method(for_type, trait_id.clone(), name, func_id);
                    }
                    ast::AssociatedItem::TypeAlias(type_alias) => {
                        let type_alias_id = lower_type_alias(type_alias, ctx, log)?;
                        ctx.tab
                            .add_impl_associated_type(for_type, trait_id.clone(), "".into(), type_alias_id);
                    }
                    ast::AssociatedItem::ConstantItem(const_var) => {
                        let const_id = lower_global_variable(&const_var, ctx, log)?;
                        ctx.tab
                            .add_impl_associated_constant(for_type, trait_id.clone(), "".into(), const_id);
                    }
                    ast::AssociatedItem::SyntaxError(_) => return Err(()),
                }
            }

            Ok(())
        }
        None => {
            for associated_item in impl_.items {
                match associated_item {
                    ast::AssociatedItem::Method(method) => {
                        let name = method.name.clone();
                        let func_id = lower_function(method, ctx, log)?;
                        ctx.tab.add_method(for_type, name, func_id);
                    }
                    ast::AssociatedItem::TypeAlias(type_alias) => {
                        let _ = lower_type_alias(type_alias, ctx, log)?;
                    }
                    ast::AssociatedItem::ConstantItem(const_var) => {
                        let _ = lower_global_variable(&const_var, ctx, log)?;
                    }
                    ast::AssociatedItem::SyntaxError(_) => return Err(()),
                }
            }

            Ok(())
        }
    };

    // Pop the impl type name from scope
    if impl_type_name.is_some() {
        ctx.current_scope.pop();
    }

    // Restore the previous self type
    ctx.current_self_type = prev_self;
    result
}

// ═══════════════════════════════════════════════════════════════════════════
// Global Variable Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_global_variable(
    var: &ast::GlobalVariable,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<GlobalVariableId, ()> {
    let span: SrcPos = var.span.start;
    let visibility = helpers::lower_visibility(var.visibility);
    helpers::reject_all_attributes(&var.attributes, ATTR_GLOBAL_VAR, log);

    let is_mutable = helpers::is_mutable(var.mutability);

    let name = helpers::qualify(&var.name, ctx);
    helpers::check_duplicate(&name, ctx, log)?;

    let ty = match var.ty.to_owned() {
        None => ctx.create_inference_placeholder().into(),
        Some(t) => lower_type(t, ctx, log)?.into(),
    };

    let init = match var.initializer.to_owned() {
        Some(expr) => lower_expr(expr, ctx, log)?.into(),
        None => {
            log.report(&HirErr::GlobalVariableMustHaveInitializer {
                span,
                name: name.to_string(),
            });
            return Err(());
        }
    };

    let global_variable = GlobalVariable {
        span,
        visibility,
        attributes: BTreeSet::new(),
        is_mutable,
        name,
        mangled_name: None,
        ty,
        initializer: init,
    };

    Ok(helpers::upsert_global_variable(global_variable, ctx))
}

// ═══════════════════════════════════════════════════════════════════════════
// Parameter Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_parameter(param: ast::FuncParam, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<ParameterId, ()> {
    let span: SrcPos = param.span.start;
    helpers::reject_all_attributes(&param.attributes, ATTR_FUNC_PARAM, log);

    let is_mutable = helpers::is_mutable(param.mutability);

    let name = helpers::qualify(&param.name, ctx);
    helpers::check_duplicate(&name, ctx, log)?;

    let ty = lower_type(param.ty.to_owned(), ctx, log)?.into();

    let default_value = match param.default_value.to_owned() {
        Some(expr) => Some(lower_expr(expr, ctx, log)?.into()),
        None => None,
    };

    let parameter_id: ParameterId = Parameter {
        span,
        attributes: BTreeSet::new(),
        is_mutable,
        name,
        ty,
        default_value,
    }
    .into();

    ctx.entities_added.insert(parameter_id.borrow().name.clone());
    ctx.tab.add_parameter(parameter_id.clone());

    Ok(parameter_id)
}

// ═══════════════════════════════════════════════════════════════════════════
// Function Lowering
// ═══════════════════════════════════════════════════════════════════════════

fn lower_function(function: ast::Function, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<FunctionId, ()> {
    let span: SrcPos = function.span.start;
    let visibility = helpers::lower_visibility(function.visibility);
    let mut attributes = helpers::parse_function_attributes(&function.attributes, log);

    // Lower extern ABI from the AST
    if let Some(abi) = &function.abi {
        attributes.insert(FunctionAttribute::ExternAbi(ExternAbi { name: abi.name.clone() }));
    }

    let name: NString = helpers::qualify(&function.name, ctx);
    helpers::check_duplicate(&name, ctx, log)?;

    ctx.current_scope.push(function.name.clone());

    let generics = helpers::lower_generic_params(function.generics, ctx, log)?;

    let mut parameters = Vec::with_capacity(function.parameters.params.len());
    for param in &function.parameters.params {
        let param_hir = lower_parameter(param.to_owned(), ctx, log)?;
        parameters.push(param_hir);
    }
    if function.parameters.variadic {
        attributes.insert(FunctionAttribute::CVariadic);
    }

    let return_type: Type = match &function.return_type {
        Some(ty) => lower_type(ty.to_owned(), ctx, log)?,
        None => Type::Unit {
            span: SrcPos::default(),
        },
    };

    let body = match function.definition {
        None => None,
        Some(block) => {
            let mut hir_elements = lower_block(block, ctx, log)?.elements;

            // Ensure the function body ends with a return statement
            let return_inserted = ensure_return_in_body(&mut hir_elements, &return_type, &name, log);

            if return_inserted {
                Some(hir_elements)
            } else {
                return Err(());
            }
        }
    };

    ctx.current_scope.pop();

    let function = Function {
        span,
        visibility,
        attributes,
        is_unsafe: false,
        name: name.clone(),
        mangled_name: None,
        generics,
        params: parameters,
        return_type: return_type.into(),
        body,
    };

    Ok(helpers::upsert_function(function, ctx))
}

/// Ensures that a function's block elements end with a `return` expression.
/// Returns `true` if the body is valid, `false` if an error occurred.
fn ensure_return_in_body(
    hir_elements: &mut Vec<BlockElement>,
    return_type: &Type,
    func_name: &NString,
    log: &CompilerLog,
) -> bool {
    match hir_elements.last() {
        Some(BlockElement::Expr(expr)) if expr.borrow().is_return() => true,
        Some(BlockElement::Expr(expr)) if !expr.borrow().is_return() => {
            *hir_elements.last_mut().unwrap() = BlockElement::Expr(
                Value::Return {
                    span: SrcPos::default(),
                    value: expr.to_owned(),
                }
                .into(),
            );
            true
        }
        _ if matches!(return_type, Type::Unit { .. }) => {
            hir_elements.push(BlockElement::Expr(
                Value::Return {
                    span: SrcPos::default(),
                    value: Value::Unit {
                        span: SrcPos::default(),
                    }
                    .into(),
                }
                .into(),
            ));
            true
        }
        _ => {
            log.report(&HirErr::MissingReturnStatement {
                span: SrcPos::default(),
                name: func_name.to_string(),
            });
            false
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Module Lowering
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) fn lower_module(module: ast::Module, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Module, ()> {
    let span: SrcPos = module.span.start;
    ctx.current_scope.push(module.name.clone());

    let visibility = helpers::lower_visibility(module.visibility);

    helpers::reject_all_attributes(&Some(module.attributes.clone().unwrap_or_default()), ATTR_MODULE, log);

    let qualified_name = helpers::qualify(&module.name, ctx);
    helpers::check_duplicate(&qualified_name, ctx, log)?;

    let mut items = Vec::with_capacity(module.items.len());

    for item in module.items {
        let lowered_item = lower_item(ctx, item, log)?;
        if let Some(item) = lowered_item {
            items.push(item);
        }
    }

    let module = Module {
        span,
        visibility,
        attributes: BTreeSet::new(),
        name: module.name,
        items,
    };

    ctx.entities_added.insert(qualified_name.clone());
    ctx.current_scope.pop();

    Ok(module)
}

// ═══════════════════════════════════════════════════════════════════════════
// Item Dispatch
// ═══════════════════════════════════════════════════════════════════════════

pub(crate) type Item = nitrate_hir::Item;

fn lower_item(ctx: &mut Ast2HirCtx, item: ast::Item, log: &CompilerLog) -> Result<Option<Item>, ()> {
    match item {
        ast::Item::Module(module) => {
            let hir_module = convert_ast_to_hir(*module, ctx, log).map_err(|_| ())?.into();
            Ok(Some(Item::Module(hir_module)))
        }

        ast::Item::Import(import) => {
            if let Some(resolved_items) = import.resolved {
                for item in resolved_items {
                    lower_item(ctx, item, log)?;
                }
            }
            Ok(None)
        }

        ast::Item::TypeAlias(type_alias) => {
            let t = lower_type_alias(type_alias, ctx, log)?;
            Ok(Some(Item::TypeAliasDef(t)))
        }

        ast::Item::Struct(struct_def) => {
            let s = lower_struct_definition(struct_def, ctx, log)?;
            Ok(Some(Item::StructDef(s)))
        }

        ast::Item::Enum(enum_def) => {
            let e = lower_enum_definition(enum_def, ctx, log)?;
            Ok(Some(Item::EnumDef(e)))
        }

        ast::Item::Trait(trait_def) => {
            let t = lower_trait_definition(&trait_def, ctx, log)?;
            Ok(Some(Item::Trait(t)))
        }

        ast::Item::Impl(impl_def) => {
            lower_implementation(*impl_def, ctx, log)?;
            Ok(None)
        }

        ast::Item::Function(func_def) => {
            let f = lower_function(func_def, ctx, log)?;
            Ok(Some(Item::Function(f)))
        }

        ast::Item::Variable(v) => {
            let g = lower_global_variable(&v, ctx, log)?;
            Ok(Some(Item::GlobalVariable(g)))
        }

        ast::Item::SyntaxError(_) => Err(()),
    }
}
