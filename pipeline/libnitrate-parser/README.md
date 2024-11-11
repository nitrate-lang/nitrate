# Nitrate Parser Library

This library is a part of the [Nitrate](https://nitrate.dev) project. It provides a
flexible and efficient parser for the [Nitrate](https://nitrate.dev) programming
language.

## Features

- **Fast**: The parser is fast and is implemented using a data-oriented design philosophy. 
Techniques such as arena allocation and string interning improve performance and decrease memory usage.
- **Flexible**: Both speed and memory-optimized builds are supported.
- **Extensible**: The parser uses Recursive Descent Parsing [RDP](https://en.wikipedia.org/wiki/Recursive_descent_parser)
 which makes it easy to extend and modify the parser.
- **Robust Error Handling**: The parser provides detailed error messages and suggestions
 for common syntax errors and their associated fixes.
- **Generic to Underlying Scanner**: The parser depends on a generic scanner object and a set of API
 functions to interact with the scanner. This abstraction enables the parser to be used with any scanner
 implementation that provides the required API functions. Implementing custom preprocessor
 functionality is possible without having to modify the parser.

## Building

This library is built as a part of the Nitrate program repository.
There is no documentation for building this library separately without the rest
of the Nitrate project build system.

## Bugs and Issues

Please report bugs or issues on the [GitHub issue tracker](https://github.com/Kracken256/nitrate/issues).

## License

This library is licensed under the LGPLv2.1+ license. See the `LICENSE` file for
more information.
