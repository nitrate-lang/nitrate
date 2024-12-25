# Nitrate Parser Library

This library is a part of the [Nitrate](https://nitrate.dev) project. It provides a 
general-purpose parser for the [Nitrate programming language](https://nitrate.dev).

## Features

- **Fast**: The parser is fast and is implemented using a data-oriented design philosophy. 
Techniques such as arena allocation and string interning improve performance and decrease memory usage.
- **Adaptable**: Both speed and memory-optimized builds are supported.
- **Quality Diagnostics**: The parser uses Recursive Descent Parsing [RDP](https://en.wikipedia.org/wiki/Recursive_descent_parser) and emits colorful syntax diagnostics with source location information.
- 
## Building

This library is built as a part of the Nitrate program repository.
There is no documentation for building this library separately without the rest
of the Nitrate project build system.

## Bugs and Issues

Please report bugs or issues on the [GitHub issue tracker](https://github.com/Kracken256/nitrate/issues).

## License

This library is licensed under the LGPLv2.1+ license. See the `LICENSE` file for
more information.
