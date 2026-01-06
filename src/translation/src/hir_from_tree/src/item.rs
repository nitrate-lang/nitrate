use crate::{
    context::Ast2HirCtx, convert_ast_to_hir, diagnosis::HirErr, expr::lower_block, lower::Ast2Hir, ty::lower_type,
};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ast::{self};
use std::collections::{BTreeMap, BTreeSet};

fn lower_type_alias(type_alias: ast::TypeAlias, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<TypeAliasDefId, ()> {
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

    let name = ctx.qualify_name(&type_alias.name).into();

    if type_alias.generics.is_some() {
        log.report(&HirErr::UnimplementedFeature("generic type aliases".into()));
    }

    let type_id = match &type_alias.alias_type {
        Some(ty) => lower_type(ty.to_owned(), ctx, log)?.into(),
        None => {
            log.report(&HirErr::TypeAliasMustHaveType);
            return Err(());
        }
    };

    let type_alias = TypeAliasDef {
        visibility,
        name,
        type_id,
    };

    if let Some(existing_type_alias_def_id) = ctx.tab.get_type_alias(&type_alias.name) {
        let mut existing_type_alias_def = existing_type_alias_def_id.borrow_mut();
        *existing_type_alias_def = type_alias;
        Ok(existing_type_alias_def_id.clone())
    } else {
        let type_alias_def_id: TypeAliasDefId = type_alias.into();
        ctx.tab.add_type_alias(type_alias_def_id.clone());
        Ok(type_alias_def_id)
    }
}

fn lower_struct_def(struct_def: ast::Struct, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<StructDefId, ()> {
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

    let name = ctx.qualify_name(&struct_def.name).into();

    if struct_def.generics.is_some() {
        log.report(&HirErr::UnimplementedFeature("generic structs".into()));
    }

    let mut fields = BTreeMap::new();
    let mut layout = StructLayout::new();

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

        let field_name = NString::from(field.name.to_string());
        let field_type = lower_type(field.ty.to_owned(), ctx, log)?.into();

        let field_default = match field.default_value.to_owned() {
            Some(expr) => Some(expr.ast2hir(ctx, log)?.into()),
            None => None,
        };

        let struct_field = StructField {
            visibility: field_visibility,
            attributes: field_attributes,
            name: field_name,
            ty: field_type,
            default_value: field_default,
        };

        let field_name = struct_field.name.clone();
        fields.insert(field_name.clone(), struct_field);
        layout.push(StructMemoryLayoutCell::Field { field_name });
    }

    let struct_def = StructDef {
        visibility,
        name,
        attributes,
        fields,
        layout,
    };

    if let Some(existing_struct_def_id) = ctx.tab.get_struct(&struct_def.name) {
        let mut existing_struct_def = existing_struct_def_id.borrow_mut();
        *existing_struct_def = struct_def;
        Ok(existing_struct_def_id.clone())
    } else {
        let struct_def_id: StructDefId = struct_def.into();
        ctx.tab.add_struct(struct_def_id.clone());
        Ok(struct_def_id)
    }
}

fn lower_enum_def(enum_def: ast::Enum, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<EnumDefId, ()> {
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

    let name: NString = ctx.qualify_name(&enum_def.name).into();

    if enum_def.generics.is_some() {
        log.report(&HirErr::UnimplementedFeature("generic enums".into()));
    }

    let mut variants = Vec::new();

    for variant in &enum_def.variants {
        let variant_attributes = BTreeSet::new();
        if let Some(ast_attributes) = &variant.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedEnumVariantAttribute);
            }
        }

        let variant_name = NString::from(variant.name.to_string());

        let variant_type = match variant.ty.to_owned() {
            Some(ty) => lower_type(ty, ctx, log)?.into(),
            None => Type::Unit.into(),
        };

        let field_default = match variant.default_value.to_owned() {
            Some(expr) => Some(expr.ast2hir(ctx, log)?.into()),
            None => None,
        };

        let variant = EnumVariant {
            attributes: variant_attributes,
            name: variant_name,
            ty: variant_type,
            default_value: field_default,
        };

        variants.push(variant);
    }

    let enum_def = EnumDef {
        visibility,
        name: name.clone(),
        attributes,
        variants: variants.clone().into(),
    };

    let enum_def_id = if let Some(existing_enum_def_id) = ctx.tab.get_enum(&enum_def.name) {
        let mut existing_enum_def = existing_enum_def_id.borrow_mut();
        *existing_enum_def = enum_def;
        existing_enum_def_id.clone()
    } else {
        let enum_def_id: EnumDefId = enum_def.into();
        ctx.tab.add_enum(enum_def_id.clone());
        enum_def_id
    };

    for variant in variants {
        let variant_name = NString::from(format!("{}::{}", name.clone(), variant.name));
        ctx.tab.add_enum_variant(variant_name, enum_def_id.clone());
    }

    Ok(enum_def_id)
}

fn lower_trait(trait_: &ast::Trait, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<TraitId, ()> {
    let visibility = match trait_.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    // let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &trait_.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedTraitAttribute);
        }
    }

    let name: NString = ctx.qualify_name(&trait_.name).into();

    if trait_.generics.is_some() {
        log.report(&HirErr::UnimplementedFeature("generic traits".into()));
    }

    let mut methods = Vec::new();
    for method in &trait_.items {
        match method {
            ast::AssociatedItem::Method(func) => {
                let func_id: FunctionId = lower_function(func.to_owned(), ctx, log)?.into();
                methods.push(func_id);
            }

            _ => {
                log.report(&HirErr::UnimplementedFeature(
                    "only method trait items are supported".into(),
                ));
                return Err(());
            }
        }
    }

    let trait_ = Trait {
        visibility,
        name: name.clone(),
        methods,
    };

    let trait_id = if let Some(existing_trait_id) = ctx.tab.get_trait(&trait_.name) {
        let mut existing_trait = existing_trait_id.borrow_mut();
        *existing_trait = trait_;
        existing_trait_id.clone()
    } else {
        let trait_id: TraitId = trait_.into();
        ctx.tab.add_trait(trait_id.clone());
        trait_id
    };

    Ok(trait_id)
}

fn lower_impl(impl_: ast::Impl, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<(), ()> {
    if let Some(_generics) = impl_.generics {
        log.report(&HirErr::UnimplementedFeature("generic impl blocks".into()));
        return Err(());
    }

    let for_type: TypeId = lower_type(impl_.for_type, ctx, log)?.into();

    match impl_.trait_path {
        Some(trait_path) => {
            let trait_name = trait_path
                .segments
                .iter()
                .map(|seg| seg.name.to_string())
                .collect::<Vec<_>>()
                .join("::");

            let trait_id = ctx.tab.get_trait_or_insert_placeholder(&trait_name.into());
            ctx.tab.add_impl_trait(for_type.clone(), trait_id.clone());

            for assosiated_item in impl_.items {
                match assosiated_item {
                    ast::AssociatedItem::Method(method) => {
                        let name = method.name.clone();
                        let func_id = lower_function(method, ctx, log)?.into();
                        ctx.tab
                            .add_trait_method(for_type.clone(), trait_id.clone(), name, func_id);
                    }

                    _ => {
                        log.report(&HirErr::UnimplementedFeature("only method impls are supported".into()));
                        return Err(());
                    }
                }
            }

            Ok(())
        }

        None => {
            for assosiated_item in impl_.items {
                match assosiated_item {
                    ast::AssociatedItem::Method(method) => {
                        let name = method.name.clone();
                        let func_id = lower_function(method, ctx, log)?.into();
                        ctx.tab.add_method(for_type.clone(), name, func_id);
                    }

                    _ => {
                        log.report(&HirErr::UnimplementedFeature("only method impls are supported".into()));
                        return Err(());
                    }
                }
            }

            Ok(())
        }
    }
}

fn lower_global_var(
    globalvar: &ast::GlobalVariable,
    ctx: &mut Ast2HirCtx,
    log: &CompilerLog,
) -> Result<GlobalVariableId, ()> {
    let visibility = match globalvar.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &globalvar.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedGlobalVariableAttribute);
        }
    }

    let is_mutable = match globalvar.mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    };

    let name = ctx.qualify_name(&globalvar.name).into();
    let mangled_name = if attributes.contains(&GlobalVariableAttribute::NoMangle) {
        globalvar.name.clone()
    } else {
        ctx.qualify_name(&globalvar.name).into()
    };

    let ty = match globalvar.ty.to_owned() {
        None => ctx.create_inference_placeholder().into(),
        Some(t) => lower_type(t, ctx, log)?.into(),
    };

    let init = match globalvar.initializer.to_owned() {
        Some(expr) => expr.ast2hir(ctx, log)?.into(),

        None => {
            log.report(&HirErr::GlobalVariableMustHaveInitializer);
            return Err(());
        }
    };

    let global_variable = GlobalVariable {
        visibility,
        attributes,
        is_mutable,
        name,
        mangled_name,
        ty,
        initializer: init,
    };

    if let Some(existing_global_id) = ctx.tab.get_global_variable(&global_variable.name) {
        let mut existing_global_variable = existing_global_id.borrow_mut();
        *existing_global_variable = global_variable;
        Ok(existing_global_id.clone())
    } else {
        let variable_id: GlobalVariableId = global_variable.into();
        ctx.tab.add_global_variable(variable_id.clone());
        Ok(variable_id)
    }
}

fn lower_parameter(param: ast::FuncParam, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<ParameterId, ()> {
    let attributes = BTreeSet::new();
    if let Some(ast_attributes) = &param.attributes {
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedFunctionParameterAttribute);
        }
    }

    let is_mutable = match param.mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    };

    let name = ctx.qualify_name(&param.name).into();
    let ty = lower_type(param.ty.to_owned(), ctx, log)?.into();

    let default_value = match param.default_value.to_owned() {
        Some(expr) => Some(expr.ast2hir(ctx, log)?.into()),
        None => None,
    };

    let parameter_id: ParameterId = Parameter {
        attributes,
        is_mutable,
        name,
        ty,
        default_value,
    }
    .into();

    ctx.tab.add_parameter(parameter_id.clone());

    Ok(parameter_id)
}

fn lower_function(function: ast::Function, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<FunctionId, ()> {
    let visibility = match function.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    let mut attributes = BTreeSet::new();
    if let Some(ast_attributes) = &function.attributes {
        for attr in ast_attributes {
            if let ast::Expr::Path(path) = &attr {
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

                    "c_variadic" => {
                        attributes.insert(FunctionAttribute::CVariadic);
                        continue;
                    }

                    _ => {}
                }
            }

            log.report(&HirErr::UnrecognizedEnumAttribute);
        }
    }

    let name: NString = ctx.qualify_name(&function.name).into();
    let mangled_name: NString = if attributes.contains(&FunctionAttribute::NoMangle) {
        function.name.clone()
    } else {
        ctx.qualify_name(&function.name).into()
    };

    ctx.current_scope.push(function.name.clone());

    if function.generics.is_some() {
        log.report(&HirErr::UnimplementedFeature("generic functions".into()));
    }

    let mut parameters = Vec::with_capacity(function.parameters.len());
    for param in &function.parameters {
        let param_hir = lower_parameter(param.to_owned(), ctx, log)?;
        parameters.push(param_hir);
    }

    let return_type = match &function.return_type {
        Some(ty) => lower_type(ty.to_owned(), ctx, log)?,
        None => Type::Unit,
    };

    let body = match function.definition {
        None => None,
        Some(block) => {
            let mut hir_elements = lower_block(block, ctx, log)?.elements;
            match hir_elements.last() {
                Some(BlockElement::Expr(expr)) if expr.borrow().is_return() => {}

                Some(BlockElement::Expr(expr)) if !expr.borrow().is_return() => {
                    *hir_elements.last_mut().unwrap() =
                        BlockElement::Expr(Value::Return { value: expr.to_owned() }.into());
                }

                _ if return_type == Type::Unit => {
                    hir_elements.push(BlockElement::Expr(
                        Value::Return {
                            value: Value::Unit.into(),
                        }
                        .into(),
                    ));
                }

                _ => log.report(&HirErr::MissingReturnStatement),
            }

            Some(hir_elements.into())
        }
    };

    ctx.current_scope.pop();

    let function = Function {
        visibility,
        attributes,
        name: name.clone(),
        mangled_name,
        params: parameters,
        return_type: return_type.into(),
        body,
    };

    if let Some(existing_function_id) = ctx.tab.get_function(&name) {
        let mut existing_function = existing_function_id.borrow_mut();
        *existing_function = function;
        Ok(existing_function_id.clone())
    } else {
        let function_id: FunctionId = function.into();
        ctx.tab.add_function(function_id.clone());
        Ok(function_id)
    }
}

pub(crate) fn lower_module(module: ast::Module, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Module, ()> {
    ctx.current_scope.push(module.name.clone());

    let visibility = match module.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    let ast_attributes = module.attributes.unwrap_or_default();
    let attributes = BTreeSet::new();
    for _attr in ast_attributes {
        log.report(&HirErr::UnrecognizedModuleAttribute);
    }

    let mut items = Vec::with_capacity(module.items.len());

    for item in module.items {
        let lowered_item = lower_item(ctx, item, log)?;
        if let Some(item) = lowered_item {
            items.push(item);
        }
    }

    let module = Module {
        visibility,
        attributes,
        name: module.name,
        items,
    };

    ctx.current_scope.pop();

    Ok(module)
}

fn lower_item(ctx: &mut Ast2HirCtx, item: ast::Item, log: &CompilerLog) -> Result<Option<Item>, ()> {
    match item {
        ast::Item::Module(module) => {
            let hir_module = convert_ast_to_hir(*module, ctx, log)?.into();
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
            let s = lower_struct_def(struct_def, ctx, log)?;
            Ok(Some(Item::StructDef(s)))
        }

        ast::Item::Enum(enum_def) => {
            let e = lower_enum_def(enum_def, ctx, log)?;
            Ok(Some(Item::EnumDef(e)))
        }

        ast::Item::Trait(trait_def) => {
            let t = lower_trait(&trait_def, ctx, log)?;
            Ok(Some(Item::Trait(t)))
        }

        ast::Item::Impl(impl_def) => {
            lower_impl(*impl_def, ctx, log)?;
            Ok(None)
        }

        ast::Item::Function(func_def) => {
            let f = lower_function(func_def, ctx, log)?;
            Ok(Some(Item::Function(f)))
        }

        ast::Item::Variable(v) => {
            let g = lower_global_var(&v, ctx, log)?;
            Ok(Some(Item::GlobalVariable(g)))
        }

        ast::Item::SyntaxError(_) => Err(()),
    }
}
