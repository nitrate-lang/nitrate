# Syntactic Parsing Subsystem

## Theoretical Foundation

Parsing (syntactic analysis) is the second phase of compilation, following lexical analysis. It consumes a stream of tokens from the lexer and produces a structured Abstract Syntax Tree (AST) — called the Parse Tree in Nitrate — that represents the grammatical structure of the source code. The parser bridges the gap between the flat, linear sequence of tokens and the hierarchical, tree-structured representation that captures the grammatical relationships between language constructs.

The parser must accomplish several fundamental tasks. **Grammar recognition** determines which sequences of tokens form valid syntactic constructs according to the language's grammar rules — for example, recognizing that `fn foo(x: i32) -> i32 { x + 1 }` is a valid function declaration while `fn 123` is not. **Structure construction** builds tree nodes that reflect the hierarchical nature of the language — a function node contains parameter nodes, a return type, and a body block, which in turn contains expression nodes. **Error reporting** produces clear, actionable diagnostics when the input does not conform to the grammar, explaining what was expected versus what was found. **Error recovery** continues parsing after an error to discover additional issues in a single pass, rather than stopping at the first problem and forcing the user into multiple edit-compile cycles.

Nitrate uses a **hand-written recursive-descent parsing** strategy: each grammar rule is implemented as a dedicated function that parses the corresponding construct by calling sub-parsers for its components. This approach — as opposed to parser generators like YACC, Bison, or ANTLR — provides maximum control over error messages, recovery strategies, and the parsing of complex constructs like operator precedence. The recursive-descent approach is intuitive because the structure of the parsing code directly mirrors the structure of the grammar: a function declaration parser calls parameter parsers, type parsers, and body parsers, just as the grammar defines a function declaration in terms of its components.

## Architecture

**Crate**: `nitrate_tree_parse` (parser implementation)  
**Crate (AST types)**: `nitrate_tree` (parse tree/AST type definitions)  
**Dependencies**: `nitrate_token_lexer` (token stream input), `nitrate_diagnosis` (error reporting)  
**Key types**: `Parser<'a, 'log>`, item/expr/type AST nodes defined in `nitrate_tree`

### The Parser Struct

```rust
pub struct Parser<'a, 'log> {
    pub(crate) lexer: Lexer<'a>,
    pub(crate) log: &'log CompilerLog,
}
```

The parser wraps a `Lexer` instance (which owns the source bytes and produces tokens) and a reference to the `CompilerLog` for diagnostic reporting. The lifetime `'a` ties the parser to the source bytes through the lexer, and `'log` ties it to the compiler log for error accumulation. The parser does not own the token stream; it accesses it on demand through the lexer's `next_tok()` and `peek_tok()` methods.

### Entry Point

```rust
pub fn parse_source(&mut self, package_name: NString) -> Module
```

Parsing begins by disabling trivia on the lexer — whitespace and comments are irrelevant for syntactic analysis and are filtered out during tokenization. The parser then repeatedly invokes `parse_item()` to consume top-level declarations until the end of the token stream is reached:

```rust
lexer.disable_trivia();
while !lexer.is_eof() {
    items.push(self.parse_item());
}
Module { name: package_name, items, ... }
```

Each call to `parse_item()` examines the next token, determines which kind of top-level declaration is expected, and dispatches to the appropriate specialized parsing method.

## Parse Tree (AST) Structure

The AST is defined in the `nitrate_tree` crate. The top-level type is `Module`, which contains a flat list of `Item` nodes. Each item represents a top-level declaration in the source file.

### Item Types

The `Item` enum covers all top-level declarations recognized by the Nitrate language:

- **`Function`**: A function declaration with a signature (name, generic parameters, parameters, return type, where clauses) and an optional body. Functions without bodies are declarations — used for trait methods, FFI declarations, and forward declarations.
- **`StructDef`**: A struct type definition with named, typed fields that can each carry individual visibility modifiers and optional default values.
- **`EnumDef`**: An enum type definition with named variants, each carrying a type and optional default value.
- **`ClassDef`**: A class definition — a Nitrate-specific construct that bundles data with methods (similar to object-oriented classes but compiled through the same HIR pipeline).
- **`UnionDef`**: A union type definition where all variants share the same storage.
- **`ContractDef`**: A contract definition — similar to traits but with stronger semantic guarantees about method behavior.
- **`TraitDef`**: A trait definition with method signatures (without bodies) and associated type and constant declarations.
- **`ImplBlock`**: An implementation block that defines methods for a type or implements a trait for a type.
- **`TypeAlias`**: A type alias declaration that provides an alternative name for an existing type.
- **`ModuleDef`**: A submodule declaration, either file-based (semicolon-terminated, loaded from a separate file) or inline (with a brace-delimited body).
- **`UseDecl`**: An import declaration that brings names from other modules into scope.
- **`Scope`**: A scope/block declaration (similar to Rust's `mod` but without a separate file).
- **`ExternDecl`**: An extern block for FFI declarations.
- **`GlobalVariable`**: A global variable declaration (using `let`, `var`, or `static` at the top level).

### Expression Types

Expressions form the core of the AST and include a wide variety of constructs:

- **Literals**: Integer, float, string, boolean (`true`/`false`), null, and unit `()` literals
- **Identifiers**: Simple variable and type name references
- **Binary operations**: Arithmetic (`+`, `-`, `*`, `/`, `%`, `**`), bitwise (`&`, `|`, `^`, `<<`, `>>`, `<<<`, `>>>`), logical (`&&`, `||`), and comparison (`<`, `>`, `<=`, `>=`, `==`, `!=`)
- **Unary operations**: Identity (`+`), negation (`-`), and logical/bitwise not (`!`)
- **Calls**: Regular function calls with positional and named arguments, and method calls on objects
- **Control flow**: `if`/`else` conditional expressions, `while` loops, `loop` infinite loops, `for` iteration loops, `match` pattern matching, `break` and `continue` loop control, and `return` value return
- **Compound expressions**: Blocks (brace-delimited sequences of statements and expressions), tuples, lists/arrays, struct literal construction, and enum variant construction
- **Access expressions**: Field access (`object.field`), index access (`array[i]`), and dereference (`*ptr`)
- **Special expressions**: Type casts (`value as Type`), borrows (`&expr`, `&mut expr`), assignments (`place = value`), and inline assembly (`asm!`)

### Type Expressions

Types in the parse tree mirror the language's type grammar:

- **Primitives**: `bool`, all integer types (`u8` through `u128`, `i8` through `i128`, `usize`), all float types (`f8` through `f128`), and `opaque`
- **Compound types**: Fixed-size arrays (`[T; N]`), heterogeneous tuples (`(T1, T2, ...)`)
- **Reference types**: Shared references (`&T`), exclusive mutable references (`&mut T`), exclusive read-only references (`&uniq T`)
- **Pointer types**: Raw pointers (`*T`, `*mut T`, `*uniq T`)
- **Slice types**: Unsized views (`[T]`), referenced slices (`&[T]`), pointer slices (`*[T]`)
- **Named types**: Struct, enum, and type-alias references, optionally with generic arguments (`Foo<T, U>`)
- **Function types**: Function pointer types (`fn(T1, T2) -> Ret`)
- **Trait object types**: Dynamic dispatch types (`dyn Trait`)
- **Inferred types**: Placeholder types for omitted annotations (`_` in expressions, implicit in `let` bindings)

## Parsing Strategy

The parser is a hand-written recursive-descent parser with one token of lookahead. Each parsing method follows a consistent pattern: examine the next token to determine which production rule applies, consume the relevant keyword or delimiter, parse the sub-components, and construct the appropriate AST node.

```rust
fn parse_something(&mut self) -> AstNode {
    let start_token = self.lexer.peek_tok();

    // Check which alternative matches
    if self.lexer.next_is(&Token::SomeKeyword) {
        self.lexer.skip_tok(); // consume the keyword
        // Parse the components of this alternative
        AstNode::Alternative { ... }
    } else {
        // Alternative 2
    }
}
```

### Operator Precedence and Associativity

Binary operators are parsed using a **precedence climbing** method (a variation of the classic Pratt parsing algorithm). Each operator is associated with a numeric precedence level and an associativity direction. The algorithm uses these values to determine when to stop parsing the current expression and return a result, versus continuing to consume more operators.

| Precedence     | Operators                                                                            | Associativity |
| -------------- | ------------------------------------------------------------------------------------ | ------------- |
| 1 (Assign)     | `=` `+=` `-=` `*=` `/=` `%=` `&=` `\|=` `^=` `<<=` `>>=` `<<<=` `>>>=` `&&=` `\|\|=` | Right         |
| 2 (Range)      | `..`                                                                                 | Left          |
| 3 (LogicOr)    | `\|\|`                                                                               | Left          |
| 4 (LogicAnd)   | `&&`                                                                                 | Left          |
| 5 (Comparison) | `==` `!=` `<` `>` `<=` `>=`                                                          | Left          |
| 6 (BitOr)      | `\|`                                                                                 | Left          |
| 7 (BitXor)     | `^`                                                                                  | Left          |
| 8 (BitAnd)     | `&`                                                                                  | Left          |
| 9 (Shift)      | `<<` `>>` `<<<` `>>>`                                                                | Left          |
| 10 (AddSub)    | `+` `-`                                                                              | Left          |
| 11 (MulDiv)    | `*` `/` `%`                                                                          | Left          |

### Table-Driven Binary Operator Detection

The original implementation used a deeply nested `match`/`if` tree spanning ~200 lines to detect and parse binary operators. This has been replaced with a **table-driven pattern matching** approach:

```rust
struct OpPattern {
    tokens: &'static [Token],
    op: BinExprOp,
}

const BINOP_PATTERNS: &[OpPattern] = &[
    OpPattern { tokens: &[Token::Lt, Token::Lt, Token::Lt, Token::Eq], op: BinExprOp::SetBitRotl },
    OpPattern { tokens: &[Token::And, Token::And, Token::Eq],       op: BinExprOp::SetLogicAnd },
    // ... all 30+ operator patterns ordered longest-first ...
    OpPattern { tokens: &[Token::Lt],                                op: BinExprOp::LogicLt },
    OpPattern { tokens: &[Token::Dot, Token::Dot],                  op: BinExprOp::Range },
];
```

The `detect_and_parse_binary_operator` method iterates through this table in declaration order (longest sequences first, single tokens last), attempting each pattern by checking if the lexer's current token stream matches the pattern's token sequence. On match, it returns the corresponding `BinExprOp` without consuming more than the matched tokens. On failure, it rewinds to the saved position and tries the next pattern.

This approach provides four benefits over the original nested branching:

1. **Declarative operator definitions** — adding, removing, or reordering operators requires only changing the table
2. **Single responsibility** — the detection method is reduced from 200 lines to ~25 lines
3. **Easier auditing** — all operator mappings are visible in one sorted table
4. **No regressions** — the pattern order (longest-first) naturally handles operators that share prefixes (e.g., `<` vs `<<` vs `<<<` vs `<<=`)

#### Struct Init Ambiguity Resolution

When parsing a path expression followed by `{`, the parser must disambiguate between a struct literal (`Foo { x: 1 }`) and a block expression following an iterable in a `for` loop (`items { break; }`). This is handled by the `peek_is_struct_field_start()` method, which peeks inside the `{` to determine the content type:

```rust
pub(crate) fn peek_is_struct_field_start(&mut self) -> bool {
    let saved = self.lexer.current_pos();
    self.lexer.skip_tok();
    let result = match self.lexer.peek_tok().token {
        Token::CloseBrace          => true,   // empty struct: Foo {}
        Token::Name(_) | Token::SelfKeyword => true,  // named field: Foo { x: ... }
        Token::Colon               => true,   // anonymous field: Foo { : 1 }
        Token::OpenBracket         => true,   // attributed field: Foo { #[attr] x: 1 }
        Token::Break | Token::Continue | Token::Ret | Token::Let | Token::Var |
        Token::If | Token::For | Token::While | Token::Match | Token::Fn |
        Token::OpenBrace | Token::Unsafe | Token::Safe | Token::Await => false,
        _                          => false,  // treat as block by default
    };
    self.lexer.rewind(saved);
    result
}
```

This heuristic treats content after `{` as a struct field initializer only when the first token could plausibly be a field name or attribute. Keywords that only appear as block-level statements (break, continue, let, var, if, for, while, etc.) cause the parser to treat the braced content as a block instead. This correctly handles `for x in items { break; }` (block body) while still accepting `Foo { x: 1 }` (struct init) and `Foo { x 1 }` (error path for struct init).

## Shared Limit Constant

A module-level constant `MAX_LIMIT: usize = 65_536` is used across all element-count limit checks instead of repeated bare literals. This constant is defined in `helper.rs` and imported by all parser submodules, making the intent clear and changes centralized.

### Common Utility Methods

The `helper.rs` module provides shared utilities that reduce duplication across the parser submodules:

- **`parse_double_colon()`** — consumes `::` and returns `true` if both colons were found
- **`parse_name()`** — reads an identifier or reports a provided error
- **`expect_*()`** — family of methods to expect and consume specific delimiters (`;`, `{`, `}`, `(`, `)`, `[`, `]`, `:`, `->`, `>`)
- **`parse_mutability()`**, **`parse_exclusivity()`**, **`parse_visibility()`** — parse optional modifiers
- **`check_limit()`** — reports an exceeded-limit error at most once
- **`parse_comma_separated_list()`** — generic comma-separated list parser with limit checks, leading-comma support, and recovery

These shared methods ensure consistent error reporting and recovery behavior across all parsing contexts.

Prefix and postfix operators (unary `+`, `-`, `!`, dereference `*`, borrow `&`, etc.) are parsed inline before descending into binary expression parsing. The parser first checks for prefix operators, parses the operand (which may itself be a prefix expression, a primary expression, or a postfix expression with calls/accesses), then enters the precedence climbing loop for binary operators.

### Item Parsing

The top-level `parse_item()` dispatches based on the first keyword or token of each declaration:

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

This dispatch structure ensures that each declaration type is handled by a dedicated, focused parsing method, keeping the code modular and maintainable.

## Module System Parsing

### Module Declarations

A module is declared with the `mod` keyword and can take two forms:

```
mod name;          // File-based module (loaded from name.nit)
mod name { ... }   // Inline module (body contained in braces)
```

File-based modules (semicolon-terminated) cause the parser to load the corresponding file from the filesystem and recursively parse it. Inline modules (brace-delimited) contain their declarations directly in the source file. This two-form system enables both coarse-grained (one file per module) and fine-grained (multiple modules in one file) module organization.

### Import Declarations

The `use` keyword imports names from other modules into the current scope:

```
use path::to::item;      // Import a specific name
use path::to::*;          // Import all public names from a module (wildcard)
use path::to::{A, B, C};  // Import multiple specific names
```

The parser captures the use path as a series of identifiers and namespace separators (`::`), plus an optional wildcard or braced list of names. Semantic processing of imports is deferred to the resolver stage.

## Function Parsing

Function declarations follow a detailed grammar that the parser recognizes and decomposes:

```
[visibility] fn name<generics>(params) [-> return_type] [where clauses] {
    body
}
```

The parser executes a sequence of steps to parse a function declaration:

1. Optionally reads a visibility modifier (`pub`, `sec`, `pro`) that controls access from other modules
2. Consumes the `fn` keyword that identifies this as a function declaration
3. Parses the function name as a `Name` token
4. Optionally parses generic parameters enclosed in angle brackets — a comma-separated list of parameter declarations, each with a name, optional trait bounds, and optional default type
5. Parses the parameter list enclosed in parentheses — a comma-separated list of `name: type = default` triples where the name, type annotation, and default value are each optional
6. Optionally parses the return type after `->`
7. Optionally parses where clauses that constrain generic parameters with trait bounds
8. Parses the function body as a block expression enclosed in braces — if the body is present, the function is a definition; if absent (just a semicolon), it's a declaration

## Struct Parsing

Struct definitions follow a straightforward syntax:

```
[visibility] struct name<generics> {
    [visibility] field_name: type [= default],
    ...
}
```

Each field can have its own visibility modifier, type annotation, and optional default value. The parser processes fields sequentially, building up the list of field declarations.

## Enum Parsing

Enum definitions:

```
[visibility] enum name<generics> {
    variant_name: type [= default],
    ...
}
```

Each variant carries a name, a type (the data payload associated with the variant), and an optional default value. The parser collects all variants into a list.

## Trait Parsing

Trait definitions:

```
[visibility] trait name<generics> [: supertraits] [where clauses] {
    [visibility] fn method_name(params) [-> return_type];
}
```

Traits can specify supertraits (colon-separated list of trait dependencies after the trait name), where clauses constraining generic parameters, and method declarations. Trait methods are signatures without bodies, terminated by semicolons rather than brace-delimited blocks.

## Impl Block Parsing

Implementation blocks provide method definitions for types:

```
impl<generics> Trait|Type for Type {
    fn method_name(params) [-> return_type] { body }
}
```

Impl blocks come in two forms: trait implementations (`impl Trait for Type { ... }`) that implement a trait's methods for a type, and inherent implementations (`impl Type { ... }`) that define methods directly on a type without a trait. Both forms can have generic parameters and contain method definitions with full bodies.

## Expression Parsing: Control Flow

### If Expressions

```
if condition { true_branch } [else { false_branch }]
```

The condition does not require parentheses (unlike C-style languages), making the syntax cleaner. The `else` clause is optional and can chain to form `else if` ladders: `if A { } else if B { } else { }`. The parser handles chaining by recursively parsing `else if` as `else { if ... }`.

### While Loops

```
while condition { body }
```

Similar to `if`, the condition is unparenthesized. The parser produces a `While` AST node with the condition expression and body block.

### For Loops

```
for pattern in expression { body }
```

Iterates over the elements of an iterable expression. The parser captures the iteration pattern (which may be a simple variable name or a destructuring pattern) and the iterable expression.

### Loop

```
loop { body }
```

An infinite loop construct that must be terminated by a `break` expression inside the body. The parser produces a `Loop` AST node with the body block.

### Match Expressions

```
match value {
    pattern => expression,
    ...
}
```

Pattern matching with multiple arms. The parser captures the matched value expression and a list of arms, each consisting of a pattern and a body expression. Patterns can include literal values, variable bindings, destructuring, and wildcards.

### Break, Continue, Return

```
break [label]         // Exit a loop, optionally targeting a specific labeled loop
continue [label]      // Skip to next iteration, optionally targeting a labeled loop
ret [expression]      // Return a value from the enclosing function
```

All three can carry optional labels (for nested loop targeting) or a value (for `ret`).

## Literal Parsing

The parser converts token literals to AST literal nodes:

- **Integer literals**: Converted from `Token::Integer` to the AST integer literal representation
- **Float literals**: Converted from `Token::Float` to AST float literal representation
- **String literals**: Captured from `Token::String` and `Token::BString` tokens
- **Boolean literals**: `true` and `false` keyword tokens map to boolean literal nodes
- **Null literal**: The `null` keyword token maps to a null literal node
- **Unit literal**: Represented as an empty tuple `()` in source, parsed as a unit literal
- **List/Array literals**: `[expr, expr, ...]` — comma-separated expressions in square brackets
- **Tuple literals**: `(expr, expr, ...)` — comma-separated expressions in parentheses
- **Struct literals**: `StructName { field: value, ... }` — named fields with values in braces

## Visibility Modifier Parsing

Visibility modifiers (`pub`, `sec`, `pro`) are parsed as prefix attributes on items. They form a three-tier access control system:

- `pub` (public): Accessible from any module in any package
- `pro` (protected): Accessible from the current module and its submodules
- `sec` (secret/private): Accessible only from the current module

Items without an explicit visibility modifier default to `sec` (private). The parser reads a visibility modifier if present, then dispatches to the appropriate item parser with the visibility information.

## Where Clause Parsing

Where clauses constrain generic parameters with trait bounds and lifetime requirements:

```
fn foo<T>(t: T) where T: Clone + Debug { }
```

Each constraint is a type name followed by a colon-separated list of bounds. Bounds can be trait names (requiring implementation of a trait) or lifetime specifiers (requiring a minimum lifetime). The parser collects these constraints into a list attached to the function or type declaration.

## Generic Parameter Parsing

Generic parameters appear in angle brackets and support bounds and default types:

```
fn foo<T: Clone, U: Debug = DefaultType>(...)
```

Each parameter has a name, optional trait bounds (after `:`), and an optional default type (after `=`). The parser handles nested angle brackets correctly — `<Vec<Option<T>>>` is a single generic parameter list with one parameter whose type is a parameterized type.

## Error Handling

The parser logs errors to the `CompilerLog` but continues parsing via recovery strategies:

- **Semicolon insertion**: If a statement is expected but missing a semicolon, the parser can synthesize one and continue
- **Brace matching**: If a closing brace is missing, the parser can skip to the next likely recovery point (next top-level item keyword)
- **Token skipping**: On unrecognized constructs, the parser skips tokens until a known restart point (semicolons, keywords like `fn`/`struct`/`enum`) is found

Each parse error includes the source position (file, line, column), a descriptive error message explaining what was expected versus what was found, and suggestions for correcting the issue where applicable.

## Testing

The parser has an extensive test suite covering:

- **Binary operations**: All operator combinations, precedence, and associativity
- **Enums**: Various enum declarations with and without generics, variants, and default values
- **Expressions**: Literal, unary, binary, call, method call, field access, and index access expressions
- **Functions**: Simple, generic, with where clauses, with attributes, and with various parameter configurations
- **Generics**: Parsing generic parameters and arguments in all positions
- **Impl blocks**: Trait impls, inherent impls, and generic impls
- **Imports**: Various `use` declaration forms including wildcard and multi-name imports
- **Literals**: All literal forms including edge cases
- **Modules**: Module declarations, both file-based and inline
- **Programs**: Multi-item compilation units with all declaration types
- **Structs**: Struct definitions with various field configurations and visibility modifiers
- **Traits**: Trait declarations and method signatures with supertraits
- **Types**: All type expression forms including compound and generic types
- **Unary operations**: All prefix operators
- **Variables**: `let`, `var`, and `static` declarations

Diagnostic format tests verify that error messages are correctly formatted, user-friendly, and include all required information.

## Error Handling and Diagnostic Infrastructure

Parse errors are reported through the `SyntaxErr` enum, which implements `FormattableDiagnosticGroup`. Each variant carries a `SourcePosition` and maps to a unique numeric error code. The diagnostic system accumulates errors across stages rather than aborting at the first failure.

The error handling system uses several patterns to maximize error discovery in a single pass:

- **Missing delimiter insertion** — when a required `}` or `)` is missing, the parser logs the error but continues parsing by skipping to the appropriate recovery point
- **Limit enforcement** — element counts (function parameters, enum variants, struct fields, path segments, etc.) are all checked against `MAX_LIMIT` and reported once per context
- **Brace-matching recovery** — on missing closing braces, the parser skips to the next likely recovery point (next top-level keyword or EOF)

### Why Recursive Descent Over Parser Generators?

Hand-written recursive descent was chosen over parser generators like YACC or ANTLR for four reasons:

1. **Error message quality**: Hand-written parsers produce the best error messages because they know exactly what construct they were attempting to parse at the point of failure. They can say "expected function parameter type" rather than "unexpected token at position 42."
2. **Flexibility**: Complex grammar constructs like operator precedence, context-sensitive keywords, and error recovery are easier to implement manually than to express in a parser-generator grammar file.
3. **Recovery control**: The parser can implement sophisticated error recovery by manipulating the lexer position, skipping to synchronization points, and synthesizing missing tokens.
4. **Dependency reduction**: No external parser generator dependency, which simplifies the build system and avoids potential version compatibility issues.

### Why Precedence Climbing for Operators?

Precedence climbing (a streamlined variant of Pratt parsing) was chosen for binary operators because it provides:

- Clean separation between operator definitions (precedence level, associativity) and the parsing algorithm
- Easy reordering of precedence levels by simply changing numeric values
- Natural support for both left and right associativity
- Linear time complexity with respect to the number of operators
- No need for grammar rewriting or left-recursion elimination

### Why Separate Parse Tree from HIR?

The parse tree (AST) preserves source-level syntax exactly as written by the programmer, while the HIR normalizes and desugars constructs into a simpler, typed representation. This separation provides:

- Simplified parser that only deals with syntactic concerns
- A clear transformation boundary between syntax and semantics
- Multiple lowering strategies for different constructs (the same AST can be lowered differently based on context)
- HIR independence from syntax changes — modifying the language's syntax requires changing only the parser and lowerer, not the entire analysis pipeline
