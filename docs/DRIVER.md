# CLI Driver and Compiler Invocation

## Overview

The Nitrate compiler driver is the entry point for user interaction. The `no3` binary (Nitrate's equivalent of Rust's `cargo` or `rustc`) provides a command-line interface for compiling, running, testing, and managing Nitrate packages. The driver orchestrates the translation pipeline, handles command-line parsing, package management, and user-facing output.

## Architecture

**Crate**: `nitrate_driver`  
**Binary**: `no3` (defined in `src/bin/no3.rs`)  
**Key types**: `Driver`, package management, command handlers

## The `no3` Binary

The `no3` binary is the main entry point. It:

1. Parses command-line arguments
2. Initializes logging
3. Creates the driver
4. Routes to the appropriate command handler
5. Handles exit codes

```rust
fn main() {
    let args: Vec<String> = env::args().collect();
    // Parse subcommand
    match args.get(1).map(String::as_str) {
        Some("new") => cmd_new(&args[2..]),
        Some("init") => cmd_init(&args[2..]),
        Some("build") => cmd_build(&args[2..]),
        Some("run") => cmd_run(&args[2..]),
        Some("test") => cmd_test(&args[2..]),
        Some("check") => cmd_check(&args[2..]),
        Some("lex") => cmd_lex(&args[2..]),
        Some("parse") => cmd_parse(&args[2..]),
        Some("doc") => cmd_doc(&args[2..]),
        Some("clean") => cmd_clean(&args[2..]),
        Some("add") => cmd_add(&args[2..]),
        Some("remove") => cmd_remove(&args[2..]),
        Some("install") => cmd_install(&args[2..]),
        Some("uninstall") => cmd_uninstall(&args[2..]),
        Some("publish") => cmd_publish(&args[2..]),
        Some("search") => cmd_search(&args[2..]),
        Some("update") => cmd_update(&args[2..]),
        Some("bench") => cmd_bench(&args[2..]),
        _ => print_usage(),
    }
}
```

## Commands

### `no3 new <name>`

Creates a new Nitrate package with the standard directory structure:

```
package-name/
├── no3.xml
├── src/
│   └── entry.nit
├── .gitignore
└── README.md
```

The `entry.nit` file contains a basic "Hello, World!" program. The `no3.xml` manifest is populated with default metadata (name, version, author).

### `no3 init`

Similar to `new`, but initializes the current directory as a Nitrate package (doesn't create a subdirectory).

### `no3 build [--release] [--emit-llvm] [--emit-asm]`

Compiles the package in the current directory:

1. Loads the package manifest (`no3.xml`)
2. Discovers source files
3. Runs the full translation pipeline
4. Produces output files in the build directory

Flags:

- `--release`: Optimize with level 3 (default is debug level 0)
- `--emit-llvm`: Output LLVM IR files instead of object files
- `--emit-asm`: Output assembly files instead of object files

### `no3 run [--release]`

Builds and runs the package. After successful compilation, executes the resulting binary.

### `no3 check`

Runs the compiler up to the validation stage without producing output. Faster than a full build, useful for checking correctness during development.

### `no3 test`

Compiles and runs test files. Discovers test files following naming conventions.

### `no3 lex <file>`

Lex-only mode: lexes the specified file and prints the token stream. Useful for debugging the lexer.

### `no3 parse <file>`

Parse-only mode: lexes and parses the specified file, printing the AST. Useful for debugging the parser.

### `no3 clean`

Removes build artifacts (the build directory).

### `no3 add <package>`

Adds a package dependency:

1. Resolves the package name to a source (registry, git, or local path)
2. Downloads or links the package
3. Updates `no3.xml` with the dependency entry

### `no3 remove <package>`

Removes a package dependency:

1. Removes the dependency entry from `no3.xml`
2. Cleans up downloaded files

### `no3 doc`

Generates documentation for the package. Processes documentation comments and produces HTML documentation.

### `no3 install <package>`

Installs a package globally (makes it available as a command).

### `no3 uninstall <package>`

Removes a globally installed package.

### `no3 publish`

Publishes the current package to the package registry.

### `no3 update`

Updates all package dependencies to their latest compatible versions.

### `no3 search <query>`

Searches the package registry for packages matching the query.

## Package Management

### Package Manifest (`no3.xml`)

The package manifest defines package metadata:

```xml
<package>
  <name>my-package</name>
  <version>0.1.0</version>
  <description>A Nitrate package</description>
  <authors>
    <author>Developer Name</author>
  </authors>
  <license>MIT</license>
  <dependencies>
    <dependency>
      <name>some-lib</name>
      <version>0.1.0</version>
    </dependency>
  </dependencies>
</package>
```

Manifest fields:

- `name`: Package name (used for naming output files)
- `version`: Semantic version string
- `description`: Human-readable description
- `authors`: List of authors
- `license`: License identifier
- `dependencies`: List of dependency specifications

### Package Resolution

The `package.rs` module handles package resolution:

1. Load `no3.xml` from the current directory
2. Parse package metadata
3. Resolve dependencies (recursively)
4. Build the dependency graph
5. Ensure version compatibility
6. Download missing dependencies

### Error Code Explanation

The `explain_code.rs` module provides detailed explanations for error codes:

```rust
pub fn explain_error_code(code: &str) -> Option<String> {
    match code {
        "L0300" => Some("Integer literals are parsed and stored as u128 values. \
                         This error occurs when the literal exceeds 2^128 - 1. \
                         Consider using a smaller literal or a different numeric representation.".to_string()),
        // ...
    }
}
```

This is used by `no3 explain <ERROR_CODE>` to provide in-depth error descriptions.

## The Driver Struct

```rust
pub struct Driver {
    // Compiler log for error accumulation
    log: CompilerLog,
    // Current package manifest
    package: Option<Package>,
    // Translation options
    options: TranslationOptions,
}
```

The driver provides:

- `new()`: Create a new driver instance
- `load_package(path)`: Load a package from a directory
- `compile()`: Run the full compilation pipeline
- `run()`: Compile and execute
- `check()`: Run validation only

## Build Output

Build artifacts are placed in a directory structure:

```
.build/
├── debug/
│   ├── package-name        (executable)
│   ├── package-name.o      (object file)
│   └── package-name.ll     (LLVM IR, with --emit-llvm)
└── release/
    ├── package-name
    ├── package-name.o
    └── package-name.ll
```

## Design Decisions

### Why a Single Binary with Subcommands?

The `no3` binary combines both compiler and package manager functionality:

1. **Simplicity**: A single tool to install and use
2. **Consistency**: Same options work for building, running, and testing
3. **Dependency management**: Tight integration between compilation and package resolution

### Why XML for the Package Manifest?

XML was chosen for the manifest format because:

1. **Widely supported**: Parsers are available in most languages
2. **Schema validation**: XML Schema enables manifest structure validation
3. **Extensible**: New manifest fields can be added without breaking compatibility
4. **Self-documenting**: The XML structure is human-readable
