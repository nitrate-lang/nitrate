use crate::diagnosis::TypeErr;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    BlockElement, BlockId, Function, GlobalVariable, PtrSize, Type, TypeId, Value, ValueId,
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
    function_return_type: Option<TypeId>,
}

impl HindleyMilner {
    pub fn new(ptr_size: PtrSize) -> Self {
        Self {
            constraints: HashMap::new(),
            ptr_size,
            errors: HashSet::new(),
            function_return_type: None,
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

            Value::StructObject {
                struct_def: _,
                fields,
            } => {
                for (_field_name, field_value) in fields {
                    self.visit(field_value);
                }
            }

            Value::EnumVariant {
                enum_def: _,
                variant: _,
                value,
            } => {
                self.visit(value);
            }

            Value::Binary { left, op: _, right } => {
                if let Some(constraints) = self.constraints.get(e).cloned() {
                    self.constraints
                        .entry(left.clone())
                        .or_default()
                        .extend(constraints.clone());

                    self.constraints
                        .entry(right.clone())
                        .or_default()
                        .extend(constraints);
                }

                self.visit(left);
                self.visit(right);
            }

            Value::Unary { operand, op: _ } => {
                self.visit(operand);
            }

            Value::FieldAccess {
                expr,
                field_name: _,
            } => {
                self.visit(expr);
            }

            Value::Assign { place, value } => {
                self.visit(place);
                self.visit(value);
            }

            Value::Deref { place } => {
                self.visit(place);
            }

            Value::Cast { value, target_type } => {
                self.constraints
                    .entry(value.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(target_type.clone()));

                self.visit(value);
            }

            Value::Borrow {
                exclusive: _,
                mutable: _,
                place,
            } => {
                self.visit(place);
            }

            Value::List { elements } => {
                for element in elements {
                    self.visit(element);
                }
            }

            Value::Tuple { elements } => {
                for element in elements {
                    self.visit(element);
                }
            }

            Value::If {
                condition,
                true_branch,
                false_branch,
            } => {
                self.visit(condition);
                self.visit_block(true_branch);
                if let Some(false_branch) = false_branch {
                    self.visit_block(false_branch);
                }
            }

            Value::While { condition, body } => {
                self.visit(condition);
                self.visit_block(body);
            }

            Value::Loop { body } => {
                self.visit_block(body);
            }

            Value::Break { label: _ } => {}
            Value::Continue { label: _ } => {}

            Value::Return { value } => {
                if let Some(ret_type) = self.function_return_type {
                    self.constraints
                        .entry(value.clone())
                        .or_default()
                        .insert(TypeConstraint::Equal(ret_type));
                }

                self.visit(value);
            }

            Value::Block { block } => {
                for element in &mut block.borrow_mut().elements {
                    self.visit_block_element(element);
                }
            }

            Value::Call {
                callee,
                positional,
                named,
            } => {
                self.visit(callee);
                for arg in positional {
                    self.visit(arg);
                }
                for (_name, arg) in named {
                    self.visit(arg);
                }
            }

            Value::MethodCall {
                object,
                method_name: _,
                positional,
                named,
            } => {
                self.visit(object);
                for arg in positional {
                    self.visit(arg);
                }
                for (_name, arg) in named {
                    self.visit(arg);
                }
            }

            Value::FunctionSymbol { .. }
            | Value::GlobalVariableSymbol { .. }
            | Value::LocalVariableSymbol { .. }
            | Value::ParameterSymbol { .. } => {}
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

    fn visit_block(&mut self, block: &BlockId) {
        for element in &mut block.borrow_mut().elements {
            self.visit_block_element(element);
        }
    }

    fn visit_block_element(&mut self, element: &mut BlockElement) {
        match element {
            BlockElement::Expr(e) => self.visit(e),

            BlockElement::Local(local_var) => {
                if local_var.borrow().ty.is_inferred() {
                    let initializer_type = local_var.borrow().initializer.borrow().determine_type();

                    if let Ok(type_constraint) = initializer_type {
                        local_var.borrow_mut().ty = type_constraint.into();
                    } else if let Err(_) = initializer_type {
                        // Type determination failed
                    }
                } else {
                    let value = local_var.borrow().initializer.clone();
                    let ty = local_var.borrow().ty.clone();

                    self.constraints
                        .entry(value)
                        .or_default()
                        .insert(TypeConstraint::Equal(ty));
                }

                self.visit(&local_var.borrow().initializer);
            }
        }
    }

    pub fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
        if let Some(body) = &mut function.body {
            self.function_return_type = Some(function.return_type);

            loop {
                let prev_constraints_len = self.constraints.len();

                for element in body.iter_mut() {
                    self.visit_block_element(element);
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
        loop {
            let prev_constraints_len = self.constraints.len();

            if g.ty.is_inferred() {
                let initializer_type = g.initializer.borrow().determine_type();

                if let Ok(type_constraint) = initializer_type {
                    g.ty = type_constraint.into();
                } else if let Err(_) = initializer_type {
                    // Type determination failed
                }
            } else {
                let value = g.initializer.clone();
                let ty = g.ty.clone();

                self.constraints
                    .entry(value)
                    .or_default()
                    .insert(TypeConstraint::Equal(ty));
            }

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
