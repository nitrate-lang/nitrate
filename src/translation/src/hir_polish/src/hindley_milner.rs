use nitrate_hir::{BlockElement, Function, GlobalVariable, TypeId, Value, ValueId};
use nitrate_hir_get_type::HirGetType;
use std::collections::{HashMap, HashSet};

enum TypeConstraint {
    Equal(TypeId),
}

pub struct HindleyMilner {
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
}

impl HindleyMilner {
    pub fn new() -> Self {
        Self {
            constraints: HashMap::new(),
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

            Value::StringLit(str) => todo!(),

            Value::BStringLit(str) => todo!(),

            Value::InferredInteger(_) => todo!(),

            Value::InferredFloat(ordered_float) => todo!(),

            Value::StructObject { struct_def, fields } => todo!(),

            Value::EnumVariant {
                enum_def,
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
                let mut local_var = e.borrow_mut();

                if local_var.ty.is_inferred() {
                    if let Some(init_value) = &mut local_var.init {
                        let local_ty = init_value.borrow().determine_type();
                    } else {
                        // Error: Cannot infer type without initializer
                    }
                } else {
                }
            }
        }
    }

    pub fn solve_function(&mut self, f: &mut Function) {
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

    pub fn solve_global_variable(&mut self, g: &mut GlobalVariable) {
        loop {
            let prev_constraints_len = self.constraints.len();

            // for element in body.iter_mut() {
            //     self.step_block_element(element);
            // }
            // TODO: Solve global variable initializers
            unimplemented!();

            if self.constraints.len() == prev_constraints_len {
                break;
            }
        }
    }
}
