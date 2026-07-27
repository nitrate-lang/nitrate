# Package Management Subsystem

## Overview

The Nitrate package manager handles dependency resolution, package publishing, and package registry interactions. It is integrated into the `no3` binary and manages packages according to the `no3.xml` manifest format. The package manager ensures that dependencies are resolved correctly, version constraints are satisfied, and the compilation pipeline receives a complete set of source files.

## Architecture

**Crate**: `nitrate_driver`  
**Key files**: `package.rs`

## Package Manifest (no3.xml)

The package manifest is an XML file defining metadata and dependencies:

```xml
<package>
  <name>my-package</name>
  <version>0.1.0</version>
  <description>A Nitrate package</description>
  <authors><author>Developer Name</author></authors>
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

A standard Nitrate package: `my-package/no3.xml`, `src/entry.nit`, `tests/`, `.gitignore`, `README.md`.

## Dependency Resolution

The resolver parses `no3.xml`, extracts dependency specifications, resolves each name to a source (registry, git URL, local path), downloads or links the package, builds the dependency graph, checks for version conflicts, and stores resolved versions in the build cache.

## Version Specification

Dependencies use semantic versioning: `^0.1.0` (compatible with 0.1.x), `~0.1.0` (compatible with 0.1.0 only), `>=0.1.0, <0.2.0` (version range), `*` (any version), `git:url` (git repository), `path:./local-dep` (local path).
