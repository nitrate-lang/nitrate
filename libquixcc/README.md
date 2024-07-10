# libquixcc - The Quix Compiler Toolchain

## Overview

The Quix Compiler Toolchain is primarily encapsulated in the libquixcc library. This library exposes a C API that enables compilation, inspection, Analysis, serialization, source-to-source trans-compilation, and programmatic interaction with Quix programs. The libquixcc library is a shared library that can be linked against most programming languages that support C bindings.

## Design Highlights

- **Performance**: The Quix Compiler Toolchain is designed to be fast and memory efficient (with a strong emphasis on the former). The Quix Compiler Toolchain is designed for real-time applications and systems where performance is critical.
- **Correctness and Security**: The Quix Compiler Toolchain is designed to be correct and secure. The Quix Project takes security very seriously and has designed the Quix Compiler Toolchain with security in mind.

## Features and Capabilities

- **Source Compilation**: Libquixcc can compile Quix source code into any of the following:
  - Quix AST (QAST)
  - Quix BitTree (QBTree)
  - Quix Intermediate Representation (Quix IR)
  - Delta Intermediate Representation (Delta IR)
  - LLVM Intermediate Representation (LLVM IR)
  - LLVM Object Code (from LLVM IR)
  - Native Executable (from LLVM Object Code and LLVM Linker)
  - Other programming languages
- **Source Semantics Analysis**: Libquixcc can analyze the semantics of Quix source code and provide detailed information about the program's structure, behavior, and correctness. Semantics include but are not limited to:
  - Type Checking
  - Memory Safety Analysis
  - Naming Issues
  - Control Flow Analysis
  - Function Call Checking
- **Serialization and Deserialization**: Libquixcc can serialize and deserialize Quix ASTs, BitTrees, Quix IRs, and Delta IRs. Serialization enables the storage and retrieval of Quix programs in various formats.

## Internal Architecture

Libquixcc is composed of four main subsystems:

1. **Quix AST (QAST)**: The Quix Abstract Syntax Tree (QAST) is the data structure that represents Quix source code in memory. The QAST is a Directed Acyclic Graph (DAG) that represents the structure of a Quix program. The QAST may be serialized and deserialized to and from disk in QAST and Quix BitTree (QBTree) (an efficient binary format).
2. **Compiler**: The subsystem compiles Quix source code into various targets. The Compiler subsystem is composed of several subcomponents:
   - **Scanner**: The Scanner is responsible for tokenizing Quix source code. It does not handle preprocessing.
   - **Preprocessor**: The Preprocessor wraps the Scanner and preprocess Quix source code. The Preprocessor expands macros, includes files, and performs other preprocessing tasks.
    - **Parser**: The Parser parses Quix source code into a Quix AST (QAST).
    - **Quix IR Lowering**: The Quix IR Lowering component lowers the QAST into Quix Intermediate Representation (Quix IR).
    - **Semantic Analysis**: The Semantic Analysis component analyzes the semantics of Quix source code. Semantics include type checking, memory safety analysis, naming issues, control flow analysis, and function call checking. Semantic Analysis is performed on the Quix IR.
    - **Delta IR Lowering**: The Delta IR Lowering component is responsible for lowering the Quix IR into Delta Intermediate Representation (Delta IR).
    - **LLVM IR Generation**: The LLVM IR Generation component generates LLVM Intermediate Representation (LLVM IR) from the Delta IR.
    - **LLVM Object Code Generation**: The LLVM Object Code Generation component generates LLVM Object Code from LLVM IR. The LLVM backend provides code generation.
    - **Native Executable Generation**: The Native Executable Generation component generates a native executable from LLVM Object Code. The LLVM linker provides this.

## Getting Started

To get started with libquixcc, you need to:

1. **Build or Install libquixcc**: You can search for pre-built binaries at the [Quix Project website](https://qpkg.io/releases) or build libquixcc from source. This section will guide you through building libquixcc from source.
2. **Link Against libquixcc**: Once you have libquixcc installed, you can link against it in your project. This section will not detail how to link against libquixcc in your project. You will need to consult your project's build system documentation for that information.
3. **Use libquixcc**: Once you have linked against libquixcc, you can use it in your project. See the [API Documentation](https://docs.qpkg.io/api) for more information on how to use libquixcc.

### Building libquixcc

Building libquixcc from source is trivial. Docker is required to build libquixcc. To build libquixcc, follow these steps:

1. Install Docker on your system. See the [Docker website](https://docs.docker.com/get-docker/) for installation instructions.
2. Install Git on your system. See the [Git website](https://git-scm.com/downloads) for installation instructions.
3. Clone the Quix Compiler Toolchain repository into a directory on your system:

```bash
git clone --recurse-submodules https://github.com/Kracken256/quix.git
```

4. Change into the `quix` directory:

```bash
cd quix
```

5. Run the `make.py` script to build the entire Quix Project:

```bash
python3 ./make.py --release --strip --upx-best
```

6. Done! The `libquixcc.so` shared library and other Quix Project binaries will be located in the `./bin` directory inside the cloned repository.

7. **Please Note**: All binaries and libraries are only compatible with Linux systems. If you are on a different operating system, Dockerized builds are the only option. See the [QPKG Downloads](https://qpkg.io/releases) page for pre-built binaries.

## API Documentation

The libquixcc API documentation is available at [https://docs.qpkg.io/api](https://docs.qpkg.io/api). The API documentation provides detailed information on how to use libquixcc in your project.
