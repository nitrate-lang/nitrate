# Nitrate: Generics & Monomorphization Architecture

## Overview

Nitrate implements generics via **monomorphization** — the same approach as C++ templates and Rust generics. A generic declaration (function, struct, enum, type-alias) serves as a _template_. When used with concrete type arguments, the compiler creates a specialized copy with all generic parameters substituted.

### Key Distinction: Two Kinds of "Type Variable"

The HIR has two type-variable mechanisms with different lifetimes:

| Mechanism            | Purpose                               | Created                            | Resolved                                        |
| -------------------- | ------------------------------------- | ---------------------------------- | ----------------------------------------------- |
| `Type::Inferred`     | Local type inference (`let x = ...`)  | By the solver during type-checking | Constraint propagation within the same function |
| `Type::GenericParam` | User-declared generic (`fn foo<T>()`) | During AST-to-HIR lowering         | Monomorphization substitution at each call site |

`Inferred` variables die when their containing function is solved. `GenericParam` variables persist until a caller provides concrete types.

---

## Pipeline Overview

```
Source Code
    │
    ▼
[AST Parser] ─────────  Produces Generics { params: [TypeParam] }
    │
    ▼
[HIR Lowering] ───────  Creates Type::GenericParam for each declared param
    │                    Creates Type::Parameterized for each type-with-args use
    ▼
[HM + Monomorphization] ─  The core pass
    │  • Walks each function body
    │  • When it finds a Call to a generic function:
    │    1. Infers concrete type args from argument types
    │    2. Clones the function body, substituting GenericParam → concrete types
    │    3. Registers the concrete copy in the symbol table
    │    4. Redirects the call site to the copy
    ▼
[LLVM Codegen] ───────  Skips generic (uninstantiated) functions
                         Compiles only concrete monomorphized copies
```

---

## 1. The Type System

### Two New Variants

**`Type::GenericParam { index, name }`** — marks the _declaration site_ of a generic parameter. Appears in parameter types, field types, and return types wherever the user wrote the parameter name.

**`Type::Parameterized { base, args }`** — marks a _use site_ where a generic type is applied to concrete arguments (e.g., `Foo<i32>`). Stores the base type ID and the argument list.

### Where Generics Live

Four item types carry a `generics: Option<BTreeMap<Name, TypeId>>` field:

- `Function` — generic function parameters
- `StructDef` — generic struct parameters
- `EnumDef` — generic enum parameters
- `TypeAliasDef` — generic type-alias parameters

When `None`, the item has no generics. When `Some(map)`, each key is a parameter name and each value is its (optional) default type.

---

## 2. The Hindley-Milner Pass

The HM pass at `hir_polish/src/hindley_milner.rs` performs two tasks simultaneously:

### 2a. Constraint-Based Type Inference

The pass maintains a `HashMap<ValueId, HashSet<TypeConstraint>>`. As it walks the expression tree, it adds equality constraints:

- **Assign**: `place` type must equal `value` type
- **Binary**: operands must have the same type as the result
- **If/While**: condition must be `Bool`
- **Return**: value type must equal function return type
- **Struct fields**: each field value must match the struct field's declared type

The pass runs a fixed-point loop: keep visiting until no new constraints are added. This ensures transitive propagation (if `x : i32` and `y = x`, then `y : i32`).

Integer and float literals are initially `InferredInteger`/`InferredFloat`. The pass resolves them by checking constraints — a literal constrained to `I32` becomes `Value::I32(42)`, constrained to `F64` becomes `Value::F64(3.14)`, etc.

### 2b. Monomorphization via Substitution

When visiting a `Value::Call`, the pass checks whether the callee is a generic function (has `generics.is_some()`). If so:

1. **Infer type arguments**: For each positional argument, unify the argument's concrete type with the parameter's type. Where the parameter type contains `GenericParam { index }`, record `index → concrete_type` in a `Substitution` map.

2. **Create monomorphized copy**: Clone the function definition. Apply the substitution to every type in the clone — parameters, return type, local variables, and cast targets. Set `generics: None` on the clone.

3. **Register**: Store the monomorphized function in both the thread-local Store and the symbol table.

4. **Redirect**: Replace the call site's callee to reference the monomorphized copy.

The substitution is recursive: it walks into compound types (arrays, tuples, references, pointers, functions) and substitutes at every level.

### 2c. The Fixed-Point Loop

```
loop {
    prev = constraints.len()
    visit all block elements (may add new constraints)
    visit all block elements (may trigger monomorphization)
    if constraints.len() == prev → break
}
```

This ensures that:

- Constraints propagate transitively (e.g., through binary ops and assignments)
- After monomorphization replaces a callee, any constraints from the new callee's return type are picked up
- Nested generics work: if a generic function's body calls another generic, the second pass picks it up

---

## 3. Integration Requirements

For monomorphization to work end-to-end, three integration points must be maintained:

### 3a. HIR Lowering: No Early Errors

The lowerers (`lower_expr_path`, `lower_struct_init`, `lower_implementation`) must not reject generic constructs. They should accept syntax like `Foo<i32>` and `impl<T> ...` even if the type arguments aren't immediately processed — the monomorphization pass handles them later.

### 3b. Symbol Table Registration

Monomorphized functions must be added to the symbol table via `add_function()`. Otherwise, LLVM codegen won't iterate them and the call site will reference an undefined symbol. This requires the HM pass to take `&mut SymbolTab` rather than `&SymbolTab`.

### 3c. LLVM Codegen: Skip Templates

The codegen loop in `generate_llvmir()` must skip any function whose `generics` field is non-empty. Only concrete (monomorphized) copies should produce LLVM IR. A panic-guard in `gen_ty()` catches any `GenericParam` that slips through, ensuring early failure rather than silent miscompilation.

---

## 4. Naming and Storage

Monomorphized functions are named `"original::<mono-N>"` (counter-based). Both `name` and `mangled_name` use this scheme.

The compiler uses a thread-local storage pattern for its stores:

- `FunctionId::from(func)` stores the function in a global `AppendOnlyVec` via TLS
- The symbol table maintains a separate `HashMap<Name, FunctionId>` for name-based lookup
- Codegen iterates the symbol table, which includes both user-declared and monomorphized functions

The system deduplicates by caching with `(generic_function_store_index, sorted_concrete_type_args)` as the key — the `mono_cache` field on `HindleyMilner`. When the same generic function is instantiated with identical concrete type arguments at multiple call sites, the existing monomorphized copy is reused.

---

## 5. Edge Cases

| Scenario                                                    | Behavior                                                                           |
| ----------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| Generic with zero call-site arguments (`fn make<T>() -> T`) | Returns `None` from infer — type cannot be inferred, requires annotation           |
| Generic calling generic                                     | Works via fixed-point loop (second pass detects the inner call after substitution) |
| Reference generic param (`fn id<T>(x: &T) -> &T`)           | Unification recurses into compound types automatically                             |
| Multiple generic params                                     | Each param gets its own index; substitution maps independently                     |
| Generic with unused param                                   | Empty substitution is applied; clone proceeds normally                             |

---

## 6. Design Rationale

**Why monomorphization at the HIR level instead of LLVM?** LLVM has no template system. Generating LLVM IR with generic placeholders would require runtime dispatch or JIT compilation, both of which defeat the purpose of monomorphization. Rust uses the same approach (monomorphization during MIR → LLVM translation).

**Why separate `GenericParam` from `Inferred`?** They have different lifetimes and resolution strategies. `Inferred` is solved by constraint propagation within a single function. `GenericParam` persists until a caller instantiates the generic, potentially across translation units. Using one variant for both purposes caused confusion and prevented clean separation of the two resolution strategies.

**Why mutable access to the symbol table?** Monomorphization creates new functions that must be discoverable by subsequent passes. The symbol table is the natural discovery mechanism. Without `&mut SymbolTab`, monomorphized functions would exist only in the TLS store and be invisible to LLVM codegen.
