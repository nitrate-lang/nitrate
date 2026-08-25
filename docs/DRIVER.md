# CLI Driver and Compiler Invocation

## Overview

The Nitrate compiler driver is the entry point for user interaction. The `no3` binary — Nitrate's equivalent of Rust's `cargo` or `rustc` — provides a command-line interface for compiling, running, testing, and managing Nitrate packages. The driver orchestrates the translation pipeline, handles command-line parsing, package management, and user-facing output.

## Architecture

**Crate**: `nitrate_driver`  
**Binary**: `no3` (defined in `src/bin/no3.rs`)  
**Key types**: `Interpreter`, package management (`Manifest`, `Lockfile`), command handlers

## The `no3` Binary

The `no3` binary parses command-line arguments, initializes logging, creates the driver, routes to the appropriate command handler, and handles exit codes. The command-line interface is intentionally drop-in compatible with Rust's `cargo`, so existing cargo users can use `no3` with the same flags and semantics.

## Commands

**`no3 new <path>`**: Creates a new package with standard directory structure (`no3.toml`, `src/entry.nit`, `.gitignore`, `README.md`). Supports `--bin`, `--lib`, `--edition`, `--name`, `--vcs` (git/hg/pijul/fossil/none), and `--registry` flags, matching `cargo new`.

**`no3 init [path]`**: Like `no3 new` but initializes in an existing directory. Defaults to the current directory; refuses to run if conflicting package files exist.

**`no3 build [OPTIONS]`**: Compiles the package by loading the manifest, discovering sources, running the full pipeline, and producing output in `target/debug/` or `target/release/` (cargo-compatible `target/` layout). Supports `-r/--release`, `--profile`, `-j/--jobs`, `--keep-going`, `--target`, `--target-dir`, `--manifest-path`, package selection (`-p`, `--workspace`, `--exclude`), target selection (`--lib`, `--bins`, `--bin`, `--examples`, `--example`, `--tests`, `--test`, `--benches`, `--bench`, `--all-targets`), and feature selection (`-F/--features`, `--all-features`, `--no-default-features`).

**`no3 run [OPTIONS] [ARGS]...`**: Builds and executes the resulting binary, passing `[ARGS]...` through to the executed program. Supports package/target/feature/compilation options like `cargo run`.

**`no3 check [OPTIONS]`**: Runs the compilation pipeline through HIR validation without producing object files — faster than a full build. Supports the same selection/feature/compilation options as `cargo check`.

**`no3 test [OPTIONS] [TESTNAME] [-- [ARGS]...]`**: Compiles and runs the package's test binary. Supports `--no-run`, `--no-fail-fast`, `-q/--quiet`, `--release`, `--profile`, and test-name filtering plus test binary args after `--`.

**`no3 bench [OPTIONS] [BENCHNAME] [-- [ARGS]...]`**: Compiles (in release mode by default) and runs the package's benchmark binary. Supports `--no-run`, `--no-fail-fast`, and standard selection/compilation options.

**`no3 clean [OPTIONS]`**: Removes build artifacts. By default removes the entire `target/` directory; `-r/--release`, `--debug`, `--doc`, `--profile <NAME>` clean only specific subdirectories. Supports `--target-dir`, `--manifest-path`, and `-n/--dry-run`.

**`no3 add <DEP>[@<VERSION>]... [OPTIONS]`**: Adds dependencies to `no3.toml`. Supports registry deps (`no3 add serde`, `no3 add serde@1`), `--path <PATH>` local deps, `--git <URL>` git deps with `--branch`/`--tag`/`--rev`, `--dev`/`--build` dependency sections, `--optional`, `--no-default-features`, `-F/--features`, `--rename`, `-p/--package`, and `-n/--dry-run` — matching `cargo add`.

**`no3 remove <DEP_ID>... [OPTIONS]`**: Removes dependencies from `no3.toml`. By default removes from all dependency sections; `--dev`/`--build` restrict to a single section. Supports `-p/--package`, `--manifest-path`, and `-n/--dry-run` — matching `cargo remove`.

**`no3 update [SPEC]... [OPTIONS]`**: Updates `no3.lock` to the latest versions matching the manifest requirements. Supports `--precise <VERSION>`, `-n/--dry-run`, `--recursive`, `-w/--workspace`, and `--manifest-path` — matching `cargo update`.

**`no3 doc [OPTIONS]`**: Generates HTML documentation from the parsed source into `target/<profile>/doc/`. Supports `--open`, `--no-deps`, `--document-private-items`, and standard package/feature/compilation options — matching `cargo doc`.

**`no3 search [QUERY]... [OPTIONS]`**: Searches the package registry. Supports `--limit` (default 10, max 100), `--index <URL>`, `--registry` — matching `cargo search`.

**`no3 publish [OPTIONS]`**: Packages the current package into `target/package/<name>-<version>.crate` (tar.gz containing sources + manifest) and uploads it to the registry. Supports `-n/--dry-run`, `--no-verify`, `--allow-dirty`, `--index`, `--registry`, `-p/--package`, `--workspace` — matching `cargo publish`.

**`no3 install [CRATE[@<VER>]]... [OPTIONS]`**: Builds and installs a package's binary to `~/.no3/bin` (or `--root <DIR>`). Supports `--path`, `--git`, `--version`, `--registry`, `--index`, `-f/--force`, `-n/--dry-run`, `--list`, `--debug`, `--bin`, `--bins` — matching `cargo install`.

**`no3 uninstall [SPEC]... [OPTIONS]`**: Removes installed binaries from `~/.no3/bin` (or `--root <DIR>`). Supports `-p/--package` and `--bin <NAME>` — matching `cargo uninstall`.

**`no3 lex <file>` / `no3 parse <file>`**: Debug modes that lex or parse a file and print the result.

## Package Manifest

The `no3.toml` manifest uses the same TOML format as `Cargo.toml`:

```toml
[package]
name = "my-package"
version = "0.1.0"
edition = "2026"
description = "A Nitrate package"
authors = ["Developer Name"]
license = "MIT"

[dependencies]
some-lib = "0.1.0"
local-dep = { path = "../local-dep" }
git-dep = { git = "https://example.com/git-dep.git", branch = "main" }

[dev-dependencies]
test-helper = "1.0"

[build-dependencies]
codegen = "0.2"

[features]
default = ["std"]
std = []
```

Dependencies use cargo-style version requirements: `^0.1.0` (compatible with 0.1.x), `~0.1.0` (compatible with 0.1.0 only), `>=0.1.0, <0.2.0` (version range), `*` (any version), plus table forms for `path`, `git`, `registry`, `features`, and `optional`.

## Lock File

The `no3.lock` lockfile mirrors `Cargo.lock`:

```toml
version = 4

[[package]]
name = "some-lib"
version = "0.1.3"
source = "https://registry.nitrate.dev"
checksum = "..."
```

`no3 update` rewrites the lockfile to the latest matching versions. The global `--locked`/`--offline`/`--frozen` flags behave exactly like cargo's.

## Error Code Explanation

The `explain_code.rs` module provides detailed explanations for error codes, used by `no3 explain <ERROR_CODE>`.

## Build Output

Build artifacts are placed in `target/debug/` or `target/release/` directories (cargo's `target/` layout), containing the executable, object file, and optionally LLVM IR file. The documentation lives in `target/<profile>/doc/`.

## Design Rationale

A single binary with subcommands combines compiler and package manager functionality for simplicity (one tool to install and use), consistency (same options across commands), and tight integration between compilation and package resolution. The TOML manifest format was chosen to be drop-in compatible with Rust's `cargo`, so Nitrate packages feel familiar to cargo users and tooling can share conventions. Flags match cargo's CLI exactly to minimize learning curve and maximize drop-in compatibility.
