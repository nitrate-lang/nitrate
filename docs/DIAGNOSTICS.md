# Diagnostic and Error Reporting Subsystem

## Theoretical Foundation

Compiler diagnostics are one of the most important aspects of the user experience — they are the primary interface between the compiler and the programmer when something goes wrong. A well-designed diagnostic system can dramatically reduce debugging time by providing clear, contextual, and actionable error messages.

A good diagnostic system must accomplish five goals. First, it must **report all errors** without stopping at the first one, discovering as many issues as possible in a single compilation pass. Second, it must **provide context** by showing the relevant source code with precise location markers so the programmer can immediately see what code triggered the error. Third, it should **suggest fixes** where possible — recommending corrections rather than just describing the problem. Fourth, it should **categorize errors** into logical groups for easier understanding and filtering. Fifth, it must **support multiple output formats** — human-readable terminal output for command-line compilation, machine-readable JSON for CI and tooling integration, and structured diagnostics for LSP integration in editors.

Nitrate's diagnostic system follows the structured diagnostic pattern: each error is a typed, structured object that carries all necessary information for presentation — source location, error code, human-readable message, and any additional context data — rather than a pre-formatted string. This design enables flexible rendering across different output formats and machine processing for editor integration.

## Architecture

**Crate**: `nitrate_diagnosis`  
**Key types**: `CompilerLog`, `FormattableDiagnosticGroup`, `DiagnosticGroupId`, `DiagnosticId`, `DiagnosticInfo`, `SourcePosition`, `Span`

The diagnostic system is designed around a few core abstractions that work together to provide comprehensive error reporting.

### Diagnostic Groups

Each compilation stage has its own diagnostic group, ensuring clear ownership of error definitions and enabling quick identification of which compilation phase produced a given error:

| Group ID | Name       | Description                                                                | Error Code Prefix |
| -------- | ---------- | -------------------------------------------------------------------------- | ----------------- |
| 0        | Scanner    | File loading and scanning errors                                           | `[S0...]`         |
| 1        | Lexical    | Tokenization errors — invalid characters, malformed literals               | `[L0...]`         |
| 2        | Syntax     | Parsing errors — grammar violations, missing tokens                        | `[P0...]`         |
| 3        | Resolution | Name resolution errors — unknown names, cyclic imports                     | `[R0...]`         |
| 4        | HIR        | HIR lowering errors — invalid type expressions                             | `[H0...]`         |
| 5        | Type       | Type inference and checking errors — mismatches, unsatisfiable constraints | `[T0...]`         |
| 6        | Semantic   | Semantic analysis errors — borrow checker, control flow                    | `[S0...]`         |

### Diagnostic IDs

Each diagnostic has a unique 16-bit identifier: 4 bits for the group ID (0-15) and 12 bits for the variant ID (0-4095). This encoding enables compact storage (a single `u16` per diagnostic kind), quick group identification (just mask the top 4 bits), up to 4096 distinct error variants per group, and clean error code generation in the format `[XNNNN]`.

### The FormattableDiagnosticGroup Trait

Every error type in the compiler implements this trait to provide a uniform interface for error rendering:

```rust
pub trait FormattableDiagnosticGroup {
    fn group_id(&self) -> DiagnosticGroupId;
    fn variant_id(&self) -> u16;
    fn format(&self) -> DiagnosticInfo;
}
```

The `DiagnosticInfo` return value contains:

- `origin`: Where the error occurred — either a precise `SourcePosition` (point), a `Span` (range), or `None` (for internal errors without source context)
- `message`: The human-readable error message describing what went wrong

### The CompilerLog

`CompilerLog` is the central error accumulator passed through the entire compilation pipeline. It stores errors as trait objects (`Box<dyn FormattableDiagnosticGroup>`), allowing each stage to define its own error types while maintaining a uniform reporting interface. The `CompilerLog` supports adding errors via `report(error)`, checking for errors via `has_errors()`, iterating over all errors via `errors()`, and resetting for a new compilation session via `clear()`.

### Source Position and Span

`SourcePosition` identifies a precise location in the source code with line (0-based), column (0-based), byte offset, and an optional `FileId` identifying the source file. `Span` represents a range from a start position to an end position, used for underlining the exact extent of an error — for example, the full expression that has a type mismatch rather than just a single character.

## Error Display Format

Errors are displayed with error codes, source location, and context lines. The format includes the error code in brackets for quick reference, a human-readable message, the file and location in `filename:line:column` format, and context lines from the source with a caret underline pointing to the exact position:

```
[E0300]: Integer literal is too large to fit in u128
--> src/main.nit:10:5
  |
9 | let x = 999999999999999999999999999999999999999999999999999999999999999999;
  |         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
```

This display format maximizes the information density of each error message: the error code allows quick lookup in documentation, the source location enables editor integration (click-to-navigate), the message explains the problem, and the context lines with caret show exactly which part of the code triggered the error.

## Error Recovery Strategy

The compiler follows a fail-soft approach across all stages. The **lexer** logs invalid input and returns `Token::Eof` for that position, continuing from the next valid position. The **parser** skips tokens until synchronization points (semicolons, braces, keywords like `fn`/`struct`/`enum`) are found. The **solver** accumulates type errors in a `HashSet` for deduplication, reporting all errors at once. The **validator** continues checking after discovering an error. Only **codegen** panics on invalid LLVM IR — this is a hard failure because invalid IR cannot produce correct machine code.

## Design Rationale

**Trait objects instead of a global error enum** provides decentralized error definitions (each crate defines its own error types without depending on a central enum), no exhaustive matching (adding a new error type doesn't require updating match statements across the compiler), encapsulation (error types carry stage-specific data without exposing it to other stages), and extensibility (new passes can define errors without modifying core diagnostic infrastructure).

**Structured errors instead of formatted strings** enable multiple output formats (the same error can render as terminal text, JSON, or LSP diagnostic), precise source locations for editor integration (click-to-navigate in IDEs), machine readability for CI tools (automated error parsing), and potential localization.

**Error accumulation instead of abort-on-first-error** provides faster edit-compile-debug cycles (developers see all issues at once rather than discovering them one at a time), enables batch fixes (multiple errors can be fixed in one edit session), supports effective LSP integration (real-time error display with all diagnostics visible), and improves CI efficiency (running the compiler once finds all issues).
