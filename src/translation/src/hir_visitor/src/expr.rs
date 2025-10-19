use interned_string::IString;
use nitrate_hir::prelude::*;
use ordered_float::OrderedFloat;

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
    fn visit_struct_object(&mut self, ty: &Type, fields: &[(IString, ValueId)]) -> T;
    fn visit_enum_variant(&mut self, ty: &Type, var: &IString, val: &Value) -> T;
    fn visit_binary(&mut self, left: &Value, op: &BinaryOp, right: &Value) -> T;
    fn visit_unary(&mut self, op: &UnaryOp, operand: &Value) -> T;
    fn visit_field_access(&mut self, expr: &Value, field: &IString) -> T;
    fn visit_index_access(&mut self, collection: &Value, index: &Value) -> T;
    fn visit_assign(&mut self, place: &Value, value: &Value) -> T;
    fn visit_deref(&mut self, place: &Value) -> T;
    fn visit_cast(&mut self, expr: &Value, to: &Type) -> T;
    fn visit_borrow(&mut self, mutable: bool, place: &Value) -> T;
    fn visit_list(&mut self, elements: &[Value]) -> T;
    fn visit_tuple(&mut self, elements: &[Value]) -> T;
    fn visit_if(&mut self, cond: &Value, true_blk: &Block, false_blk: Option<&Block>) -> T;
    fn visit_while(&mut self, condition: &Value, body: &Block) -> T;
    fn visit_loop(&mut self, body: &Block) -> T;
    fn visit_break(&mut self, label: &Option<IString>) -> T;
    fn visit_continue(&mut self, label: &Option<IString>) -> T;
    fn visit_return(&mut self, value: &Value) -> T;
    fn visit_block(&mut self, safety: BlockSafety, elements: &[BlockElement]) -> T;
    fn visit_closure(&mut self, captures: &[SymbolId], callee: &Function) -> T;
    fn visit_call(&mut self, callee: &Value, arguments: &[ValueId]) -> T;
    fn visit_symbol(&mut self, path: &IString, link: Option<&SymbolId>) -> T;

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
            } => self.visit_struct_object(&store[struct_type], fields),

            Value::EnumVariant {
                enum_type,
                variant,
                value,
            } => self.visit_enum_variant(&store[enum_type], variant, &store[value].borrow()),

            Value::Binary { left, op, right } => {
                self.visit_binary(&store[left].borrow(), op, &store[right].borrow())
            }

            Value::Unary { op, operand } => self.visit_unary(op, &store[operand].borrow()),

            Value::FieldAccess { expr, field } => {
                self.visit_field_access(&store[expr].borrow(), field)
            }

            Value::IndexAccess { collection, index } => {
                self.visit_index_access(&store[collection].borrow(), &store[index].borrow())
            }

            Value::Assign { place, value } => {
                self.visit_assign(&store[place].borrow(), &store[value].borrow())
            }

            Value::Deref { place } => self.visit_deref(&store[place].borrow()),

            Value::Cast { expr, to } => self.visit_cast(&store[expr].borrow(), &store[to]),

            Value::Borrow { mutable, place } => self.visit_borrow(*mutable, &store[place].borrow()),

            Value::List { elements } => self.visit_list(elements),

            Value::Tuple { elements } => self.visit_tuple(elements),

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => match false_branch {
                Some(else_blk) => self.visit_if(
                    &store[condition].borrow(),
                    &store[true_branch].borrow(),
                    Some(&store[else_blk].borrow()),
                ),

                None => self.visit_if(
                    &store[condition].borrow(),
                    &store[true_branch].borrow(),
                    None,
                ),
            },

            Value::While { condition, body } => {
                self.visit_while(&store[condition].borrow(), &store[body].borrow())
            }

            Value::Loop { body } => self.visit_loop(&store[body].borrow()),

            Value::Break { label } => self.visit_break(label),

            Value::Continue { label } => self.visit_continue(label),

            Value::Return { value } => self.visit_return(&store[value].borrow()),

            Value::Block { block } => {
                let block = &store[block].borrow();
                self.visit_block(block.safety, &block.elements)
            }

            Value::Closure { captures, callee } => {
                self.visit_closure(captures, &store[callee].borrow())
            }

            Value::Call { callee, arguments } => {
                self.visit_call(&store[callee].borrow(), arguments)
            }

            Value::Symbol(symbol) => self.visit_symbol(&symbol.path, symbol.link.get()),
        }
    }
}
