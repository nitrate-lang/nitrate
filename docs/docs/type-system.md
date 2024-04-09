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
- [Unions](#union-types)
- [Functions](#function-types)
- [Enums](#enum-types)
- [Arrays](#array-types)
- [Optional](#optional-types)

### Primitive Types
Primitive types are the basic building blocks of J. They are the simplest types and cannot be broken down further. Some of the primitive types in QUIX include:
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
### Function Types
Function types are types that represent a function. Function types are distinguished by the following properties:
- Return type
- An ordered set of parameter types
- Whether the function may return
- If the function is fallible (it could fail)

Function types are used to represent the type of a function. For example:
```c
// This function is infallible and returns an i32
fn add(a: i32, b: i32): i32 {
    return a + b;
}
// Type: fn(i32,i32):i32

// This function is fallible
fn divide(a: i32, b: i32): f32? {
    abortif b == 0;

    return a / b;
}
// Type: fn_fallible(i32,i32):f32?
```

### Enum Types
Enum types are types that represent a set of named constants. Enum types are distinguished by the following properties:
- Name

Example:
```c
enum Color: i32 {
    Red = 0;
    Green = 1;
    Blue = 2;
};

// Type: enum(Color)
```

### Array Types
Array types are types that represent a sequence of elements. Array types are distinguished by the following properties:
- Element type
- Length

Example:
```c
// This is an array of 10 i32s
// Type: [i32;10]
let arr: [i32;10] = [1,2,3,4,5,6,7,8,9,10];

```

### Optional Types
Optional types are types that represent a value that may or may not be present. Optional types are distinguished by the following properties:
- Inner type

Example:
```rs
fn hello(): void {
    // Type: ?i32
    let fd: ?i32 = std::net::socket("IPv4", "TCP"); // function is fallible
    abortif fd == null, println!("Failed to create socket");

    // Now fd can be cast to a non-optional i32 because we know it's not null
    fd = fd!;
}
```

## QUIX Default Serialization V1
QUIX has a standard type serialization format that can be used to serialize types to a string and deserialize types from a string. The serialization format is as follows:
1. Get human-readable type name
1. Reduce type name by removing all whitespace
1. Base58 encode the above string with the following alphabet: `123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz`
1. Prepend _ZJC0 to the encoded string

For example, the type `fn(i32,i32):i32` would be serialized to `_ZJC03sDA64nmSQXj9kvkqwFYd`.

## Deserialization
To deserialize a type from a string, the following steps are taken:
1. Verify that the string starts with `_ZJC0`
1. Remove the `_ZJC0` prefix
1. Base58 decode the string using the following alphabet: `123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz`
1. Done    