use crate::{HirCtx, TryIntoHir, diagnosis::HirErr};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_parsetree::kind as ast;

impl TryIntoHir for ast::Module {
    type Hir = Module;

    fn try_into_hir(self, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let ast_attributes = self.attributes.unwrap_or_default();
        let name = match self.name {
            Some(n) => EntityName(n.to_string().into()),
            None => ctx.next_unique_name(),
        };

        let items = Vec::with_capacity(self.items.len());

        for item in self.items {
            match item {
                ast::Item::Module(m) => {
                    log.report(&HirErr::UnimplementedFeature("nested modules".into()));
                }

                ast::Item::Import(i) => {
                    log.report(&HirErr::UnimplementedFeature("import statements".into()));
                }

                ast::Item::TypeAlias(ta) => {
                    log.report(&HirErr::UnimplementedFeature("type aliases".into()));
                }

                ast::Item::Struct(s) => {
                    log.report(&HirErr::UnimplementedFeature("structs".into()));
                }

                ast::Item::Enum(e) => {
                    log.report(&HirErr::UnimplementedFeature("enums".into()));
                }

                ast::Item::Trait(t) => {
                    log.report(&HirErr::UnimplementedFeature("traits".into()));
                }

                ast::Item::Impl(i) => {
                    log.report(&HirErr::UnimplementedFeature("impl blocks".into()));
                }

                ast::Item::Function(f) => {
                    log.report(&HirErr::UnimplementedFeature("functions".into()));
                }

                ast::Item::Variable(v) => {
                    log.report(&HirErr::UnimplementedFeature("global variables".into()));
                }

                ast::Item::SyntaxError(e) => {
                    log.report(&HirErr::UnspecifiedError);
                    continue;
                }
            }
        }

        let attributes = Vec::with_capacity(ast_attributes.len());
        for _attr in ast_attributes {
            log.report(&HirErr::UnrecognizedModuleAttribute);
        }

        let visibility = match self.visibility {
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
}
