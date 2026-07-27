# Hindley-Milner Type Inference and Monomorphization

## Theoretical Foundation

The Hindley-Milner (HM) type system is the foundation of Nitrate's type inference. Originally developed by Roger Hindley and later extended by Robin Milner, HM provides:

1. **Principal types**: Every well-typed expression has a unique most general type
2. **Complete inference**: Types can be inferred without explicit annotations (though annotations are supported)
3. **Let-polymorphism**: `let` bindings can be polymorphic (pre-monomorphization)

Nitrate's implementation extends classical HM with:

- **Constraint-based unification**: Rather than the classic unification algorithm with destructive substitution, Nitrate uses constraint accumulation with fixed-point solving
- **Refinement type propagation**: Bounds tracking through arithmetic operations
- **Monomorphization**: Generic instantiation via code cloning and type substitution
- **Method call resolution**: Integration with the trait system for method dispatch
- **Struct generic inference**: Generic parameter inference from struct field values

## Architecture

**Crate**: `nitrate_hir_solve`  
**Key types**: `Solver`, `TypeConstraint`, `Substitution`, `NodeAction`  
**Key files**: `solver.rs` (core solving algorithm), `substitution.rs` (type substitution), `monomorphize.rs` (generic instantiation), `diagnosis.rs` (error types)  
**Entry points**: `resolve_function()`, `resolve_global()`

## The Solver

The `Solver` struct is the core of the type inference engine, defined in `solver.rs`:

```rust
pub(crate) struct Solver<'m> {
    // Per-expression constraint map
    constraints: HashMap<ValueId, HashSet<TypeConstraint>>,

    // Mutable symbol table reference (for registering monomorphized functions)
    m: &'m mut SymbolTab,

    // Accumulated type errors (HashSet for deduplication)
    errors: HashSet<TypeErr>,

    // Return type of the current function being solved
    function_return_type: Option<TypeId>,

    // Monotonically increasing counter for naming monomorphized copies
    mono_counter: u32,

    // Deduplication cache: (generic_func_store_index, [(param_index, concrete_type), ...]) -> monomorphized_id
    mono_cache: HashMap<(usize, Vec<(u32, TypeId)>), FunctionId>,
}
```

### Key Fields Explained

- **`constraints`**: A `HashMap<ValueId, HashSet<TypeConstraint>>` mapping each expression to its expected types. The `HashSet` ensures deduplication (multiple sources may assert the same constraint). Each constraint is an equality: the expression must equal a specific `TypeId`.

- **`m`**: Mutable reference to the `SymbolTab`. The solver registers monomorphized functions and structs here, making them visible to the LLVM codegen phase.

- **`mono_cache`**: The cache key is a tuple of `(original_function_store_index, sorted_type_args)`. The `store_index` is obtained from `FunctionId::as_usize()`, which returns the underlying `NonZeroU32` as a `usize`. The type arguments are sorted by parameter index to ensure canonical cache keys regardless of inference order.

## Constraint Types

Defined in `substitution.rs`:

```rust
pub(crate) enum TypeConstraint {
    Equal(TypeId),  // This value's type must equal TypeId
}
```

Currently, all constraints are equality constraints. Future extensions could add subtyping (`SubtypeOf`) or trait bounds (`Implies(TraitId)`).

## Node Actions

```rust
pub(crate) enum NodeAction {
    NoChange,       // Keep the current value, continue visiting children
    Replace(Value), // Replace this value with a new one (e.g., InferredInteger -> I32)
}
```

## The Solving Algorithm

### Entry Points

```rust
pub fn resolve_function(function: &mut Function, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = Solver::new(m);
    hm.solve_function(function, log)
}

pub fn resolve_global(global: &mut GlobalVariable, m: &mut SymbolTab, log: &CompilerLog) -> Result<(), ()> {
    let mut hm = Solver::new(m);
    hm.solve_global_variable(global, log)
}
```

Each function is solved independently. Global variables are solved through a simpler path (no return type tracking).

### Phase 1: Constraint Accumulation

The `visit()` method walks each expression and dispatches to `visit_children()`:

```rust
fn visit(&mut self, e: &ValueId) {
    let action = {
        let current_value = e.borrow();
        self.determine_action(&current_value, e)
    };
    match action {
        NodeAction::Replace(new_value) => {
            e.replace(new_value);  // In-place mutation via RefCell
        }
        NodeAction::NoChange => self.visit_children(e),
    }
}
```

The `determine_action()` method checks if the value is an unresolved literal:

```rust
fn determine_action(&mut self, value: &Value, id: &ValueId) -> NodeAction {
    match value {
        Value::InferredInteger(integer) => self.solve_inferred_integer(id, **integer),
        Value::InferredFloat(float) => self.solve_inferred_float(id, *float),
        _ => NodeAction::NoChange,  // All other values need child visiting
    }
}
```

### Constraint Propagation by Value Type

The `visit_children()` method handles each Value variant differently:

**Binary Operations:**

```rust
Binary { left, op, right } => {
    // 1. Check refinement bounds BEFORE propagating (avoid contamination)
    let constraints_copy = self.constraints.get(e).cloned().unwrap_or_default();
    for c in &constraints_copy {
        let TypeConstraint::Equal(result_ty) = c;
        if matches!(op, Add | Sub | Mul | Div | ... ) {
            if let (Some(lb), Some(rb)) = (self.get_effective_bounds(left), self.get_effective_bounds(right)) {
                if let Some(res) = Self::compute_binary_bounds(op, lb, rb) {
                    self.check_bounds_against_constraint(res, result_ty);
                }
            }
        }
    }
    // 2. Propagate base type constraints to children (unwrap Refine)
    if let Some(parent_constraints) = self.constraints.get(e).cloned() {
        let child_constraints: HashSet<TypeConstraint> = parent_constraints.iter()
            .map(|c| match c {
                TypeConstraint::Equal(ty) => match &**ty {
                    Type::Refine { base, .. } => TypeConstraint::Equal(*base),
                    _ => c.clone(),
                },
            }).collect();
        self.constraints.entry(left.clone()).or_default().extend(child_constraints.clone());
        self.constraints.entry(right.clone()).or_default().extend(child_constraints);
    }
    self.visit(left);
    self.visit(right);
}
```

**Calls (generic detection):**

```rust
Call { callee, args } => {
    let callee_func_id: Option<FunctionId> = match &*callee.borrow() {
        Value::FunctionSymbol { id } => {
            let func = id.borrow();
            if func.generics.is_some() && func.generics.as_ref().is_some_and(|g| !g.is_empty()) {
                Some(id.clone())
            } else {
                None
            }
        }
        _ => None,
    };

    // If generic, infer type args and monomorphize
    if let Some(func_id) = callee_func_id
        && let Some(subst) = self.infer_generic_args_from_call(&func_id, &args.positional)
    {
        let mono_id = self.monomorphize_function(&func_id, &subst);
        callee.replace(Value::FunctionSymbol { id: mono_id });  // Redirect call site
    }

    // Add parameter type constraints to arguments
    if let Value::FunctionSymbol { id } = &*callee.borrow() {
        let func = id.borrow();
        for (i, arg) in args.positional.iter().enumerate() {
            if let Some(param) = func.params.get(i) {
                let param_type = param.borrow().ty;
                self.constraints.entry(arg.clone()).or_default()
                    .insert(TypeConstraint::Equal(param_type));
            }
        }
    }
    // Visit all arguments
    for arg in &args.positional { self.visit(arg); }
    for (_name, arg) in &args.named { self.visit(arg); }
}
```

**Method Calls (with generic method resolution):**

```rust
MethodCall { object, method_name, args } => {
    let obj_type: Option<TypeId> = object.borrow().determine_type(self.m).ok().map(|ty| ty.into());
    let method_id_opt: Option<FunctionId> =
        obj_type.and_then(|obj_type| self.m.get_method(&obj_type, method_name).cloned());

    if let Some(method_id) = method_id_opt {
        let is_generic = {
            let mf = method_id.borrow();
            mf.generics.is_some() && mf.generics.as_ref().is_some_and(|g| !g.is_empty())
        };

        // If generic, monomorphize and rewrite as direct call
        if is_generic && let Some(subst) = self.infer_generic_args_from_call(&method_id, &args.positional) {
            let mono_id = self.monomorphize_function(&method_id, &subst);
            let new_call = Value::Call {
                callee: ValueId::from(Value::FunctionSymbol { id: mono_id }),
                args: args.clone(),
            };
            e.replace(new_call);  // Rewrite MethodCall to Call
            self.visit(e);
            return;
        }
    }
    // ... visit object and arguments
}
```

**Struct Construction (generic struct inference):**

```rust
StructObject { struct_def, fields } => {
    let has_generics = struct_def.borrow().generics.is_some();

    if has_generics {
        // Try to infer concrete type args from field values
        if let Some(subst) = self.infer_generic_args_from_struct_fields(struct_def, fields) {
            let mono_struct_id = self.monomorphize_struct(struct_def, &subst);
            // Replace the struct_def in the original value
            let mut original = e.borrow_mut();
            if let Value::StructObject { struct_def: sd, .. } = &mut *original {
                *sd = mono_struct_id;
            }
        }
    }
    // ... add field type constraints and visit children
}
```

### Phase 2: Inferred Type Resolution

When `determine_action()` encounters `Value::InferredInteger(value)` or `Value::InferredFloat(value)`, it resolves them:

**Integer resolution:**

```rust
fn solve_inferred_integer(&mut self, id: &ValueId, value: u128) -> NodeAction {
    let constraints: Vec<TypeConstraint> = self.constraints
        .get(id).cloned().unwrap_or_default().into_iter().collect();

    for constraint in &constraints {
        let TypeConstraint::Equal(ty) = constraint;

        // Handle Refine types by unwrapping to base
        let effective_ty = match &**ty {
            Type::Refine { base, .. } => base.deref(),
            _ => ty.deref(),
        };

        if !effective_ty.is_integer_primitive() {
            self.errors.insert(TypeErr::IntegerLiteralUnsatisfiable {
                value, unsatisfiable_type: *ty,
            });
            break;
        }

        return match effective_ty {
            Type::I8   => try_convert(value, i8::try_from),
            Type::I16  => try_convert(value, i16::try_from),
            Type::I32  => try_convert(value, i32::try_from),
            Type::I64  => try_convert(value, i64::try_from),
            Type::I128 => try_convert(value, i128::try_from),
            Type::U8   => try_convert(value, u8::try_from),
            Type::U16  => try_convert(value, u16::try_from),
            Type::U32  => try_convert(value, u32::try_from),
            Type::U64  => try_convert(value, u64::try_from),
            Type::U128 => try_convert(value, u128::try_from),
            Type::USize => match self.m.arch_ptr_size() {
                PtrSize::U32 => try_convert(value, u32::try_from).map(|v| Value::USize(32, u64::from(v))),
                PtrSize::U64 => try_convert(value, u64::try_from).map(|v| Value::USize(64, v)),
            },
            _ => NodeAction::NoChange,
        };
    }
    NodeAction::NoChange
}
```

### Phase 3: Monomorphization via Substitution

The `Substitution` system is defined in `substitution.rs`:

```rust
#[derive(Debug, Clone, Default)]
pub(crate) struct Substitution {
    pub mapping: HashMap<u32, TypeId>,  // param_index -> concrete_type
}

impl Substitution {
    pub fn apply(&self, ty: &Type) -> Type {
        match ty {
            Type::GenericParam { index, .. } => {
                if let Some(concrete) = self.mapping.get(index) {
                    (**concrete).clone()  // Replace GenericParam with concrete type
                } else { ty.clone() }
            }
            Type::Inferred { id, .. } => {
                if let Some(concrete) = self.mapping.get(&id.get()) {
                    (**concrete).clone()
                } else { ty.clone() }
            }
            Type::Parameterized { base, .. } => self.apply(base),
            Type::Array { element_type, len } => {
                Type::Array { element_type: TypeId::from(self.apply(element_type)), len: *len }
            }
            Type::Tuple { element_types } => {
                let new: Vec<TypeId> = element_types.iter().map(|et| TypeId::from(self.apply(et))).collect();
                Type::Tuple { element_types: new.into() }
            }
            Type::Function { function_type } => {
                let new_params: Vec<(NString, TypeId)> = function_type.params.iter()
                    .map(|(n, p)| (n.clone(), TypeId::from(self.apply(p)))).collect();
                let new_ret = self.apply(&function_type.return_type);
                Type::Function {
                    function_type: Box::new(FunctionType {
                        attributes: function_type.attributes.clone(),
                        params: new_params.into(),
                        return_type: TypeId::from(new_ret),
                    }),
                }
            }
            Type::Reference { lifetime, exclusive, mutable, to } => {
                Type::Reference { lifetime: lifetime.clone(), exclusive: *exclusive, mutable: *mutable, to: TypeId::from(self.apply(to)) }
            }
            Type::Pointer { .. } | Type::SliceRef { .. } | Type::SlicePtr { .. } => {
                // Recursively substitute the inner type
                // (full code omitted for brevity but follows same pattern)
                ty.clone()
            }
            Type::Refine { base, min, max } => {
                Type::Refine { base: TypeId::from(self.apply(base)), min: *min, max: *max }
            }
            Type::TypeAlias { def } => {
                let type_alias = def.borrow();
                self.apply(&type_alias.type_id)  // Resolve alias first, then substitute
            }
            _ => ty.clone(),  // Primitives, Struct, Enum, TraitObject don't contain generic params
        }
    }
}
```

### Type Unification for Generic Inference

The `unify_types_with_subst` function in `monomorphize.rs` handles matching argument types to parameter types:

```rust
pub(crate) fn unify_types_with_subst(arg_type: &Type, param_type: &Type, subst: &mut Substitution) {
    match (arg_type, param_type) {
        // Direct generic param: record the mapping
        (concrete, Type::GenericParam { index, .. }) => {
            subst.mapping.entry(*index).or_insert_with(|| TypeId::from(concrete.clone()));
        }
        // Inference variable: record the mapping
        (concrete, Type::Inferred { id, .. }) => {
            subst.mapping.entry(id.get()).or_insert_with(|| TypeId::from(concrete.clone()));
        }
        // Recurse into compound types
        (Type::Pointer { to: a_to, .. }, Type::Pointer { to: p_to, .. }) =>
            Self::unify_types_with_subst(a_to, p_to, subst),
        (Type::SliceRef { element_type: a_e, .. }, Type::SliceRef { element_type: p_e, .. }) =>
            Self::unify_types_with_subst(a_e, p_e, subst),
        (Type::Reference { to: a_to, .. }, Type::Reference { to: p_to, .. }) =>
            Self::unify_types_with_subst(a_to, p_to, subst),
        (Type::Array { element_type: a_e, .. }, Type::Array { element_type: p_e, .. }) =>
            Self::unify_types_with_subst(a_e, p_e, subst),
        (Type::Tuple { element_types: a_ets }, Type::Tuple { element_types: p_ets }) => {
            for (a_et, p_et) in a_ets.iter().zip(p_ets.iter()) {
                Self::unify_types_with_subst(a_et, p_et, subst);
            }
        }
        (Type::Function { function_type: a_ft }, Type::Function { function_type: p_ft }) => {
            Self::unify_types_with_subst(&a_ft.return_type, &p_ft.return_type, subst);
            for ((_, a_p), (_, p_p)) in a_ft.params.iter().zip(p_ft.params.iter()) {
                Self::unify_types_with_subst(a_p, p_p, subst);
            }
        }
        // Reverse: param has concrete, arg has generic (contra-variant inference)
        (Type::GenericParam { index, .. }, concrete) => {
            subst.mapping.entry(*index).or_insert_with(|| TypeId::from(concrete.clone()));
        }
        // For struct types, try to inspect field types
        (arg, Type::Struct { def: struct_def_id }) => {
            let struct_def = struct_def_id.borrow();
            if struct_def.generics.is_some() {
                // Extract generic indices from field types
                for field in struct_def.fields.values() {
                    if let Type::GenericParam { .. } = &*field.ty { /* record */ }
                }
            }
        }
        _ => {}  // Non-generic types: skip
    }
}
```

### Struct Generic Inference

The `infer_generic_args_from_struct_fields` method in `monomorphize.rs` handles generic structs:

```rust
pub(crate) fn infer_generic_args_from_struct_fields(
    &self, struct_def_id: &StructDefId, field_values: &[(NString, ValueId)],
) -> Option<Substitution> {
    let struct_def = struct_def_id.borrow();
    let generics = struct_def.generics.as_ref()?;
    if generics.is_empty() { return Some(Substitution::default()); }

    let mut subst = Substitution::default();
    let mut any_concrete_type_found = false;

    for (field_name, field_value_id) in field_values {
        if let Some(field) = struct_def.fields.get(field_name) {
            let field_type = &*field.ty;
            if let Ok(arg_type) = field_value_id.borrow().determine_type(self.m) {
                if arg_type.is_inferred() { continue; }  // Not concrete yet
                any_concrete_type_found = true;
                Self::unify_types_with_subst(&arg_type, field_type, &mut subst);
            }
        }
    }

    if !any_concrete_type_found { return None; }

    // Verify all generic params used in fields have been bound
    for _param_name in generics.keys() {
        // Check if the param appears in any field type
        let appears_in_fields = struct_def.fields.values()
            .any(|f| Self::type_contains_generic_param(&f.ty, _param_name));
        if appears_in_fields {
            // Find this param's index and check if it was bound
            let generic_idx = struct_def.fields.values().find_map(|f| match &*f.ty {
                Type::GenericParam { index, name } if name == _param_name => Some(*index),
                _ => Self::find_generic_index_in_type_deep(&f.ty, _param_name),
            }).unwrap_or(0);

            if generic_idx > 0 && !subst.mapping.contains_key(&generic_idx) {
                return None;  // Defer monomorphization
            }
        }
    }

    Some(subst)
}
```

### Function Monomorphization

```rust
pub(crate) fn monomorphize_function(&mut self, func_id: &FunctionId, subst: &Substitution) -> FunctionId {
    // Check cache first
    let cache_key = self.mono_cache_key(func_id, subst);
    if let Some(existing) = self.mono_cache.get(&cache_key) {
        return existing.clone();
    }

    let func = func_id.borrow();
    self.mono_counter += 1;
    let mono_name = format!("{}::<mono-{}>", func.name, self.mono_counter);

    // Clone parameters with substituted types
    let new_params: Vec<ParameterId> = func.params.iter().map(|param_id| {
        let param = param_id.borrow();
        let new_ty = subst.apply(&param.ty);
        ParameterId::from(Parameter {
            attributes: param.attributes.clone(),
            is_mutable: param.is_mutable,
            name: param.name.clone(),
            ty: TypeId::from(new_ty),
            default_value: param.default_value.clone(),
        })
    }).collect();

    let new_return_type = TypeId::from(subst.apply(&func.return_type));
    let new_body = func.body.as_ref().map(|body| {
        body.iter().map(|element| self.clone_block_element(element, subst)).collect()
    });

    let mono_func = Function {
        visibility: func.visibility,
        attributes: func.attributes.clone(),
        name: mono_name.clone().into(),
        mangled_name: mono_name.into(),
        generics: None,  // No longer generic
        params: new_params,
        return_type: new_return_type,
        body: new_body,
    };

    let mono_id: FunctionId = mono_func.into();
    self.m.add_function(mono_id.clone());
    self.mono_cache.insert(cache_key, mono_id.clone());
    mono_id
}
```

### Struct Monomorphization

```rust
pub(crate) fn monomorphize_struct(&mut self, struct_id: &StructDefId, subst: &Substitution) -> StructDefId {
    let struct_def = struct_id.borrow();
    self.mono_counter += 1;
    let mono_name = format!("{}::<mono-{}>", struct_def.name, self.mono_counter);

    let mut new_fields = BTreeMap::new();
    let mut new_layout = Vec::new();

    for (field_name, field) in &struct_def.fields {
        let new_field_ty = subst.apply(&field.ty);
        let new_field = StructField {
            visibility: field.visibility,
            attributes: field.attributes.clone(),
            name: field.name.clone(),
            ty: TypeId::from(new_field_ty),
            default_value: field.default_value.clone(),
        };
        new_fields.insert(field_name.clone(), new_field);
        new_layout.push(StructMemoryLayoutCell::Field { field_name: field_name.clone() });
    }

    let mono_struct = StructDef {
        visibility: struct_def.visibility,
        name: mono_name.into(),
        attributes: struct_def.attributes.clone(),
        fields: new_fields,
        generics: None,
        layout: new_layout.into(),
    };

    let mono_id: StructDefId = mono_struct.into();
    self.m.add_struct(mono_id.clone());
    mono_id
}
```

### Fixed-Point Iteration

```rust
fn solve_function(&mut self, function: &mut Function, log: &CompilerLog) -> Result<(), ()> {
    if let Some(body) = &mut function.body {
        self.function_return_type = Some(function.return_type);
        loop {
            let prev_len = self.constraints.len();
            for element in body.iter_mut() {
                self.visit_block_element(element);
            }
            if self.constraints.len() == prev_len {
                break;  // Fixed point reached
            }
        }
    }
    // Report errors
    for error in &self.errors { log.report(error); }
    if self.errors.is_empty() { Ok(()) } else { Err(()) }
}
```

## Refinement Type Checking

The solver tracks value bounds through arithmetic. Key methods in `solver.rs`:

**Bounds extraction:**

```rust
fn get_effective_bounds(&self, id: &ValueId) -> Option<Bounds> {
    let own_bounds = {
        let value = id.borrow();
        match &*value {
            Value::I8(_) => Some((-128, 127)),
            Value::U8(_) => Some((0, 255)),
            Value::U16(_) => Some((0, 65535)),
            Value::U32(_) => Some((0, 4294967295)),
            Value::U64(_) => Some((0, 18446744073709551615)),
            Value::I32(_) => Some((-2147483648, 2147483647)),
            // ... all integer primitives
            Value::InferredInteger(v) => Some((**v as i128, **v as i128)),
            Value::LocalVariableSymbol { id } => Self::extract_bounds_from_type(id.borrow().ty.deref()),
            Value::ParameterSymbol { id } => Self::extract_bounds_from_type(id.borrow().ty.deref()),
            _ => None,
        }
    };

    // Intersect own bounds with constraint bounds
    if let Some(constraints) = self.constraints.get(id) {
        let mut effective = own_bounds;
        for constraint in constraints {
            let TypeConstraint::Equal(ty) = constraint;
            if let Some(bounds) = Self::extract_bounds_from_type(ty) {
                effective = match effective {
                    Some((cur_min, cur_max)) => Some((cur_min.max(bounds.0), cur_max.min(bounds.1))),
                    None => Some(bounds),
                };
            }
        }
        return effective;
    }
    own_bounds
}
```

**Bounds computation for binary ops:**

```rust
fn compute_binary_bounds(op: &BinaryOp, left: Bounds, right: Bounds) -> Option<Bounds> {
    let (l_min, l_max) = left;
    let (r_min, r_max) = right;
    match op {
        BinaryOp::Add => Some((l_min.saturating_add(r_min), l_max.saturating_add(r_max))),
        BinaryOp::Sub => Some((l_min.saturating_sub(r_max), l_max.saturating_sub(r_min))),
        BinaryOp::Mul => {
            let products = [
                l_min.saturating_mul(r_min), l_min.saturating_mul(r_max),
                l_max.saturating_mul(r_min), l_max.saturating_mul(r_max),
            ];
            Some((*products.iter().min().unwrap(), *products.iter().max().unwrap()))
        }
        BinaryOp::Div => {
            // Handle division by zero case
            if r_min <= 0 && r_max >= 0 {
                // Could divide by zero → unbounded
                Some((i128::MIN, i128::MAX))
            } else {
                let candidates = vec![
                    l_min.saturating_div(r_min), l_min.saturating_div(r_max),
                    l_max.saturating_div(r_min), l_max.saturating_div(r_max),
                ];
                Some((*candidates.iter().min().unwrap(), *candidates.iter().max().unwrap()))
            }
        }
        BinaryOp::Mod => {
            if r_min <= 0 && r_max >= 0 {
                Some((i128::MIN, i128::MAX))  // Division by zero possible
            } else {
                let a = r_min.abs().max(r_max.abs());
                Some((0, a - 1))
            }
        }
        // ... bitwise, shift operations
        _ => None,  // Comparisons, logical ops don't propagate bounds
    }
}
```

## Error Reporting

Errors are stored in a `HashSet<TypeErr>` for deduplication:

```rust
pub enum TypeErr {
    IntegerLiteralUnsatisfiable { value: u128, unsatisfiable_type: TypeId },
    IntegerLiteralOutsizeRange { value: u128, target_type: TypeId },
    FloatLiteralUnsatisfiable { value: OrderedFloat<f64>, unsatisfiable_type: TypeId },
    IntegerLiteralOutOfRefinementBounds { value: u128, refinement_type: TypeId },
    OperationResultOutOfRefinementBounds { refinement_type: TypeId, computed_min: u128, computed_max: u128 },
}
```

## Integration

```
HIR (with Type::Inferred, Type::GenericParam)
    │
    ▼ [Solver::new(m)]
    │
    ▼ [solve_function() — fixed-point loop]
    │  For each function body:
    │    Loop until constraints stabilize:
    │      Visit all block elements
    │      Accumulate constraints via visit_children()
    │      Resolve InferredInteger/InferredFloat via determine_action()
    │      Monomorphize generic calls via infer_generic_args + monomorphize_function
    │
    ▼ [Errors reported to CompilerLog]
    │
    ▼ [Solved HIR — all Inferred* resolved, all generics monomorphized]
    │
    ▼ [Validator → Codegen]
```

## Design Rationale

### Why Constraint-Based Instead of Classical Unification?

Traditional HM uses destructive unification with type variable substitution. Constraints provide:

1. **Better error messages**: The constraint history tells us _why_ a type was expected
2. **Refinement types**: Bounds tracking requires knowing specific range constraints, not just equality
3. **Monomorphization ordering**: The solver needs to detect when arguments have concrete types before monomorphizing
4. **Mutable store**: Values are in `RefCell`s — destructive unification would require mutable access patterns that conflict

### Why Per-Function Solver Instead of Global?

1. **Modularity**: Function bodies don't share type variables — each function is an independent inference problem
2. **Monomorphization independence**: Generic function bodies are cloned, so their type variables are independent
3. **Parallelism**: Functions can be solved concurrently (potential future optimization)
4. **Incremental compilation**: Only changed functions need re-solving

### Why Fixed-Point Loop?

1. **Mutual dependencies**: `let x = y; let y = 42;` — x depends on y, resolved in second pass
2. **Nested generics**: `map(list, fn(x) -> identity(x))` — first pass monomorphizes `identity<i32>`, second pass monomorphizes `map<List<i32>, fn(i32) -> i32>`
3. **Default types**: Inferring generic defaults may require seeing that a parameter is unconstrained

### Why Separate Monomorphization Cache?

Without caching, each call site of `identity::<i32>(x)` produces a separate monomorphized copy. The cache:

- Reduces code bloat (one copy per unique type argument combination)
- Prevents infinite recursion in recursive generic instantiation
- Speeds compilation by avoiding redundant cloning and substitution
