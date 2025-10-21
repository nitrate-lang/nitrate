use crate::Ast2HirCtx;
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_visitor::{HirItemVisitor, HirTypeVisitor, HirValueVisitor, HirVisitor};
use ordered_float::OrderedFloat;
use std::{collections::BTreeSet, num::NonZero};

struct DefaultCallArguments;

impl HirItemVisitor<()> for DefaultCallArguments {
    fn visit_module(
        &mut self,
        vis: Visibility,
        name: Option<&IString>,
        attrs: &BTreeSet<ModuleAttribute>,
        items: &[Item],
    ) -> () {
        todo!()
    }

    fn visit_global_variable(
        &mut self,
        vis: Visibility,
        attrs: &BTreeSet<GlobalVariableAttribute>,
        is_mutable: bool,
        name: &IString,
        ty: &Type,
        init: Option<&Value>,
    ) -> () {
        todo!()
    }

    fn visit_function(
        &mut self,
        vis: Visibility,
        attrs: &BTreeSet<FunctionAttribute>,
        name: &IString,
        params: &[ParameterId],
        ret: &Type,
        body: Option<&Block>,
    ) -> () {
        todo!()
    }

    fn visit_type_alias(&mut self, vis: Visibility, name: &IString, ty: &Type) -> () {
        todo!()
    }

    fn visit_struct_def(
        &mut self,
        vis: Visibility,
        name: &IString,
        fields_extra: &[(Visibility, Option<ValueId>)],
        struct_ty: &StructTypeId,
    ) -> () {
        todo!()
    }

    fn visit_enum_def(
        &mut self,
        vis: Visibility,
        name: &IString,
        variants: &[Option<ValueId>],
        enum_ty: &EnumTypeId,
    ) -> () {
        todo!()
    }
}

impl HirValueVisitor<()> for DefaultCallArguments {
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

    fn visit_f32(&mut self, value: OrderedFloat<f32>) -> () {
        todo!()
    }

    fn visit_f64(&mut self, value: OrderedFloat<f64>) -> () {
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

    fn visit_inferred_float(&mut self, value: OrderedFloat<f64>) -> () {
        todo!()
    }

    fn visit_struct_object(&mut self, struct_path: &IString, fields: &[(IString, ValueId)]) -> () {
        todo!()
    }

    fn visit_enum_variant(&mut self, ty: &EnumType, var: &IString, val: &Value) -> () {
        todo!()
    }

    fn visit_binary(&mut self, left: &Value, op: &BinaryOp, right: &Value) -> () {
        todo!()
    }

    fn visit_unary(&mut self, op: &UnaryOp, operand: &Value) -> () {
        todo!()
    }

    fn visit_field_access(&mut self, expr: &Value, field: &IString) -> () {
        todo!()
    }

    fn visit_index_access(&mut self, collection: &Value, index: &Value) -> () {
        todo!()
    }

    fn visit_assign(&mut self, place: &Value, value: &Value) -> () {
        todo!()
    }

    fn visit_deref(&mut self, place: &Value) -> () {
        todo!()
    }

    fn visit_cast(&mut self, expr: &Value, to: &Type) -> () {
        todo!()
    }

    fn visit_borrow(&mut self, mutable: bool, place: &Value) -> () {
        todo!()
    }

    fn visit_list(&mut self, elements: &[Value]) -> () {
        todo!()
    }

    fn visit_tuple(&mut self, elements: &[Value]) -> () {
        todo!()
    }

    fn visit_if(&mut self, cond: &Value, true_blk: &Block, false_blk: Option<&Block>) -> () {
        todo!()
    }

    fn visit_while(&mut self, condition: &Value, body: &Block) -> () {
        todo!()
    }

    fn visit_loop(&mut self, body: &Block) -> () {
        todo!()
    }

    fn visit_break(&mut self, label: &Option<IString>) -> () {
        todo!()
    }

    fn visit_continue(&mut self, label: &Option<IString>) -> () {
        todo!()
    }

    fn visit_return(&mut self, value: &Value) -> () {
        todo!()
    }

    fn visit_block(&mut self, safety: BlockSafety, elements: &[BlockElement]) -> () {
        todo!()
    }

    fn visit_closure(&mut self, captures: &[SymbolId], callee: &Function) -> () {
        todo!()
    }

    fn visit_call(&mut self, callee: &Value, arguments: &[(IString, ValueId)]) -> () {
        todo!()
    }

    fn visit_method_call(
        &mut self,
        obj: &Value,
        name: &IString,
        args: &[(IString, ValueId)],
    ) -> () {
        todo!()
    }

    fn visit_symbol(&mut self, path: &IString) -> () {
        todo!()
    }
}

impl HirTypeVisitor<()> for DefaultCallArguments {
    fn visit_never(&mut self) -> () {
        todo!()
    }

    fn visit_unit(&mut self) -> () {
        todo!()
    }

    fn visit_bool(&mut self) -> () {
        todo!()
    }

    fn visit_u8(&mut self) -> () {
        todo!()
    }

    fn visit_u16(&mut self) -> () {
        todo!()
    }

    fn visit_u32(&mut self) -> () {
        todo!()
    }

    fn visit_u64(&mut self) -> () {
        todo!()
    }

    fn visit_u128(&mut self) -> () {
        todo!()
    }

    fn visit_usize(&mut self) -> () {
        todo!()
    }

    fn visit_i8(&mut self) -> () {
        todo!()
    }

    fn visit_i16(&mut self) -> () {
        todo!()
    }

    fn visit_i32(&mut self) -> () {
        todo!()
    }

    fn visit_i64(&mut self) -> () {
        todo!()
    }

    fn visit_i128(&mut self) -> () {
        todo!()
    }

    fn visit_f32(&mut self) -> () {
        todo!()
    }

    fn visit_f64(&mut self) -> () {
        todo!()
    }

    fn visit_opaque(&mut self, name: &IString) -> () {
        todo!()
    }

    fn visit_array(&mut self, element_type: &Type, len: u64) -> () {
        todo!()
    }

    fn visit_tuple(&mut self, element_types: &[TypeId]) -> () {
        todo!()
    }

    fn visit_slice(&mut self, element_type: &Type) -> () {
        todo!()
    }

    fn visit_struct(&mut self, attrs: &BTreeSet<StructAttribute>, fields: &[StructField]) -> () {
        todo!()
    }

    fn visit_enum(&mut self, attrs: &BTreeSet<EnumAttribute>, variants: &[EnumVariant]) -> () {
        todo!()
    }

    fn visit_refine(&mut self, base: &Type, min: &LiteralId, max: &LiteralId) -> () {
        todo!()
    }

    fn visit_bitfield(&mut self, base: &Type, len: u8) -> () {
        todo!()
    }

    fn visit_function_type(
        &mut self,
        attrs: &BTreeSet<FunctionAttribute>,
        params: &[ParameterId],
        ret: &Type,
    ) -> () {
        todo!()
    }

    fn visit_reference(&mut self, life: &Lifetime, excl: bool, mutable: bool, to: &Type) -> () {
        todo!()
    }

    fn visit_pointer(&mut self, excl: bool, mutable: bool, to: &Type) -> () {
        todo!()
    }

    fn visit_symbol(&mut self, path: &IString) -> () {
        todo!()
    }

    fn visit_inferred_float(&mut self) -> () {
        todo!()
    }

    fn visit_inferred_integer(&mut self) -> () {
        todo!()
    }

    fn visit_inferred(&mut self, id: NonZero<u32>) -> () {
        todo!()
    }
}

pub(crate) fn module_put_defaults(module: &mut Module, ctx: &mut Ast2HirCtx, log: &CompilerLog) {
    let mut visitor = DefaultCallArguments;
    visitor.visit_module(
        module.visibility,
        module.name.as_ref(),
        &module.attributes,
        &module.items,
    );
}

pub(crate) fn value_put_defaults(value: &mut Value, ctx: &mut Ast2HirCtx, log: &CompilerLog) {
    let mut visitor = DefaultCallArguments;
    visitor.visit_value(value, &ctx.store);
}

pub(crate) fn type_put_defaults(ty: &mut Type, ctx: &mut Ast2HirCtx, log: &CompilerLog) {
    let mut visitor = DefaultCallArguments;
    visitor.visit_type(ty, &ctx.store);
}
