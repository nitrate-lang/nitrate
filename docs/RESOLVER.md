# Name Resolution Subsystem

## Overview and Purpose

Name resolution is the phase between parsing and HIR lowering that transforms the source-level, context-dependent name references in the parse tree into fully qualified, unambiguous symbol references. Before this phase, a reference to `foo` could refer to any of several different declarations — a local variable, a function parameter, a global function, a struct type, or an imported name from another module. After resolution, every name reference is mapped to its unique declaration, resolving all ambiguity introduced by scoping and imports.

The resolver processes `use` declarations (imports), resolves all name paths in the parse tree to their fully qualified equivalents, and builds a symbol table that maps names to their declarations. This symbol table is then used by the HIR lowerer to create direct references (`FunctionId`, `StructDefId`, etc.) rather than string-based name lookups.

**Key distinction**: The resolver does not perform type checking or semantic analysis beyond name resolution. It answers only the question "which declaration does this name refer to?" — leaving questions about type compatibility, method lookup, and expression validity to the solver and validator stages.

## Architecture

**Crate**: `nitrate_tree_resolve`  
**Key types**: `SymbolTab`, resolved paths, import resolution data structures  
**Key files**: `lib.rs`, `resolve_import.rs`, `resolve_path.rs`, `symbol_table.rs`, `diagnosis.rs`

## Resolver Pipeline

The resolver operates in three sequential phases, each building on the results of the previous phase:

```
Parse Tree (with unresolved names)
    │
    ▼ [Phase 1: Import Resolution]
    ├── Process 'use' declarations throughout the module tree
    ├── Resolve module paths to their target modules
    └── Build import graph for cycle detection
    │
    ▼ [Phase 2: Symbol Table Population]
    ├── Visit all item declarations via depth-first traversal
    ├── Register each declared symbol with its fully qualified name
    ├── Register generic parameters in their scope
    └── Record method implementations for later dispatch
    │
    ▼ [Phase 3: Name Path Resolution]
    ├── Visit all expression and type paths
    ├── Resolve each name segment through scope lookup
    ├── Convert to resolved path representations
    └── Report unresolved or ambiguous references
    │
Resolved Parse Tree + Symbol Table (→ HIR Lowering)
```

## Import Resolution

The `resolve_import.rs` module processes `use` declarations throughout the module tree:

```rust
pub fn resolve_import(import: &UseDecl, symbol_table: &mut SymbolTab) -> Result<(), ResolveError> {
    // Resolve the import path to its target
    let resolved_path = resolve_path(&import.path, symbol_table)?;

    // Register the imported name in the local scope
    symbol_table.add_import(import.name.clone(), resolved_path);

    Ok(())
}
```

Import paths can be specified in several forms:

- **Absolute paths**: Starting from the package root (e.g., `std::collections::HashMap`), identified by the package name as the first segment
- **Relative paths**: Starting from the current module (e.g., `helper::util`), resolved by walking the module tree from the current position
- **Self references**: `self` referring to the current module, used for relative imports within the same module
- **Super references**: `super` referring to the parent module, used for navigating up the module hierarchy (multiple `super::super::` sequences are supported)

Wildcard imports (`use path::to::*`) import all public names from the target module into the current scope. The resolver expands wildcard imports by enumerating the target module's public declarations and adding each as a name binding in the current scope.

## Path Resolution

The `resolve_path.rs` module resolves path expressions to their target symbols. A resolved path maps to one of several possible target types:

```rust
pub enum ResolvedPath {
    Module(ModuleId),             // A module reference
    Function(FunctionId),         // A function declaration
    Struct(StructDefId),          // A struct definition
    Enum(EnumDefId),              // An enum definition
    TypeAlias(TypeAliasDefId),    // A type alias declaration
    Trait(TraitId),               // A trait definition
    GlobalVariable(GlobalVariableId), // A global variable
    LocalVariable(LocalVariableId),   // A local variable
    Parameter(ParameterId),       // A function parameter
}
```

Path resolution proceeds through a multi-step algorithm:

1. **Segment the path**: Split the path into its individual components separated by `::` — for example, `std::collections::HashMap` produces segments `["std", "collections", "HashMap"]`
2. **Resolve the first segment**: Determine whether the first segment refers to a package name (absolute path), a module name (relative path), or a special keyword like `self` or `super`
3. **Walk the module tree**: For each subsequent segment, look up the name in the current module's scope, descending into submodules as needed
4. **Verify accessibility**: Check that the target symbol is visible from the current scope (respecting visibility modifiers)
5. **Return the resolved target**: If all segments resolve successfully, return the final target with its type

## Symbol Table

The `SymbolTab` (defined in `symbol_table.rs`) is the central registry of all symbols in a compilation session. It is built during the resolution phase and used by all subsequent phases (HIR lowering, type solving, validation, and code generation):

```rust
pub struct SymbolTab {
    // Function lookup: name → FunctionId
    functions: HashMap<NString, FunctionId>,
    // Type lookup: name → TypeOrDef (structs, enums, type aliases)
    named_types: HashMap<NString, TypeOrDef>,
    // Global variables: name → GlobalVariableId
    globals: HashMap<NString, GlobalVariableId>,
    // Method dispatch: TypeId → method_name → FunctionId
    methods: HashMap<TypeId, HashMap<NString, FunctionId>>,
    // Module tree for hierarchical name resolution
    module_tree: ModuleTree,
    // Target architecture pointer size
    ptr_size: PtrSize,
}
```

### Key Operations

- **`add_function(func)`**: Register a function in the symbol table, making it available for call resolution and code generation iteration
- **`get_function(name)`**: Look up a function by its fully qualified name, returning the `FunctionId` handle
- **`add_global(global)`**: Register a global variable for visibility to other passes
- **`globals()`**: Iterate over all registered global variables (used by codegen to emit LLVM globals)
- **`functions()`**: Iterate over all registered functions including monomorphized copies (used by codegen and validation)
- **`get_method(type_id, method_name)`**: Look up a method implementation for a specific type, enabling method call syntax resolution

### Method Resolution

Methods are resolved by their receiver type during the type solving phase. The symbol table maintains a nested map structure for efficient method lookup:

```rust
pub fn get_method(&self, type_id: &TypeId, method_name: &NString) -> Option<&FunctionId> {
    self.methods.get(type_id)
        .and_then(|methods| methods.get(method_name))
}
```

This enables the solver to resolve `object.method(args)` by determining the type of `object`, looking up that type in the methods map, and finding the matching method name. Method resolution occurs during type inference (the solver phase), not during name resolution, because the receiver type may not be known until type constraints are processed.

## Module Tree

The module tree tracks the hierarchical module structure of the entire package:

```rust
pub struct ModuleTree {
    // Root module
    root: ModuleNode,
    // All modules by path (for O(1) lookup by qualified path)
    modules: HashMap<Vec<NString>, ModuleNode>,
}

struct ModuleNode {
    name: NString,
    children: Vec<ModuleNode>,
    items: Vec<Item>,
}
```

The module tree is built during import resolution and used throughout the compilation session for path resolution, import processing, and module-level visibility checks.

## Symbol Discovery via Depth-First Traversal

The core symbol discovery algorithm walks the entire parse tree using a depth-first iterator, entering and leaving scopes as it goes:

```rust
pub fn discover_symbols(module: &mut Module) -> HashMap<NString, SymbolKind> {
    let mut symbol_map = HashMap::new();
    let mut scope_vec = Vec::new();

    module.depth_first_iter(&mut |order, node| {
        if order == Order::Enter {
            match node {
                RefNode::ItemFunction(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Function);
                    enumerate_generics(&mut scope_vec, sym.name.clone(),
                        &sym.generics, &mut symbol_map);
                }
                RefNode::ItemStruct(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Struct);
                    enumerate_generics(&mut scope_vec, sym.name.clone(),
                        &sym.generics, &mut symbol_map);
                }
                // Handles ItemEnum, ItemTrait, ItemTypeAlias, ItemGlobalVariable,
                // ItemFuncParam, ExprLocalVariable, ItemEnumVariant, ItemImpl, ItemModule
            }
        }
        // Track scope enter/leave for name qualification
    });
    symbol_map
}
```

The `qualify_name` function builds a fully qualified name by joining the current scope vector with `::` separators and appending the symbol's local name. This ensures that symbols with the same local name in different modules produce distinct fully qualified names. The `enumerate_generics` function registers generic parameter names within their parent's scope, making them available for type references within the generic declaration.

## Error Handling

The resolver reports the following error types (defined in `diagnosis.rs`):

- **UnknownName**: A name could not be resolved in any visible scope — no declaration with that name exists in the current scope or any accessible module
- **AmbiguousName**: Multiple symbols match the given name in the current scope, and the ambiguity cannot be resolved by context
- **ModuleNotFound**: The specified module path in an import or path expression does not correspond to any known module
- **CyclicImport**: A circular import dependency was detected — module A imports from module B which imports from module A
- **VisibilityViolation**: An attempt to access a symbol that is not visible from the current scope (e.g., accessing a `sec` symbol from another module)
- **NameConflict**: Two symbols with the same name were declared in the same scope, creating an unresolvable conflict

## Integration

The resolver operates between parsing and HIR lowering in the compilation pipeline:

```
Parser → [Resolver] → HIR Lowering → Solver → Validator → Codegen
```

The resolved parse tree and populated symbol table are passed to `nitrate_hir_from_tree`, which uses the symbol table to create `FunctionSymbol`, `GlobalVariableSymbol`, `LocalVariableSymbol`, and `ParameterSymbol` value nodes during lowering. The symbol table also persists to the solver phase, where it is used for method resolution and monomorphized function registration.
