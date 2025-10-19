use crate::lower::Ast2Hir;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_visitor::{HirTypeVisitor, HirValueVisitor, HirVisitor};
use nitrate_source::ast;

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

    fn visit_array(&mut self, element_type: &TypeId, len: u64) -> () {
        self.visit_type(&self.store[element_type], self.store)
    }

    fn visit_tuple(&mut self, element_types: &[TypeId]) -> () {
        for ty in element_types {
            self.visit_type(&self.store[ty], self.store);
        }
    }

    fn visit_slice(&mut self, element_type: &TypeId) -> () {
        self.visit_type(&self.store[element_type], self.store)
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

    fn visit_refine(&mut self, base: &TypeId, min: &LiteralId, max: &LiteralId) -> () {
        self.visit_type(&self.store[base], self.store);
    }

    fn visit_bitfield(&mut self, base: &TypeId, len: u8) -> () {
        self.visit_type(&self.store[base], self.store);
    }

    fn visit_function(
        &mut self,
        _attrs: &std::collections::BTreeSet<FunctionAttribute>,
        params: &[ParameterId],
        ret: &TypeId,
    ) -> () {
        for param in params {
            let param = &self.store[param].borrow();
            self.visit_type(&self.store[&param.ty], self.store);
            if let Some(default_value) = &param.default_value {
                let default = &self.store[default_value].borrow();
                self.visit_value(default, self.store);
            }
        }

        self.visit_type(&self.store[ret], self.store);
    }

    fn visit_reference(
        &mut self,
        _life: &Lifetime,
        _excl: bool,
        _mutable: bool,
        to: &TypeId,
    ) -> () {
        self.visit_type(&self.store[to], self.store);
    }

    fn visit_pointer(&mut self, _excl: bool, _mutable: bool, to: &TypeId) -> () {
        self.visit_type(&self.store[to], self.store);
    }

    fn visit_symbol(&mut self, path: &str, link: &Option<TypeId>) -> () {
        // TODO: Resolve symbol link
        todo!()
    }

    fn visit_inferred_float(&mut self) -> () {}
    fn visit_inferred_integer(&mut self) -> () {}
    fn visit_inferred(&mut self, _id: std::num::NonZero<u32>) -> () {}
}

impl HirValueVisitor<()> for LinkResolver<'_> {
    fn visit_unit(&mut self) -> () {
        todo!()
    }

    fn visit_bool(&mut self, value: bool) -> () {
        todo!()
    }

    fn visit_i8(&mut self, value: i8) -> () {
        todo!()
    }

    fn visit_i16(&mut self, value: i16) -> () {
        todo!()
    }

    fn visit_i32(&mut self, value: i32) -> () {
        todo!()
    }

    fn visit_i64(&mut self, value: i64) -> () {
        todo!()
    }

    fn visit_i128(&mut self, value: i128) -> () {
        todo!()
    }

    fn visit_u8(&mut self, value: u8) -> () {
        todo!()
    }

    fn visit_u16(&mut self, value: u16) -> () {
        todo!()
    }

    fn visit_u32(&mut self, value: u32) -> () {
        todo!()
    }

    fn visit_u64(&mut self, value: u64) -> () {
        todo!()
    }

    fn visit_u128(&mut self, value: u128) -> () {
        todo!()
    }

    fn visit_f32(&mut self, value: ordered_float::OrderedFloat<f32>) -> () {
        todo!()
    }

    fn visit_f64(&mut self, value: ordered_float::OrderedFloat<f64>) -> () {
        todo!()
    }

    fn visit_usize32(&mut self, value: u32) -> () {
        todo!()
    }

    fn visit_usize64(&mut self, value: u64) -> () {
        todo!()
    }

    fn visit_string_lit(&mut self, value: &str) -> () {
        todo!()
    }

    fn visit_bstring_lit(&mut self, value: &[u8]) -> () {
        todo!()
    }

    fn visit_inferred_integer(&mut self, value: u128) -> () {
        todo!()
    }

    fn visit_inferred_float(&mut self, value: ordered_float::OrderedFloat<f64>) -> () {
        todo!()
    }

    fn visit_struct_object(
        &mut self,
        ty: &TypeId,
        fields: &[(interned_string::IString, ValueId)],
    ) -> () {
        todo!()
    }

    fn visit_enum_variant(
        &mut self,
        ty: &TypeId,
        var: &interned_string::IString,
        val: &ValueId,
    ) -> () {
        todo!()
    }

    fn visit_binary(&mut self, left: &ValueId, op: &BinaryOp, right: &ValueId) -> () {
        todo!()
    }

    fn visit_unary(&mut self, op: &UnaryOp, operand: &ValueId) -> () {
        todo!()
    }

    fn visit_field_access(&mut self, expr: &ValueId, field: &interned_string::IString) -> () {
        todo!()
    }

    fn visit_index_access(&mut self, collection: &ValueId, index: &ValueId) -> () {
        todo!()
    }

    fn visit_assign(&mut self, place: &ValueId, value: &ValueId) -> () {
        todo!()
    }

    fn visit_deref(&mut self, place: &ValueId) -> () {
        todo!()
    }

    fn visit_cast(&mut self, expr: &ValueId, to: &TypeId) -> () {
        todo!()
    }

    fn visit_borrow(&mut self, mutable: bool, place: &ValueId) -> () {
        todo!()
    }

    fn visit_list(&mut self, elements: &[Value]) -> () {
        todo!()
    }

    fn visit_tuple(&mut self, elements: &[Value]) -> () {
        todo!()
    }

    fn visit_if(&mut self, cond: &ValueId, true_blk: &BlockId, false_blk: &Option<BlockId>) -> () {
        todo!()
    }

    fn visit_while(&mut self, condition: &ValueId, body: &BlockId) -> () {
        todo!()
    }

    fn visit_loop(&mut self, body: &BlockId) -> () {
        todo!()
    }

    fn visit_break(&mut self, label: &Option<interned_string::IString>) -> () {
        todo!()
    }

    fn visit_continue(&mut self, label: &Option<interned_string::IString>) -> () {
        todo!()
    }

    fn visit_return(&mut self, value: &ValueId) -> () {
        todo!()
    }

    fn visit_block(&mut self, safety: BlockSafety, elements: &[BlockElement]) -> () {
        todo!()
    }

    fn visit_closure(&mut self, captures: &[SymbolId], callee: &FunctionId) -> () {
        todo!()
    }

    fn visit_call(&mut self, callee: &ValueId, arguments: &[ValueId]) -> () {
        todo!()
    }

    fn visit_symbol(&mut self, path: &interned_string::IString, link: &Option<SymbolId>) -> () {
        todo!()
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
