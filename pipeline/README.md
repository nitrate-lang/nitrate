# Nitrate Compiler Pipeline

## Overview

This directory contains the Nitrate compiler pipeline. The pipeline is a series of stages, implemented as libraries, that facilitate the translation of source code into machine code.

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
| `libnitrate-lexer`  | [`libnitrate-core`]                                                                                                |
| `libnitrate-seq`    | [`libnitrate-lexer`, `libnitrate-core`]                                                                            |
| `libnitrate-parser` | [`libnitrate-lexer`, `libnitrate-core`]                                                                            |
| `libnitrate-ir`     | [`libnitrate-parser`, `libnitrate-lexer`, `libnitrate-core`]                                                       |
| `libnitrate-emit`   | [`libnitrate-ir`, `libnitrate-lexer`, `libnitrate-core`]                                                           |
| `libnitrate`        | [`libnitrate-emit`, `libnitrate-ir`, `libnitrate-parser`, `libnitrate-seq`, `libnitrate-lexer`, `libnitrate-core`] |
