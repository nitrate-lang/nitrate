use crate::{ast_mod2hir, diagnosis::HirErr, lower::lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_source::ast::{self};
use std::collections::BTreeSet;

fn ast_typealias2hir(
    type_alias: &ast::TypeAlias,
    ctx: &mut HirCtx,
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

    let name = IString::from(HirCtx::join_path(ctx.current_scope(), &type_alias.name));

    if type_alias.generics.is_some() {
        // TODO: support generic type aliases
        log.report(&HirErr::UnimplementedFeature("generic type aliases".into()));
    }

    let type_id = match &type_alias.alias_type {
        Some(ty) => ty.to_owned().ast2hir(ctx, log)?.into_id(ctx.store()),
        None => {
            log.report(&HirErr::TypeAliasMustHaveType);
            return Err(());
        }
    };

    let definition = TypeDefinition::TypeAliasDef(TypeAliasDef {
        visibility,
        name,
        type_id,
    });

    ctx.register_type(definition);

    Ok(())
}

fn ast_struct2hir(struct_def: &ast::Struct, ctx: &mut HirCtx, log: &CompilerLog) -> Result<(), ()> {
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

    let name = HirCtx::join_path(ctx.current_scope(), &struct_def.name).into();

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
        let field_type = field.ty.to_owned().ast2hir(ctx, log)?.into_id(ctx.store());

        let field_default = match field.default_value.to_owned() {
            Some(expr) => Some(expr.ast2hir(ctx, log)?.into_id(ctx.store())),
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

    let struct_id = StructType { attributes, fields }.into_id(ctx.store());

    let definition = TypeDefinition::StructDef(StructDef {
        visibility,
        name,
        field_extras,
        struct_id,
    });

    ctx.register_type(definition);

    Ok(())
}

fn ast_enum2hir(enum_def: &ast::Enum, ctx: &mut HirCtx, log: &CompilerLog) -> Result<(), ()> {
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

    let name = HirCtx::join_path(ctx.current_scope(), &enum_def.name).into();

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
            Some(ty) => ty.ast2hir(ctx, log)?.into_id(ctx.store()),
            None => Type::Unit.into_id(ctx.store()),
        };

        let field_default = match variant.default_value.to_owned() {
            Some(expr) => Some(expr.ast2hir(ctx, log)?.into_id(ctx.store())),
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
    .into_id(ctx.store());

    let definition = TypeDefinition::EnumDef(EnumDef {
        visibility,
        name,
        variant_extras,
        enum_id,
    });

    ctx.register_type(definition);

    Ok(())
}

fn ast_trait2hir(_trait: &ast::Trait, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<(), ()> {
    // TODO: implement trait lowering
    log.report(&HirErr::UnimplementedFeature("trait definitions".into()));
    Err(())
}

fn ast_impl2hir(_impl: &ast::Impl, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<(), ()> {
    // TODO: implement impl block lowering
    log.report(&HirErr::UnimplementedFeature("impl blocks".into()));
    Err(())
}

fn ast_param2hir(
    param: &ast::FuncParam,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<Parameter, ()> {
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

    let name = IString::from(param.name.to_string());
    let ty = param.ty.to_owned().ast2hir(ctx, log)?.into_id(ctx.store());

    let default_value = match param.default_value.to_owned() {
        Some(expr) => Some(expr.ast2hir(ctx, log)?.into_id(ctx.store())),
        None => None,
    };

    Ok(Parameter {
        attributes,
        is_mutable,
        name,
        ty,
        default_value,
    })
}

fn ast_function2hir(
    function: &ast::Function,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<Function, ()> {
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

    let name = IString::from(function.name.to_string());

    if function.generics.is_some() {
        // TODO: support generic functions
        log.report(&HirErr::UnimplementedFeature("generic functions".into()));
    }

    let mut parameters = Vec::with_capacity(function.parameters.len());
    for param in &function.parameters {
        let param_hir = ast_param2hir(param, ctx, log)?;
        parameters.push(param_hir.into_id(ctx.store()));
    }

    let return_type = match &function.return_type {
        Some(ty) => ty.to_owned().ast2hir(ctx, log)?.into_id(ctx.store()),
        None => Type::Unit.into_id(ctx.store()),
    };

    let body = match &function.definition {
        Some(block) => Some(block.to_owned().ast2hir(ctx, log)?.into_id(ctx.store())),
        None => None,
    };

    let hir_function = Function {
        visibility,
        attributes,
        name,
        parameters,
        return_type,
        body,
    };

    Ok(hir_function)
}

fn ast_variable2hir(
    var: &ast::Variable,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<GlobalVariable, ()> {
    let visibility = match var.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    match var.kind {
        ast::VariableKind::Const | ast::VariableKind::Static => {}
        ast::VariableKind::Let | ast::VariableKind::Var => {
            log.report(&HirErr::GlobalVariableMustBeConstOrStatic);
            return Err(());
        }
    }

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
        None => ctx.create_inference_placeholder().into_id(ctx.store()),
        Some(t) => {
            let ty_hir = t.ast2hir(ctx, log)?.into_id(ctx.store());
            ty_hir
        }
    };

    let initializer = match var.initializer.to_owned() {
        Some(expr) => {
            let expr_hir = expr.ast2hir(ctx, log)?.into_id(ctx.store());
            expr_hir
        }

        None => {
            log.report(&HirErr::GlobalVariableMustHaveInitializer);
            return Err(());
        }
    };

    Ok(GlobalVariable {
        visibility,
        attributes,
        is_mutable,
        name,
        ty,
        initializer,
    })
}

impl Ast2Hir for ast::Module {
    type Hir = Module;

    fn ast2hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        fn lower_module(
            this: ast::Module,
            ctx: &mut HirCtx,
            log: &CompilerLog,
        ) -> Result<Module, ()> {
            let visibility = match this.visibility {
                Some(ast::Visibility::Public) => Visibility::Pub,
                Some(ast::Visibility::Protected) => Visibility::Pro,
                Some(ast::Visibility::Private) | None => Visibility::Sec,
            };

            let ast_attributes = this.attributes.unwrap_or_default();
            let attributes = BTreeSet::new();
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedModuleAttribute);
            }

            let name = match this.name {
                Some(n) => Some(IString::from(n.to_string())),
                None => None,
            };

            let mut items = Vec::with_capacity(this.items.len());
            for item in this.items {
                match item {
                    ast::Item::Module(submodule) => {
                        let hir_submodule = ast_mod2hir(*submodule, ctx, log)?.into_id(ctx.store());
                        items.push(Item::Module(hir_submodule));
                    }

                    ast::Item::Import(_) => {
                        // TODO: lower import statements
                        log.report(&HirErr::UnimplementedFeature("import statements".into()));
                    }

                    ast::Item::TypeAlias(type_alias) => {
                        ast_typealias2hir(&type_alias, ctx, log)?;
                    }

                    ast::Item::Struct(struct_def) => {
                        ast_struct2hir(&struct_def, ctx, log)?;
                    }

                    ast::Item::Enum(enum_def) => {
                        ast_enum2hir(&enum_def, ctx, log)?;
                    }

                    ast::Item::Trait(trait_def) => {
                        ast_trait2hir(&trait_def, ctx, log)?;
                    }

                    ast::Item::Impl(impl_def) => {
                        ast_impl2hir(&impl_def, ctx, log)?;
                    }

                    ast::Item::Function(func_def) => {
                        let f = ast_function2hir(&func_def, ctx, log)?;
                        items.push(Item::Function(f.into_id(ctx.store())));
                    }

                    ast::Item::Variable(v) => {
                        let g = ast_variable2hir(&v, ctx, log)?;
                        items.push(Item::GlobalVariable(g.into_id(ctx.store())));
                    }

                    ast::Item::SyntaxError(_) => {
                        continue;
                    }
                }
            }

            let module = Module {
                visibility,
                attributes,
                name,
                items,
            };

            Ok(module)
        }

        match self.name {
            Some(ref name) => ctx.push_current_scope(name.to_string()),
            None => ctx.push_current_scope(String::default()),
        }

        let result = lower_module(self, ctx, log);
        ctx.pop_current_scope();

        result
    }
}
