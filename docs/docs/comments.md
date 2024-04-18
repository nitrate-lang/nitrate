---
layout: page
title: Comments
permalink: /docs/comments/
parent: Documentation
nav_order: 4
---

Comments
---

QUIX allows you to add comments to your code. Comments are ignored by the compiler and may be used to provide additional information about the code. Comments may contain UTF-8 Unicode characters in them. There are two classes of comments:
- Single-line comments
- Multi-line comments

### Single-line comments
Single-line comments are created by any of the following methods:
- Using the `// ` prefix. Use for arbitrary text
- Using the `# ` prefix. Use for arbitrary text
- Using the `/// ` prefix. Use for documentation
- Using the `~> ` prefix. For documentation

### Multi-line comments
Multi-line comments are created by enclosing the comment in `/*` and `*/` tags. 

### Example
```rs
// This is a single-line comment
/// @brief This is some documentation
/// @param x The first parameter
/// @return The return value
fn foo(x: Int): i32 { return x; }   ~> This is a documentation comment

# This is a single-line comment

/* This is a multi-line comment
   It can span multiple lines */

// There are lots of ways to comment your code. Pick the one that works best for you.
// If you're use a lot of programming languages, most of the comment styles will `just work`.
```