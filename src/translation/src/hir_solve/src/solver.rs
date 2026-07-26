use crate::diagnosis::TypeErr;
use crate::substitution::{NodeAction, TypeConstraint};
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::{
    BlockElement, BlockId, Function, FunctionId, GlobalVariable, PtrSize, SymbolTab, Type, TypeId, Value, ValueId,
};
use nitrate_hir_get_type::HirGetType;
use ordered_float::OrderedFloat;
use std::collections::{HashMap, HashSet};

/// The main type solver. Replaces the old HindleyMilner.
pub(crate) struct Solver<'m> {
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,
    pub(crate) m: &'m mut SymbolTab,
    errors: HashSet<TypeErr>,
    function_return_type: Option<TypeId>,
    pub(crate) mono_counter: u32,
    /// Cache of monomorphized function copies keyed by (generic_function_store_index, sorted_concrete_args).
    /// Prevents creating duplicate copies when the same generic function is instantiated
    /// with identical concrete type arguments at multiple call sites.
    pub(crate) mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), FunctionId>,
}

impl<'m> Solver<'m> {
    pub(crate) fn new(m: &'m mut SymbolTab) -> Self {
        Self {
            constraints: HashMap::new(),
            m,
            errors: HashSet::new(),
            function_return_type: None,
            mono_counter: 0,
            mono_cache: HashMap::new(),
        }
    }

    fn report_out_of_range(&mut self, integer: u128, target_type: TypeId) {
        self.errors.insert(TypeErr::IntegerLiteralOutsizeRange {
            value: integer,
            target_type,
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
                            Type::USize => match self.m.arch_ptr_size() {
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
        let value = e.borrow().clone();
        match &value {
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
                for (field_name, field_value) in fields {
                    let struct_def_b = struct_def.borrow();
                    if let Some(field) = struct_def_b.fields.get(field_name) {
                        let field_type = field.ty;
                        self.constraints
                            .entry(field_value.clone())
                            .or_default()
                            .insert(TypeConstraint::Equal(field_type));
                        self.visit(field_value);
                    }
                }
            }

            Value::EnumVariant {
                enum_def,
                variant,
                value: inner_value,
            } => {
                let variant_type = enum_def
                    .borrow()
                    .variants
                    .iter()
                    .find(|item| item.name == *variant)
                    .expect("variant not present")
                    .ty;
                self.constraints
                    .entry(inner_value.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(variant_type));
                self.visit(inner_value);
            }

            Value::Binary { left, op: _, right } => {
                if let Some(constraints) = self.constraints.get(e).cloned() {
                    self.constraints
                        .entry(left.clone())
                        .or_default()
                        .extend(constraints.clone());
                    self.constraints.entry(right.clone()).or_default().extend(constraints);
                }
                self.visit(left);
                self.visit(right);
            }

            Value::Unary { operand, .. } => {
                if let Some(constraints) = self.constraints.get(e).cloned() {
                    self.constraints.entry(operand.clone()).or_default().extend(constraints);
                }
                self.visit(operand);
            }

            Value::FieldAccess { expr, .. } => {
                self.visit(expr);
            }

            Value::Assign { place, value: v } => {
                if let Ok(place_type) = place.borrow().determine_type(self.m) {
                    self.constraints
                        .entry(v.clone())
                        .or_default()
                        .insert(TypeConstraint::Equal(place_type.into()));
                }
                self.visit(place);
                self.visit(v);
            }

            Value::Deref { place } => self.visit(place),

            Value::Cast { value: v, target_type } => {
                self.constraints
                    .entry(v.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(target_type.clone()));
                self.visit(v);
            }

            Value::Borrow { place, .. } => self.visit(place),

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
                self.constraints
                    .entry(condition.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(Type::Bool.into()));
                self.visit(condition);
                self.visit_block(true_branch);
                if let Some(false_branch) = false_branch {
                    self.visit_block(false_branch);
                }
            }

            Value::While { condition, body } => {
                self.constraints
                    .entry(condition.clone())
                    .or_default()
                    .insert(TypeConstraint::Equal(Type::Bool.into()));
                self.visit(condition);
                self.visit_block(body);
            }

            Value::Loop { body } => self.visit_block(body),

            Value::Break { .. } | Value::Continue { .. } => {}

            Value::Return { value: v } => {
                if let Some(ret_type) = self.function_return_type {
                    self.constraints
                        .entry(v.clone())
                        .or_default()
                        .insert(TypeConstraint::Equal(ret_type));
                }
                self.visit(v);
            }

            Value::Block { block } => {
                for element in &mut block.borrow_mut().elements {
                    self.visit_block_element(element);
                }
            }

            Value::Call { callee, args } => {
                // Check if callee is a generic function - monomorphize if so
                // We need to drop the borrow before mutating, so clone what we need
                let callee_func_id: Option<FunctionId> = match &*callee.borrow() {
                    Value::FunctionSymbol { id } => {
                        let func = id.borrow();
                        if func.generics.is_some() && func.generics.as_ref().map_or(false, |g| !g.is_empty()) {
                            Some(id.clone())
                        } else {
                            None
                        }
                    }
                    _ => None,
                };

                if let Some(func_id) = callee_func_id {
                    if let Some(subst) = self.infer_generic_args_from_call(&func_id, &args.positional) {
                        let mono_id = self.monomorphize_function(&func_id, &subst);
                        callee.replace(Value::FunctionSymbol { id: mono_id });
                    }
                }

                self.visit(callee);
                for arg in &args.positional {
                    self.visit(arg);
                }
                for (_name, arg) in &args.named {
                    self.visit(arg);
                }
            }

            Value::MethodCall {
                object,
                method_name,
                args,
            } => {
                // Phase 1: Resolve the method type without borrowing self.m in a closure
                let obj_type: Option<TypeId> = object.borrow().determine_type(self.m).ok().map(|ty| ty.into());

                // Phase 2: Look up method - get the FunctionId (clone it to avoid borrow issues)
                let method_id_opt: Option<FunctionId> =
                    obj_type.and_then(|obj_type| self.m.get_method(&obj_type, method_name).cloned());

                // Phase 3: Check if generic and monomorphize if needed
                if let Some(method_id) = method_id_opt {
                    let is_generic = {
                        let method_func = method_id.borrow();
                        method_func.generics.is_some() && method_func.generics.as_ref().map_or(false, |g| !g.is_empty())
                    };

                    if is_generic {
                        if let Some(subst) = self.infer_generic_args_from_call(&method_id, &args.positional) {
                            let mono_id = self.monomorphize_function(&method_id, &subst);
                            let new_callee = Value::FunctionSymbol { id: mono_id };
                            let new_call = Value::Call {
                                callee: ValueId::from(new_callee),
                                args: args.clone(),
                            };
                            e.replace(new_call);
                            self.visit(e);
                            return;
                        }
                    }
                }

                self.visit(object);
                for arg in &args.positional {
                    self.visit(arg);
                }
                for (_name, arg) in &args.named {
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
                    let init_ty = local_var.borrow().initializer.borrow().determine_type(self.m);
                    if let Ok(ty) = init_ty {
                        local_var.borrow_mut().ty = ty.into();
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

    fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
        if let Some(body) = &mut function.body {
            self.function_return_type = Some(function.return_type);
            loop {
                let prev_len = self.constraints.len();
                for element in body.iter_mut() {
                    self.visit_block_element(element);
                }
                if self.constraints.len() == prev_len {
                    break;
                }
            }
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }

    fn solve_global_variable(&mut self, g: &mut GlobalVariable, log: &CompilerLog) -> Result<(), ()> {
        loop {
            let prev_len = self.constraints.len();
            if g.ty.is_inferred() {
                let init_ty = g.initializer.borrow().determine_type(self.m);
                if let Ok(ty) = init_ty {
                    g.ty = ty.into();
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
            if self.constraints.len() == prev_len {
                break;
            }
        }
        for error in &self.errors {
            log.report(error);
        }
        if self.errors.is_empty() { Ok(()) } else { Err(()) }
    }
}

pub fn resolve_function(function: &mut Function, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = Solver::new(m);
    hm.solve_function(function, log)
}

pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = Solver::new(m);
    hm.solve_global_variable(global, log)
}
