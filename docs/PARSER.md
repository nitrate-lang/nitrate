# Syntactic Parsing Subsystem

## Theoretical Foundation

Parsing (syntactic analysis) is the second phase of compilation. It consumes a stream of tokens from the lexer and produces a structured Abstract Syntax Tree (AST) — called the Parse Tree in Nitrate — that represents the grammatical structure of the source code.

The parser must:

- **Recognize grammar**: Determine which sequences of tokens form valid syntactic constructs
- **Build structure**: Construct tree nodes that reflect the hierarchical nature of the language
- **Report errors**: Produce clear, actionable diagnostics for malformed input
- **Recover synchronization**: Continue parsing after errors to discover more issues in a single pass

Nitrate uses a **recursive-descent parsing** strategy: each grammar rule is implemented as a function that parses the corresponding construct. This approach is hand-coded (not parser-generator based), providing maximum control over error messages, recovery strategies, and parsing of complex constructs like operator precedence.

## Architecture

**Crate**: `nitrate_tree_parse`  
**Crate (AST types)**: `nitrate_tree`  
**Dependencies**: `nitrate_token_lexer` (token stream), `nitrate_diagnosis` (error logging)  
**Key types**: `Parser<'a, 'log>`, item/expr/type AST nodes

### The Parser Struct

```rust
pub struct Parser<'a, 'log> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) log: &'log CompilerLog,
}
```

The parser wraps a `Lexer` instance and a reference to the `CompilerLog` for diagnostic reporting. The lifetime `'a` ties the parser to the source bytes (through the lexer), and `'log` ties it to the compiler log.

### Entry Point

```rust
pub fn parse_source(&mut self, package_name: NString) -> Module
```

Parsing begins by disabling trivia (whitespace/comments are irrelevant for syntax analysis), then repeatedly parsing items until EOF:

```rust
lexer.disable_trivia();
while !lexer.is_eof() {
    items.push(self.parse_item());
}
Module { name: package_name, items, ... }
```

## Parse Tree (AST) Structure

The AST is defined in the `nitrate_tree` crate. The top-level type is `Module`, which contains a flat list of `Item` nodes. Each item represents a top-level declaration.

### Item Types

The `Item` enum covers all top-level declarations:

- **`Function`**: A function declaration with signature (name, generic parameters, parameters, return type, where clauses) and optional body
- **`StructDef`**: A struct type definition with named fields
- **`EnumDef`**: An enum type definition with named variants (each carrying a type)
- **`ClassDef`**: A class definition
- **`UnionDef`**: A union type definition
- **`ContractDef`**: A contract (like a trait but with stronger guarantees)
- **`TraitDef`**: A trait definition with methods and associated types/constants
- **`ImplBlock`**: An implementation block for a type
- **`TypeAlias`**: A type alias declaration
- **`ModuleDef`**: A submodule declaration
- **`UseDecl`**: An import/use declaration
- **`Scope`**: A scope/block
- **`ExternDecl`**: An extern declaration for FFI
- **`GlobalVariable`**: A global variable declaration

### Expression Types

Expressions form the core of the AST and include:

- **Literals**: Integer, float, string, boolean, null, unit
- **Identifiers**: Simple variable/type references
- **Binary operations**: Arithmetic (`+`, `-`, `*`, `/`, `%`, `**`, `<<`, `>>`, `<<<`, `>>>`), bitwise (`&`, `|`, `^`), logical (`&&`, `||`), comparison (`<`, `>`, `<=`, `>=`, `==`, `!=`)
- **Unary operations**: `+`, `-`, `!`
- **Calls**: Regular function calls, method calls
- **Control flow**: `if`, `while`, `loop`, `for`, `match`, `break`, `continue`, `return`
- **Compound expressions**: Blocks, tuples, lists/arrays, struct literals, enum literals
- **Access expressions**: Field access, index access, dereference
- **Special expressions**: Cast, borrow, assignment, `asm`

### Type Expressions

Types in the parse tree mirror the language's type grammar:

- **Primitives**: `bool`, `u8`..`u128`, `i8`..`i128`, `f8`..`f128`, `usize`, `opaque`
- **Compound types**: Arrays (`[T; N]`), tuples (`(T1, T2, ...)`)
- **Reference types**: `&T`, `&mut T`, `&uniq T`
- **Pointer types**: `*T`, `*mut T`, `*uniq T`
- **Slice types**: `[T]`, `&[T]`, `*[T]`
- **Named types**: Struct/enum/type-alias references, possibly with generic arguments (`Foo<T>`)
- **Function types**: `fn(T1, T2) -> Ret`
- **Trait object types**: `dyn Trait`
- **Inferred types**: `_`, `let` binding with type omitted

## Parsing Strategy

The parser is a hand-written recursive-descent parser with one token of lookahead. Each parsing method follows a consistent pattern:

```rust
fn parse_something(&mut self) -> Result<AstNode, Error> {
    let start_token = self.lexer.peek_tok();

    // Check which alternative matches
    if self.lexer.next_is(&Token::SomeKeyword) {
        self.lexer.skip_tok(); // consume the keyword
        // Parse the components of this alternative
        Ok(AstNode::Alternative { ... })
    } else {
        // Alternative 2
    }
}
```

### Operator Precedence and Associativity

Binary operators are parsed using a **precedence climbing** method (a variation of the classic Pratt parser). Each operator has an associated precedence level:

| Precedence | Operators                   | Associativity |
| ---------- | --------------------------- | ------------- |
| 1          | `\|\|`                      | Left          |
| 2          | `&&`                        | Left          |
| 3          | `==` `!=` `<` `>` `<=` `>=` | Left          |
| 4          | `\|`                        | Left          |
| 5          | `^`                         | Left          |
| 6          | `&`                         | Left          |
| 7          | `<<` `>>` `<<<` `>>>`       | Left          |
| 8          | `+` `-`                     | Left          |
| 9          | `*` `/` `%`                 | Left          |
| 10         | `**` (exponentiation)       | Right         |

Prefix and postfix operators are parsed inline before descending into binary expressions.

### Item Parsing

The top-level `parse_item()` dispatches based on the first keyword or token:

```
peek token → dispatch:
  "fn"       → parse_function
  "struct"   → parse_struct
  "enum"     → parse_enum
  "class"    → parse_class
  "union"    → parse_union
  "contract" → parse_contract
  "trait"    → parse_trait
  "impl"     → parse_impl
  "type"     → parse_type_alias
  "scope"    → parse_scope
  "mod"      → parse_module
  "use"      → parse_use
  "extern"   → parse_extern
  "let"/"var"/"static" → parse_global_variable
  "pub"/"sec"/"pro" → parse visibility, then dispatch again
```

## Module System Parsing

### Module Declarations

A module is declared with the `mod` keyword:

```
mod name;
mod name { ... }
```

Modules can be file-based (semicolon-terminated, loaded from a separate file) or inline (block body).

### Import Declarations

The `use` keyword imports names from other modules:

```
use path::to::item;
use path::to::*;
```

The parser captures the use path as a series of identifiers and separators, plus an optional wildcard.

## Function Parsing

Function declarations follow a detailed grammar:

```
[visibility] fn name<generics>(params) [-> return_type] [where clauses] {
    body
}
```

The parser:

1. Optionally reads a visibility modifier (`pub`, `sec`, `pro`)
2. Consumes the `fn` keyword
3. Parses the function name
4. Optionally parses generic parameters (angle-bracket delimited list)
5. Parses parameter list (parenthesis delimited comma-separated list of `name: type = default` triples)
6. Optionally parses return type (after `->`)
7. Optionally parses where clauses
8. Parses the function body as a block expression

## Struct Parsing

Struct definitions follow:

```
[visibility] struct name<generics> {
    [visibility] field_name: type [= default],
    ...
}
```

Fields can have individual visibility modifiers and optional default values.

## Enum Parsing

Enum definitions:

```
[visibility] enum name<generics> {
    variant_name: type [= default],
    ...
}
```

Each variant has a name, a type, and an optional default value.

## Trait Parsing

Trait definitions:

```
[visibility] trait name<generics> [: supertraits] [where clauses] {
    [visibility] fn method_name(params) [-> return_type];
}
```

Traits can have supertraits (colon-separated list after the trait name), where clauses, and method declarations (signatures without bodies, terminated by semicolons).

## Impl Block Parsing

Implementation blocks:

```
impl<generics> Trait|Type for Type {
    fn method_name(params) [-> return_type] { body }
}
```

Impl blocks implement either a trait for a type or inherent methods for a type.

## Expression Parsing: Control Flow

### If Expressions

```
if condition { true_branch } [else { false_branch }]
```

The condition does not require parentheses (unlike C-style languages). The `else` clause is optional and can chain: `if A { } else if B { } else { }`.

### While Loops

```
while condition { body }
```

Similar to `if`, the condition is unparenthesized.

### For Loops

```
for pattern in expression { body }
```

Iterates over the elements of an iterable expression.

### Loop

```
loop { body }
```

Infinite loop construct, terminated by `break`.

### Match Expressions

```
match value {
    pattern => expression,
    ...
}
```

Pattern matching with multiple arms. The parser captures the matched value and a list of arms (each with a pattern and body expression).

### Break, Continue, Return

```
break [label]
continue [label]
ret [expression]
```

All three can carry optional labels (for nested loops) or a value (for `ret`).

## Literal Parsing

The parser handles:

- **Integer literals**: Converted from `Token::Integer` to the AST representation
- **Float literals**: Converted from `Token::Float`
- **String literals**: Captured directly from `Token::String` and `Token::BString`
- **Boolean literals**: `true` and `false` tokens
- **Null literal**: The `null` token
- **Unit literal**: Represented as an empty tuple `()`
- **List literals**: `[expr, expr, ...]`
- **Tuple literals**: `(expr, expr, ...)`
- **Struct literals**: `StructName { field: value, ... }`

## Visibility Modifier Parsing

Visibility modifiers (`pub`, `sec`, `pro`) are parsed as prefix attributes on items. They form a three-tier access control system:

- `pub` (public): Accessible from any module
- `pro` (protected): Accessible from the current module and submodules
- `sec` (secret/private): Accessible only from the current module

Items without explicit visibility default to `sec` (private).

## Where Clause Parsing

Where clauses constrain generic parameters:

```
fn foo<T>(t: T) where T: Clone + Debug { }
```

Each constraint is a type name followed by a colon-separated list of bounds. Bounds can be trait names or lifetime specifiers.

## Generic Parameter Parsing

Generic parameters appear in angle brackets:

```
fn foo<T: Clone, U: Debug = DefaultType>(...)
```

Each parameter has a name, optional bounds, and an optional default type.

## Error Handling

The parser logs errors to the `CompilerLog` but continues parsing via recovery strategies:

- **Semicolon insertion**: If a statement is expected but missing a semicolon, the parser can synthesize one
- **Brace matching**: If a closing brace is missing, the parser can skip to the next top-level item
- **Token skipping**: On unrecognized constructs, the parser skips tokens until a known restart point is found

Each parse error includes:

- The source position (file, line, column)
- A descriptive error message
- The expected token(s) vs. the actual token found
- Suggestions for correcting the issue (where applicable)

## Testing

The parser has an extensive test suite covering:

- **Binary operations**: All operator combinations and precedence
- **Enums**: Various enum declarations with and without generics
- **Expressions**: Literal, unary, binary, call, method call, field access, index access
- **Functions**: Simple, generic, with where clauses, with attributes
- **Generics**: Parsing generic parameters and arguments
- **Impl blocks**: Trait impls, inherent impls, generic impls
- **Imports**: Various use declaration forms
- **Literals**: All literal forms
- **Modules**: Module declarations and file structure
- **Programs**: Multi-item compilation units
- **Structs**: Struct definitions with various field configurations
- **Traits**: Trait declarations and method signatures
- **Types**: All type expression forms
- **Unary operations**: Prefix operators
- **Variables**: `let`, `var`, and `static` declarations

Diagnostic format tests verify that error messages are correctly formatted and user-friendly.

## Design Decisions

### Why Recursive Descent Over Parser Generators?

1. **Error message quality**: Hand-written parsers produce the best error messages because they know what construct they expected
2. **Flexibility**: Complex grammar constructs (like operator precedence) are easier to implement manually
3. **Recovery control**: The parser can implement sophisticated error recovery by manipulating the lexer position
4. **Dependency reduction**: No external parser generator dependency

### Why Precedence Climbing for Operators?

Precedence climbing (a streamlined Pratt parsing variant) provides:

- Clean separation between operator definitions and parsing logic
- Easy reordering of precedence levels (just change the number)
- Left and right associativity support
- Linear time complexity
- No need for grammar rewriting

### Why Separate Parse Tree from HIR?

The parse tree (AST) preserves source-level syntax exactly as written, while the HIR normalizes and desugars constructs. This separation:

- Simplifies the parser (only syntactic concerns)
- Provides a clear transformation boundary
- Enables multiple lowering strategies for different constructs
- Makes the HIR independent of syntax changes
