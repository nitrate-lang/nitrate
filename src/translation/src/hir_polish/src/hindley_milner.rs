use crate::diagnosis::TypeErr;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    BlockElement, Function, GlobalVariable, GlobalVariableId, LocalVariableId, PtrSize, Type,
    TypeId, Value, ValueId,
};
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
    errors: HashSet<TypeErr>,
}

impl HindleyMilner {
    pub fn new(ptr_size: PtrSize) -> Self {
        Self {
            constraints: HashMap::new(),
            ptr_size,
            errors: HashSet::new(),
        }
    }

    fn report_out_of_range(&mut self, integer: u128, target_type: TypeId) {
        self.errors.insert(TypeErr::IntegerLiteralOutsizeRange {
            value: integer,
            target_type: target_type.clone(),
        });
    }

    fn solve_inferred_integer(&mut self, id: &ValueId, value: u128) -> NodeAction {
        if let Some(constraints) = self.constraints.get(id) {
            for constraint in constraints {
                match constraint {
                    TypeConstraint::Equal(ty) => {
                        if !ty.is_integer_primitive() {
                            self.errors.insert(TypeErr::IntegerLiteralUnsatisfiable {
                                value,
                                unsatisfiable_type: ty.clone(),
                            });
                            break;
                        }

                        return match **ty {
                            Type::I8 => match i8::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::I8(v)),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::I16 => match i16::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::I16(v)),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::I32 => match i32::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::I32(v)),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::I64 => match i64::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::I64(v)),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::I128 => match i128::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::I128(Box::new(v))),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::U8 => match u8::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::U8(v)),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::U16 => match u16::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::U16(v)),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::U32 => match u32::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::U32(v)),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::U64 => match u64::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::U64(v)),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::U128 => match u128::try_from(value) {
                                Ok(v) => NodeAction::Replace(Value::U128(Box::new(v))),
                                Err(_) => {
                                    self.report_out_of_range(value, ty.clone());
                                    NodeAction::NoChange
                                }
                            },

                            Type::USize => match self.ptr_size {
                                PtrSize::U32 => match u32::try_from(value) {
                                    Ok(v) => NodeAction::Replace(Value::USize32(v)),
                                    Err(_) => {
                                        self.report_out_of_range(value, ty.clone());
                                        NodeAction::NoChange
                                    }
                                },

                                PtrSize::U64 => match u64::try_from(value) {
                                    Ok(v) => NodeAction::Replace(Value::USize64(v)),
                                    Err(_) => {
                                        self.report_out_of_range(value, ty.clone());
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

    fn solve_inferred_float(&mut self, id: &ValueId, value: OrderedFloat<f64>) -> NodeAction {
        if let Some(constraints) = self.constraints.get(id) {
            for constraint in constraints {
                match constraint {
                    TypeConstraint::Equal(ty) => {
                        if !ty.is_float_primitive() {
                            self.errors.insert(TypeErr::FloatLiteralUnsatisfiable {
                                value,
                                unsatisfiable_type: ty.clone(),
                            });
                            break;
                        }

                        return match **ty {
                            Type::F32 => NodeAction::Replace(Value::F32((*value as f32).into())),
                            Type::F64 => NodeAction::Replace(Value::F64(value)),
                            _ => unreachable!(),
                        };
                    }
                }
            }
        }

        NodeAction::NoChange
    }

    fn determine_action(&mut self, value: &Value, id: &ValueId) -> NodeAction {
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

            Value::InferredInteger(integer) => self.solve_inferred_integer(id, **integer),
            Value::InferredFloat(float) => self.solve_inferred_float(id, *float),
        }
    }

    fn visit_children(&mut self, e: &ValueId) {
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
            | Value::USize64(_)
            | Value::StringLit(_)
            | Value::BStringLit(_)
            | Value::InferredInteger(_)
            | Value::InferredFloat(_) => {}

            Value::StructObject { struct_def, fields } => {
                // TODO: Recurse
            }

            Value::EnumVariant {
                enum_def,
                variant,
                value,
            } => {
                // TODO: Recurse
            }

            Value::Binary { left, op, right } => {
                // TODO: Recurse
            }

            Value::Unary { op, operand } => {
                // TODO: Recurse
            }

            Value::FieldAccess { expr, field_name } => {
                // TODO: Recurse
            }

            Value::Assign { place, value } => {
                // TODO: Recurse
            }

            Value::Deref { place } => {
                // TODO: Recurse
            }

            Value::Cast { value, target_type } => {
                // TODO: Recurse
            }

            Value::Borrow {
                exclusive,
                mutable,
                place,
            } => {
                // TODO: Recurse
            }

            Value::List { elements } => {
                // TODO: Recurse
            }

            Value::Tuple { elements } => {
                // TODO: Recurse
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                // TODO: Recurse
            }

            Value::While { condition, body } => {
                // TODO: Recurse
            }

            Value::Loop { body } => {
                // TODO: Recurse
            }

            Value::Break { label } => {
                // TODO: Recurse
            }

            Value::Continue { label } => {
                // TODO: Recurse
            }

            Value::Return { value } => {
                // TODO: Recurse
            }

            Value::Block { block } => {
                // TODO: Recurse
            }

            Value::Closure { captures, callee } => {
                // TODO: Recurse
            }

            Value::Call {
                callee,
                positional,
                named,
            } => {
                // TODO: Recurse
            }

            Value::MethodCall {
                object,
                method_name,
                positional,
                named,
            } => {
                // TODO: Recurse
            }

            Value::FunctionSymbol { id } => {
                // TODO: Recurse
            }

            Value::GlobalVariableSymbol { id } => {
                // TODO: Recurse
            }

            Value::LocalVariableSymbol { id } => {
                // TODO: Recurse
            }

            Value::ParameterSymbol { id } => {
                // TODO: Recurse
            }
        }
    }

    fn visit(&mut self, e: &ValueId) {
        let action = {
            let current_value = e.borrow();
            self.determine_action(&*current_value, e)
        };

        match action {
            NodeAction::Replace(new_value) => {
                e.replace(new_value);
            }

            NodeAction::NoChange => self.visit_children(e),
        }
    }

    pub fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
        if let Some(body) = &mut function.body {
            loop {
                let prev_constraints_len = self.constraints.len();

                for element in body.iter_mut() {
                    match element {
                        BlockElement::Expr(e) => self.visit(e),

                        BlockElement::Local(local_var) => {
                            if !local_var.borrow().ty.is_inferred() {
                                let value = local_var.borrow().initializer.clone();
                                let ty = local_var.borrow().ty.clone();

                                self.constraints
                                    .entry(value)
                                    .or_default()
                                    .insert(TypeConstraint::Equal(ty));
                            } else {
                                let initializer_type =
                                    local_var.borrow().initializer.borrow().determine_type();

                                if let Ok(type_constraint) = initializer_type {
                                    local_var.borrow_mut().ty = type_constraint.into();
                                } else if let Err(_) = initializer_type {
                                    // Type determination failed
                                }
                            }

                            self.visit(&local_var.borrow().initializer);
                        }
                    }
                }

                if self.constraints.len() == prev_constraints_len {
                    break;
                }
            }
        }

        for error in &self.errors {
            log.report(error);
        }

        if self.errors.is_empty() {
            Ok(())
        } else {
            Err(())
        }
    }

    pub fn solve_global_variable(
        &mut self,
        g: &mut GlobalVariable,
        log: &CompilerLog,
    ) -> Result<(), ()> {
        if !g.ty.is_inferred() {
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
                // Type determination failed
            }
        }

        loop {
            let prev_constraints_len = self.constraints.len();
            self.visit(&mut g.initializer);
            if self.constraints.len() == prev_constraints_len {
                break;
            }
        }

        for error in &self.errors {
            log.report(error);
        }

        if self.errors.is_empty() {
            Ok(())
        } else {
            Err(())
        }
    }
}
