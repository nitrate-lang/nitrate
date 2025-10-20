use crate::{diagnosis::HirErr, lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_visitor::{HirItemVisitor, HirTypeVisitor, HirValueVisitor, HirVisitor};
use nitrate_source::ast;
use once_cell_serde::sync::OnceCell;
use ordered_float::OrderedFloat;

struct LinkResolver<'ctx, 'log> {
    ctx: &'ctx HirCtx,
    log: &'log CompilerLog,
}

impl HirTypeVisitor<()> for LinkResolver<'_, '_> {
    fn visit_never(&mut self) -> () {}
    fn visit_unit(&mut self) -> () {}
    fn visit_bool(&mut self) -> () {}
    fn visit_u8(&mut self) -> () {}
    fn visit_u16(&mut self) -> () {}
    fn visit_u32(&mut self) -> () {}
    fn visit_u64(&mut self) -> () {}
    fn visit_u128(&mut self) -> () {}
    fn visit_usize(&mut self) -> () {}
    fn visit_i8(&mut self) -> () {}
    fn visit_i16(&mut self) -> () {}
    fn visit_i32(&mut self) -> () {}
    fn visit_i64(&mut self) -> () {}
    fn visit_i128(&mut self) -> () {}
    fn visit_f32(&mut self) -> () {}
    fn visit_f64(&mut self) -> () {}
    fn visit_opaque(&mut self, _name: &IString) -> () {}

    fn visit_array(&mut self, element_type: &Type, _len: u64) -> () {
        self.visit_type(element_type, self.ctx.store())
    }

    fn visit_tuple(&mut self, element_types: &[TypeId]) -> () {
        for ty in element_types {
            self.visit_type(&self.ctx[ty], self.ctx.store());
        }
    }

    fn visit_slice(&mut self, element_type: &Type) -> () {
        self.visit_type(element_type, self.ctx.store())
    }

    fn visit_struct(
        &mut self,
        _attrs: &std::collections::BTreeSet<StructAttribute>,
        fields: &[StructField],
    ) -> () {
        for field in fields {
            self.visit_type(&self.ctx[&field.ty], self.ctx.store());
        }
    }

    fn visit_enum(
        &mut self,
        _attrs: &std::collections::BTreeSet<EnumAttribute>,
        variants: &[EnumVariant],
    ) -> () {
        for variant in variants {
            self.visit_type(&self.ctx[&variant.ty], self.ctx.store());
        }
    }

    fn visit_refine(&mut self, base: &Type, _min: &LiteralId, _max: &LiteralId) -> () {
        self.visit_type(base, self.ctx.store());
    }

    fn visit_bitfield(&mut self, base: &Type, _len: u8) -> () {
        self.visit_type(base, self.ctx.store());
    }

    fn visit_function_type(
        &mut self,
        _attrs: &std::collections::BTreeSet<FunctionAttribute>,
        params: &[ParameterId],
        ret: &Type,
    ) -> () {
        for param in params {
            let param = &self.ctx[param].borrow();
            self.visit_type(&self.ctx[&param.ty], self.ctx.store());
            if let Some(default_value) = &param.default_value {
                let default = &self.ctx[default_value].borrow();
                self.visit_value(default, self.ctx.store());
            }
        }

        self.visit_type(ret, self.ctx.store());
    }

    fn visit_reference(&mut self, _life: &Lifetime, _excl: bool, _mutable: bool, to: &Type) -> () {
        self.visit_type(to, self.ctx.store());
    }

    fn visit_pointer(&mut self, _excl: bool, _mutable: bool, to: &Type) -> () {
        self.visit_type(to, self.ctx.store());
    }

    fn visit_symbol(&mut self, path: &IString, link: &OnceCell<TypeId>) -> () {
        if link.get().is_some() {
            return;
        }

        match self.ctx.lookup_type(path) {
            None => self.log.report(&HirErr::UnresolvedTypePath),

            Some(TypeDefinition::EnumDef(enum_def)) => {
                // TODO: check visibility

                let enum_type = Type::Enum {
                    enum_type: self.ctx[enum_def].borrow().enum_id,
                };

                link.set(enum_type.into_id(self.ctx.store()))
                    .expect("unexpected value in cell");
            }

            Some(TypeDefinition::StructDef(struct_def)) => {
                // TODO: check visibility

                let struct_type = Type::Struct {
                    struct_type: self.ctx[struct_def].borrow().struct_id,
                };

                link.set(struct_type.into_id(self.ctx.store()))
                    .expect("unexpected value in cell");
            }

            Some(TypeDefinition::TypeAliasDef(type_alias_def)) => {
                // TODO: check visibility

                let aliased_type = self.ctx[type_alias_def].borrow().type_id;
                link.set(aliased_type).expect("unexpected value in cell");
            }
        }
    }

    fn visit_inferred_float(&mut self) -> () {}
    fn visit_inferred_integer(&mut self) -> () {}
    fn visit_inferred(&mut self, _id: std::num::NonZero<u32>) -> () {}
}

impl HirValueVisitor<()> for LinkResolver<'_, '_> {
    fn visit_unit(&mut self) -> () {}
    fn visit_bool(&mut self, _value: bool) -> () {}
    fn visit_i8(&mut self, _value: i8) -> () {}
    fn visit_i16(&mut self, _value: i16) -> () {}
    fn visit_i32(&mut self, _value: i32) -> () {}
    fn visit_i64(&mut self, _value: i64) -> () {}
    fn visit_i128(&mut self, _value: i128) -> () {}
    fn visit_u8(&mut self, _value: u8) -> () {}
    fn visit_u16(&mut self, _value: u16) -> () {}
    fn visit_u32(&mut self, _value: u32) -> () {}
    fn visit_u64(&mut self, _value: u64) -> () {}
    fn visit_u128(&mut self, _value: u128) -> () {}
    fn visit_f32(&mut self, _value: OrderedFloat<f32>) -> () {}
    fn visit_f64(&mut self, _value: OrderedFloat<f64>) -> () {}
    fn visit_usize32(&mut self, _value: u32) -> () {}
    fn visit_usize64(&mut self, _value: u64) -> () {}
    fn visit_string_lit(&mut self, _value: &str) -> () {}
    fn visit_bstring_lit(&mut self, _value: &[u8]) -> () {}
    fn visit_inferred_integer(&mut self, _value: u128) -> () {}
    fn visit_inferred_float(&mut self, _value: OrderedFloat<f64>) -> () {}

    fn visit_struct_object(&mut self, ty: &Type, fields: &[(IString, ValueId)]) -> () {
        self.visit_type(ty, self.ctx.store());
        for (_name, val) in fields {
            self.visit_value(&self.ctx[val].borrow(), self.ctx.store());
        }
    }

    fn visit_enum_variant(&mut self, ty: &Type, _var: &IString, val: &Value) -> () {
        self.visit_type(ty, self.ctx.store());
        self.visit_value(val, self.ctx.store());
    }

    fn visit_binary(&mut self, left: &Value, _op: &BinaryOp, right: &Value) -> () {
        self.visit_value(left, self.ctx.store());
        self.visit_value(right, self.ctx.store());
    }

    fn visit_unary(&mut self, _op: &UnaryOp, operand: &Value) -> () {
        self.visit_value(operand, self.ctx.store());
    }

    fn visit_field_access(&mut self, expr: &Value, _field: &IString) -> () {
        self.visit_value(expr, self.ctx.store());
    }

    fn visit_index_access(&mut self, collection: &Value, index: &Value) -> () {
        self.visit_value(collection, self.ctx.store());
        self.visit_value(index, self.ctx.store());
    }

    fn visit_assign(&mut self, place: &Value, value: &Value) -> () {
        self.visit_value(place, self.ctx.store());
        self.visit_value(value, self.ctx.store());
    }

    fn visit_deref(&mut self, place: &Value) -> () {
        self.visit_value(place, self.ctx.store());
    }

    fn visit_cast(&mut self, expr: &Value, to: &Type) -> () {
        self.visit_value(expr, self.ctx.store());
        self.visit_type(to, self.ctx.store());
    }

    fn visit_borrow(&mut self, _mutable: bool, place: &Value) -> () {
        self.visit_value(place, self.ctx.store());
    }

    fn visit_list(&mut self, elements: &[Value]) -> () {
        for element in elements {
            self.visit_value(element, self.ctx.store());
        }
    }

    fn visit_tuple(&mut self, elements: &[Value]) -> () {
        for element in elements {
            self.visit_value(element, self.ctx.store());
        }
    }

    fn visit_if(&mut self, cond: &Value, true_blk: &Block, false_blk: Option<&Block>) -> () {
        self.visit_value(cond, self.ctx.store());
        self.visit_block(true_blk.safety, &true_blk.elements);

        if let Some(false_blk) = false_blk {
            self.visit_block(false_blk.safety, &false_blk.elements);
        }
    }

    fn visit_while(&mut self, condition: &Value, body: &Block) -> () {
        self.visit_value(condition, self.ctx.store());
        self.visit_block(body.safety, &body.elements);
    }

    fn visit_loop(&mut self, body: &Block) -> () {
        self.visit_block(body.safety, &body.elements);
    }

    fn visit_break(&mut self, _label: &Option<IString>) -> () {}
    fn visit_continue(&mut self, _label: &Option<IString>) -> () {}

    fn visit_return(&mut self, value: &Value) -> () {
        self.visit_value(value, self.ctx.store());
    }

    fn visit_block(&mut self, _safety: BlockSafety, elements: &[BlockElement]) -> () {
        for element in elements {
            match element {
                BlockElement::Expr(expr) => {
                    self.visit_value(&self.ctx[expr].borrow(), self.ctx.store());
                }

                BlockElement::Stmt(expr) => {
                    self.visit_value(&self.ctx[expr].borrow(), self.ctx.store());
                }

                BlockElement::Local(local) => {
                    let local = &self.ctx[local].borrow();
                    self.visit_type(&self.ctx[&local.ty], self.ctx.store());
                    if let Some(init) = &local.initializer {
                        let init = &self.ctx[init].borrow();
                        self.visit_value(init, self.ctx.store());
                    }
                }
            }
        }
    }

    fn visit_closure(&mut self, _captures: &[SymbolId], callee: &Function) -> () {
        match &callee.body {
            Some(body_id) => {
                self.visit_function(
                    callee.visibility,
                    &callee.attributes,
                    &callee.name,
                    &callee.params,
                    &self.ctx[&callee.return_type],
                    Some(&self.ctx[body_id].borrow()),
                );
            }

            None => {
                self.visit_function(
                    callee.visibility,
                    &callee.attributes,
                    &callee.name,
                    &callee.params,
                    &self.ctx[&callee.return_type],
                    None,
                );
            }
        }
    }

    fn visit_call(&mut self, callee: &Value, arguments: &[ValueId]) -> () {
        self.visit_value(callee, self.ctx.store());
        for arg in arguments {
            self.visit_value(&self.ctx[arg].borrow(), self.ctx.store());
        }
    }

    fn visit_symbol(&mut self, path: &IString, link: &OnceCell<SymbolId>) -> () {
        if link.get().is_some() {
            return;
        }

        let id = match self.ctx.lookup_symbol(path) {
            Some(id) => id.to_owned(),
            None => {
                self.log.report(&HirErr::UnresolvedSymbol);
                return;
            }
        };

        match &id {
            SymbolId::Function(_function) => {
                // TODO: check visibility
                link.set(id).expect("unexpected value in cell");
            }

            SymbolId::GlobalVariable(_global) => {
                // TODO: check visibility
                link.set(id).expect("unexpected value in cell");
            }

            SymbolId::LocalVariable(_local) => {
                // TODO: check visibility
                link.set(id).expect("unexpected value in cell");
            }

            SymbolId::Parameter(_parameter) => {
                // TODO: check visibility
                link.set(id).expect("unexpected value in cell");
            }
        }
    }
}

impl HirItemVisitor<()> for LinkResolver<'_, '_> {
    fn visit_module(
        &mut self,
        _vis: Visibility,
        _name: Option<&IString>,
        _attrs: &std::collections::BTreeSet<ModuleAttribute>,
        items: &[Item],
    ) -> () {
        for item in items {
            self.visit_item(item, self.ctx.store());
        }
    }

    fn visit_global_variable(
        &mut self,
        _vis: Visibility,
        _attrs: &std::collections::BTreeSet<GlobalVariableAttribute>,
        _is_mutable: bool,
        _name: &IString,
        ty: &Type,
        init: &Value,
    ) -> () {
        self.visit_type(ty, self.ctx.store());
        self.visit_value(init, self.ctx.store());
    }

    fn visit_function(
        &mut self,
        _vis: Visibility,
        _attrs: &std::collections::BTreeSet<FunctionAttribute>,
        _name: &IString,
        params: &[ParameterId],
        ret: &Type,
        body: Option<&Block>,
    ) -> () {
        for param in params {
            let param = &self.ctx[param].borrow();
            self.visit_type(&self.ctx[&param.ty], self.ctx.store());

            if let Some(default_value) = &param.default_value {
                let default = &self.ctx[default_value].borrow();
                self.visit_value(default, self.ctx.store());
            }
        }

        self.visit_type(ret, self.ctx.store());

        if let Some(body) = body {
            self.visit_block(body.safety, &body.elements);
        }
    }

    fn visit_type_alias(&mut self, _vis: Visibility, _name: &IString, ty: &Type) -> () {
        self.visit_type(ty, self.ctx.store());
    }

    fn visit_struct_def(
        &mut self,
        _vis: Visibility,
        _name: &IString,
        fields_extra: &[(Visibility, Option<ValueId>)],
        struct_ty: &StructTypeId,
    ) -> () {
        for (_vis, initializer) in fields_extra {
            if let Some(initializer) = initializer {
                let init = &self.ctx[initializer].borrow();
                self.visit_value(init, self.ctx.store());
            }
        }

        let struct_def = &self.ctx[struct_ty];
        for field in &struct_def.fields {
            self.visit_type(&self.ctx[&field.ty], self.ctx.store());
        }
    }

    fn visit_enum_def(
        &mut self,
        _vis: Visibility,
        _name: &IString,
        variants: &[Option<ValueId>],
        enum_ty: &EnumTypeId,
    ) -> () {
        for variant_initializer in variants {
            if let Some(initializer) = variant_initializer {
                let init = &self.ctx[initializer].borrow();
                self.visit_value(init, self.ctx.store());
            }
        }

        let enum_def = &self.ctx[enum_ty];
        for variant in &enum_def.variants {
            self.visit_type(&self.ctx[&variant.ty], self.ctx.store());
        }
    }
}

impl HirVisitor<()> for LinkResolver<'_, '_> {}

pub fn ast_mod2hir(module: ast::Module, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Module, ()> {
    let hir_module = module.ast2hir(ctx, log)?;

    let mut resolver = LinkResolver { ctx, log };
    resolver.visit_module(
        hir_module.visibility,
        hir_module.name.as_ref(),
        &hir_module.attributes,
        &hir_module.items,
    );

    Ok(hir_module)
}

pub fn ast_expr2hir(expr: ast::Expr, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let hir_value = expr.ast2hir(ctx, log)?;

    let mut resolver = LinkResolver { ctx, log };
    resolver.visit_value(&hir_value, ctx.store());

    Ok(hir_value)
}

pub fn ast_type2hir(ty: ast::Type, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    let hir_ty = ty.ast2hir(ctx, log)?;

    let mut resolver = LinkResolver { ctx, log };
    resolver.visit_type(&hir_ty, ctx.store());

    Ok(hir_ty)
}
