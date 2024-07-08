# Note to self / TODO

# Write Test vectors!!!

# Language Features

- Support static storage duration for local and global variables.
- Support anonymous function callbacks, lambdas, and closures.
- Support function overloading and covariant return types.
- Support generics with type safety.
- Implement the `var` keyword for: heap allocation with built-in allocator.
- C++20 interop

```rs
/* Functions can do these things:
  * 1. Allocate memory (stack or heap)
  * 2. Calculate expressions
  * 3. Read values from any memory location
  * 4. Store values to any memory location
  * 5. Call other functions
  * 6. Return values to caller
  * 7. Contain statements:
  *    - Control flow statements:
  *      - if, else, else if
  *      - switch, case, default
  *      - while, do while, for, form, foreach
  *      - break, continue, return
  *      - try, catch, throw, finally
  *    - Declaration statements:
  *      - var, let, const
  *      - struct, group, region, union, enum
  *      - function
  *      - import, using, pub
  *    - Expression statements:
  *      - Assignment
  *      - Function call
  *      - Unary operators
  *      - Binary operators
  *      - Ternary operators
  *      - Cast operators
  *      - Array access
  *      - Member access
  *      - Pointer access
  *      - Address of
  *      - Dereference
  *      - Sizeof
  *      - Slice
  *      - Range
  *      - Tuple
  **/

```

# Semantic Analysis
- **Mutability Checking (QUIX-MC)**
- **Construct Existence Qualifier (QUIX-CEQ)**
- **Composite Invariant Verification (QUIX-CIV)**
- **Composite Size Resolution (QUIX-CSR)**
- **Bitfield Alias Qualifier (QUIX-BAQ)**
- **Derteministic Construction Verification (QUIX-DCV)**

# Optimizations

- **String Type Specialization Solver (QUIX-STSS)**
- **Vector Type Specialization Solver (QUIX-VTSS)**
- **Hashmap Type Specialization Solver (QUIX-HTSS)**
- **Number Type Specialization Solver (QUIX-NTSS)**
- **Reference Type Solver (QUIX-RTS)**

- Transform single threaded code into multi-threaded code
- Recognize common structural patterns and replace them with optimized equivalents (remove polymorphism, etc.)
- Move large code segments into dynamic library bundles
- Replace math with SIMD instructions
- Algebraic reduction
- Remove redundant locks
- Auto-generate comparison methods for user defined types
- Remove redundant threads
- Remove redundant memory allocations
- Replace allocators with flyweight patterns
- Replace heap with stack whenever possible
- Replace stack with static memory whenever possible
- Replace static memory with compile time constants whenever possible
- If allowed, install huristics into opaque algorithms
- Replace opaque algorithms with specialized algorithms
- Replace out-of-band messaging with direct functional messaging
- Replace exceptions with composite return values
- Eliminate dynamic runtime deallocations (garbage collection) whenever possible
- Replace stringy code with enums
- Native TypeSize optimizations
- Generator unrolling
- Cache the results of complex pure functions
- Replace pure functions with lookup tables
- Remove trivially constructable unused allocations/deallocations
- Replace non-trivially constructable unused allocations with direct constructor calls
- ML-smart branch prediction / rearangement
- Don't construct fields that are unused

# Standard Library

- Smart string and Smart Vector
- Reference counting
- Garbage collection
- Write standard library