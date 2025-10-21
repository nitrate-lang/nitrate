use std::collections::BTreeSet;

use crate::{context::Ast2HirCtx, diagnosis::HirErr, lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_source::{
    Order, ParseTreeIterMut, RefNodeMut, ast,
    tag::{intern_function_name, intern_type_name, intern_variable_name},
};

fn gendecl_for_typealias(
    type_alias: &ast::TypeAlias,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<(), ()> {
    let visibility = match type_alias.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    if let Some(ast_attributes) = &type_alias.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedTypeAliasAttribute);
        }
    }

    let name = type_alias.name.to_string().into();

    if type_alias.generics.is_some() {
        // TODO: support generic type aliases
        log.report(&HirErr::UnimplementedFeature("generic type aliases".into()));
    }

    let type_id = match &type_alias.alias_type {
        Some(ty) => ty.to_owned().ast2hir(ctx, log)?.into_id(&ctx.store),
        None => {
            log.report(&HirErr::TypeAliasMustHaveType);
            return Ok(());
        }
    };

    let type_alias_id = TypeAliasDef {
        visibility,
        name,
        type_id,
    }
    .into_id(&ctx.store);

    let definition = TypeDefinition::TypeAliasDef(type_alias_id.clone());
    ctx.symbol_tab.add_type(definition, &ctx.store);

    Ok(())
}

fn gendecl_for_structdef(
    struct_def: &ast::Struct,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<(), ()> {
    let visibility = match struct_def.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &struct_def.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedStructAttribute);
        }
    }

    let name = struct_def.name.to_string().into();

    if struct_def.generics.is_some() {
        // TODO: support generic structs
        log.report(&HirErr::UnimplementedFeature("generic structs".into()));
    }

    let mut field_extras = Vec::new();
    let mut fields = Vec::new();

    for field in &struct_def.fields {
        let field_visibility = match field.visibility {
            Some(ast::Visibility::Public) => Visibility::Pub,
            Some(ast::Visibility::Protected) => Visibility::Pro,
            Some(ast::Visibility::Private) | None => Visibility::Sec,
        };

        let field_attributes = BTreeSet::new();
        if let Some(ast_attributes) = &field.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedStructFieldAttribute);
            }
        }

        let field_name = IString::from(field.name.to_string());
        let field_type = field.ty.to_owned().ast2hir(ctx, log)?.into_id(&ctx.store);

        let field_default = match field.default_value.to_owned() {
            Some(expr) => Some(expr.ast2hir(ctx, log)?.into_id(&ctx.store)),
            None => None,
        };

        let struct_field = StructField {
            attributes: field_attributes,
            name: field_name,
            ty: field_type,
        };

        field_extras.push((field_visibility, field_default));
        fields.push(struct_field);
    }

    let struct_id = StructType { attributes, fields }.into_id(&ctx.store);

    let struct_def_id = StructDef {
        visibility,
        name,
        field_extras,
        struct_id,
    }
    .into_id(&ctx.store);

    let definition = TypeDefinition::StructDef(struct_def_id.clone());
    ctx.symbol_tab.add_type(definition, &ctx.store);

    Ok(())
}

fn gendecl_for_enumdef(
    enum_def: &ast::Enum,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<(), ()> {
    let visibility = match enum_def.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &enum_def.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedEnumAttribute);
        }
    }

    let name = enum_def.name.to_string().into();

    if enum_def.generics.is_some() {
        // TODO: support generic enums
        log.report(&HirErr::UnimplementedFeature("generic enums".into()));
    }

    let mut variants = Vec::new();
    let mut variant_extras = Vec::new();

    for variant in &enum_def.variants {
        let variant_attributes = BTreeSet::new();
        if let Some(ast_attributes) = &variant.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedEnumVariantAttribute);
            }
        }

        let variant_name = IString::from(variant.name.to_string());

        let variant_type = match variant.ty.to_owned() {
            Some(ty) => ty.ast2hir(ctx, log)?.into_id(&ctx.store),
            None => Type::Unit.into_id(&ctx.store),
        };

        let field_default = match variant.default_value.to_owned() {
            Some(expr) => Some(expr.ast2hir(ctx, log)?.into_id(&ctx.store)),
            None => None,
        };

        let variant = EnumVariant {
            attributes: variant_attributes,
            name: variant_name,
            ty: variant_type,
        };

        variants.push(variant);
        variant_extras.push(field_default);
    }

    let enum_id = EnumType {
        attributes,
        variants,
    }
    .into_id(&ctx.store);

    let enum_def_id = EnumDef {
        visibility,
        name,
        variant_extras,
        enum_id,
    }
    .into_id(&ctx.store);

    let definition = TypeDefinition::EnumDef(enum_def_id.clone());
    ctx.symbol_tab.add_type(definition, &ctx.store);

    Ok(())
}

fn gendecl_for_globalvar(
    var: &ast::GlobalVariable,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<(), ()> {
    let visibility = match var.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &var.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedGlobalVariableAttribute);
        }
    }

    let is_mutable = match var.mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    };

    let name = var.name.to_string().into();

    let ty = match var.ty.to_owned() {
        None => ctx.create_inference_placeholder().into_id(&ctx.store),
        Some(t) => {
            let ty_hir = t.ast2hir(ctx, log)?.into_id(&ctx.store);
            ty_hir
        }
    };

    let global_variable_id = GlobalVariable {
        visibility,
        attributes,
        is_mutable,
        name,
        ty,
        init: None,
    }
    .into_id(&ctx.store);

    let variable = SymbolId::GlobalVariable(global_variable_id.clone());
    ctx.symbol_tab.add_symbol(variable, &ctx.store);

    Ok(())
}

fn gendecl_for_function(
    function: &ast::Function,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<(), ()> {
    let visibility = match function.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &function.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedFunctionAttribute);
        }
    }

    let name = function.name.to_string().into();

    if function.generics.is_some() {
        // TODO: support generic functions
        log.report(&HirErr::UnimplementedFeature("generic functions".into()));
    }

    let mut parameters = Vec::with_capacity(function.parameters.len());
    for param in &function.parameters {
        let param_attributes = BTreeSet::new();
        if let Some(ast_attributes) = &param.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedFunctionParameterAttribute);
            }
        }

        let param_is_mutable = match param.mutability {
            Some(ast::Mutability::Mut) => true,
            Some(ast::Mutability::Const) | None => false,
        };

        let param_name = IString::from(param.name.to_string());
        let param_ty = param.ty.to_owned().ast2hir(ctx, log)?.into_id(&ctx.store);

        let param_default_value = match param.default_value.to_owned() {
            Some(expr) => Some(expr.ast2hir(ctx, log)?.into_id(&ctx.store)),
            None => None,
        };

        let parameter = Parameter {
            attributes: param_attributes,
            is_mutable: param_is_mutable,
            name: param_name,
            ty: param_ty,
            default_value: param_default_value,
        }
        .into_id(&ctx.store);

        parameters.push(parameter);
    }

    let return_type = match &function.return_type {
        Some(ty) => ty.to_owned().ast2hir(ctx, log)?.into_id(&ctx.store),
        None => Type::Unit.into_id(&ctx.store),
    };

    let body = match &function.definition {
        Some(block) => Some(block.to_owned().ast2hir(ctx, log)?.into_id(&ctx.store)),
        None => None,
    };

    let function_id = Function {
        visibility,
        attributes,
        name,
        params: parameters,
        return_type,
        body,
    }
    .into_id(&ctx.store);

    let function = SymbolId::Function(function_id.clone());
    ctx.symbol_tab.add_symbol(function, &ctx.store);

    Ok(())
}

fn gendecl_visit_node(order: Order, node: RefNodeMut, ctx: &mut Ast2HirCtx, log: &CompilerLog) {
    match node {
        RefNodeMut::ExprSyntaxError
        | RefNodeMut::ExprParentheses(_)
        | RefNodeMut::ExprBooleanLit(_)
        | RefNodeMut::ExprIntegerLit(_)
        | RefNodeMut::ExprFloatLit(_)
        | RefNodeMut::ExprStringLit(_)
        | RefNodeMut::ExprBStringLit(_)
        | RefNodeMut::ExprTypeInfo(_)
        | RefNodeMut::ExprList(_)
        | RefNodeMut::ExprTuple(_)
        | RefNodeMut::ExprStructInit(_)
        | RefNodeMut::ExprUnaryExpr(_)
        | RefNodeMut::ExprBinExpr(_)
        | RefNodeMut::ExprCast(_)
        | RefNodeMut::ExprLocalVariable(_)
        | RefNodeMut::ExprBlockItem(_)
        | RefNodeMut::ExprBlock(_)
        | RefNodeMut::ExprAttributeList(_)
        | RefNodeMut::ExprClosure(_)
        | RefNodeMut::ExprPathTypeArgument(_)
        | RefNodeMut::ExprPath(_)
        | RefNodeMut::ExprIndexAccess(_)
        | RefNodeMut::ExprIf(_)
        | RefNodeMut::ExprWhile(_)
        | RefNodeMut::ExprMatchCase(_)
        | RefNodeMut::ExprMatch(_)
        | RefNodeMut::ExprBreak(_)
        | RefNodeMut::ExprContinue(_)
        | RefNodeMut::ExprReturn(_)
        | RefNodeMut::ExprFor(_)
        | RefNodeMut::ExprAwait(_)
        | RefNodeMut::ExprCallArgument(_)
        | RefNodeMut::ExprFunctionCall(_)
        | RefNodeMut::ExprMethodCall(_)
        | RefNodeMut::TypeSyntaxError
        | RefNodeMut::TypeBool
        | RefNodeMut::TypeUInt8
        | RefNodeMut::TypeUInt16
        | RefNodeMut::TypeUInt32
        | RefNodeMut::TypeUInt64
        | RefNodeMut::TypeUInt128
        | RefNodeMut::TypeUSize
        | RefNodeMut::TypeInt8
        | RefNodeMut::TypeInt16
        | RefNodeMut::TypeInt32
        | RefNodeMut::TypeInt64
        | RefNodeMut::TypeInt128
        | RefNodeMut::TypeFloat32
        | RefNodeMut::TypeFloat64
        | RefNodeMut::TypeInferType
        | RefNodeMut::TypePath(_)
        | RefNodeMut::TypeRefinementType(_)
        | RefNodeMut::TypeTupleType(_)
        | RefNodeMut::TypeArrayType(_)
        | RefNodeMut::TypeSliceType(_)
        | RefNodeMut::TypeFunctionType(_)
        | RefNodeMut::TypeReferenceType(_)
        | RefNodeMut::TypeOpaqueType(_)
        | RefNodeMut::TypeLatentType(_)
        | RefNodeMut::TypeLifetime(_)
        | RefNodeMut::TypeParentheses(_)
        | RefNodeMut::ItemSyntaxError
        | RefNodeMut::ItemPath(_)
        | RefNodeMut::ItemTypeParams(_)
        | RefNodeMut::ItemImport(_)
        | RefNodeMut::ItemStructField(_)
        | RefNodeMut::ItemEnumVariant(_)
        | RefNodeMut::ItemImpl(_)
        | RefNodeMut::ItemFuncParam(_) => {}

        RefNodeMut::ItemModule(module) => match order {
            Order::Enter => match &module.name {
                Some(name) => ctx.current_scope.push(name.to_string()),
                None => ctx.current_scope.push(String::default()),
            },

            Order::Leave => {
                ctx.current_scope.pop();
            }
        },

        RefNodeMut::ItemTypeAlias(type_alias) => match order {
            Order::Enter => {
                type_alias.name =
                    intern_type_name(Ast2HirCtx::join_path(&ctx.current_scope, &type_alias.name));

                gendecl_for_typealias(type_alias, ctx, log).ok();
            }

            Order::Leave => {}
        },

        RefNodeMut::ItemStruct(struct_def) => match order {
            Order::Enter => {
                struct_def.name =
                    intern_type_name(Ast2HirCtx::join_path(&ctx.current_scope, &struct_def.name));

                gendecl_for_structdef(struct_def, ctx, log).ok();
            }

            Order::Leave => {}
        },

        RefNodeMut::ItemEnum(enum_def) => match order {
            Order::Enter => {
                enum_def.name =
                    intern_type_name(Ast2HirCtx::join_path(&ctx.current_scope, &enum_def.name));

                gendecl_for_enumdef(enum_def, ctx, log).ok();
            }

            Order::Leave => {}
        },

        RefNodeMut::ItemTrait(_) => match order {
            Order::Enter => {
                // TODO: implement traits
                println!("scope = {:?}", ctx.current_scope);
            }

            Order::Leave => {}
        },

        RefNodeMut::ItemFunction(function) => match order {
            Order::Enter => {
                function.name =
                    intern_function_name(Ast2HirCtx::join_path(&ctx.current_scope, &function.name));

                gendecl_for_function(function, ctx, log).ok();
            }

            Order::Leave => {}
        },

        RefNodeMut::ItemGlobalVariable(variable) => match order {
            Order::Enter => {
                variable.name =
                    intern_variable_name(Ast2HirCtx::join_path(&ctx.current_scope, &variable.name));

                gendecl_for_globalvar(variable, ctx, log).ok();
            }

            Order::Leave => {}
        },
    }
}

pub(crate) fn generate_forward_declarations<T: ParseTreeIterMut>(
    node: &mut T,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) {
    node.depth_first_iter_mut(&mut |order, node| gendecl_visit_node(order, node, ctx, log));
}
