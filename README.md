# Nitrate Project

[![Build and Test](https://github.com/Kracken256/nitrate/actions/workflows/cmake-check.yml/badge.svg)](https://github.com/Kracken256/nitrate/actions/workflows/cmake-check.yml)
[![Publish SNAP](https://github.com/Kracken256/nitrate/actions/workflows/publish-snap.yml/badge.svg)](https://github.com/Kracken256/nitrate/actions/workflows/snapcraft-check.yml)

<div align="center">
    <img src="https://github.com/Kracken256/nitrate/blob/main/static-data/logo-dark-1024x1024.png?raw=true" width="400" height="400" alt="css-in-readme">
</div>

*This project is in rapid development. It will not compile to anything useful yet.*

Nitrate is a high-level, statically typed, memory-safe, garbage-collected\*, high-performance, object-oriented, general-purpose programming language designed to be safe, expressive, and performant. Nitrate supports runtime reflection, low-level memory access, namespaces, generics, type inference, packed data types, classes, operator overloading, coroutines, default initializers, arbitrary precision math, RAII, automatic memory management, inline assembly, and compile time evaluation metaprogramming.
Nitrate is especially well-suited for library development, system service development, and general applications. Nitrate is a modern, safe, and expressive alternative to C++.

Official website: [https://nitrate.dev](https://nitrate.dev)

## Table of Contents

- [Nitrate Project](#nitrate-project)
  - [Table of Contents](#table-of-contents)
  - [Technical Detail](#technical-detail)
    - [General Architecture](#general-architecture)
    - [Preprocessor Engine Architecture](#preprocessor-engine-architecture)
    - [Optimizer Engine Architecture](#optimizer-engine-architecture)
    - [Diagnostic Engine Architecture](#diagnostic-engine-architecture)
  - [Usage](#usage)
  - [Building](#building)
  - [Contributing](#contributing)
  - [Diagnostic Reporting](#diagnostic-reporting)
  - [License](#license)

## Technical Detail

### General Architecture

---

The Nitrate Compiler has a reasonably standard architecture, with a complex preprocessor that is essentially a LUA interpreter, followed by an optimizer, and the LLVM backend. Two custom IRs are used in this compiler's middle-end to facilitate the correct and comprehendible translation of the high-level language into native binary code. The compiler is written entirely in modern C++20 and is designed to be fast, efficient, easy to maintain, and extend.

![Nitrate](https://github.com/user-attachments/assets/f814a347-fb0a-485c-bb7a-8d8a7706ee22)

### Preprocessor Engine Architecture

---

Some programming languages like C++ have strange and complex macro/template metaprogramming systems that can feel more like writing theoretical computer science papers than writing traditional application code. Nitrate has the most potent macro system of any programming language. Nitrate accomplishes this by embedding a complete LUA interpreter into the preprocessor. You can write LUA functions that may take in arguments and return values that will be recursively preprocessed by LUA until there are no more macros to expand. This recursion enables the creation of extraordinarily powerful custom constructs that are *readable*, *understandable*, and *compile-time performant*. The LUA interpreter context exists for the entire duration of the preprocessor, which is the entire compilation unit, meaning the state is preserved between macros. How hard would it be if you wanted to write a macro in C++ that would do a SHA-3 cryptographic hash at compile time? In Nitrate, simply use LUA to implement the standard algorithm as you would in regular languages and then call your macro like `@sha3_256("You data to hash")` which may occur anywhere in your code (as it applies before parsing). Nitrate just made elegant macros a solved problem. Also, macros are optionally type-checked on input and output, which catches many weird issues early.

![Nitrate](https://github.com/user-attachments/assets/754f8c82-bcbe-4a30-98dc-10312979b784)

### Optimizer Engine Architecture

TODO: write this section

### Diagnostic Engine Architecture

TODO: write this section

## Usage

TODO: write this section

## Building

**These notes are pending cross-distribution testing.**

Build on Debian systems:

```bash
sudo apt install -y git
cd ~/Downloads
git clone --recurse-submodules git@github.com:Kracken256/nitrate.git
cd nitrate

# Install dependencies
# libpolly-14-dev is not necessary in all distributions. If it does not exist in your package manager, try building without it.
# If libPolly will be in issue in your build it will manifest as a linker error.

sudo apt install -y clang cmake make llvm-14 upx libssl-dev libboost-all-dev libzstd-dev libclang-common-14-dev rapidjson-dev libdeflate-dev libreadline-dev libclang-dev libclang-cpp-dev nlohmann-json3-dev libpolly-14-dev

# Build the toolchain in release mode
cmake -S . -B .build/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./build -DCMAKE_CXX_COMPILER=/usr/bin/clang++
cmake --build .build/release -j$(nproc)
cmake --install .build/release

```

## Contributing

TODO: write this section

## Diagnostic Reporting

The following is an example of an error message generated by the Nitrate compiler:
In reality, the error message would be colorized.

```plaintext
examples/test.q:3:9-?:?: error: ptree-invalid
 ╔ This function is expected to return in all possible branches. Why is your 
 ╚ function failing to do so?

 ╔═ Code Intelligence:
 ╠═ Make sure you have a return statement when you need one.
 ╠═ If you are using a loop and avoiding a return, ensure that it is knowable 
 ║   that the loop will always terminate.
 ╠═ If you are optimizing, make sure it is knowable that that a particular 
 ║   branch is impossible to avoid this error.

   ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
 3 ┃ pub "c" fn main(args: [string]): i32 {                       ┃
 4 ┃   print(20); // Hello world                                  ┃
 5 ┃ }                                                            ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
```

## License

The Nitrate Project is free software released under the GNU Lesser General Public License 2.1 (LGPL 2.1). See the `LICENSE` file for more information.

