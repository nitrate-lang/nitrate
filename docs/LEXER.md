# Lexical Analysis Subsystem

## Theoretical Foundation

Lexical analysis (scanning or tokenization) is the first phase of compilation. It transforms a raw sequence of source code characters (bytes) into a structured stream of tokens — the smallest meaningful units of the language. The lexer must handle several fundamental tasks that collectively define the boundary between the raw text of a source file and the structured representation that subsequent compilation phases operate on.

**Character classification** determines which bytes form identifiers, numbers, operators, string literals, and other token categories. This requires understanding the language's lexical grammar — the rules that define valid character sequences for each token type. **Token recognition** maps these character sequences to specific token types in the `Token` enum. **Literal parsing** converts numeric and string representations from their textual form into internal value representations suitable for the compiler's type system. **Whitespace and comment handling** (collectively called trivia) recognizes non-significant characters that separate tokens but carry no semantic meaning — and optionally preserves them for tooling purposes. **Error recovery** ensures that malformed input produces useful diagnostics rather than cryptic failures or compiler crashes.

Nitrate's lexer follows the classic **maximal munch** principle: at each step, it consumes the longest possible valid token from the current position. This ensures unambiguous tokenization — for example, `>>` is always a shift-right operator token rather than two separate `>` comparison tokens, because the two-character sequence is the longest valid match.

## Architecture

**Crate**: `nitrate_token_lexer`  
**Dependencies**: `nitrate_token` (token type definitions), `nitrate_diagnosis` (source positions, error reporting)  
**Key types**: `Lexer<'a>`, `AnnotatedToken`, `SourcePosition`, `Token`

### The Lexer Struct

```rust
pub struct Lexer<'a> {
    source: &'a [u8],
    internal_getc_pos: SourcePosition,
    current_pos: SourcePosition,
    preread_token: Option<AnnotatedToken>,
    trivia_enabled: bool,
}
```

The lexer borrows the source bytes as a `&[u8]` slice, maintaining several pieces of internal state:

- **`internal_getc_pos`**: The current scanning position in the source — the "read cursor" that advances as characters are consumed during token formation
- **`current_pos`**: The position after the last consumed token — used by the parser for error reporting when it needs to know where a parse failure occurred
- **`preread_token`**: A one-token lookahead buffer that enables `peek_tok()` to examine the next token without advancing the lexer position
- **`trivia_enabled`**: A boolean flag controlling whether whitespace, newlines, and comments are emitted as explicit tokens or silently skipped

### Position Tracking

The `SourcePosition` struct tracks position using four fields:

- **`line`**: 0-based line number within the source file
- **`column`**: 0-based column offset within the current line
- **`offset`**: 0-based byte offset from the start of the source
- **`fileid`**: An optional `FileId` identifying which source file this position refers to

Position tracking must account for UTF-8 multi-byte sequences: continuation bytes in the range 0x80-0xBF do not increment the column counter, ensuring that column numbers align with Unicode code points rather than raw bytes. This means the column number directly corresponds to the character position visible to the user in their editor, regardless of whether characters are ASCII (1 byte) or multi-byte Unicode (2-4 bytes).

### AnnotatedToken

Each token carries its exact source location for precise error reporting:

```rust
pub struct AnnotatedToken {
    pub token: Token,
    pub start_line: u32,
    pub start_column: u32,
    pub start_offset: u32,
    pub end_line: u32,
    pub end_column: u32,
    pub end_offset: u32,
    pub fileid: Option<FileId>,
}
```

The combination of start and end positions enables the error reporting system to underline the exact range of the offending token, rather than just pointing to a single character. For example, a malformed string literal can be highlighted from its opening quote to its last scanned character, making the extent of the error immediately visible.

## Token Categories

The `Token` enum (defined in `nitrate_token`) encompasses all lexical elements of the Nitrate language.

### Keywords (54 tokens)

Nitrate reserves 54 keywords that cannot be used as identifiers (except through the atypical identifier mechanism described below):

| Token      | Keyword    | Token         | Keyword    | Token    | Keyword  |
| ---------- | ---------- | ------------- | ---------- | -------- | -------- |
| `Let`      | `let`      | `Var`         | `var`      | `Fn`     | `fn`     |
| `Enum`     | `enum`     | `Struct`      | `struct`   | `Class`  | `class`  |
| `Union`    | `union`    | `Contract`    | `contract` | `Trait`  | `trait`  |
| `Impl`     | `impl`     | `Type`        | `type`     | `Scope`  | `scope`  |
| `Use`      | `use`      | `Mod`         | `mod`      | `Safe`   | `safe`   |
| `Unsafe`   | `unsafe`   | `Promise`     | `promise`  | `Static` | `static` |
| `Mut`      | `mut`      | `Const`       | `const`    | `Poly`   | `poly`   |
| `Iso`      | `iso`      | `Pub`         | `pub`      | `Sec`    | `sec`    |
| `Pro`      | `pro`      | `If`          | `if`       | `Else`   | `else`   |
| `For`      | `for`      | `In`          | `in`       | `While`  | `while`  |
| `Do`       | `do`       | `Match`       | `match`    | `Break`  | `break`  |
| `Continue` | `continue` | `Ret`         | `ret`      | `Async`  | `async`  |
| `Await`    | `await`    | `Asm`         | `asm`      | `Null`   | `null`   |
| `True`     | `true`     | `False`       | `false`    | `Bool`   | `bool`   |
| `As`       | `as`       | `Typeof`      | `typeof`   | `Extern` | `extern` |
| `SelfType` | `Self`     | `SelfKeyword` | `self`     | `Super`  | `super`  |
| `Crate`    | `crate`    |               |            |          |          |

The keyword set includes control flow (`if`, `else`, `while`, `for`, `match`, `break`, `continue`, `ret`, `loop`), type system (`struct`, `enum`, `trait`, `impl`, `type`, `union`, `contract`, `class`, `scope`), visibility (`pub`, `sec`, `pro`), memory management (`let`, `var`, `mut`, `const`, `static`), safety (`safe`, `unsafe`), concurrency (`async`, `await`), and special constructs (`asm`, `extern`, `use`, `mod`, `promise`).

### Type Keywords (19 tokens)

Numeric type names are keywords rather than identifiers: `u8`, `u16`, `u32`, `u64`, `u128`, `usize`, `i8`, `i16`, `i32`, `i64`, `i128`, `f8`, `f16`, `f32`, `f64`, `f128`, `opaque`. This design choice means that type names cannot be accidentally shadowed by variable declarations, and the lexer can immediately recognize type annotations without consulting the symbol table.

### Punctuation and Operators (26 tokens)

Single-character punctuation and operator tokens include: `'`, `;`, `,`, `.`, `(`, `)`, `{`, `}`, `[`, `]`, `@`, `~`, `?`, `:`, `$`, `=`, `!`, `<`, `>`, `-`, `&`, `|`, `+`, `*`, `/`, `^`, `%`. Multi-character operators like `>>`, `<<`, `&&`, `||`, `==`, `!=`, `<=`, `>=`, `->`, `**` are recognized by the parser after receiving single-character tokens, not by the lexer itself — the lexer uses maximal munch for multi-byte sequences but does not combine operators.

### Trivia Tokens (6 tokens)

Whitespace characters are emitted as explicit tokens only when trivia mode is enabled: `HorizontalTab` (`\t`), `NewLine` (`\n`), `VerticalTab` (`\x0b`), `FormFeed` (`\x0c`), `CarriageReturn` (`\r`), `Space` (` `).

### Literal Tokens

Literals carry their parsed values within the token:

- **Integer**: `Integer { value: u128, kind: IntegerKind }` — stores the decoded value and radix indication (Bin, Oct, Dec, Hex)
- **Float**: `Float(NotNan<f64>)` — stores the parsed float value using `ordered_float::NotNan`, which rejects NaN at the token level
- **String**: `String(String)` — a fully processed UTF-8 string with all escape sequences resolved
- **BString**: `BString(Vec<u8>)` — a byte string (potentially non-UTF-8) with escape sequences processed
- **Comment**: `Comment { text: String, kind: CommentKind }` — stores the comment text and whether it was single-line or multi-line
- **Name**: `Name(String)` — user-defined identifiers that are not keywords

## Lexing Algorithm

### Entry Point

The core method `parse_next_token()` dispatches based on the first byte of the upcoming token:

```
peek_byte → dispatch:
  '`'        → parse_atypical_identifier
  alpha/_    → parse_typical_identifier
  digit      → parse_number
  '"'        → parse_string
  '#'        → parse_comment
  '/'        → parse_slash_or_comment
  other      → parse_single_byte
  EOF        → Token::Eof
```

This first-byte dispatch strategy is efficient because it uses a simple byte comparison or small lookup table to determine which specialized parser to invoke, rather than attempting to match patterns sequentially.

### Identifier Parsing

**Typical identifiers** (`parse_typical_identifier`): Start with an alphabetic character, underscore, or non-ASCII byte, then consume a run of alphanumeric + underscore + non-ASCII characters. The resulting string is first checked against the keyword table; if no match is found, it becomes a `Token::Name`.

Keyword matching uses exact byte comparison against a compiled list of keyword strings. The match order matters: longer keywords must be checked before shorter ones that are prefixes. For example, `struct` is checked before any shorter match that might begin with `str`, ensuring that the full keyword is recognized rather than a partial match.

**Atypical identifiers** (`parse_atypical_identifier`): Backtick-delimited names enable the use of reserved words as identifiers:

```
`some keyword`  → Name("some keyword")
```

This mechanism is similar to Rust's `r#` raw identifiers. Atypical identifiers are essential for FFI scenarios where external library symbols may have names that collide with Nitrate keywords. The backtick syntax is visually distinct, making it clear in source code that a keyword is being used as an identifier.

### Integer Literal Parsing

`parse_number()` handles all numeric literals through a multi-step process:

```
Start with a digit:
  1. Read all ASCII digits and underscores into a buffer
  2. If the literal is exactly "0":
     a. Check the next byte for a radix prefix:
        'b' → binary (base 2) — valid digits: 0, 1
        'o' → octal (base 8) — valid digits: 0-7
        'd' → decimal (base 10) — valid digits: 0-9
        'x' → hexadecimal (base 16) — valid digits: 0-9, a-f, A-F
     b. If no radix prefix, continue as decimal
  3. If no radix was found and the next char is '.' followed by a digit,
     parse as float literal
  4. Decode the digit string using `radix_decode()`, which:
     - Strips underscore separators (used for visual grouping, e.g., 1_000_000)
     - Checks for overflow beyond u128 (the maximum integer width)
     - Returns the decoded value or logs an error
```

**Error cases** include literal overflow (`Integer literal is too large to fit in u128`, code L0300) and missing digits after radix prefix (`Binary integer literal must contain at least one digit after '0b'`, codes L0301-L0304).

### Float Literal Parsing

`parse_float()` is called when a decimal integer is followed by `.` and a digit:

1. Save the current scan position and advance past the `.`
2. If the next character is a digit, read all digits and underscores for the fractional part
3. Parse the complete float representation using `convert_float_repr()`
4. Underscores are stripped before parsing to `f64`
5. If the float parse fails (e.g., the string is not a valid float representation), log error code L0200

The lexer currently produces only `f64` tokens for all float literals. Distinguishing between `f32` and `f64` types is deferred to the type inference phase, where the solver resolves `InferredFloat` variables based on usage context.

### String Literal Parsing

`parse_string()` handles the full complexity of string and byte string literals, processing escape sequences into their actual byte values:

```
Consume opening '"':
  buffer = []
  loop:
    match peek:
      '\\' → parse escape sequence:
        \0 → null byte (0x00)
        \a → alert (0x07)
        \b → backspace (0x08)
        \t → tab (0x09)
        \n → newline (0x0A)
        \v → vertical tab (0x0B)
        \f → form feed (0x0C)
        \r → carriage return (0x0D)
        \\ → backslash (0x5C)
        \' → single quote (0x27)
        \" → double quote (0x22)
        \xNN → hex escape — exactly 2 hex digits
        \oNNN → octal escape — exactly 3 octal digits
        \u{N} or \u{U+N} → unicode escape — 1 to 8 hex digits
      '"' → end of string — return buffer as String or BString
      EOF → unterminated string error
      other → append byte to buffer
```

The lexer uses a **lazy allocation** optimization: if no escape sequences are encountered in the string, the result is returned as a direct borrow into the source bytes, avoiding any heap allocation. If any escape sequence is encountered, a dynamic buffer is allocated and the escape processing proceeds.

### Comment Parsing

Three comment forms are supported:

1. **Line comments** (`#` or `//`): From the comment marker to the end of the current line (up to the `\r` before a newline or the end of the file)
2. **Block comments** (`/* */`): Nested-aware parsing that tracks `/*` entries and matches them with corresponding `*/` exits

Line comments use `#` (similar to Python and shell scripts) with `//` as an alternative (familiar from C, Rust, and JavaScript). Block comments handle arbitrary nesting by counting `/*` entries and `*/` exits, ensuring proper matching even when block comments contain embedded `/*` sequences in their text.

### Operator and Punctuation Parsing

`parse_single_byte()` handles all single-character tokens. Each valid byte value maps directly to a token variant through a lookup structure. Invalid bytes that don't correspond to any valid token produce a diagnostic: "The token `X` is not valid."

The slash `/` requires special handling because it has three possible interpretations: a division operator (`/`), a line comment (`//`), or the start of a block comment (`/*`). The `parse_slash_or_comment()` function peeks ahead at the next byte to determine which case applies.

## Trivia Management

The lexer supports two modes controlled by `enable_trivia()` and `disable_trivia()`:

- **Trivia enabled**: Whitespace, newlines, and comments are emitted as explicit token variants in the token stream. This mode is used for pretty-printing, source formatting tools, and LSP features where preserving the original layout is important.
- **Trivia disabled** (default): Whitespace and comments are silently skipped during scanning. When trivia is disabled, `parse_next_token()` recursively skips any trivia tokens it encounters, effectively filtering them from the stream. This is the mode used during normal parsing.

The default state after construction has trivia disabled, since the parser does not need whitespace information to build the AST.

## Lexer API

### Core Methods

- **`next_tok()`**: Returns the next token from the stream and advances the lexer position past it. This is the primary method for consuming tokens.
- **`peek_tok()`**: Returns the next token without consuming it, using the one-token lookahead buffer (`preread_token`). This enables the parser to examine upcoming tokens without committing to consuming them.
- **`skip_tok()`**: Advances past the next token without returning it. Useful when the parser determines that a token should be skipped as part of error recovery.
- **`rewind(pos)`**: Resets the lexer to a previous `SourcePosition`. This is used during error recovery when the parser needs to back up and try an alternative parse strategy.
- **`is_eof()`**: Returns true if the next unconsumed token is `Token::Eof`.

### Convenience Methods

- **`next_is(token)`**: Checks if the next token matches a specific token variant without consuming it. Implemented by calling `peek_tok()` and comparing.
- **`skip_if(token)`**: Consumes the next token if it matches a specific variant, returning a boolean indicating whether the skip occurred.
- **`skip_while(not)`**: Consumes tokens until a specific token is found, skipping all intervening tokens.
- **`next_if_name()`**: Returns the identifier string if the next token is `Name`, `SelfType`, or `SelfKeyword` — handling the common case of parsing a name reference.
- **`current_pos()`**: Returns the `SourcePosition` after the last consumed token, useful for error reporting at the current parse position.
- **`peek_pos()`**: Returns the `SourcePosition` of the next unconsumed token.

### Iterator Interface

The `LexerIterator` wrapper implements `Iterator<Item = AnnotatedToken>`, halting before `Token::Eof`. This enables idiomatic Rust iteration over the token stream:

```rust
let lexer = Lexer::new(source, fileid)?;
for token in LexerIterator::new(lexer) {
    // process each token as it arrives
}
```

## Source Size Limit

The lexer enforces a maximum source size of 4 GiB (`u32::MAX` bytes). This limit derives from the `SourcePosition::offset` field being a `u32`, which cannot represent offsets beyond 4 GiB. In test configurations, the limit is reduced to 4096 bytes to catch testing mistakes early (e.g., accidentally passing a file path instead of file contents).

## Reserved Prefix

Identifiers starting with `⚙️` (U+2699 + U+FE0F, the "gear" emoji as a variation sequence) are reserved for compiler-generated internal names. If user code attempts to use such identifiers, the lexer produces error code L0002. This reserved space ensures no collisions between user-declared names and compiler-internal symbols, particularly during monomorphization and code generation where the compiler needs unique internal names.

## Error Handling and Recovery

The lexer uses `log::error!` macros to record diagnostics but does not abort on errors. When it encounters an invalid byte sequence, it produces `Token::Eof` for the problematic position and continues scanning from the next valid position. This fail-soft approach ensures that the parser can proceed past lexical errors and discover additional issues.

Error codes follow a systematic numbering scheme:

- **L0000-L0099**: General lexer errors (atypical identifiers, reserved prefixes)
- **L0100-L0199**: Identifier errors (invalid UTF-8 in identifiers)
- **L0200-L0299**: Float literal errors (malformed float representations)
- **L0300-L0399**: Integer literal errors (overflow, missing digits after radix)
- **L0400-L0499**: String literal errors (invalid escapes, unterminated strings)
- **L0600-L0699**: Comment errors (unterminated block comments, invalid UTF-8)
- **L0700-L0799**: Invalid token errors (bytes that don't form any valid token)

## Design Decisions

### Why `u8` Slice Instead of `char` Iterator?

The lexer operates on raw bytes (`&[u8]`) rather than decoded `char` values for three reasons:

1. **Handling invalid UTF-8 gracefully**: Byte strings and certain literal content may contain non-UTF-8 byte sequences. Operating on raw bytes preserves the ability to represent and process these sequences.
2. **Maximum performance**: Byte comparisons and increment operations are significantly simpler and faster than Unicode-aware character iteration, which requires decoding multi-byte sequences at each step.
3. **Encoding flexibility**: The lexer does not assume the entire file is valid UTF-8. Identifiers are validated as UTF-8 separately, but the rest of the file can contain arbitrary bytes.

### Why Explicit Trivia Tokens?

Rather than stripping trivia during lexing (the traditional approach in many compilers), Nitrate's lexer can optionally emit trivia as explicit tokens. This design serves:

- **Pretty-printing**: Whitespace and comments can be faithfully reproduced in formatted output
- **LSP features**: Whitespace-aware code actions, formatting, and refactoring
- **Source-level tooling**: The token stream preserves all source information, enabling tools that need to understand the original formatting

### Why Both `#` and `//` Comments?

Supporting two line comment styles serves different developer communities:

- `#` is familiar to Python, shell script, Ruby, and many configuration language programmers
- `//` and `/* */` are familiar to C, C++, Rust, JavaScript, Java, and C# programmers
- `/* */` block comments enable multi-line commenting and temporary code disabling in a way that line comments cannot

### Why `AnnotatedToken` Instead of Simple Token + Position Pair?

Each `AnnotatedToken` carries both start and end positions, rather than just a single position. This enables three critical use cases:

- **Precise error ranges**: Errors can underline the exact offending tokens, from start to end, rather than pointing at a single character
- **Automatic code fixes**: The LSP can suggest replacements that target exact token ranges
- **Source mapping**: Debug information maps generated code spans back to their original source ranges
