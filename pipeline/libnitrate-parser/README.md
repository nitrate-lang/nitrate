# Nitrate Language Parser

This library is a part of the [Nitrate](https://nitrate.dev) project. It provides a 
general-purpose parser for the [Nitrate programming language](https://nitrate.dev).

## Features

- **Fast**: The parser is quite fast and is implemented with a data-oriented design philosophy. 
- **Minimal Memory Usage**: The node structures are small to minimize memory usage.
- **Debuggable**: The library can be built with a runtime node allocation origin tracker to save the C++ source location (from compiler source code) of the column/function/source file that created each node. 
 The metadata is stored in the nodes and can be seen in the serialized format.
- **Quality Diagnostics**: The parser uses [Recursive Descent Parsing](https://en.wikipedia.org/wiki/Recursive_descent_parser) and emits colorful syntax diagnostics with source location information.
- **Error Recovery**: The parser can recover from syntax errors and continue parsing the rest of the source file.
- **Modern**: The parser is written in modern C++20 and uses the latest language features.
- **Adaptable**: Both speed and memory-optimized builds are supported.

## Limitations

- **Whitespace tracking**: The parser does not track whitespace.
- **Comments**: Comment tracking is supported. However, the parser does not track 
 the exact position of comments in the source file. Comments will likely 
 be attached to nodes different from those in the source file.
- **Macro preservation**: The parser does not preserve macros in the Abstract Syntax Tree. The limitations
 of the aforementioned comment tracking apply to macros as well. However, macros would
 be sensitive to displacement in the Abstract Syntax Tree. So, it is not
 currently implemented at all.

## Building

This library is built as a part of the Nitrate program repository.

## Bugs and Issues

Please report bugs or issues on the [GitHub issue tracker](https://github.com/Kracken256/nitrate/issues).

## License

This library is licensed under the LGPLv2.1+ license. See the `LICENSE` file for
more information.
