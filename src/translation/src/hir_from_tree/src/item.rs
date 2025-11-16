use crate::{context::Ast2HirCtx, convert_ast_to_hir, diagnosis::HirErr, lower::Ast2Hir};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_nstring::NString;
use nitrate_tree::ast::{self};
use std::collections::BTreeSet;

impl Ast2Hir for ast::TypeAlias {
    type Hir = TypeAliasDefId;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let visibility = match self.visibility {
            Some(ast::Visibility::Public) => Visibility::Pub,
            Some(ast::Visibility::Protected) => Visibility::Pro,
            Some(ast::Visibility::Private) | None => Visibility::Sec,
        };

        if let Some(ast_attributes) = &self.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedTypeAliasAttribute);
            }
        }

        let name = ctx.qualify_name(&self.name).into();

        if self.generics.is_some() {
            // TODO: support generic type aliases
            log.report(&HirErr::UnimplementedFeature("generic type aliases".into()));
        }

        let type_id = match &self.alias_type {
            Some(ty) => ty.to_owned().ast2hir(ctx, log)?.into_id(&ctx.store),
            None => {
                log.report(&HirErr::TypeAliasMustHaveType);
                return Err(());
            }
        };

        let type_alias_id = TypeAliasDef {
            visibility,
            name,
            type_id,
        }
        .into_id(&ctx.store);

        let definition = TypeDefinition::TypeAliasDef(type_alias_id.clone());
        ctx.tab.add_type(definition, &ctx.store);

        Ok(type_alias_id)
    }
}

impl Ast2Hir for ast::Struct {
    type Hir = StructDefId;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let visibility = match self.visibility {
            Some(ast::Visibility::Public) => Visibility::Pub,
            Some(ast::Visibility::Protected) => Visibility::Pro,
            Some(ast::Visibility::Private) | None => Visibility::Sec,
        };

        let attributes = BTreeSet::new();
        if let Some(ast_attributes) = &self.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedStructAttribute);
            }
        }

        let name = ctx.qualify_name(&self.name).into();

        if self.generics.is_some() {
            // TODO: support generic structs
            log.report(&HirErr::UnimplementedFeature("generic structs".into()));
        }

        let mut field_extras = Vec::new();
        let mut fields = Vec::new();

        for field in &self.fields {
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

        let struct_id = StructType {
            attributes,
            fields: fields.into(),
        }
        .into_id(&ctx.store);

        let struct_def_id = StructDef {
            visibility,
            name,
            field_extras,
            struct_id,
        }
        .into_id(&ctx.store);

        let definition = TypeDefinition::StructDef(struct_def_id.clone());
        ctx.tab.add_type(definition, &ctx.store);

        Ok(struct_def_id)
    }
}

impl Ast2Hir for ast::Enum {
    type Hir = EnumDefId;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let visibility = match self.visibility {
            Some(ast::Visibility::Public) => Visibility::Pub,
            Some(ast::Visibility::Protected) => Visibility::Pro,
            Some(ast::Visibility::Private) | None => Visibility::Sec,
        };

        let attributes = BTreeSet::new();
        if let Some(ast_attributes) = &self.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedEnumAttribute);
            }
        }

        let name = ctx.qualify_name(&self.name).into();

        if self.generics.is_some() {
            // TODO: support generic enums
            log.report(&HirErr::UnimplementedFeature("generic enums".into()));
        }

        let mut variants = Vec::new();
        let mut variant_extras = Vec::new();

        for variant in &self.variants {
            let variant_attributes = BTreeSet::new();
            if let Some(ast_attributes) = &variant.attributes {
                for _attr in ast_attributes {
                    log.report(&HirErr::UnrecognizedEnumVariantAttribute);
                }
            }

            let variant_name = NString::from(variant.name.to_string());

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
            variants: variants.into(),
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
        ctx.tab.add_type(definition, &ctx.store);

        Ok(enum_def_id)
    }
}

fn ast_trait2hir(_trait: &ast::Trait, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<(), ()> {
    // TODO: implement trait lowering
    log.report(&HirErr::UnimplementedFeature("trait definitions".into()));
    Err(())
}

fn ast_impl2hir(_impl: &ast::Impl, _ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<(), ()> {
    // TODO: implement impl block lowering
    log.report(&HirErr::UnimplementedFeature("impl blocks".into()));
    Err(())
}

impl Ast2Hir for ast::GlobalVariable {
    type Hir = GlobalVariableId;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let visibility = match self.visibility {
            Some(ast::Visibility::Public) => Visibility::Pub,
            Some(ast::Visibility::Protected) => Visibility::Pro,
            Some(ast::Visibility::Private) | None => Visibility::Sec,
        };

        let attributes = BTreeSet::new();
        if let Some(ast_attributes) = &self.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedGlobalVariableAttribute);
            }
        }

        let is_mutable = match self.mutability {
            Some(ast::Mutability::Mut) => true,
            Some(ast::Mutability::Const) | None => false,
        };

        let name = ctx.qualify_name(&self.name).into();
        let mangled_name = if attributes.contains(&GlobalVariableAttribute::NoMangle) {
            self.name.clone().into()
        } else {
            ctx.qualify_name(&self.name).into()
        };

        let ty = match self.ty.to_owned() {
            None => ctx.create_inference_placeholder().into_id(&ctx.store),
            Some(t) => {
                let ty_hir = t.ast2hir(ctx, log)?.into_id(&ctx.store);
                ty_hir
            }
        };

        let init = match self.initializer.to_owned() {
            Some(expr) => {
                let expr_hir = expr.ast2hir(ctx, log)?.into_id(&ctx.store);
                expr_hir
            }

            None => {
                log.report(&HirErr::GlobalVariableMustHaveInitializer);
                return Err(());
            }
        };

        let global_variable_id = GlobalVariable {
            visibility,
            attributes,
            is_mutable,
            name,
            mangled_name,
            ty,
            init,
        }
        .into_id(&ctx.store);

        let variable = SymbolId::GlobalVariable(global_variable_id.clone());
        ctx.tab.add_symbol(variable, &ctx.store);

        Ok(global_variable_id)
    }
}

impl Ast2Hir for ast::FuncParam {
    type Hir = ParameterId;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let attributes = BTreeSet::new();
        if let Some(ast_attributes) = &self.attributes {
            for _attr in ast_attributes {
                log.report(&HirErr::UnrecognizedFunctionParameterAttribute);
            }
        }

        let is_mutable = match self.mutability {
            Some(ast::Mutability::Mut) => true,
            Some(ast::Mutability::Const) | None => false,
        };

        let name = ctx.qualify_name(&self.name).into();
        let ty = self.ty.to_owned().ast2hir(ctx, log)?.into_id(&ctx.store);

        let default_value = match self.default_value.to_owned() {
            Some(expr) => Some(expr.ast2hir(ctx, log)?.into_id(&ctx.store)),
            None => None,
        };

        let parameter_id = Parameter {
            attributes,
            is_mutable,
            name,
            ty,
            default_value,
        }
        .into_id(&ctx.store);

        let parameter = SymbolId::Parameter(parameter_id.clone());
        ctx.tab.add_symbol(parameter, &ctx.store);

        Ok(parameter_id)
    }
}

impl Ast2Hir for ast::Function {
    type Hir = FunctionId;

    fn ast2hir(self, ctx: &mut Ast2HirCtx, log: &CompilerLog) -> Result<Self::Hir, ()> {
        let visibility = match self.visibility {
            Some(ast::Visibility::Public) => Visibility::Pub,
            Some(ast::Visibility::Protected) => Visibility::Pro,
            Some(ast::Visibility::Private) | None => Visibility::Sec,
        };

        let mut attributes = BTreeSet::new();
        if let Some(ast_attributes) = &self.attributes {
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

        let name: NString = ctx.qualify_name(&self.name).into();
        let mangled_name: NString = if attributes.contains(&FunctionAttribute::NoMangle) {
            self.name.clone().into()
        } else {
            ctx.qualify_name(&self.name).into()
        };

        ctx.current_scope.push(self.name.clone());

        if self.generics.is_some() {
            // TODO: support generic functions
            log.report(&HirErr::UnimplementedFeature("generic functions".into()));
        }

        let mut parameters = Vec::with_capacity(self.parameters.len());
        for param in &self.parameters {
            let param_hir = param.to_owned().ast2hir(ctx, log)?;
            parameters.push(param_hir);
        }

        let return_type = match &self.return_type {
            Some(ty) => ty.to_owned().ast2hir(ctx, log)?,
            None => Type::Unit,
        };

        let body = match self.definition {
            None => None,
            Some(block) => {
                let mut hir_block = block.ast2hir(ctx, log)?;
                match hir_block.elements.last() {
                    Some(BlockElement::Expr(expr)) if ctx.store[expr].borrow().is_return() => {}

                    Some(BlockElement::Expr(expr)) if !ctx.store[expr].borrow().is_return() => {
                        *hir_block.elements.last_mut().unwrap() = BlockElement::Expr(
                            Value::Return {
                                value: expr.to_owned(),
                            }
                            .into_id(&ctx.store),
                        );
                    }

                    _ if return_type == Type::Unit => {
                        hir_block.elements.push(BlockElement::Expr(
                            Value::Return {
                                value: Value::Unit.into_id(&ctx.store),
                            }
                            .into_id(&ctx.store),
                        ));
                    }

                    _ => log.report(&HirErr::MissingReturnStatement),
                }

                Some(hir_block.into_id(&ctx.store))
            }
        };

        let function_id = Function {
            visibility,
            attributes,
            name,
            mangled_name,
            params: parameters,
            return_type: return_type.into_id(&ctx.store),
            body,
        }
        .into_id(&ctx.store);

        let function = SymbolId::Function(function_id.clone());
        ctx.tab.add_symbol(function, &ctx.store);

        ctx.current_scope.pop();

        Ok(function_id)
    }
}

fn lower_item(
    ctx: &mut Ast2HirCtx,
    current_module_items: &mut Vec<Item>,
    item: ast::Item,
    log: &CompilerLog,
) -> Result<(), ()> {
    match item {
        ast::Item::Module(module) => {
            let hir_module = convert_ast_to_hir(*module, ctx, log)?.into_id(&ctx.store);
            current_module_items.push(Item::Module(hir_module));
            Ok(())
        }

        ast::Item::Import(import) => {
            if let Some(resolved_items) = import.resolved {
                for item in resolved_items {
                    lower_item(ctx, current_module_items, item, log)?;
                }
            }
            Ok(())
        }

        ast::Item::TypeAlias(type_alias) => {
            let t = type_alias.ast2hir(ctx, log)?;
            current_module_items.push(Item::TypeAliasDef(t));
            Ok(())
        }

        ast::Item::Struct(struct_def) => {
            let s = struct_def.ast2hir(ctx, log)?;
            current_module_items.push(Item::StructDef(s));
            Ok(())
        }

        ast::Item::Enum(enum_def) => {
            let e = enum_def.ast2hir(ctx, log)?;
            current_module_items.push(Item::EnumDef(e));
            Ok(())
        }

        ast::Item::Trait(trait_def) => {
            ast_trait2hir(&trait_def, ctx, log)?;
            Ok(())
        }

        ast::Item::Impl(impl_def) => {
            ast_impl2hir(&impl_def, ctx, log)?;
            Ok(())
        }

        ast::Item::Function(func_def) => {
            let f = func_def.ast2hir(ctx, log)?;
            current_module_items.push(Item::Function(f));
            Ok(())
        }

        ast::Item::Variable(v) => {
            let g = v.ast2hir(ctx, log)?;
            current_module_items.push(Item::GlobalVariable(g));
            Ok(())
        }

        ast::Item::SyntaxError(_) => Ok(()),
    }
}

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

            let mut items = Vec::with_capacity(this.items.len());
            for item in this.items {
                lower_item(ctx, &mut items, item, log)?;
            }

            let module = Module {
                visibility,
                attributes,
                name: this.name,
                items,
            };

            Ok(module)
        }

        ctx.current_scope.push(self.name.clone());
        let result = lower_module(self, ctx, log);
        ctx.current_scope.pop();
        result
    }
}
