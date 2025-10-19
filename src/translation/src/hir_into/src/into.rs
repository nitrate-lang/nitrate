use crate::lower::Ast2Hir;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_visitor::HirTypeVisitor;
use nitrate_source::ast;

struct TypeResolution<'store> {
    store: &'store Store,
}

impl HirTypeVisitor<()> for TypeResolution<'_> {
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

    fn visit_struct(&mut self, struct_type: &StructType) -> () {
        for field in &struct_type.fields {
            self.visit_type(&self.store[&field.ty], self.store);
        }
    }

    fn visit_enum(&mut self, enum_type: &EnumType) -> () {
        for variant in &enum_type.variants {
            self.visit_type(&self.store[&variant.ty], self.store);
        }
    }

    fn visit_refine(&mut self, base: &TypeId, min: &LiteralId, max: &LiteralId) -> () {
        self.visit_type(&self.store[base], self.store);
    }

    fn visit_bitfield(&mut self, base: &TypeId, len: u8) -> () {
        self.visit_type(&self.store[base], self.store);
    }

    fn visit_function(&mut self, function_type: &FunctionType) -> () {
        for param in &function_type.params {
            let param = &self.store[param].borrow();
            self.visit_type(&self.store[&param.ty], self.store);
        }
        self.visit_type(&self.store[&function_type.return_type], self.store);
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

pub fn ast_mod2hir(module: ast::Module, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Module, ()> {
    module.ast2hir(ctx, log)
    // TODO: Finalize by resolving symbol links
}

pub fn ast_expr2hir(expr: ast::Expr, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Value, ()> {
    expr.ast2hir(ctx, log)
    // TODO: Finalize by resolving symbol links
}

pub fn ast_type2hir(ty: ast::Type, ctx: &mut HirCtx, log: &CompilerLog) -> Result<Type, ()> {
    let hir_ty = ty.ast2hir(ctx, log)?;

    let mut resolver = TypeResolution { store: ctx.store() };
    resolver.visit_type(&hir_ty, ctx.store());

    Ok(hir_ty)
}
