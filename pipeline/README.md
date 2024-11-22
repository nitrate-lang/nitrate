# Nitrate Compiler Pipeline

## Overview

This directory contains the Nitrate compiler pipeline. The pipeline is a series of stages, implemented as libraries, that facilitate the translation of source code into machine code.

The Nitrate Compiler has a reasonably standard architecture, with a complex preprocessor that is essentially a LUA interpreter, followed by an optimizer, and the LLVM backend. One custom intermediate representation is used in this compiler's middle-end to facilitate the correct and comprehendible translation of the high-level language into native binary code. The compiler is written entirely in modern C++20 and is designed to be fast, efficient, and easy to maintain.

The following diagram illustrates how information flows through the pipeline:

![Nitrate](https://github.com/user-attachments/assets/f814a347-fb0a-485c-bb7a-8d8a7706ee22)

## Stages

The compilation pipeline consists of the following stages:

| Stage     | Description                                                                |
| --------- | -------------------------------------------------------------------------- |
| Lexer     | Tokenizes the input source code                                            |
| Sequencer | Turing-complete preprocessing                                              |
| Parser    | Parses the token stream into an Abstract Syntax Tree (AST)                 |
| IR        | Converts the AST into a language-specific Intermediate Representation (IR) |
| Optimizer | Optimizes the IR                                                           |
| LLVM      | Converts the IR into LLVM Intermediate Representation (LLVM IR)            |
| Codegen   | Generates binary artifacts from the LLVM IR                                |

## Pipeline Dependency Graph

| Library Name        | Pipeline Dependency List                                                                                           |
| ------------------- | ------------------------------------------------------------------------------------------------------------------ |
| `libnitrate-core`   | []                                                                                                                 |
| `libnitrate-lexer`  | [ `libnitrate-core` ]                                                                                              |
| `libnitrate-seq`    | [`libnitrate-lexer`, `libnitrate-core`]                                                                            |
| `libnitrate-parser` | [`libnitrate-lexer`, `libnitrate-core`]                                                                            |
| `libnitrate-ir`     | [`libnitrate-parser`,  `libnitrate-core`]                                                                          |
| `libnitrate-emit`   | [`libnitrate-ir`,  `libnitrate-core`]                                                                              |
| `libnitrate`        | [`libnitrate-emit`, `libnitrate-ir`, `libnitrate-parser`, `libnitrate-seq`, `libnitrate-lexer`, `libnitrate-core`] |
