use crate::{ast_mod2hir, context::Ast2HirCtx, diagnosis::HirErr, lower::lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_source::ast::{self};
use std::{collections::BTreeSet, ops::Deref};

impl Ast2Hir for ast::Module {
    type Hir = Module;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        fn lower_module(
            this: ast::Module,
            ctx: &mut Ast2HirCtx,
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
                        // TODO: handle submodules
                        let hir_submodule = ast_mod2hir(*submodule, ctx, log)?.into_id(&ctx.store);
                        items.push(Item::Module(hir_submodule));
                    }

                    ast::Item::Import(_) => {
                        // TODO: lower import statements
                        log.report(&HirErr::UnimplementedFeature("import statements".into()));
                    }

                    ast::Item::TypeAlias(type_alias) => {
                        let t = ctx
                            .symbol_tab
                            .get_type_alias(&IString::from(type_alias.name.deref()))
                            .ok_or(())?
                            .to_owned();

                        items.push(Item::TypeAliasDef(t));
                    }

                    ast::Item::Struct(struct_def) => {
                        let s = ctx
                            .symbol_tab
                            .get_struct(&IString::from(struct_def.name.deref()))
                            .ok_or(())?
                            .to_owned();

                        items.push(Item::StructDef(s));
                    }

                    ast::Item::Enum(enum_def) => {
                        let e = ctx
                            .symbol_tab
                            .get_enum(&IString::from(enum_def.name.deref()))
                            .ok_or(())?
                            .to_owned();

                        items.push(Item::EnumDef(e));
                    }

                    ast::Item::Trait(_trait_def) => {
                        // TODO: handle trait definitions
                        unimplemented!()
                    }

                    ast::Item::Impl(_impl_def) => {
                        // TODO: handle impl blocks
                        unimplemented!()
                    }

                    ast::Item::Function(func_def) => {
                        let f = ctx
                            .symbol_tab
                            .get_function(&IString::from(func_def.name.deref()))
                            .ok_or(())?
                            .to_owned();

                        items.push(Item::Function(f));
                    }

                    ast::Item::Variable(v) => {
                        let g = ctx
                            .symbol_tab
                            .get_global_variable(&IString::from(v.name.deref()))
                            .ok_or(())?
                            .to_owned();

                        items.push(Item::GlobalVariable(g));
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

        lower_module(self, ctx, log)
    }
}
