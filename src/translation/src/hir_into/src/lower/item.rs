use crate::{astmod2hir, diagnosis::HirErr, lower::lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::ast;

fn global_variable_ast2hir(
    glb: &ast::Variable,
    ctx: &mut HirCtx,
    log: &CompilerLog,
) -> Result<GlobalVariable, ()> {
    let visibility = match glb.visibility {
        Some(ast::Visibility::Public) => Visibility::Pub,
        Some(ast::Visibility::Protected) => Visibility::Pro,
        Some(ast::Visibility::Private) | None => Visibility::Sec,
    };

    match glb.kind {
        ast::VariableKind::Const | ast::VariableKind::Static => {}
        ast::VariableKind::Let | ast::VariableKind::Var => {
            log.report(&HirErr::GlobalVariableMustBeConstOrStatic);
            return Err(());
        }
    }

    let ast_attributes = glb.attributes.to_owned().unwrap_or_default();
    // let _attributes = Vec::with_capacity(ast_attributes.len());
    for _attr in ast_attributes {
        log.report(&HirErr::UnrecognizedGlobalVariableAttribute);
    }

    let is_mutable = match glb.mutability {
        Some(ast::Mutability::Mut) => true,
        Some(ast::Mutability::Const) | None => false,
    };

    let name = glb.name.to_string().into();

    let ty = match glb.ty.to_owned() {
        None => ctx.create_inference_placeholder().into_id(ctx.store()),
        Some(t) => {
            let ty_hir = t.ast2hir(ctx, log)?.into_id(ctx.store());
            ty_hir
        }
    };

    let initializer = match glb.initializer.to_owned() {
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
            let ast_attributes = this.attributes.unwrap_or_default();
            let name = match this.name {
                Some(n) => Some(IString::from(n.to_string())),
                None => None,
            };

            let mut items = Vec::with_capacity(this.items.len());

            for item in this.items {
                match item {
                    ast::Item::Module(submodule) => {
                        let hir_submodule = astmod2hir(*submodule, ctx, log)?.into_id(ctx.store());
                        items.push(Item::Module(hir_submodule));
                    }

                    ast::Item::Import(_) => {
                        // TODO: lower import statements
                        log.report(&HirErr::UnimplementedFeature("import statements".into()));
                    }

                    ast::Item::TypeAlias(_) => {
                        // TODO: lower type aliases
                        log.report(&HirErr::UnimplementedFeature("type aliases".into()));
                    }

                    ast::Item::Struct(_) => {
                        // TODO: lower struct definitions
                        log.report(&HirErr::UnimplementedFeature("structs".into()));
                    }

                    ast::Item::Enum(_) => {
                        // TODO: lower enum definitions
                        log.report(&HirErr::UnimplementedFeature("enums".into()));
                    }

                    ast::Item::Trait(_) => {
                        // TODO: lower trait definitions
                        log.report(&HirErr::UnimplementedFeature("traits".into()));
                    }

                    ast::Item::Impl(_) => {
                        // TODO: lower impl blocks
                        log.report(&HirErr::UnimplementedFeature("impl blocks".into()));
                    }

                    ast::Item::Function(_) => {
                        // TODO: lower function definitions
                        log.report(&HirErr::UnimplementedFeature("functions".into()));
                    }

                    ast::Item::Variable(v) => {
                        let variable = v.read().unwrap();
                        let g = global_variable_ast2hir(&variable, ctx, log)?.into_id(ctx.store());
                        items.push(Item::GlobalVariable(g));
                    }

                    ast::Item::SyntaxError(_) => {
                        continue;
                    }
                }
            }

            let attributes = Vec::with_capacity(ast_attributes.len());
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedModuleAttribute);
            }

            let visibility = match this.visibility {
                Some(ast::Visibility::Public) => Visibility::Pub,
                Some(ast::Visibility::Protected) => Visibility::Pro,
                Some(ast::Visibility::Private) | None => Visibility::Sec,
            };

            let module = Module {
                name,
                visibility,
                attributes,
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
