# Nitrate Compiler Core Utilities

## Features

Unless otherwise specified, all the utilities in this library are thread-safe and can be used in a multi-threaded environment.

1. **Dynamic Arena Allocator**: A performant arena allocator that works without any prior knowledge of the maximum size of the arena. It allocates in blocks and stores these chunks in a vector. This avoids invalidating pointers when the arena grows.
2. **String Interning**: A string interner that stores strings in a hash map and returns a reference to the interned string. This allows for fast string comparison and equality checks by simply comparing the integer identifiers. The interning is process-local and synchronized internally.
3. **Shared Variable Map**: An in memory key-value store that allows for storing and retrieving variables by name. This is used primarily for storing and retrieving configuration variables such as compiler flags.
4. **Error Reporting**: A simple error reporting system that allows for reporting of message with a severity level. The messages are dispatched to a callback function in real-time.
5. **Macros**: Basic macros like for for library symbol exports.
6. **Cache**: Simple cache implementation that allows for storing and retrieving data by key.
