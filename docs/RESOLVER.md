# Name Resolution Subsystem

## Overview

Name resolution is the phase between parsing and HIR lowering. It processes `use` declarations and resolves all name references in the parse tree to their fully qualified equivalents. This stage ensures that every name used in the source code can be mapped to a specific declaration before the HIR lowering phase.

## Architecture

**Crate**: `nitrate_tree_resolve`  
**Key types**: `SymbolTab`, path resolution, import resolution  
**Key files**: `lib.rs`, `resolve_import.rs`, `resolve_path.rs`, `symbol_table.rs`, `diagnosis.rs`

## Resolver Pipeline

```
Parse Tree (with unresolved names)
    │
    ▼ [Import Resolution]
    ├── Process 'use' declarations
    ├── Resolve module paths
    └── Build import graph

    ▼ [Path Resolution]
    ├── Resolve type paths (Foo, std::collections::HashMap)
    ├── Resolve value paths
    └── Convert to resolved path representations

    ▼ [Symbol Table Population]
    ├── Register all declared symbols
    └── Build preliminary symbol table entries

Resolved Parse Tree + Symbol Table
```

## Import Resolution

The `resolve_import.rs` module handles `use` declarations:

```rust
pub fn resolve_import(import: &UseDecl, symbol_table: &mut SymbolTab) -> Result<(), ResolveError> {
    // Resolve the import path
    let resolved_path = resolve_path(&import.path, symbol_table)?;

    // Register the imported name in the local scope
    symbol_table.add_import(import.name.clone(), resolved_path);

    Ok(())
}
```

Import paths can be:

- **Absolute**: Starting from the package root (e.g., `std::collections::HashMap`)
- **Relative**: Starting from the current module (e.g., `super::helper::util`)
- **Self**: Referring to the current module
- **Super**: Referring to the parent module

Wildcard imports (`use path::to::*`) import all public names from the target module.

## Path Resolution

The `resolve_path.rs` module resolves path expressions to their targets:

```rust
pub enum ResolvedPath {
    Module(ModuleId),
    Function(FunctionId),
    Struct(StructDefId),
    Enum(EnumDefId),
    TypeAlias(TypeAliasDefId),
    Trait(TraitId),
    GlobalVariable(GlobalVariableId),
    LocalVariable(LocalVariableId),
    Parameter(ParameterId),
}
```

Path resolution:

1. Segments the path into components (separated by `::`)
2. Resolves each segment by looking up the name in the current scope
3. Verifies that the target exists and is accessible
4. Returns the resolved target

## Symbol Table

The `SymbolTab` (defined in `symbol_table.rs`) is the central registry of all symbols in a compilation session:

```rust
pub struct SymbolTab {
    // Function lookup: name → FunctionId
    functions: HashMap<NString, FunctionId>,
    // Type lookup: name → TypeId (for named types)
    named_types: HashMap<NString, TypeOrDef>,
    // Global variables
    globals: HashMap<NString, GlobalVariableId>,
    // Method dispatch: TypeId → method_name → FunctionId
    methods: HashMap<TypeId, HashMap<NString, FunctionId>>,
    // Module tree
    module_tree: ModuleTree,
    // Architecture pointer size
    ptr_size: PtrSize,
}
```

### Key Operations

- `add_function(func)`: Register a function in the symbol table
- `get_function(name)`: Look up a function by name
- `add_global(global)`: Register a global variable
- `globals()`: Iterate over all registered globals
- `functions()`: Iterate over all registered functions
- `get_method(type_id, method_name)`: Look up a method for a type

### Method Resolution

Methods are resolved by their receiver type:

```rust
pub fn get_method(&self, type_id: &TypeId, method_name: &NString) -> Option<&FunctionId> {
    self.methods.get(type_id)
        .and_then(|methods| methods.get(method_name))
}
```

This enables method call syntax: `object.method(args)`.

## Module Tree

The module tree tracks the hierarchical module structure:

```rust
pub struct ModuleTree {
    // Root module
    root: ModuleNode,
    // All modules by path
    modules: HashMap<Vec<NString>, ModuleNode>,
}

struct ModuleNode {
    name: NString,
    children: Vec<ModuleNode>,
    items: Vec<Item>,
}
```

The module tree is built during import resolution and used for path resolution.

## Error Handling

The resolver reports the following errors (in `diagnosis.rs`):

- **UnknownName**: A name could not be resolved in any visible scope
- **AmbiguousName**: Multiple symbols match the given name
- **ModuleNotFound**: The specified module path doesn't exist
- **CyclicImport**: A circular import dependency was detected
- **VisibilityViolation**: Attempting to access a private symbol
- **NameConflict**: Two symbols with the same name in the same scope

## Integration

The resolver operates between parsing and HIR lowering:

```
Parser → [Resolver] → HIR Lowering → ...
```

The resolved parse tree is passed to `nitrate_hir_from_tree`, which uses the symbol table to create `FunctionSymbol`, `GlobalVariableSymbol`, etc. during lowering.
