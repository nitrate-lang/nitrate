# Orbit - The QUIX Standard Library

Orbit is the standard library for QUIX. It contains all the built-in functions and classes available to QUIX programs by default. It is a collection of packages that provide a wide range of functionality, including I/O, string manipulation, math, and more.

**Note:** Each bullet point is a package that contains multiple modules. The Orbit standard library is a bundle or `meta-package` of these packages.

### Packages contained in the standard library
- **Containers**

    *Note: These modules require a heap allocator to be implemented. Otherwise, they are fully operating system and architecture-independent.*
    | Module | Description |
    | ------ | ----------- |
    | `string` | String manipulation functions |
    | `vector` | Create and manipulate dynamic arrays |
    | `set` | Create and manipulate sets |
    | `map` | Create and manipulate associative arrays |
    | `stack` | Create and manipulate stacks |
    | `queue` | Create and manipulate queues |
    | `deque` | Create and manipulate double-ended queues |
    | `ring` | Create and manipulate ring buffers |
    | `tensor` | Create and manipulate multi-dimensional arrays |
    | `matrix` | Create and manipulate numeric matrices |
    | `graph` | Create and manipulate generic, (un)directed, (a)cyclic, (un)weighted graphs |
    | `tree` | Create and manipulate generic trees |
    | `binary_tree` | Create and manipulate binary trees |
    | `@container` | Package for all container modules |

- **Container Algorithms**

    *Note: All of these modules are fully operating system and architecture-independent.*
    | Module | Description |
    | ------ | ----------- |
    | `sort` | Sorting algorithms |
    | `search` | Searching algorithms |
    | `path` | Pathfinding algorithms |
    | `count` | Counting algorithms |
    | `sequence` | Create a generator to return a sequence of objects |
    | `qrandom` | Generate pseudo-random object instances by filling the structure(s) with random data. Will likely break invariant properties of the structure(s) |
    | `objhash` | Object-specific perfect hash functions |
    | `@algorithm` | Package for all container algorithm modules |

- **Math**

    *Note: All of these modules are operating system and architecture-independent. However, many require floating-point support.*
    | Module | Description |
    | ------ | ----------- |
    | `number` | Arbitrary precision integer and floating-point mathematics |
    | `complex` | Complex number mathematics and functions |
    | `calc` | Calculus functions (multivariant differentiation, integration, etc.) |
    | `trig` | Trigonometric functions |
    | `geometry` | Geometric functions |
    | `statistics` | Statistical functions |
    | `random` | High-quality deterministic random number generators |
    | `constants` | Mathematical constants |
    | `@math` | Package for all math modules |

- **Time**

    *Note: The `ostime` module is operating system specific. Many time modules depend on it.*
    | Module | Description |
    | ------ | ----------- |
    | `ostime` | Operating system specific time functions |
    | `datetime` | Time manipulation functions |
    | `date` | Date manipulation functions |
    | `calendar` | Calendar manipulation functions |
    | `timezone` | Timezone manipulation functions |
    | `timer` | Timer manipulation functions |
    | `stopwatch` | Stopwatch manipulation functions |
    | `@time` | Package for all time modules |

- **Cryptography**

    *Note: All of these modules are operating system and architecture-independent.*
    | Module | Description | Cryptographically Secure | Built with |
    | ------ | ----------- | ---------------------- | ---------- |
    | `hash` | Hash functions | Yes | OpenSSL EVP_Digest* |
    | `cipher` | Symmetric and asymmetric encryption | Yes | OpenSSL EVP_Cipher and EVP_PKEY |
    | `sign` | HMAC and asymmetric signing | Yes | OpenSSL EVP signing |
    | `keygen` | Key generation | Yes | OpenSSL EVP_PKEY and RAND |
    | `crypto` | Recommended cryptographic primitives | Yes | Abstracts above modules |
    | `@crypt` | Package for all cryptography modules |

- **Data Transcoding and Compression**

    *Note: All of these modules are operating system and architecture-independent.*
    | Module | Description |
    | ------ | ----------- |
    | `basen` | Stream encoding and decoding. base64, base16, and generic baseN with custom alphabets |
    | `endian` | Endian conversion functions |
    | `zlib` | Zlib compression and decompression |
    | `lz4` | LZ4 compression and decompression |
    | `bzip2` | Bzip2 compression and decompression |
    | `lzma` | LZMA compression and decompression |
    | `gzip` | Gzip compression and decompression |
    | `tar` | Tar file manipulation |
    | `zip` | Zip file manipulation |
    | `7z` | 7z file manipulation |
    | `rar` | Rar file manipulation |
    | `streamconv` | Stream conversion generics |
    | `@data` | Package for all transcoding and compression modules |

- **Util**

    *Note: Lots of dependencies on other modules.*
    | Module | Description |
    | ------ | ----------- |
    | `uuid` | UUID manipulation functions |
    | `regex` | Regular expression functions |
    | `format` | String formatting functions |
    | `ansicolor` | ANSI color codes |
    | `log` | Logging core. Can be hooked programmatically to intercept log messages |
    | `exception` | Exception handling |
    | `assert` | Assertion functions |
    | `debug` | Debugging functions |
    | `dev` | Common development tools. unsafe hex dump object, stack trace, symbol lookup, etc. |
    | `macro` | Many macro definitions |
    | `@util` | Package for all util modules |

- **sysio**

    *Note: All of these modules are operating system and architecture-dependent.*
    | Module | Description |
    | ------ | ----------- |
    | `osfile` | Operating system file manipulation functions |
    | `osdir` | Operating system directory manipulation functions |
    | `file` | High-level file manipulation functions |
    | `dir` | High-level directory manipulation functions |
    | `pipe` | Operating system pipe manipulation functions |
    | `socket` | Socket manipulation functions |
    | `serial` | Serial port manipulation functions |
    | `net` | TCP, UDP, QUIX transport layer protocols |
    | `tls` | TLS client and server support (OpenSSL) |   
    | `protocol` | Higher level protocols |
    | `io` | I/O stream manipulation functions |
    | `@sysio` | Package for all sysio modules |

- **runtime**

    *Note: All of these modules are operating system and architecture-dependent.*
    | Module | Description |
    | ------ | ----------- |
    | `process` | Operating system process manipulation functions |
    | `thread` | Operating system thread manipulation functions |
    | `mutex` | Operating system mutex manipulation functions |
    | `semaphore` | Operating system semaphore manipulation functions |
    | `event` | Operating system event manipulation functions |
    | `signal` | Operating system signal manipulation functions |
    | `memory` | Operating system memory statistics and manipulation functions |
    | `plugin` | Dynamically load and unload shared libraries or modules |
    | `@runtime` | Package for all runtime modules |

- **reflection**

    *Note: All of these modules are operating system and architecture-independent.*
    | Module | Description |
    | ------ | ----------- |
    | `serialize` | Serialization and deserialization functions |
    | `json` | JSON serialization and deserialization functions |
    | `xml` | XML serialization and deserialization functions |
    | `yaml` | YAML serialization and deserialization functions |
    | `toml` | TOML serialization and deserialization functions |
    | `protobuf` | Protocol Buffers serialization and deserialization functions |
    | `@reflect` | Package for all reflection modules |


# How to build?
- Clone the repository
- Navigate to the root of the repository
- Run `quixcc build -O5 -g2 -o orbit.qlib .`
- It will take a while to build. Also, the output jlib file will be quite large. It contains extra debug symbols and QUIX metadata sections. It will be significantly smaller when linked and stripped to a final executable.

# How to install?
- Run `quixcc install orbit.qlib`
- If the command fails, try running it with `sudo`

# How to use?
- Add `import [module]` to the top of your QUIX source file. For example, `import string` to use the string manipulation functions.
- Compile the QUIX source file with `quixcc`. Linking will be done automatically if the standard library is installed in the default location. If it is not, add the absolute path to the library to the `quixcc` command. For example, `quixcc /usr/lib/lib/orbit.qlib [source file] -o [output file]`

