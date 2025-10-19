use interned_string::IString;
use nitrate_hir::prelude::*;
use ordered_float::OrderedFloat;
use std::{collections::BTreeSet, num::NonZero};

pub trait HirTypeVisitor<T> {
    fn visit_never(&mut self) -> T;
    fn visit_unit(&mut self) -> T;
    fn visit_bool(&mut self) -> T;
    fn visit_u8(&mut self) -> T;
    fn visit_u16(&mut self) -> T;
    fn visit_u32(&mut self) -> T;
    fn visit_u64(&mut self) -> T;
    fn visit_u128(&mut self) -> T;
    fn visit_usize(&mut self) -> T;
    fn visit_i8(&mut self) -> T;
    fn visit_i16(&mut self) -> T;
    fn visit_i32(&mut self) -> T;
    fn visit_i64(&mut self) -> T;
    fn visit_i128(&mut self) -> T;
    fn visit_f32(&mut self) -> T;
    fn visit_f64(&mut self) -> T;
    fn visit_opaque(&mut self, name: &str) -> T;
    fn visit_array(&mut self, element_type: &TypeId, len: u64) -> T;
    fn visit_tuple(&mut self, element_types: &[TypeId]) -> T;
    fn visit_slice(&mut self, element_type: &TypeId) -> T;
    fn visit_struct(&mut self, attrs: &BTreeSet<StructAttribute>, fields: &[StructField]) -> T;
    fn visit_enum(&mut self, attrs: &BTreeSet<EnumAttribute>, variants: &[EnumVariant]) -> T;
    fn visit_refine(&mut self, base: &TypeId, min: &LiteralId, max: &LiteralId) -> T;
    fn visit_bitfield(&mut self, base: &TypeId, len: u8) -> T;

    fn visit_function(
        &mut self,
        attrs: &BTreeSet<FunctionAttribute>,
        params: &[ParameterId],
        ret: &TypeId,
    ) -> T;

    fn visit_reference(&mut self, life: &Lifetime, excl: bool, mutable: bool, to: &TypeId) -> T;
    fn visit_pointer(&mut self, excl: bool, mutable: bool, to: &TypeId) -> T;
    fn visit_symbol(&mut self, path: &str, link: &Option<TypeId>) -> T;
    fn visit_inferred_float(&mut self) -> T;
    fn visit_inferred_integer(&mut self) -> T;
    fn visit_inferred(&mut self, id: NonZero<u32>) -> T;

    fn visit_type(&mut self, ty: &Type, store: &Store) -> T {
        match ty {
            Type::Never => self.visit_never(),
            Type::Unit => self.visit_unit(),
            Type::Bool => self.visit_bool(),
            Type::U8 => self.visit_u8(),
            Type::U16 => self.visit_u16(),
            Type::U32 => self.visit_u32(),
            Type::U64 => self.visit_u64(),
            Type::U128 => self.visit_u128(),
            Type::USize => self.visit_usize(),
            Type::I8 => self.visit_i8(),
            Type::I16 => self.visit_i16(),
            Type::I32 => self.visit_i32(),
            Type::I64 => self.visit_i64(),
            Type::I128 => self.visit_i128(),
            Type::F32 => self.visit_f32(),
            Type::F64 => self.visit_f64(),
            Type::Opaque { name } => self.visit_opaque(name),
            Type::Array { element_type, len } => self.visit_array(element_type, *len),
            Type::Tuple { element_types } => self.visit_tuple(element_types),
            Type::Slice { element_type } => self.visit_slice(element_type),

            Type::Struct { struct_type } => {
                let struct_type = &store[struct_type];
                self.visit_struct(&struct_type.attributes, &struct_type.fields)
            }

            Type::Enum { enum_type } => {
                let enum_type = &store[enum_type];
                self.visit_enum(&enum_type.attributes, &enum_type.variants)
            }

            Type::Refine { base, min, max } => self.visit_refine(base, min, max),
            Type::Bitfield { base, bits } => self.visit_bitfield(base, *bits),

            Type::Function { function_type } => {
                let function_type = &store[function_type];
                self.visit_function(
                    &function_type.attributes,
                    &function_type.params,
                    &function_type.return_type,
                )
            }

            Type::Reference {
                lifetime,
                exclusive,
                mutable,
                to,
            } => self.visit_reference(lifetime, *exclusive, *mutable, to),

            Type::Pointer {
                exclusive,
                mutable,
                to,
            } => self.visit_pointer(*exclusive, *mutable, to),

            Type::Symbol { path, link } => self.visit_symbol(path, link),
            Type::InferredFloat => self.visit_inferred_float(),
            Type::InferredInteger => self.visit_inferred_integer(),
            Type::Inferred { id } => self.visit_inferred(*id),
        }
    }
}

pub trait HirValueVisitor<T> {
    fn visit_unit(&mut self) -> T;
    fn visit_bool(&mut self, value: bool) -> T;
    fn visit_i8(&mut self, value: i8) -> T;
    fn visit_i16(&mut self, value: i16) -> T;
    fn visit_i32(&mut self, value: i32) -> T;
    fn visit_i64(&mut self, value: i64) -> T;
    fn visit_i128(&mut self, value: i128) -> T;
    fn visit_u8(&mut self, value: u8) -> T;
    fn visit_u16(&mut self, value: u16) -> T;
    fn visit_u32(&mut self, value: u32) -> T;
    fn visit_u64(&mut self, value: u64) -> T;
    fn visit_u128(&mut self, value: u128) -> T;
    fn visit_f32(&mut self, value: OrderedFloat<f32>) -> T;
    fn visit_f64(&mut self, value: OrderedFloat<f64>) -> T;
    fn visit_usize32(&mut self, value: u32) -> T;
    fn visit_usize64(&mut self, value: u64) -> T;
    fn visit_string_lit(&mut self, value: &str) -> T;
    fn visit_bstring_lit(&mut self, value: &[u8]) -> T;
    fn visit_inferred_integer(&mut self, value: u128) -> T;
    fn visit_inferred_float(&mut self, value: OrderedFloat<f64>) -> T;
    fn visit_struct_object(&mut self, ty: &TypeId, fields: &[(IString, ValueId)]) -> T;
    fn visit_enum_variant(&mut self, ty: &TypeId, var: &IString, val: &ValueId) -> T;
    fn visit_binary(&mut self, left: &ValueId, op: &BinaryOp, right: &ValueId) -> T;
    fn visit_unary(&mut self, op: &UnaryOp, operand: &ValueId) -> T;
    fn visit_field_access(&mut self, expr: &ValueId, field: &IString) -> T;
    fn visit_index_access(&mut self, collection: &ValueId, index: &ValueId) -> T;
    fn visit_assign(&mut self, place: &ValueId, value: &ValueId) -> T;
    fn visit_deref(&mut self, place: &ValueId) -> T;
    fn visit_cast(&mut self, expr: &ValueId, to: &TypeId) -> T;
    fn visit_borrow(&mut self, mutable: bool, place: &ValueId) -> T;
    fn visit_list(&mut self, elements: &[Value]) -> T;
    fn visit_tuple(&mut self, elements: &[Value]) -> T;
    fn visit_if(&mut self, cond: &ValueId, true_blk: &BlockId, false_blk: &Option<BlockId>) -> T;
    fn visit_while(&mut self, condition: &ValueId, body: &BlockId) -> T;
    fn visit_loop(&mut self, body: &BlockId) -> T;
    fn visit_break(&mut self, label: &Option<IString>) -> T;
    fn visit_continue(&mut self, label: &Option<IString>) -> T;
    fn visit_return(&mut self, value: &ValueId) -> T;
    fn visit_block(&mut self, safety: BlockSafety, elements: &[BlockElement]) -> T;
    fn visit_closure(&mut self, captures: &[SymbolId], callee: &FunctionId) -> T;
    fn visit_call(&mut self, callee: &ValueId, arguments: &[ValueId]) -> T;
    fn visit_symbol(&mut self, path: &IString, link: &Option<SymbolId>) -> T;

    fn visit_value(&mut self, value: &Value, store: &Store) -> T {
        match value {
            Value::Unit => self.visit_unit(),
            Value::Bool(value) => self.visit_bool(*value),
            Value::I8(value) => self.visit_i8(*value),
            Value::I16(value) => self.visit_i16(*value),
            Value::I32(value) => self.visit_i32(*value),
            Value::I64(value) => self.visit_i64(*value),
            Value::I128(value) => self.visit_i128(**value),
            Value::U8(value) => self.visit_u8(*value),
            Value::U16(value) => self.visit_u16(*value),
            Value::U32(value) => self.visit_u32(*value),
            Value::U64(value) => self.visit_u64(*value),
            Value::U128(value) => self.visit_u128(**value),
            Value::F32(value) => self.visit_f32(*value),
            Value::F64(value) => self.visit_f64(*value),
            Value::USize32(value) => self.visit_usize32(*value),
            Value::USize64(value) => self.visit_usize64(*value),
            Value::StringLit(value) => self.visit_string_lit(value),
            Value::BStringLit(value) => self.visit_bstring_lit(value),
            Value::InferredInteger(value) => self.visit_inferred_integer(**value),
            Value::InferredFloat(value) => self.visit_inferred_float(*value),

            Value::StructObject {
                struct_type,
                fields,
            } => self.visit_struct_object(struct_type, fields),

            Value::EnumVariant {
                enum_type,
                variant,
                value,
            } => self.visit_enum_variant(enum_type, variant, value),

            Value::Binary { left, op, right } => self.visit_binary(left, op, right),
            Value::Unary { op, operand } => self.visit_unary(op, operand),
            Value::FieldAccess { expr, field } => self.visit_field_access(expr, field),
            Value::IndexAccess { collection, index } => self.visit_index_access(collection, index),
            Value::Assign { place, value } => self.visit_assign(place, value),
            Value::Deref { place } => self.visit_deref(place),
            Value::Cast { expr, to } => self.visit_cast(expr, to),
            Value::Borrow { mutable, place } => self.visit_borrow(*mutable, place),
            Value::List { elements } => self.visit_list(elements),
            Value::Tuple { elements } => self.visit_tuple(elements),

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => self.visit_if(condition, true_branch, false_branch),

            Value::While { condition, body } => self.visit_while(condition, body),
            Value::Loop { body } => self.visit_loop(body),
            Value::Break { label } => self.visit_break(label),
            Value::Continue { label } => self.visit_continue(label),
            Value::Return { value } => self.visit_return(value),

            Value::Block { block } => {
                let block = &store[block].borrow();
                self.visit_block(block.safety, &block.elements)
            }

            Value::Closure { captures, callee } => self.visit_closure(captures, callee),
            Value::Call { callee, arguments } => self.visit_call(callee, arguments),
            Value::Symbol { path, link } => self.visit_symbol(path, link),
        }
    }
}

pub trait HirVisitor<T>: HirTypeVisitor<T> + HirValueVisitor<T> {}
