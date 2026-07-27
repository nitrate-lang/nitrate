# CLI Driver and Compiler Invocation

## Overview

The Nitrate compiler driver is the entry point for user interaction. The `no3` binary — Nitrate's equivalent of Rust's `cargo` or `rustc` — provides a command-line interface for compiling, running, testing, and managing Nitrate packages. The driver orchestrates the translation pipeline, handles command-line parsing, package management, and user-facing output.

## Architecture

**Crate**: `nitrate_driver`  
**Binary**: `no3` (defined in `src/bin/no3.rs`)  
**Key types**: `Driver`, package management, command handlers

## The `no3` Binary

The `no3` binary parses command-line arguments, initializes logging, creates the driver, routes to the appropriate command handler, and handles exit codes. The main function matches the first argument as a subcommand:

```
new | init | build | run | test | check | lex | parse | doc | clean
add | remove | install | uninstall | publish | search | update | bench
```

## Commands

**`no3 new <name>`**: Creates a new package with standard directory structure (`no3.xml`, `src/entry.nit`, `.gitignore`, `README.md`). The `entry.nit` contains a basic "Hello, World!" program.

**`no3 build [--release] [--emit-llvm] [--emit-asm]`**: Compiles the package by loading the manifest, discovering sources, running the full pipeline, and producing output files. The `--release` flag uses optimization level 3; default is debug level 0.

**`no3 run [--release]`**: Builds and executes the resulting binary.

**`no3 check`**: Runs compilation through validation without producing output — faster than a full build.

**`no3 test`**: Compiles and runs test files.

**`no3 lex <file>` / `no3 parse <file>`**: Debug modes that lex or parse a file and print the result.

**`no3 clean`**: Removes build artifacts.

**`no3 add <package>` / `no3 remove <package>`**: Manages package dependencies by resolving package sources and updating `no3.xml`.

**`no3 doc`**: Generates HTML documentation from documentation comments.

**`no3 install <package>` / `no3 uninstall <package>`**: Global package installation.

**`no3 publish`**: Publishes the current package to the registry.

**`no3 update`**: Updates dependencies to latest compatible versions.

**`no3 search <query>`**: Searches the package registry.

## Package Manifest

The `no3.xml` manifest uses XML format with fields for name, version, description, authors, license, and dependencies. Dependencies specify name and version constraint (e.g., `^0.1.0`).

## Error Code Explanation

The `explain_code.rs` module provides detailed explanations for error codes, used by `no3 explain <ERROR_CODE>`.

## Build Output

Build artifacts are placed in `.build/debug/` or `.build/release/` directories, containing the executable, object file, and optionally LLVM IR file.

## Design Rationale

A single binary with subcommands combines compiler and package manager functionality for simplicity (one tool to install and use), consistency (same options across commands), and tight integration between compilation and package resolution. XML was chosen for the manifest format for its wide parser support, schema validation capability, extensibility, and human readability.
