# Package Management Subsystem

## Overview

The Nitrate package manager handles dependency resolution, package publishing, and package registry interactions. It is integrated into the `no3` binary and manages packages according to the `no3.xml` manifest format.

## Architecture

**Crate**: `nitrate_driver`  
**Key files**: `package.rs`

## Package Manifest (`no3.xml`)

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
      <version>^0.1.0</version>
    </dependency>
  </dependencies>
</package>
```

## Package Structure

A standard Nitrate package:

```
my-package/
├── no3.xml
├── src/
│   └── entry.nit
├── tests/
├── .gitignore
└── README.md
```

## Commands

### `no3 new <name>`

Creates a new package with default structure.

### `no3 init`

Initializes the current directory as a package.

### `no3 add <package>`

Resolves and adds a dependency.

### `no3 remove <package>`

Removes a dependency.

### `no3 install <package>`

Installs a package globally.

### `no3 uninstall <package>`

Uninstalls a globally installed package.

### `no3 publish`

Publishes the current package to the registry.

### `no3 search <query>`

Searches the package registry.

### `no3 update`

Updates dependencies to latest compatible versions.

## Dependency Resolution

1. Parse `no3.xml` to extract dependency specifications
2. Resolve each dependency name to a source (registry, git URL, local path)
3. Download or link the dependency
4. Build the dependency graph
5. Check for version conflicts
6. Store resolved versions in the build cache

## Version Specification

Dependencies use semantic versioning:

- `^0.1.0`: Compatible with 0.1.x
- `~0.1.0`: Compatible with 0.1.0 only
- `>=0.1.0, <0.2.0`: Version range
- `*`: Any version
- `git:url`: Git repository
- `path:./local-dep`: Local path dependency
