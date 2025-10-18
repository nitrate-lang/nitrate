use std::collections::BTreeSet;

use crate::{ast_mod2hir, diagnosis::HirErr, lower::lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::ast::{self};

fn ast_typealias2hir(
    _type_alias: &ast::TypeAlias,
    _ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<(), ()> {
    // TODO: implement type alias lowering
    log.report(&HirErr::UnimplementedFeature("type aliases".into()));
    Err(())
}

fn ast_struct2hir(_struct: &ast::Struct, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<(), ()> {
    // TODO: implement struct lowering
    log.report(&HirErr::UnimplementedFeature("struct definitions".into()));
    Err(())
}

fn ast_enum2hir(_enum: &ast::Enum, _ctx: &mut HirCtx, log: &CompilerLog) -> Result<(), ()> {
    // TODO: implement enum lowering
    log.report(&HirErr::UnimplementedFeature("enum definitions".into()));
    Err(())
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
                        ast_typealias2hir(&type_alias.read().unwrap(), ctx, log)?;
                    }

                    ast::Item::Struct(struct_def) => {
                        ast_struct2hir(&struct_def.read().unwrap(), ctx, log)?;
                    }

                    ast::Item::Enum(enum_def) => {
                        ast_enum2hir(&enum_def.read().unwrap(), ctx, log)?;
                    }

                    ast::Item::Trait(trait_def) => {
                        ast_trait2hir(&trait_def.read().unwrap(), ctx, log)?;
                    }

                    ast::Item::Impl(impl_def) => {
                        ast_impl2hir(&impl_def, ctx, log)?;
                    }

                    ast::Item::Function(func_def) => {
                        let f = ast_function2hir(&func_def.read().unwrap(), ctx, log)?;
                        items.push(Item::Function(f.into_id(ctx.store())));
                    }

                    ast::Item::Variable(v) => {
                        let g = ast_variable2hir(&v.read().unwrap(), ctx, log)?;
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
