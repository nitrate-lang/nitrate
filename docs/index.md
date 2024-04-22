---

layout: home
---

# Welcome to the QUIX website!

**Everything on this site is a work in progress. And the language is in rapid development.**

What is QUIX?
---

QUIX is a high-level, statically typed, memory-safe, garbage-collected, high-performance, object-oriented, general-purpose systems programming language designed to be safe, expressive, and performant. QUIX supports runtime reflection, low-level memory access, namespaces, mandatory explicit typing, packed data types, classes, operator overloading, coroutines, default initializers, arbitrary precision math, destructors, automatic memory management, and metaprogramming.

QUIX is especially well-suited for library development, game development, and systems programming. QUIX is designed to be a modern, safe, and expressive alternative to C++ and Rust.

Why QUIX?
---

1. **Safety**: QUIX is memory-safe and type-safe.
1. **Performance**: QUIX is designed to be as fast as C++. QUIX supports simultaneous garbage collection, reference counting, and manual memory management. 
1. **Expressiveness**: QUIX uses human-readable keywords and is whitespace insensitive.
1. **Compatible**: QUIX links to C, C++, and DLang object files out of the box. There is no need for annoying and ugly bindings or wrappers, and there is no overhead when calling C, C++, or DLang functions.
1. **Reusability**: Because QUIX plays well with C++, the QUIX standard library is vast. The library contains many useful tools, such as Math, File I/O, Networking, Compression, Cryptography (OpenSSL wrapper), and more.
1. **Small Binaries**: QUIX produces small, packed binaries. The standard library is modular, and unrelated code is not linked to the final binary. If you don't call it, it's not there. Binaries start at ~8KB. QUIX does not depend on libc; everything is built in from the ground up.
1. **Pretty**: IMO: C++ is beautiful, but it is not safe. Rust is hideous, but it is safe. Go is nice, but it is limited. QUIX is designed to be safe, pretty, and feature-rich.
1. **QUIX Does it All** With multiple memory management strategies, inheritance, operator overloading, runtime meta-programming/reflection, concurrency, contract programming, interoperability, an all-in-one compiler, and more, QUIX is designed to be a one-stop shop for all your programming needs.
1. **MACROs**: QUIX has a mighty macro engine that is essentially just QUIX code wrapped in a macroblock. This sublanguage is Turing complete and supports all the features of QUIX, including loops, conditionals, recursion, and even system IO. Want to download a CRC32 lookup table at compile time? Just write a macro. Want to build a source-level optimizer and load a neural network from disk? Easy with QUIX macros.

[Read the Documentation](/docs/)
