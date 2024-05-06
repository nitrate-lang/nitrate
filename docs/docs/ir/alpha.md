---
layout: page
title: AlphaIR (Architectural IR)
permalink: /docs/ir/alpha/
parent: Documentation
has_children: false
---

# Alpha IR (Architectural IR)

Revision: 0.1-dev

This document is a work in progress and is pretty much a scratchpad for ideas.

templates must be expanding prior to translation

## Introduction

## Statements

- `region` - packed structure of fields
- `group` - defines a new unordered group of statements
- `enum` - defines a new enumeration type
- `asm` - inline assembly (has no inherit return value)
- `contract` - defines a new contract (includes other not listed)
- `view` - visibility controller (export symbol and import symbol (like extern))

- `if-else` - conditional statement
- `while` - loop statement
- `for` - loop statement
- `switch` - switch statement
- `ret` - return statement
- `break` - break statement
- `continue` - continue statement
- `try-catch` - exception handling statement
- `throw` - throw statement
- `

## Expressions

The usual unary and binary operators are supported. Not going to list them again.

## Types

