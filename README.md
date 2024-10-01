# Quix Compiler

*Project is in rapid development. It will not compile to anything useful yet.*

QUIX is a high-level, statically typed, memory-safe\*, garbage-collected\*, high-performance, object-oriented, general-purpose systems programming language designed to be safe, expressive, and performant. QUIX supports runtime reflection, low-level memory access, namespaces, generics, type inference, packed data types, classes, operator overloading, coroutines, default initializers, arbitrary precision math, RAII, automatic memory management, and metaprogramming.
QUIX is especially well-suited for library development, game development, and systems programming. QUIX is designed to be a modern, safe, and expressive alternative to C++ and Rust.

## Table of Contents

- [Technical Detail](#technical-detail)
- - [General Architecture](#general-architecture)
- - [Preprocessor Engine Architecture](#preprocessor-engine-architecture)
- - [Optimizer Engine Architecture](#optimizer-engine-architecture)
- - [Diagnostic Engine Architecture](#diagnostic-engine-architecture)
- [Building](#building)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Technical Detail

### General Architecture

---

The QUIX Compiler has a reasonably standard architecture, with a complex preprocessor that is essentially an LUA interpreter, an optimizer, and the LLVM backend. Two custom IRs are used in this compiler's middle end to facilitate the correct and comprehendible translation of the high-level language into native binary code. The compiler is written entirely in "modern" C++ and is designed to be fast, efficient, easy to maintain, and extend.

![QUIX](https://github.com/user-attachments/assets/f814a347-fb0a-485c-bb7a-8d8a7706ee22)

### Preprocessor Engine Architecture

---

Some programming languages like Rust have extremely complex macro systems that can feel more like writing theoretical computer science papers than writing helpful code. QUIX has the most potent macro systems of any programming language. QUIX accomplishes this by just embedding a complete LUA interpreter into the preprocessor. You can write `metastatic` functions that may take in arguments and return values that will be recursively preprocessed until there are no more macros to expand. This recursion enables the creation of extraordinarily powerful custom constructs that are *readable*, *understandable*, and *compiler-time performant*. The LUA interpreter context exists for the entire duration of the preprocessor, which is effectively the entire compilation unit, meaning the state is preserved between macros. How hard would it be if you wanted to write a macro in Rust that would do a SHA-3 cryptographic hash at compile time? In QUIX, simply use LUA to implement the standard algorithm as you would in regular programs and then call your macro like `@sha3_256("You data to hash")` which may occur anywhere in your code (as it applies before parsing). QUIX just made elegant macros a solved problem. Also, macros are loosely (optionally) type-checked on input and output, which catches wierd issues early.

![QUIX](https://github.com/user-attachments/assets/754f8c82-bcbe-4a30-98dc-10312979b784)

### Optimizer Engine Architecture

TODO: write this section

### Diagnostic Engine Architecture

TODO: write this section

## Usage

TODO: write this section

## Building

To build the Quix compiler from source, follow these steps:

1. **Install Dependencies**: Make sure you have all necessary dependencies installed on your system.
2. **Clone the Repository**: Clone the Quix compiler repository from GitHub using the following command:
   ```
   git clone https://github.com/Kracken256/quix
   ```
3. **Navigate to the Repository**: Change your current directory to the cloned repository directory:
   ```
   cd quix
   ```
4. **Build the Compiler**: Build the Quix compiler by running the `make.py --release` command. This will compile the compiler in release mode, which is optimized for performance:
   ```
   python make.py --release
   ```
   If you want to build the compiler in debug mode, which includes additional debugging information, run the following command instead:
   ```
   python make.py --debug
   ```
   The build process may take some time to complete, depending on your system's hardware and the size of the Quix compiler source code.
5. **Verify**: The pipeline will produce a `./bin` folder in the repository root. Inside this folder, you will find the `qpkg` executable, among other binaries. Congratulations! You have successfully built the Quix compiler.

## Contributing

TODO: write this section

## License

The Quix compiler is free software released under the GNU Lesser General Public License 2.1 (LGPL 2.1). See the `LICENSE` file for more information.

