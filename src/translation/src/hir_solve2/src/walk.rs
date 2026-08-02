//! Expression tree walker: traverses HIR values and collects type constraints.
//!
//! The walker is responsible for:
//! - Creating fresh type variables for `Inferred` types
//! - Collecting equality constraints from assignments, calls, binary ops, etc.
//! - Detecting generic call sites for monomorphization
//! - Tracking function return types for backward inference

use crate::constraints::{CanonicalType, ConstraintGraph, ConstraintSource};
use crate::diagnosis::TypeErr;
use crate::monomorphize::Monomorphizer;
use nitrate_hir::{
    BlockElement, BlockId, Function, FunctionId, GlobalVariable, StructDef, SymbolTab, Type, TypeId, Value, ValueId,
};
use nitrate_hir_type::HirGetType;
use nitrate_tree::ByteSpan;
use std::collections::{HashMap, HashSet};

/// Maps each ValueId to its corresponding canonical type in the constraint graph.
///
/// NodeTypes are populated during the walk and resolved during rewriting.
/// They provide the bridge between the HIR expression graph and the constraint graph.
pub(crate) struct NodeTypes {
    /// For each HIR ValueId, the canonical type (variable or concrete) it represents.
    mapping: HashMap<ValueId, CanonicalType>,
}

impl NodeTypes {
    pub fn new() -> Self {
        Self {
            mapping: HashMap::new(),
        }
    }

    /// Get the canonical type for a value node. Returns None if not yet visited.
    pub fn get(&self, id: &ValueId) -> Option<&CanonicalType> {
        self.mapping.get(id)
    }

    /// Set the canonical type for a value node.
    pub fn set(&mut self, id: &ValueId, ct: CanonicalType) {
        self.mapping.insert(id.clone(), ct);
    }

    /// Resolve a value node's type through the constraint graph.
    /// Returns the concrete TypeId if unified, or None if still a variable.
    pub fn resolve(&self, id: &ValueId, graph: &mut ConstraintGraph) -> Option<TypeId> {
        self.mapping.get(id).and_then(|ct| graph.resolve(ct))
    }
}

/// Context accumulated during a single walk of the expression tree.
pub(crate) struct WalkContext<'a> {
    /// The constraint graph being populated.
    pub graph: &'a mut ConstraintGraph,
    /// Node type tracking.
    pub node_types: &'a mut NodeTypes,
    /// The function being solved (if any).
    pub current_function: Option<CurrentFunction<'a>>,
    /// The symbol table for type lookups.
    pub symbol_tab: &'a SymbolTab,
    /// Errors collected during walking.
    pub errors: &'a mut HashSet<TypeErr>,
    /// Monomorphization state.
    pub mono: &'a mut Monomorphizer,
}

/// Information about the function currently being solved.
pub(crate) struct CurrentFunction<'a> {
    pub func: &'a Function,
    pub return_type: TypeId,
}

/// Walk a function body, collecting all type constraints into the graph.
pub(crate) fn walk_function(
    graph: &mut ConstraintGraph,
    node_types: &mut NodeTypes,
    func: &Function,
    symbol_tab: &SymbolTab,
    mono: &mut Monomorphizer,
    errors: &mut HashSet<TypeErr>,
) {
    let mut ctx = WalkContext {
        graph,
        node_types,
        current_function: Some(CurrentFunction {
            func,
            return_type: func.return_type,
        }),
        symbol_tab,
        errors,
        mono,
    };

    if let Some(body) = &func.body {
        for element in body {
            ctx.walk_block_element(element);
        }
    }
}

/// Walk a global variable initializer, collecting constraints.
pub(crate) fn walk_global(
    graph: &mut ConstraintGraph,
    node_types: &mut NodeTypes,
    global: &GlobalVariable,
    symbol_tab: &SymbolTab,
    mono: &mut Monomorphizer,
    errors: &mut HashSet<TypeErr>,
) {
    let mut ctx = WalkContext {
        graph,
        node_types,
        current_function: None,
        symbol_tab,
        errors,
        mono,
    };

    let ty = global.ty;
    let init_id = &global.initializer;

    if ty.is_inferred() {
        // Global with inferred type: type flows from initializer to global.
        let init_ct = ensure_var(init_id, &mut ctx);
        // No constraint needed — the init's type becomes the global's type.
    } else {
        // Global with explicit type: constrain the initializer.
        let init_ct = ensure_var(init_id, &mut ctx);
        ctx.graph
            .add_type_constraint(&init_ct, ty, ConstraintSource::Assignment);
    }

    ctx.walk_value(init_id);
}

impl<'a> WalkContext<'a> {
    fn walk_block_element(&mut self, element: &BlockElement) {
        match element {
            BlockElement::Expr(expr_id) => {
                self.walk_value(expr_id);
            }
            BlockElement::Local(local_var) => {
                let lv = local_var.borrow();
                let ty = lv.ty;

                if let Some(init_id) = &lv.initializer {
                    let init_ct = ensure_var(init_id, self);

                    if ty.is_inferred() {
                        // `let x = expr` — type flows from init to local.
                        // We don't add a constraint yet; the local type will be resolved
                        // from the init's resolved type during rewriting.
                    } else {
                        // `let x: T = expr` — constrain init to T.
                        self.graph
                            .add_type_constraint(&init_ct, ty, ConstraintSource::LocalType);
                    }

                    self.walk_value(init_id);
                }
            }
        }
    }

    fn walk_value(&mut self, id: &ValueId) {
        // If already visited, skip to avoid cycles.
        if self.node_types.get(id).is_some() {
            return;
        }

        // Create a type variable for this node (unless it's a leaf with a known type).
        let _ct = ensure_var(id, self);

        let value = id.borrow();
        let tag = classify_value(&value);

        match tag {
            // Leaves: no children to walk, type is self-evident.
            0 => { /* literals, symbols */ }

            // Struct construction.
            1 => self.walk_struct_object(id),
            // Enum variant construction.
            2 => self.walk_enum_variant(id),
            // Binary operations.
            3 => self.walk_binary(id),
            // Unary operations.
            4 => self.walk_unary(id),
            // Index access.
            5 => self.walk_index_access(id),
            // Field access.
            6 => self.walk_field_access(id),
            // Assignment.
            7 => self.walk_assign(id),
            // Dereference.
            8 => self.walk_deref(id),
            // Cast.
            9 => self.walk_cast(id),
            // Borrow.
            10 => self.walk_borrow(id),
            // List literal.
            11 => self.walk_list(id),
            // Tuple literal.
            12 => self.walk_tuple(id),
            // If expression.
            13 => self.walk_if(id),
            // While loop.
            14 => self.walk_while(id),
            // Loop.
            15 => self.walk_loop(id),
            // Break/Continue — no type needed.
            16 => {}
            // Return statement.
            17 => self.walk_return(id),
            // Block expression.
            18 => self.walk_block_value(id),
            // Function call.
            19 => self.walk_call(id),
            // Method call.
            20 => self.walk_method_call(id),
            // Symbol references — no children.
            21 => {}
            // Range expression.
            22 => self.walk_range(id),

            _ => {}
        }
    }

    fn walk_struct_object(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::StructObject { struct_def, fields, .. } = &*value else {
            return;
        };

        // Walk all field values first.
        for (_name, field_value) in fields {
            self.walk_value(field_value);
        }

        // Constrain field values to match struct field types.
        let struct_def_b = struct_def.borrow();
        for (field_name, field_value) in fields {
            if let Some(field) = struct_def_b.fields.get(field_name) {
                let fv_ct = ensure_var(field_value, self);
                self.graph
                    .add_type_constraint(&fv_ct, field.ty, ConstraintSource::StructField);
            }
        }
    }

    fn walk_enum_variant(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::EnumVariant {
            enum_def,
            variant,
            value: inner_value,
            ..
        } = &*value
        else {
            return;
        };

        self.walk_value(inner_value);

        let variant_type = enum_def
            .borrow()
            .variants
            .iter()
            .find(|item| item.name == *variant)
            .expect("variant not present")
            .ty;

        let inner_ct = ensure_var(inner_value, self);
        self.graph
            .add_type_constraint(&inner_ct, variant_type, ConstraintSource::Other);
    }

    fn walk_binary(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Binary { left, op, right, span } = &*value else {
            return;
        };
        let span = *span;

        // Walk children first.
        self.walk_value(left);
        self.walk_value(right);

        let left_ct = self.node_types.get(left).cloned();
        let right_ct = self.node_types.get(right).cloned();
        let result_ct = self.node_types.get(e).cloned();

        if let (Some(lct), Some(rct), Some(res_ct)) = (&left_ct, &right_ct, &result_ct) {
            if crate::constraints::is_arithmetic_op(op) {
                // Arithmetic: both operands and the result share the same type.
                self.graph
                    .add_equality(lct.clone(), rct.clone(), ConstraintSource::BinaryOp);
                self.graph
                    .add_equality(res_ct.clone(), lct.clone(), ConstraintSource::BinaryOp);
            } else if crate::constraints::is_comparison_or_logical_op(op) {
                // Comparison/logical: result is Bool, operands must agree.
                self.graph.add_type_constraint(
                    res_ct,
                    TypeId::from(Type::Bool {
                        span: ByteSpan::default(),
                    }),
                    ConstraintSource::BinaryOp,
                );
                self.graph
                    .add_equality(lct.clone(), rct.clone(), ConstraintSource::BinaryOp);
            }
        }
    }

    fn walk_unary(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Unary { operand, .. } = &*value else {
            return;
        };

        self.walk_value(operand);

        let operand_ct = self.node_types.get(operand).cloned();
        let result_ct = self.node_types.get(e).cloned();
        if let (Some(oct), Some(rct)) = (&operand_ct, &result_ct) {
            self.graph
                .add_equality(rct.clone(), oct.clone(), ConstraintSource::BinaryOp);
        }
    }

    fn walk_range(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Range { start, end, .. } = &*value else {
            return;
        };

        if let Some(start) = start {
            self.walk_value(start);
        }
        if let Some(end) = end {
            self.walk_value(end);
        }
    }

    fn walk_index_access(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::IndexAccess { collection, index, .. } = &*value else {
            return;
        };

        self.walk_value(collection);
        self.walk_value(index);

        // Index must be usize.
        let index_ct = ensure_var(index, self);
        self.graph.add_type_constraint(
            &index_ct,
            TypeId::from(Type::USize {
                span: ByteSpan::default(),
            }),
            ConstraintSource::Other,
        );
    }

    fn walk_field_access(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::FieldAccess { expr, .. } = &*value else {
            return;
        };
        self.walk_value(expr);
    }

    fn walk_assign(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Assign { place, value: v, .. } = &*value else {
            return;
        };

        self.walk_value(place);
        self.walk_value(v);

        // RHS type must match LHS type.
        let place_ct = self.node_types.get(place).cloned();
        let value_ct = self.node_types.get(v).cloned();
        if let (Some(pct), Some(vct)) = (&place_ct, &value_ct) {
            self.graph
                .add_equality(vct.clone(), pct.clone(), ConstraintSource::Assignment);
        }
    }

    fn walk_deref(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Deref { place, .. } = &*value else {
            return;
        };
        self.walk_value(place);
    }

    fn walk_cast(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Cast {
            value: v, target_type, ..
        } = &*value
        else {
            return;
        };

        self.walk_value(v);

        // The source does not need to match the target type (it's a cast).
        // But we set the result type to the target.
        let result_ct = ensure_var(e, self);
        self.graph
            .add_type_constraint(&result_ct, *target_type, ConstraintSource::Cast);
    }

    fn walk_borrow(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Borrow { place, .. } = &*value else {
            return;
        };
        self.walk_value(place);
    }

    fn walk_list(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::List { elements, .. } = &*value else {
            return;
        };

        // Walk all elements.
        for element in elements {
            self.walk_value(element);
        }

        // All elements must have the same type.
        if let Some(first) = elements.first() {
            let first_ct = ensure_var(first, self);
            for element in elements.iter().skip(1) {
                let el_ct = ensure_var(element, self);
                self.graph
                    .add_equality(el_ct.clone(), first_ct.clone(), ConstraintSource::ListElement);
            }
        }
    }

    fn walk_tuple(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Tuple { elements, .. } = &*value else {
            return;
        };
        // Tuples don't impose constraints between elements.
        for element in elements {
            self.walk_value(element);
        }
    }

    fn walk_if(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::If {
            condition,
            true_branch,
            false_branch,
            ..
        } = &*value
        else {
            return;
        };

        // Condition must be Bool.
        let cond_ct = ensure_var(condition, self);
        self.graph.add_type_constraint(
            &cond_ct,
            TypeId::from(Type::Bool {
                span: ByteSpan::default(),
            }),
            ConstraintSource::Condition,
        );

        self.walk_value(condition);
        self.walk_block(true_branch);

        if let Some(false_branch) = false_branch {
            self.walk_block(false_branch);
        }
    }

    fn walk_while(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::While { condition, body, .. } = &*value else {
            return;
        };

        // Condition must be Bool.
        let cond_ct = ensure_var(condition, self);
        self.graph.add_type_constraint(
            &cond_ct,
            TypeId::from(Type::Bool {
                span: ByteSpan::default(),
            }),
            ConstraintSource::Condition,
        );

        self.walk_value(condition);
        self.walk_block(body);
    }

    fn walk_loop(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Loop { body, .. } = &*value else {
            return;
        };
        self.walk_block(body);
    }

    fn walk_return(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Return { value: v, .. } = &*value else {
            return;
        };

        // Extract return type before mutable borrow.
        let return_type = self.current_function.as_ref().map(|cf| cf.return_type);

        self.walk_value(v);

        // Return value must match function return type.
        if let Some(ret_ty) = return_type {
            let ret_ct = ensure_var(v, self);
            self.graph
                .add_type_constraint(&ret_ct, ret_ty, ConstraintSource::ReturnType);
        }
    }

    fn walk_block_value(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Block { block, .. } = &*value else {
            return;
        };
        for element in &mut block.borrow_mut().elements {
            self.walk_block_element(element);
        }
    }

    fn walk_block(&mut self, block: &BlockId) {
        for element in &mut block.borrow_mut().elements {
            self.walk_block_element(element);
        }
    }

    fn walk_call(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::Call { callee, args, .. } = &*value else {
            return;
        };

        self.walk_value(callee);

        // If the callee is a known function symbol, constrain args to param types.
        if let Value::FunctionSymbol { id: func_id, .. } = &*callee.borrow() {
            let func = func_id.borrow();
            for (i, arg) in args.positional.iter().enumerate() {
                self.walk_value(arg);
                if let Some(param) = func.params.get(i) {
                    let arg_ct = ensure_var(arg, self);
                    self.graph
                        .add_type_constraint(&arg_ct, param.borrow().ty, ConstraintSource::CallArgument);
                }
            }
            for (name, arg) in &args.named {
                self.walk_value(arg);
                if let Some(param) = func.params.iter().find(|p| p.borrow().name == *name) {
                    let arg_ct = ensure_var(arg, self);
                    self.graph
                        .add_type_constraint(&arg_ct, param.borrow().ty, ConstraintSource::CallArgument);
                }
            }
        } else {
            // Callee type unknown — walk args anyway.
            for arg in &args.positional {
                self.walk_value(arg);
            }
            for (_name, arg) in &args.named {
                self.walk_value(arg);
            }
        }
    }

    fn walk_method_call(&mut self, e: &ValueId) {
        let value = e.borrow();
        let Value::MethodCall {
            object,
            method_name,
            args,
            ..
        } = &*value
        else {
            return;
        };

        self.walk_value(object);

        // Look up the method to constrain arguments.
        let obj_type = object.borrow().determine_type(self.symbol_tab).ok();
        if let Some(obj_type) = obj_type {
            let obj_type_id: TypeId = obj_type.into();
            if let Some(method_id) = self.symbol_tab.get_method(&obj_type_id, method_name) {
                let mf = method_id.borrow();
                // First arg (index 0) is self.
                for (i, arg) in args.positional.iter().enumerate() {
                    self.walk_value(arg);
                    if let Some(param) = mf.params.get(i + 1) {
                        let arg_ct = ensure_var(arg, self);
                        self.graph
                            .add_type_constraint(&arg_ct, param.borrow().ty, ConstraintSource::CallArgument);
                    }
                }
                for (name, arg) in &args.named {
                    self.walk_value(arg);
                    if let Some(param) = mf.params.iter().find(|p| p.borrow().name == *name) {
                        let arg_ct = ensure_var(arg, self);
                        self.graph
                            .add_type_constraint(&arg_ct, param.borrow().ty, ConstraintSource::CallArgument);
                    }
                }
                return;
            }
        }

        // Method not found — just walk args.
        for arg in &args.positional {
            self.walk_value(arg);
        }
        for (_name, arg) in &args.named {
            self.walk_value(arg);
        }
    }
}

/// Ensure a value node has a canonical type assigned (for tracking in the constraint graph).
/// If not yet assigned, creates a fresh type variable.
fn ensure_var(id: &ValueId, ctx: &mut WalkContext) -> CanonicalType {
    if let Some(ct) = ctx.node_types.get(id) {
        return ct.clone();
    }

    // Try to determine the type directly.
    let value = id.borrow();
    let ct = match &*value {
        Value::Unit { .. } => CanonicalType::Concrete(TypeId::from(Type::Unit { span: value.span() })),
        Value::Bool { .. } => CanonicalType::Concrete(TypeId::from(Type::Bool { span: value.span() })),
        Value::I8 { .. } => CanonicalType::Concrete(TypeId::from(Type::I8 { span: value.span() })),
        Value::I16 { .. } => CanonicalType::Concrete(TypeId::from(Type::I16 { span: value.span() })),
        Value::I32 { .. } => CanonicalType::Concrete(TypeId::from(Type::I32 { span: value.span() })),
        Value::I64 { .. } => CanonicalType::Concrete(TypeId::from(Type::I64 { span: value.span() })),
        Value::I128 { .. } => CanonicalType::Concrete(TypeId::from(Type::I128 { span: value.span() })),
        Value::U8 { .. } => CanonicalType::Concrete(TypeId::from(Type::U8 { span: value.span() })),
        Value::U16 { .. } => CanonicalType::Concrete(TypeId::from(Type::U16 { span: value.span() })),
        Value::U32 { .. } => CanonicalType::Concrete(TypeId::from(Type::U32 { span: value.span() })),
        Value::U64 { .. } => CanonicalType::Concrete(TypeId::from(Type::U64 { span: value.span() })),
        Value::U128 { .. } => CanonicalType::Concrete(TypeId::from(Type::U128 { span: value.span() })),
        Value::F32 { .. } => CanonicalType::Concrete(TypeId::from(Type::F32 { span: value.span() })),
        Value::F64 { .. } => CanonicalType::Concrete(TypeId::from(Type::F64 { span: value.span() })),
        Value::USize { .. } => CanonicalType::Concrete(TypeId::from(Type::USize { span: value.span() })),
        Value::StringLit { .. } => CanonicalType::Concrete(TypeId::from(Type::Str { span: value.span() })),
        Value::BStringLit { .. } => CanonicalType::Concrete(TypeId::from(Type::Array {
            span: value.span(),
            element_type: TypeId::from(Type::U8 { span: value.span() }),
            len: 0, // Will be resolved later.
        })),
        // Inferred literals: create a fresh type variable.
        Value::InferredInteger { .. } | Value::InferredFloat { .. } => CanonicalType::Variable(ctx.graph.fresh_var()),
        // Structured values: create a fresh type variable (to be constrained by children).
        _ => CanonicalType::Variable(ctx.graph.fresh_var()),
    };

    ctx.node_types.set(id, ct.clone());
    ct
}

/// Classify a value into a numeric tag for dispatch.
fn classify_value(value: &Value) -> u8 {
    match value {
        Value::Unit { .. }
        | Value::Bool { .. }
        | Value::I8 { .. }
        | Value::I16 { .. }
        | Value::I32 { .. }
        | Value::I64 { .. }
        | Value::I128 { .. }
        | Value::U8 { .. }
        | Value::U16 { .. }
        | Value::U32 { .. }
        | Value::U64 { .. }
        | Value::U128 { .. }
        | Value::F32 { .. }
        | Value::F64 { .. }
        | Value::USize { .. }
        | Value::StringLit { .. }
        | Value::BStringLit { .. }
        | Value::InferredInteger { .. }
        | Value::InferredFloat { .. } => 0,

        Value::StructObject { .. } => 1,
        Value::EnumVariant { .. } => 2,
        Value::Binary { .. } => 3,
        Value::Unary { .. } => 4,
        Value::IndexAccess { .. } => 5,
        Value::FieldAccess { .. } => 6,
        Value::Assign { .. } => 7,
        Value::Deref { .. } => 8,
        Value::Cast { .. } => 9,
        Value::Borrow { .. } => 10,
        Value::List { .. } => 11,
        Value::Tuple { .. } => 12,
        Value::If { .. } => 13,
        Value::While { .. } => 14,
        Value::Loop { .. } => 15,
        Value::Break { .. } | Value::Continue { .. } => 16,
        Value::Return { .. } => 17,
        Value::Block { .. } => 18,
        Value::Call { .. } => 19,
        Value::MethodCall { .. } => 20,
        Value::FunctionSymbol { .. }
        | Value::GlobalVariableSymbol { .. }
        | Value::LocalVariableSymbol { .. }
        | Value::ParameterSymbol { .. } => 21,
        Value::Range { .. } => 22,
    }
}
