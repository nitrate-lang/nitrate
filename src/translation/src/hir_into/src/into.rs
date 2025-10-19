use crate::lower::Ast2Hir;
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_visitor::{HirTypeVisitor, HirValueVisitor, HirVisitor};
use nitrate_source::ast;
use ordered_float::OrderedFloat;

struct LinkResolver<'store> {
    store: &'store Store,
}

impl HirTypeVisitor<()> for LinkResolver<'_> {
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
    fn visit_opaque(&mut self, _name: &str) -> () {}

    fn visit_array(&mut self, element_type: &Type, _len: u64) -> () {
        self.visit_type(element_type, self.store)
    }

    fn visit_tuple(&mut self, element_types: &[TypeId]) -> () {
        for ty in element_types {
            self.visit_type(&self.store[ty], self.store);
        }
    }

    fn visit_slice(&mut self, element_type: &Type) -> () {
        self.visit_type(element_type, self.store)
    }

    fn visit_struct(
        &mut self,
        _attrs: &std::collections::BTreeSet<StructAttribute>,
        fields: &[StructField],
    ) -> () {
        for field in fields {
            self.visit_type(&self.store[&field.ty], self.store);
        }
    }

    fn visit_enum(
        &mut self,
        _attrs: &std::collections::BTreeSet<EnumAttribute>,
        variants: &[EnumVariant],
    ) -> () {
        for variant in variants {
            self.visit_type(&self.store[&variant.ty], self.store);
        }
    }

    fn visit_refine(&mut self, base: &Type, _min: &LiteralId, _max: &LiteralId) -> () {
        self.visit_type(base, self.store);
    }

    fn visit_bitfield(&mut self, base: &Type, _len: u8) -> () {
        self.visit_type(base, self.store);
    }

    fn visit_function(
        &mut self,
        _attrs: &std::collections::BTreeSet<FunctionAttribute>,
        params: &[ParameterId],
        ret: &Type,
    ) -> () {
        for param in params {
            let param = &self.store[param].borrow();
            self.visit_type(&self.store[&param.ty], self.store);
            if let Some(default_value) = &param.default_value {
                let default = &self.store[default_value].borrow();
                self.visit_value(default, self.store);
            }
        }

        self.visit_type(ret, self.store);
    }

    fn visit_reference(&mut self, _life: &Lifetime, _excl: bool, _mutable: bool, to: &Type) -> () {
        self.visit_type(to, self.store);
    }

    fn visit_pointer(&mut self, _excl: bool, _mutable: bool, to: &Type) -> () {
        self.visit_type(to, self.store);
    }

    fn visit_symbol(&mut self, _path: &str, _link: &Option<TypeId>) -> () {
        // TODO: Resolve symbol link
        unimplemented!()
    }

    fn visit_inferred_float(&mut self) -> () {}
    fn visit_inferred_integer(&mut self) -> () {}
    fn visit_inferred(&mut self, _id: std::num::NonZero<u32>) -> () {}
}

impl HirValueVisitor<()> for LinkResolver<'_> {
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
        self.visit_type(ty, self.store);
        for (_name, val) in fields {
            self.visit_value(&self.store[val].borrow(), self.store);
        }
    }

    fn visit_enum_variant(&mut self, ty: &Type, _var: &IString, val: &Value) -> () {
        self.visit_type(ty, self.store);
        self.visit_value(val, self.store);
    }

    fn visit_binary(&mut self, left: &Value, _op: &BinaryOp, right: &Value) -> () {
        self.visit_value(left, self.store);
        self.visit_value(right, self.store);
    }

    fn visit_unary(&mut self, _op: &UnaryOp, operand: &Value) -> () {
        self.visit_value(operand, self.store);
    }

    fn visit_field_access(&mut self, expr: &Value, _field: &IString) -> () {
        self.visit_value(expr, self.store);
    }

    fn visit_index_access(&mut self, collection: &Value, index: &Value) -> () {
        self.visit_value(collection, self.store);
        self.visit_value(index, self.store);
    }

    fn visit_assign(&mut self, place: &Value, value: &Value) -> () {
        self.visit_value(place, self.store);
        self.visit_value(value, self.store);
    }

    fn visit_deref(&mut self, place: &Value) -> () {
        self.visit_value(place, self.store);
    }

    fn visit_cast(&mut self, expr: &Value, to: &Type) -> () {
        self.visit_value(expr, self.store);
        self.visit_type(to, self.store);
    }

    fn visit_borrow(&mut self, _mutable: bool, place: &Value) -> () {
        self.visit_value(place, self.store);
    }

    fn visit_list(&mut self, elements: &[Value]) -> () {
        for element in elements {
            self.visit_value(element, self.store);
        }
    }

    fn visit_tuple(&mut self, elements: &[Value]) -> () {
        for element in elements {
            self.visit_value(element, self.store);
        }
    }

    fn visit_if(&mut self, cond: &Value, true_blk: &Block, false_blk: Option<&Block>) -> () {
        self.visit_value(cond, self.store);
        self.visit_block(true_blk.safety, &true_blk.elements);

        if let Some(false_blk) = false_blk {
            self.visit_block(false_blk.safety, &false_blk.elements);
        }
    }

    fn visit_while(&mut self, condition: &Value, body: &Block) -> () {
        self.visit_value(condition, self.store);
        self.visit_block(body.safety, &body.elements);
    }

    fn visit_loop(&mut self, body: &Block) -> () {
        self.visit_block(body.safety, &body.elements);
    }

    fn visit_break(&mut self, _label: &Option<IString>) -> () {}
    fn visit_continue(&mut self, _label: &Option<IString>) -> () {}

    fn visit_return(&mut self, value: &Value) -> () {
        self.visit_value(value, self.store);
    }

    fn visit_block(&mut self, _safety: BlockSafety, elements: &[BlockElement]) -> () {
        for element in elements {
            match element {
                BlockElement::Expr(expr) => {
                    self.visit_value(&self.store[expr].borrow(), self.store);
                }

                BlockElement::Stmt(expr) => {
                    self.visit_value(&self.store[expr].borrow(), self.store);
                }

                BlockElement::Local(local) => {
                    // TODO: Handle local initializers
                    let _local = &self.store[local].borrow();
                    unimplemented!("local variable visitation in link resolver");
                }
            }
        }
    }

    fn visit_closure(&mut self, _captures: &[SymbolId], callee: &Function) -> () {
        self.visit_function(
            &callee.attributes,
            &callee.params,
            &self.store[&callee.return_type],
        );
    }

    fn visit_call(&mut self, callee: &Value, arguments: &[ValueId]) -> () {
        self.visit_value(callee, self.store);
        for arg in arguments {
            self.visit_value(&self.store[arg].borrow(), self.store);
        }
    }

    fn visit_symbol(&mut self, _path: &IString, _link: &Option<SymbolId>) -> () {
        // TODO: Resolve symbol link
        unimplemented!()
    }
}

impl HirVisitor<()> for LinkResolver<'_> {}

pub fn ast_mod2hir(module: ast::Module, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Module, ()> {
    module.ast2hir(ctx, log)
    // TODO: Finalize by resolving symbol links
}

pub fn ast_expr2hir(expr: ast::Expr, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    let hir_value = expr.ast2hir(ctx, log)?;

    let mut resolver = LinkResolver { store: ctx.store() };
    resolver.visit_value(&hir_value, ctx.store());

    Ok(hir_value)
}

pub fn ast_type2hir(ty: ast::Type, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    let hir_ty = ty.ast2hir(ctx, log)?;

    let mut resolver = LinkResolver { store: ctx.store() };
    resolver.visit_type(&hir_ty, ctx.store());

    Ok(hir_ty)
}
