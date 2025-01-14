# Nitrate Language Parser

This library is a part of the [Nitrate](https://nitrate.dev) project. It provides a 
general-purpose parser for the [Nitrate programming language](https://nitrate.dev).

## Features

- **Fast**: The parser is quite fast and is implemented using a data-oriented design philosophy. 
- **Minimal Memory Usage**: The AST structs are small to minimize memory usage.
- **Very Debugable**: The library can be built with a runtime AST allocation origin tracker. 
                 This will save the C++ source location (from compiler source code) of the line/function/source-file that created each AST node. 
                 The metadata is stored in the AST nodes and can be seen in the serialized AST.
- **Adaptable**: Both speed and memory-optimized builds are supported.
- **Quality Diagnostics**: The parser uses Recursive Descent Parsing [RDP](https://en.wikipedia.org/wiki/Recursive_descent_parser) and emits colorful syntax diagnostics with source location information.
- **Error Recovery**: The parser can recover from syntax errors and continue parsing the rest of the source file.
- **Modern**: The parser is written in modern C++20 and uses the latest language features.


## Limitations

- **Whitespace tracking**: The parser does not track whitespace at all.
- **Comments**: Comment tracking is supported. However, the parser does not track 
                the exact position of comments in the source file. Comments will likely 
                be attached to different AST nodes than they are in the source file.
- **Macro preservation**: The parser does not preserve macros in the AST. The limitations
                          of the comment tracking apply to macros as well. However, macros would
                          be extreamly sensitive to displacement in the AST. And so it is not
                          currently implemented at all.

## Building

This library is built as a part of the Nitrate program repository.
There is no documentation for building this library separately without the rest
of the Nitrate project build system.

## Bugs and Issues

Please report bugs or issues on the [GitHub issue tracker](https://github.com/Kracken256/nitrate/issues).

## License

This library is licensed under the LGPLv2.1+ license. See the `LICENSE` file for
more information.
