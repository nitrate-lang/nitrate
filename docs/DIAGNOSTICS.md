# Diagnostic and Error Reporting Subsystem

## Theoretical Foundation

Compiler diagnostics are one of the most important aspects of the user experience. A good diagnostic system:

- **Reports all errors**: Doesn't stop at the first error; discovers as many issues as possible in a single pass
- **Provides context**: Shows the relevant source code with precise location markers
- **Suggests fixes**: Where possible, recommends how to correct the issue
- **Categories errors**: Groups related errors for easier understanding
- **Supports multiple output formats**: Human-readable terminal output, machine-readable JSON, LSP integration

Nitrate's diagnostic system follows the structured diagnostic pattern: each error is a typed, structured object that carries all necessary information for presentation, rather than a pre-formatted string.

## Architecture

**Crate**: `nitrate_diagnosis`  
**Key types**: `CompilerLog`, `FormattableDiagnosticGroup`, `DiagnosticGroupId`, `DiagnosticId`, `DiagnosticInfo`, `SourcePosition`, `Span`

## Diagnostic Groups

Each compilation stage has its own diagnostic group:

| Group ID | Name         | Description                        |
| -------- | ------------ | ---------------------------------- |
| 0        | `Scanner`    | File loading and scanning errors   |
| 1        | `Lexical`    | Tokenization errors                |
| 2        | `Syntax`     | Parsing errors                     |
| 3        | `Resolution` | Name resolution errors             |
| 4        | `HIR`        | HIR lowering errors                |
| 5        | `Type`       | Type inference and checking errors |
| 6        | `Semantic`   | Semantic analysis errors           |

## Diagnostic IDs

Each diagnostic has a unique 16-bit identifier encoded as:

```
 GGGG VVVV VVVV VVVV
```

- **4 bits**: Group ID (0-15, currently 0-6 used)
- **12 bits**: Variant ID (0-4095)

The `DiagnosticId` struct encapsulates this encoding:

```rust
pub struct DiagnosticId(pub(crate) u16);

impl DiagnosticId {
    pub const UNKNOWN: Self = DiagnosticId(0xFFFF);

    pub fn new(group_id: DiagnosticGroupId, variant: u16) -> Option<Self> {
        if variant > 0x0FFF || (group_id as u32) > 0x0F {
            return None;
        }
        let id = ((group_id as u32) & 0x0F) << 12 | (variant as u32) & 0xFFF;
        Some(DiagnosticId(id as u16))
    }
}
```

This encoding enables:

- Compact storage (u16 per diagnostic kind)
- Quick group identification (top 4 bits)
- Up to 4096 variants per group
- Error code generation (e.g., `E0001` for group 0 variant 1)

## The FormattableDiagnosticGroup Trait

Every error type in the compiler implements this trait:

```rust
pub trait FormattableDiagnosticGroup {
    fn group_id(&self) -> DiagnosticGroupId;
    fn variant_id(&self) -> u16;
    fn format(&self) -> DiagnosticInfo;
}
```

The `DiagnosticInfo` return value contains:

```rust
pub struct DiagnosticInfo {
    pub origin: Origin,     // Where the error occurred
    pub message: String,    // Human-readable error message
}

pub enum Origin {
    Point(SourcePosition),  // A single source location
    Span(Span),            // A range of source (start + end)
    None,                   // No source location (e.g., internal errors)
}
```

## The CompilerLog

`CompilerLog` is the central error accumulator. It is passed through the compiler pipeline and collects errors from each stage:

```rust
pub struct CompilerLog {
    errors: Vec<Box<dyn FormattableDiagnosticGroup>>,
}
```

The log supports:

- **`log.report(error)`**: Add an error to the log
- **`has_errors()`**: Check if any errors were reported
- **`errors()`**: Iterate over all reported errors
- **`clear()`**: Reset for a new compilation session

Errors are stored as trait objects (`Box<dyn FormattableDiagnosticGroup>`), allowing each stage to define its own error types while maintaining a uniform reporting interface.

## Source Position and Span

### SourcePosition

Identifies a precise location in the source code:

```rust
pub struct SourcePosition {
    pub line: u32,         // 0-based line number
    pub column: u32,       // 0-based column within line
    pub offset: u32,       // 0-based byte offset from file start
    pub fileid: Option<FileId>,  // Which file (None for synthetic positions)
}
```

Display format: `filename:line:column` (with 1-based line/column for user display).

### Span

Represents a range in the source:

```rust
pub struct Span {
    pub start: SourcePosition,
    pub end: SourcePosition,
}
```

Used for underlining the exact extent of an error (e.g., the full expression that has a type mismatch).

## File ID System

The `FileId` type provides interned file identifiers:

```rust
// Intern a file path → get a compact ID
let file_id = intern_file_id("path/to/file.nit");

// The FileId implements Deref<Target = str> for path display
println!("{}", &*file_id);  // "path/to/file.nit"
```

This enables:

- Compact storage (IDs instead of full path strings)
- Fast comparison (integer equality)
- Consistent file naming across all diagnostics

## Error Code Convention

Errors are displayed with error codes in the format `[XNNNN]` where X is a letter and NNNN is a number:

- `[L0000]` - Lexer errors
- `[P0000]` - Parser errors
- `[R0000]` - Resolution errors
- `[H0000]` - HIR errors
- `[T0000]` - Type errors
- `[S0000]` - Semantic errors
- `[E0000]` - General errors

Specific error codes used in the current codebase:

### Lexer Error Codes (L0xxx)

| Code  | Description                          |
| ----- | ------------------------------------ |
| L0000 | Unterminated atypical identifier     |
| L0001 | Invalid UTF-8 in identifier          |
| L0002 | Reserved prefix in identifier        |
| L0100 | Invalid UTF-8 in typical identifier  |
| L0200 | Invalid float literal                |
| L0300 | Integer literal too large for u128   |
| L0301 | Binary literal missing digits        |
| L0302 | Octal literal missing digits         |
| L0303 | Decimal literal missing digits       |
| L0304 | Hex literal missing digits           |
| L0400 | Invalid hex escape                   |
| L0401 | Invalid octal escape                 |
| L0402 | Invalid unicode escape (missing `{`) |
| L0403 | Invalid unicode escape (missing `+`) |
| L0404 | Empty unicode escape                 |
| L0405 | Unicode codepoint too large          |
| L0406 | Missing `}` in unicode escape        |
| L0407 | Invalid escape sequence character    |
| L0408 | Unexpected EOF in string             |
| L0600 | Invalid UTF-8 in comment             |
| L0601 | Unterminated block comment           |
| L0602 | Invalid UTF-8 in block comment       |
| L0700 | Invalid token character              |

### Type Error Codes (T0xxx)

| Code  | Description                                |
| ----- | ------------------------------------------ |
| T0001 | Integer literal unsatisfiable              |
| T0002 | Integer literal out of range               |
| T0003 | Float literal unsatisfiable                |
| T0004 | Integer literal outside refinement bounds  |
| T0005 | Operation result outside refinement bounds |

## Error Types by Stage

### Lexer Errors (`nitrate_token_lexer`)

Defined in the lexer implementation. Each error logs a formatted message via `log::error!` with the pattern:

```
[LXXXX]: description
--> filename:line:column
```

### Parser Errors (`nitrate_tree_parse`)

Defined in `nitrate_tree_parse::diagnosis`. Covers syntax errors like:

- Expected token not found
- Unexpected token
- Malformed construct (e.g., malformed generic parameters)

### Resolution Errors (`nitrate_tree_resolve`)

Defined in `nitrate_tree_resolve::diagnosis`. Covers:

- Unknown name
- Ambiguous name
- Cyclic import
- Module not found
- Visibility violation

### Type Errors (`nitrate_hir_solve`)

Defined in `nitrate_hir_solve::diagnosis`. Covers type inference errors as enumerated above.

### HIR Validation Errors (`nitrate_hir_validate`)

Defined in `nitrate_hir_validate::diagnosis`. Covers:

- Invalid function signature
- Mismatched types
- Invalid expression
- Invalid struct/enum definition

## Error Recovery Strategy

The compiler follows a **fail-soft** approach:

1. **Lexer**: On invalid input, logs an error and returns `Token::Eof` for that position. Continues scanning from the next valid position.

2. **Parser**: On syntax errors, skips tokens until a synchronization point is found (semicolons, braces, keywords like `fn`/`struct`/`enum`).

3. **Solver**: Accumulates all type errors in a `HashSet` (deduplicated). Reports all errors at once rather than stopping at the first.

4. **Validator**: Continues validation after discovering an error, checking as many aspects as possible.

5. **Codegen**: Panics on invalid LLVM IR (hard failure — invalid IR cannot produce correct machine code).

## Error Display

When the compiler encounters errors, it produces output like:

```
[E0300]: Integer literal is too large to fit in u128
--> src/main.nit:10:5
  |
9 | let x = 999999999999999999999999999999999999999999999999999999999999999999999999999999999999;
  |         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
```

The display format:

1. Error code in brackets
2. Human-readable error message
3. Source location (`filename:line:column`)
4. Context lines from source
5. Caret (`^`) underline pointing to the exact position

## Error Codes JSON

The `docs/error_codes.json` file provides a machine-readable catalog of all error codes:

```json
{
    "L0001": {
        "message": "Identifier contains invalid UTF-8",
        "stage": "Lexical",
        "description": "Raised when an identifier contains bytes that are not valid UTF-8."
    },
    ...
}
```

This enables:

- Error code lookup in documentation
- LSP integration for hover information
- Automated error code verification

## Design Decisions

### Why Trait Objects Instead of an Error Enum?

Using `Box<dyn FormattableDiagnosticGroup>` instead of a global `Error` enum:

1. **Decentralized error definitions**: Each crate defines its own error types without a central enum that all crates must depend on
2. **No exhaustive matching**: Adding a new error type doesn't require updating match statements across the compiler
3. **Encapsulation**: Error types can carry stage-specific data without exposing it to other stages
4. **Extensibility**: New passes can define errors without modifying core diagnostic infrastructure

### Why Structured Errors Instead of Formatted Strings?

Structured errors (typed objects) are superior to pre-formatted strings because:

1. **Multiple output formats**: The same error can be rendered as terminal text, JSON, or LSP diagnostic
2. **Precise locations**: Structured source positions enable editor integration (click-to-navigate)
3. **Machine readability**: LSP and CI tools can parse structured errors for automated processing
4. **Localization potential**: Structured errors can be rendered in different languages

### Why Error Accumulation Instead of Abort-on-First-Error?

Accumulating all errors before reporting provides:

1. **Faster edit-compile-debug cycles**: Developers see all issues at once, not one at a time
2. **Batch fixes**: Multiple errors can be fixed in one edit session
3. **LSP integration**: Real-time error display works best with all errors visible
4. **CI efficiency**: Running the compiler once finds all issues, not just the first
