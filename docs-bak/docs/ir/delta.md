---
layout: page
title: DeltaIR (Decoupled IR)
permalink: /docs/ir/delta/
parent: Documentation
has_children: false
---

# Delta IR (Decoupled IR)

Revision: 0.1-dev

This document is a work in progress and is pretty much a scratchpad for ideas.

## Introduction

QUIX Delta IR is an LLVM IR-like intermediate representation (IR) designed to decouple
the QUIX compiler from complete LLVM backend dependence. QUIX Delta IR (DIR hereafter)
describes a program's control and data flow; it is designed to be compiler-backend
agnostic and suitable for source-to-source compilation to systems programming
languages like C and Haskell.

## Constructs and Features

Because DIR is a low-level IR, it is simple and minimalistic. Herein, we overview
DIR's constructs and features and see how they are defined syntactically.

### Segments (Functions)

In DIR, sequences of statements are organized into discrete units called segments.
A segment differs from a function in that it lacks the built-in concept of a
stack frame. Segments may have mutable arguments and do not have a return value.
Imagine a segment as an operation that you can call that mutates the program's
state. It does not need to transfer control or occupy physical memory, but it
may.

The following properties classify segments:

- `stateful`: A stateful segment (non-pure) is any segment that reads and/or writes
 to a global variable and/or directly manipulates memory (does not include
 its arguments) and/or calls a `stateful` segment. Any segment not meeting this
 criteria is considered stateless (pure).
- `pure`: A pure segment is a segment that does not read or write to a global
 variable, does not directly manipulate memory (does not include its arguments),
 and does not call a `stateful` segment.
- `internal`: An internal segment is not externally visible. It is assumed to
 be only callable by other segments within the same module. Calling an internal
 segment from another module is an undefined behavior.
- `external`: An external segment is an externally visible segment. It is assumed
 that an external segment is callable by any code.

The following properties distinguish segments:

- `name`: The name of the segment.

### Primitives

A primitive is a simple data type not composed of other data types.
Primitives are the building blocks of all other data types.

| Primitive Type | Size (bytes) | Description           |
|----------------|--------------|-----------------------|
| `i1`           | 1            | 1-bit int             |
| `i8`           | 1            | 8-bit signed int      |
| `i16`          | 2            | 16-bit signed int     |
| `i32`          | 4            | 32-bit signed int     |
| `i64`          | 8            | 64-bit signed int     |
| `i128`         | 16           | 128-bit signed int    |
| `i256`         | 32           | 256-bit signed int    |
| `u8`           | 1            | 8-bit unsigned int    |
| `u16`          | 2            | 16-bit unsigned int   |
| `u32`          | 4            | 32-bit unsigned int   |
| `u64`          | 8            | 64-bit unsigned int   |
| `u128`         | 16           | 128-bit unsigned int  |
| `u256`         | 32           | 256-bit unsigned int  |
| `f32`          | 4            | 32-bit floating point |
| `f64`          | 8            | 64-bit floating point |

The following properties classify primitives:

- `size`: The size of the primitive in bytes.
- `signed`: A signed primitive can represent positive and negative numbers.
- `unsigned`: An unsigned primitive can only represent positive numbers.
- `floating`: A floating primitive can represent fractional numbers.

### Pointers

A pointer is, well, a pointer (like in C). It is a mutable reference to a (possibly)
mutable value. Pointers may be null. Any pointer may be dereferenced, but the behavior may be undefined based
on the particular context in which this is done.

### Packets (Packed Structures)

A packet is an ordered, finite collection of fields. Each field has a name and
a type. Packets have no padding between fields, nor do they have any alignment
requirements. Packets are used to represent data structures in memory. They lack
methods and are not self-referential. Data fields are always public and mutable.
Bitfields are supported. All fields in a packet are addressable and dereferenceable
with well-defined behavior, including bitfields. Fields can not have a default value.
All initializations must be explicit.

The following properties classify packets:

- `size`: The size of the packet in bytes.
- `fields`: The fields in the packet.

The following properties distinguish packets:

- `name`: The name of the packet.

### Variables

A variable is a named storage location that holds a value.
Variables may be global or local with respect to a segment. Variables
may or may not occupy a physical memory location. Variables may be of any type
supported by DIR (e.g., primitive types, packets, pointers).

The following properties classify variables:

- `type`: The type of the variable.
- `volatile`: A volatile variable may be mutated in an unforeseen
 , unpredictable/undefined manner. The compiler shalt not optimize volatile
 variables.

The following properties distinguish variables:

- `name`: The name of the variable.
- `scope`: The variable's scope (global or local).

### Constants

Constants are not supported in DIR. 

### Load and Store

A load operation reads a value from a memory location. A store operation writes
a value to a memory location.

The following properties classify load and store operations:

- `type`: The type of the value being loaded or stored.
- `volatile`: A volatile load or store operation may be mutated in an
 unforeseen or unpredictable/undefined manner. The compiler shalt not optimize
 volatile load or store operations.
- `atomic`: An atomic load or store operation is guaranteed to be
 atomic with respect to other atomic load or store operations on the exact
 memory location.

### Arithmetic Operations

Arithmetic operations are operations that perform arithmetic on values. Arithmetic
operations may be unary or binary. All arithmetic operations are `pure`.
Arithmetic is defined over the primitive types. Floating point and integer arithmetic
are supported and use the same instruction set.

#### Binary Arithmetic Operations

| Operator | Operand A Type | Operand B Type | Result Type   | Description                                                             |
|----------|----------------|----------------|---------------|-------------------------------------------------------------------------|
| `+`      | `i*`           | `i*`           | maxsize(`i*`) | Addition of two integers. Over/underflow is undefined.                  |
| `-`      | `i*`           | `i*`           | maxsize(`i*`) | Subtraction of two integers. Over/underflow is undefined.               |
| `*`      | `i*`           | `i*`           | maxsize(`i*`) | Multiplication of two integers. Over/underflow is undefined.            |
| `/`      | `i*`           | `i*`           | maxsize(`i*`) | Division of two integers. Division by zero is undefined.                |
| `%`      | `i*`           | `i*`           | maxsize(`i*`) | Modulus of two integers. Modulus by zero is undefined.                  |
| `+`      | `u*`           | `u*`           | maxsize(`u*`) | Addition of two unsigned integers. Over/underflow is defined.           |
| `-`      | `u*`           | `u*`           | maxsize(`u*`) | Subtraction of two unsigned integers. Over/underflow is defined.        |
| `*`      | `u*`           | `u*`           | maxsize(`u*`) | Multiplication of two unsigned integers. Over/underflow is defined.     |
| `/`      | `u*`           | `u*`           | maxsize(`u*`) | Division of two unsigned integers. Division by zero is undefined.       |
| `%`      | `u*`           | `u*`           | maxsize(`u*`) | Modulus of two unsigned integers. Modulus by zero is undefined.         |
| `+`      | `f*`           | `f*`           | maxsize(`f*`) | Addition of two floating point numbers.                                 |
| `-`      | `f*`           | `f*`           | maxsize(`f*`) | Subtraction of two floating point numbers.                              |
| `*`      | `f*`           | `f*`           | maxsize(`f*`) | Multiplication of two floating point numbers.                           |
| `/`      | `f*`           | `f*`           | maxsize(`f*`) | Division of two floating point numbers. Division by zero is undefined.  |

#### Unary Arithmetic Operations

| Operator | Operand Type | Result Type | Description                                            |
|----------|--------------|-------------|--------------------------------------------------------|
| `+`      | `i*`         | `i*`        | Identity operation.                                    |
| `-`      | `i*`         | `i*`        | Negation operation.                                    |
| `+`      | `u*`         | `u*`        | Identity operation.                                    |
| `-`      | `u*`         | `u*`        | Negation operation.                                    |
| `+`      | `f*`         | `f*`        | Identity operation.                                    |
| `-`      | `f*`         | `f*`        | Negation operation.                                    |

### Bitwise Operations

Bitwise operations are operations that perform bitwise operations on values. Bitwise
operations may be unary or binary. All bitwise operations are `pure`.
Bitwise operations are defined over the primitive types.

#### Binary Bitwise Operations

| Operator | Operand A Type | Operand B Type | Result Type       | Description                                            |
|----------|----------------|----------------|-------------------|--------------------------------------------------------|
| `&`      | `i*`           | `i*`           | maxsize(`i*`)     | Bitwise AND of two integers.                           |
| `\|`     | `i*`           | `i*`           | maxsize(`i*`)     | Bitwise OR of two integers.                            |
| `^`      | `i*`           | `i*`           | maxsize(`i*`)     | Bitwise XOR of two integers.                           |
| `&`      | `u*`           | `u*`           | maxsize(`u*`)     | Bitwise AND of two unsigned integers.                  |
| `\|`     | `u*`           | `u*`           | maxsize(`u*`)     | Bitwise OR of two unsigned integers.                   |
| `^`      | `u*`           | `u*`           | maxsize(`u*`)     | Bitwise XOR of two unsigned integers.                  |

#### Unary Bitwise Operations

| Operator | Operand Type | Result Type | Description                                            |
|----------|--------------|-------------|--------------------------------------------------------|
| `~`      | `i*`         | `i*`        | Bitwise NOT of an integer.                             |
| `~`      | `u*`         | `u*`        | Bitwise NOT of an unsigned integer.                    |

### Logical Operations

Logical operations are operations that perform logical operations on values. Logical
operations may be unary or binary. All logical operations are `pure`.
Logical operations are defined over the primitive types.

#### Binary Logical Operations

| Operator | Operand A Type | Operand B Type | Result Type | Description                                            |
|----------|----------------|----------------|-------------|--------------------------------------------------------|
| `&&`     | `i*`           | `i*`           | `i1`        | Logical AND of two integers.                           |
| `\|\|`   | `i*`           | `i*`           | `i1`        | Logical OR of two integers.                            |
| `&&`     | `u*`           | `u*`           | `i1`        | Logical AND of two unsigned integers.                  |
| `\|\|`   | `u*`           | `u*`           | `i1`        | Logical OR of two unsigned integers.                   |

#### Unary Logical Operations

| Operator | Operand Type | Result Type | Description                                            |
|----------|--------------|-------------|--------------------------------------------------------|
| `!`      | `i*`         | `i1`        | Logical NOT of an integer.                             |
| `!`      | `u*`         | `i1`        | Logical NOT of an unsigned integer.                    |
| `!`      | `f*`         | `i1`        | Logical NOT of a floating point number.                |

### Pointer Operations

Pointer operations are operations that perform pointer operations on values. Pointer
operations may be unary or binary. All pointer operations are `impure`.
Pointer operations are defined over the pointer types.
Pointer operations do not support arithmetic directly. Instead, cast the pointer
to an integer, perform the arithmetic, and cast it back to a pointer.

#### Unary Pointer Operations

| Operator | Operand Type | Result Type | Description                                            |
|----------|--------------|-------------|--------------------------------------------------------|
| `*`      | `*T`         | `T`         | Dereference a pointer.                                 |
| `&`      | `T`          | `*T`        | Reference a value.                                     |

### Control Flow

Control flow operations are operations that change execution flow. Control flow
operations may be conditional or unconditional. Control flow operations are `pure` if
and only if the target segment is `pure`.

| Operation | Description                                            |
|-----------|--------------------------------------------------------|
| `if-else` | Conditional execution with mandatory `else` branch.    |
| `while`   | Loop with conditional execution.                       |
| `jmp`     | Unconditional jump to a label.                         |
| `label:`  | A named label for control flow jumps.                  |
| `ret`     | Return from a segment. Does not return a value.        |
| `call`    | Call a segment.                                        |
| `ptrcall` | Call a function pointer.                               |
| `halt`    | Halt execution. (Implementation defined)               |

### Casting

Casting operations are operations that convert values from one type to another.
All casting operations are `pure`. All casts are explicit.

| Cast Type | Description                                            |
|-----------|--------------------------------------------------------|
| `scast`   | Signed integer cast up or down.                        |
| `ucast`   | Unsigned integer cast up or down.                      |
| `ptricast`| Pointer to integer cast.                               |
| `iptrcast`| Integer to pointer cast.                               |
| `bitcast` | Bitcase cast.                                          |


### Memory Operations

Allocations are implicit in DIR. The compiler is responsible for allocating
stack memory for local variables. Heap allocations require an external memory
allocator. These are out-of-scope for DIR.

## Example

A program that calculates the 10th Fibonacci number:

```ir
@_fib10([]->i32) = segment internal pure (i32) () {
    %a(i32) = scast i32, 0;
    %b(i32) = scast i32, 1;
    %i(i32) = scast i32, 0;
    %tmp(i32) = scast i32, 0;
    
    while %i(i32) < scast i32, 10 {
        %tmp(i32) = %a(i32) + %b(i32);
        %a(i32) = %b(i32);
        %b(i32) = %tmp(i32);
        %i(i32) = %i(i32) + scast i32, 1;
    }
    ret %a(i32);
}

@main([]->i32) = segment external pure (i32) () {
    %result(i32) = call @_fib10 ();
    ret %result(i32);
}
```