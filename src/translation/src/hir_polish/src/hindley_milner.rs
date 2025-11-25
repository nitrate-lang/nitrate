use nitrate_hir::{BlockElement, Function, LocalVariableId, ParameterId, TypeId, Value, ValueId};

enum SymbolId {
    LocalVar(LocalVariableId),
    Parameter(ParameterId),
}

enum TypeConstraint {
    Equal(SymbolId, TypeId),
}

pub struct HindleyMilner {
    constraints: Vec<TypeConstraint>,
}

impl HindleyMilner {
    pub fn new() -> Self {
        Self {
            constraints: Vec::new(),
        }
    }

    fn step(&mut self, e: &mut ValueId) {
        match &*e.borrow() {
            Value::Unit
            | Value::Bool(_)
            | Value::I8(_)
            | Value::I16(_)
            | Value::I32(_)
            | Value::I64(_)
            | Value::I128(_)
            | Value::U8(_)
            | Value::U16(_)
            | Value::U32(_)
            | Value::U64(_)
            | Value::U128(_)
            | Value::F32(_)
            | Value::F64(_)
            | Value::USize32(_)
            | Value::USize64(_) => {}

            Value::StringLit(thin_str) => todo!(),

            Value::BStringLit(thin_vec) => todo!(),

            Value::InferredInteger(_) => todo!(),

            Value::InferredFloat(ordered_float) => todo!(),

            Value::StructObject {
                struct_path,
                fields,
            } => todo!(),

            Value::EnumVariant {
                enum_path,
                variant,
                value,
            } => todo!(),

            Value::Binary { left, op, right } => todo!(),

            Value::Unary { op, operand } => todo!(),

            Value::FieldAccess { expr, field_name } => todo!(),

            Value::Assign { place, value } => todo!(),

            Value::Deref { place } => todo!(),

            Value::Cast { value, target_type } => todo!(),

            Value::Borrow {
                exclusive,
                mutable,
                place,
            } => todo!(),

            Value::List { elements } => todo!(),

            Value::Tuple { elements } => todo!(),

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => todo!(),

            Value::While { condition, body } => todo!(),

            Value::Loop { body } => todo!(),

            Value::Break { label } => todo!(),

            Value::Continue { label } => todo!(),

            Value::Return { value } => todo!(),

            Value::Block { block } => todo!(),

            Value::Closure { captures, callee } => todo!(),

            Value::Call {
                callee,
                positional,
                named,
            } => todo!(),

            Value::MethodCall {
                object,
                method_name,
                positional,
                named,
            } => todo!(),

            Value::FunctionSymbol { id } => todo!(),

            Value::GlobalVariableSymbol { id } => todo!(),

            Value::LocalVariableSymbol { id } => todo!(),

            Value::ParameterSymbol { id } => todo!(),
        }
    }

    fn step_block_element(&mut self, e: &mut BlockElement) {
        match e {
            BlockElement::Expr(e) => self.step(e),
            BlockElement::Local(e) => {
                if let Some(init) = &mut e.borrow_mut().init {
                    self.step(init);
                }
            }
        }
    }

    pub fn solve(&mut self, f: &mut Function) {
        if let Some(body) = &mut f.body {
            loop {
                let prev_constraints_len = self.constraints.len();

                for element in body.iter_mut() {
                    self.step_block_element(element);
                }

                if self.constraints.len() == prev_constraints_len {
                    break;
                }
            }
        }
    }
}
