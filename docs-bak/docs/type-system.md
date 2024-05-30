---
layout: page
title: Type System
permalink: /docs/type-system/
parent: Documentation
nav_order: 5
---

Type System
---

QUIX is a statically-typed language. QUIX requires EVERYTHING to have an explicit type (no type inference). QUIX has a rich type system that includes the following types:

- [Primitives](#primitive-types)
- [Composites](#composite-types)
- [Functions](#function-types)
- [Enums](#enum-types)
- [Arrays](#array-types)
- [Optional](#optional-types)

### Primitive Types
Primitive types are the basic building blocks of QUIX. They are the simplest types and cannot be broken down further. Some of the primitive types in QUIX include:
- 8, 16, 32, 64, 128, 256-bit signed and unsigned integers
- 32, 64, 128, 256-bit floating point numbers
- `bool` (true or false)
- `char` (UTF-8 Unicode character)
- `real` (an arbitrary precision floating point number)
- `unsafe_ptr` (a pointer to a memory location)
- `unsafe_exec_ptr` (a pointer to a function)

For a full list of primitive types, see the [Lexical Structure](/docs/lexical-structure/#primitive-types) documentation.

### Composite Types
Composite types are types that are composed of other types. Composite types are distinguished by the following properties:
- Name
- An ordered set of member types
- Packing / alignment rules

All composite types are user-defined and can be created using the `struct` keyword. For example:
```c
struct Point {
    x: i32;
    y: i32;
};
// Type: struct(Point)

region FileHeader {
    magic: u32;
    version: u32;
    flags: u32;
}; // `region` packs the struct members
// Type: region(FileHeader)
```

### Union Types
Union types are types that can hold any one of a set of types. Union types are distinguished by the following properties:
- Name
- An unordered set of member types
- Packing / alignment rules

All union types are user-defined and can be created using the `union` keyword. For example:
```c
union IntOrFloat {
    i: i32;
    f: f32;
};
// Type: union(IntOrFloat)

packet PackedUnion {
    magic: u32;
    data: union(IntOrFloat);
}; // `packet` packs the union members
// Type: packet(PackedUnion)
```
