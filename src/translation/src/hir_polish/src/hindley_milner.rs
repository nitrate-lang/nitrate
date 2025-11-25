use crate::diagnosis::TypeErr;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{BlockElement, Function, GlobalVariable, PtrSize, Type, TypeId, Value, ValueId};
use nitrate_hir_get_type::HirGetType;
use ordered_float::OrderedFloat;
use std::collections::{HashMap, HashSet};

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
enum TypeConstraint {
    Equal(TypeId),
}

enum NodeAction {
    NoChange,
    Replace(Value),
}

pub struct HindleyMilner {
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    ptr_size: PtrSize,
}

impl HindleyMilner {
    pub fn new(ptr_size: PtrSize) -> Self {
        Self {
            constraints: HashMap::new(),
            ptr_size,
        }
    }

    fn solve_inferred_integer(
        &mut self,
        id: &ValueId,
        integer: u128,
        log: &CompilerLog,
    ) -> NodeAction {
        fn report_out_of_range(integer: u128, target_type: TypeId, log: &CompilerLog) {
            let issue = TypeErr::IntegerLiteralOutsizeRange {
                value: integer,
                target_type: target_type.clone(),
            };
            log.report(&issue);
        }

        if let Some(constraints) = self.constraints.get(id) {
            for constraint in constraints {
                match constraint {
                    TypeConstraint::Equal(ty) => {
                        if !ty.is_integer_primitive() {
                            // TODO: report error
                            break;
                        }

                        return match **ty {
                            Type::I8 => match i8::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::I8(v)),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::I16 => match i16::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::I16(v)),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::I32 => match i32::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::I32(v)),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::I64 => match i64::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::I64(v)),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::I128 => match i128::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::I128(Box::new(v))),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::U8 => match u8::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::U8(v)),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::U16 => match u16::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::U16(v)),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::U32 => match u32::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::U32(v)),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::U64 => match u64::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::U64(v)),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::U128 => match u128::try_from(integer) {
                                Ok(v) => NodeAction::Replace(Value::U128(Box::new(v))),
                                Err(_) => {
                                    report_out_of_range(integer, ty.clone(), log);
                                    NodeAction::NoChange
                                }
                            },

                            Type::USize => match self.ptr_size {
                                PtrSize::U32 => match u32::try_from(integer) {
                                    Ok(v) => NodeAction::Replace(Value::USize32(v)),
                                    Err(_) => {
                                        report_out_of_range(integer, ty.clone(), log);
                                        NodeAction::NoChange
                                    }
                                },

                                PtrSize::U64 => match u64::try_from(integer) {
                                    Ok(v) => NodeAction::Replace(Value::USize64(v)),
                                    Err(_) => {
                                        report_out_of_range(integer, ty.clone(), log);
                                        NodeAction::NoChange
                                    }
                                },
                            },

                            _ => unreachable!(),
                        };
                    }
                }
            }
        }

        NodeAction::NoChange
    }

    fn solve_inferred_float(
        &mut self,
        id: &ValueId,
        float: OrderedFloat<f64>,
        log: &CompilerLog,
    ) -> NodeAction {
        if let Some(constraints) = self.constraints.get(id) {
            for constraint in constraints {
                match constraint {
                    TypeConstraint::Equal(ty) => {
                        if !ty.is_float_primitive() {
                            // TODO: report error
                            break;
                        }

                        return match **ty {
                            Type::F32 => NodeAction::Replace(Value::F32((*float as f32).into())),
                            Type::F64 => NodeAction::Replace(Value::F64(float)),
                            _ => unreachable!(),
                        };
                    }
                }
            }
        }

        NodeAction::NoChange
    }

    fn determine_action(&mut self, value: &Value, id: &ValueId, log: &CompilerLog) -> NodeAction {
        match value {
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
            | Value::USize64(_)
            | Value::StringLit(_)
            | Value::BStringLit(_)
            | Value::StructObject { .. }
            | Value::EnumVariant { .. }
            | Value::Binary { .. }
            | Value::Unary { .. }
            | Value::FieldAccess { .. }
            | Value::Assign { .. }
            | Value::Deref { .. }
            | Value::Cast { .. }
            | Value::Borrow { .. }
            | Value::List { .. }
            | Value::Tuple { .. }
            | Value::If { .. }
            | Value::While { .. }
            | Value::Loop { .. }
            | Value::Break { .. }
            | Value::Continue { .. }
            | Value::Return { .. }
            | Value::Block { .. }
            | Value::Closure { .. }
            | Value::Call { .. }
            | Value::MethodCall { .. }
            | Value::FunctionSymbol { .. }
            | Value::GlobalVariableSymbol { .. }
            | Value::LocalVariableSymbol { .. }
            | Value::ParameterSymbol { .. } => NodeAction::NoChange,

            Value::InferredInteger(integer) => self.solve_inferred_integer(id, **integer, log),
            Value::InferredFloat(float) => self.solve_inferred_float(id, *float, log),
        }
    }

    fn recurse(&mut self, e: &ValueId, log: &CompilerLog) {
        let action = {
            let current_value = e.borrow();
            self.determine_action(&*current_value, e, log)
        };

        match action {
            NodeAction::NoChange => {
                // TODO: Recurse into child nodes
            }

            NodeAction::Replace(new_value) => {
                e.replace(new_value);
            }
        }
    }

    fn step_block_element(&mut self, e: &mut BlockElement, log: &CompilerLog) {
        match e {
            BlockElement::Expr(e) => self.recurse(e, log),

            BlockElement::Local(local_var) => {
                let has_type_constraint = !local_var.borrow().ty.is_inferred();

                if has_type_constraint {
                    let value = local_var.borrow().initializer.clone();
                    let ty = local_var.borrow().ty.clone();

                    self.constraints
                        .entry(value)
                        .or_default()
                        .insert(TypeConstraint::Equal(ty));
                } else {
                    let initializer_type = local_var.borrow().initializer.borrow().determine_type();

                    if let Ok(type_constraint) = initializer_type {
                        local_var.borrow_mut().ty = type_constraint.into();
                    } else if let Err(_) = initializer_type {
                        // Type inference failed
                    }
                }

                self.recurse(&local_var.borrow().initializer, log);
            }
        }
    }

    pub fn solve_function(&mut self, f: &mut Function, log: &CompilerLog) {
        if let Some(body) = &mut f.body {
            loop {
                let prev_constraints_len = self.constraints.len();

                for element in body.iter_mut() {
                    self.step_block_element(element, log);
                }

                if self.constraints.len() == prev_constraints_len {
                    break;
                }
            }
        }
    }

    pub fn solve_global_variable(&mut self, g: &mut GlobalVariable, log: &CompilerLog) {
        let has_type_constraint = !g.ty.is_inferred();

        if has_type_constraint {
            let value = g.initializer.clone();
            let ty = g.ty.clone();

            self.constraints
                .entry(value)
                .or_default()
                .insert(TypeConstraint::Equal(ty));
        } else {
            let initializer_type = g.initializer.borrow().determine_type();

            if let Ok(type_constraint) = initializer_type {
                g.ty = type_constraint.into();
            } else if let Err(_) = initializer_type {
                // Type inference failed
            }
        }

        loop {
            let prev_constraints_len = self.constraints.len();
            self.recurse(&mut g.initializer, log);
            if self.constraints.len() == prev_constraints_len {
                break;
            }
        }
    }
}
