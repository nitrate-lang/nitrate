use crate::{ast_mod2hir, diagnosis::HirErr, lower::lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::ast;

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

fn ast_function2hir(
    _function: &ast::Function,
    _ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<(), ()> {
    // TODO: implement function lowering
    log.report(&HirErr::UnimplementedFeature("function definitions".into()));
    Err(())
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

    let ast_attributes = var.attributes.to_owned().unwrap_or_default();
    for _attr in ast_attributes {
        log.report(&HirErr::UnrecognizedGlobalVariableAttribute);
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
            let attributes = Vec::with_capacity(ast_attributes.len());
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
                        ast_function2hir(&func_def.read().unwrap(), ctx, log)?
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
