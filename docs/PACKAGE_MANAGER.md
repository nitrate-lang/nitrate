# Package Management Subsystem

## Overview

The Nitrate package manager handles dependency resolution, package publishing, and package registry interactions. It is integrated into the `no3` binary and manages packages according to the `no3.toml` manifest format — which is deliberately drop-in compatible with Rust's `Cargo.toml`. The package manager ensures that dependencies are resolved correctly, version constraints are satisfied, and the compilation pipeline receives a complete set of source files.

## Architecture

**Crate**: `nitrate_driver`  
**Key files**: `package.rs`, `commands/add.rs`, `commands/remove.rs`, `commands/update.rs`, `commands/search.rs`, `commands/publish.rs`, `commands/install.rs`, `commands/uninstall.rs`

## Package Manifest (no3.toml)

The package manifest is a TOML file using the same structure as `Cargo.toml`:

```toml
[package]
name = "my-package"
version = "0.1.0"
edition = "2026"
description = "A Nitrate package"
authors = ["Developer Name <dev@example.com>"]
license = "MIT"

[dependencies]
some-lib = "0.1.0"
local-dep = { path = "../local-dep" }
git-dep = { git = "https://example.com/git-dep.git", branch = "main" }
renamed = { package = "real-name", version = "2.0" }

[dev-dependencies]
test-helper = "1.0"

[build-dependencies]
codegen = "0.2"

[features]
default = ["std"]
std = []

[workspace]
members = ["members/*"]
```

### Package Section Fields

- `name` — package name (alphanumeric, `-`, `_`; may not start/end with `-`)
- `version` — semantic version (e.g. `0.1.0`)
- `edition` — language edition year (e.g. `2026`)
- `description`, `authors`, `license`, `readme`, `repository`, `homepage`, `documentation`, `keywords`, `categories`, `publish` — informational metadata

## Package Structure

A standard Nitrate package: `my-package/no3.toml`, `src/entry.nit`, `tests/`, `.gitignore`, `README.md`. Build artifacts go in `target/` (cargo-compatible layout).

## Dependency Format

Dependencies support both the simple string form and the full table form:

| Form                                                                       | Meaning                                 |
| -------------------------------------------------------------------------- | --------------------------------------- |
| `foo = "0.1.0"`                                                            | Registry dep with a version requirement |
| `foo = { version = "0.1.0" }`                                              | Registry dep, explicit table            |
| `foo = { path = "../foo" }`                                                | Local path dep                          |
| `foo = { git = "https://...", branch = "main" }`                           | Git dep                                 |
| `foo = { version = "1.0", optional = true }`                               | Optional feature-gated dep              |
| `renamed = { package = "real", version = "1" }`                            | Renamed dep                             |
| `foo = { version = "1.0", features = ["json"], default-features = false }` | Feature selection                       |

## Version Specification

Dependencies use cargo-compatible semantic versioning requirements: `^0.1.0` (compatible with 0.1.x), `~0.1.0` (compatible with 0.1.0 only), `>=0.1.0, <0.2.0` (version range), `*` (any version), and exact pins `=1.2.3`.

## Dependency Resolution

The resolver parses `no3.toml`, extracts dependency specifications, resolves each name to a source (registry, git URL, local path), downloads or links the package, builds the dependency graph, checks for version conflicts, and stores resolved versions in `no3.lock`.

## Lock File (no3.lock)

The lockfile mirrors `Cargo.lock` — version 4 format with `[[package]]` tables:

```toml
version = 4

[[package]]
name = "some-lib"
version = "0.1.3"
source = "https://registry.nitrate.dev"
checksum = "..."

[[package]]
name = "local-dep"
version = "0.1.0"
source = "path+../local-dep"
```

`no3 update [SPEC]...` rewrites the lockfile to the latest versions matching the manifest; `--precise <VERSION>` pins a specific version; `--dry-run` previews without writing.

## Commands

| Command                   | Description                                                                           |
| ------------------------- | ------------------------------------------------------------------------------------- |
| `no3 add <dep>@<version>` | Add a dependency to `[dependencies]`, `[dev-dependencies]`, or `[build-dependencies]` |
| `no3 remove <dep>`        | Remove a dependency from the manifest                                                 |
| `no3 update [spec]`       | Regenerate `no3.lock` to latest matching versions                                     |
| `no3 search <query>`      | Search the registry for packages                                                      |
| `no3 publish`             | Package `target/package/<name>-<version>.crate` and upload to the registry            |
| `no3 install <crate>`     | Build and install a binary to `~/.no3/bin`                                            |
| `no3 uninstall <name>`    | Remove an installed binary                                                            |

## Registry

The default registry is `https://registry.nitrate.dev` (overridable via the `NO3_REGISTRY` environment variable or the `--registry`/`--index` flags). The registry API mirrors the crates.io API shape: `/api/v1/crates/{name}` for metadata, `/api/v1/crates/{name}/versions` for version listings, `/api/v1/crates/new` for publishing, and `.crate` tarball downloads for installation.

## Design Rationale

The TOML manifest and cargo-compatible CLI were chosen to make `no3` a drop-in replacement for `cargo` in Nitrate projects. Sharing cargo's manifest conventions means existing tooling, editor plugins, and mental models carry over, and the lockfile guarantees reproducible builds.
