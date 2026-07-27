# Lexical Analysis Subsystem

## Theoretical Foundation

Lexical analysis (scanning/tokenization) is the first phase of compilation. It transforms a raw sequence of source code characters (bytes) into a structured stream of tokens — the smallest meaningful units of the language. The lexer must handle:

- **Character classification**: Determining which bytes form identifiers, numbers, operators, etc.
- **Token recognition**: Mapping character sequences to token types
- **Literal parsing**: Converting numeric/string representations to internal values
- **Whitespace and comment handling** (trivia): Recognizing and optionally preserving non-significant characters
- **Error recovery**: Producing useful diagnostics for malformed input

Nitrate's lexer follows the classic maximal munch principle: at each step, it consumes the longest possible valid token from the current position.

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

The lexer borrows the source bytes as a `&[u8]` slice, maintaining:

- **`internal_getc_pos`**: The current scanning position in the source (the "read cursor")
- **`current_pos`**: The position after the last consumed token (used by the parser for error reporting)
- **`preread_token`**: One-token lookahead buffer for `peek_tok()` without advancing
- **`trivia_enabled`**: Controls whether whitespace, newlines, and comments are emitted as tokens

### Position Tracking

The `SourcePosition` struct tracks position as:

- **`line`**: 0-based line number
- **`column`**: 0-based column within the line
- **`offset`**: 0-based byte offset from start of source
- **`fileid`**: Optional `FileId` identifying the source file

The lexer carefully tracks UTF-8 multi-byte sequences: continuation bytes (0x80-0xBF) do not increment the column counter, ensuring column numbers align with Unicode code points, not bytes.

### AnnotatedToken

Each token carries its exact source location:

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

This enables precise error messages that point to the exact source location of any token.

## Token Categories

The `Token` enum (defined in `nitrate_token`) encompasses all lexical elements:

### Keywords (52 tokens)

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
| `SelfType` | `Self`     | `SelfKeyword` | `self`     |          |          |

### Type Keywords (19 tokens)

Numeric type names are keywords, not identifiers: `u8`, `u16`, `u32`, `u64`, `u128`, `usize`, `i8`, `i16`, `i32`, `i64`, `i128`, `f8`, `f16`, `f32`, `f64`, `f128`, `opaque`.

### Punctuation/Operators (26 tokens)

Single-character tokens: `'`, `;`, `,`, `.`, `(`, `)`, `{`, `}`, `[`, `]`, `@`, `~`, `?`, `:`, `$`, `=`, `!`, `<`, `>`, `-`, `&`, `|`, `+`, `*`, `/`, `^`, `%`.

### Trivia Tokens (6 tokens)

Whitespace characters are emitted as explicit tokens when trivia mode is enabled: `HorizontalTab` (`\t`), `NewLine` (`\n`), `VerticalTab` (`\x0b`), `FormFeed` (`\x0c`), `CarriageReturn` (`\r`), `Space` (` `).

### Literal Tokens

- **Integer**: `Integer { value: u128, kind: IntegerKind }` — stores the raw value and radix (Bin, Oct, Dec, Hex)
- **Float**: `Float(NotNan<f64>)` — stores the parsed float value via `ordered_float::NotNan` (rejects NaN)
- **String**: `String(String)` — UTF-8 string, fully processed with escape sequences
- **BString**: `BString(Vec<u8>)` — byte string (non-UTF-8), escape-processed
- **Comment**: `Comment { text: String, kind: CommentKind }` — stores comment text and kind (SingleLine, MultiLine)
- **Name**: `Name(String)` — user-defined identifiers

## Lexing Algorithm

### Entry Point

`parse_next_token()` dispatches based on the first byte of the next token:

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

### Identifier Parsing

**Typical identifiers** (`parse_typical_identifier`): Start with an alphabetic character, underscore, or non-ASCII byte, then consume a run of alphanumeric + underscore + non-ASCII characters. The result is first checked against the keyword table; if no match, it becomes a `Token::Name`.

Keywords are matched by exact byte comparison against a compiled list. The match order matters: longer keywords must be checked before shorter ones that are prefixes (e.g., `struct` is checked before any potential shorter match).

**Atypical identifiers** (`parse_atypical_identifier`): Backtick-delimited names enabling reserved words as identifiers:

```
`some keyword`  → Name("some keyword")
```

This is similar to Rust's `r#` raw identifiers. Atypical identifiers are useful for FFI where external symbols may have names that clash with keywords.

### Integer Literal Parsing

`parse_number()` handles all numeric literals:

```
Start with a digit:
  1. Read all ASCII digits and underscores
  2. If the literal is exactly "0":
     a. Check the next byte for radix prefix:
        'b' → binary (base 2) — digits: 0, 1
        'o' → octal (base 8) — digits: 0-7
        'd' → decimal (base 10) — digits: 0-9
        'x' → hexadecimal (base 16) — digits: 0-9, a-f, A-F
     b. If no radix prefix, continue as decimal
  3. If no radix was found and the next char is '.' followed by a digit,
     parse as float literal
  4. Decode the digit string using `radix_decode()`, which:
     - Skips underscore separators
     - Checks for overflow beyond u128
     - Returns the value or logs an error
```

**Error cases**:

- `Integer literal is too large to fit in u128` (E0300)
- `Binary integer literal must contain at least one digit after '0b'` (E0301)
- `Octal literal must contain at least one digit after '0o'` (E0302)
- `Decimal literal must contain at least one digit after '0d'` (E0303)
- `Hexadecimal literal must contain at least one digit after '0x'` (E0304)

### Float Literal Parsing

`parse_float()` is called when a decimal integer is followed by `.digit`:

1. Save the position and advance past the `.`
2. If the next character is a digit, read all digits and underscores
3. Parse the complete float representation with `convert_float_repr()`
4. Underscores are stripped before parsing to `f64`
5. If the float parse fails, log error E0200

Note: The lexer currently supports only `f64` floats. Float literal types (f32, f64) are distinguished during the type inference phase, not during lexing.

### String Literal Parsing

`parse_string()` handles the full complexity of string and byte string literals:

```
Consume opening '"':
  buffer = []
  loop:
    match peek:
      '\\' → parse escape sequence:
        \0 → null byte
        \a → alert (0x07)
        \b → backspace (0x08)
        \t → tab (0x09)
        \n → newline (0x0A)
        \v → vertical tab (0x0B)
        \f → form feed (0x0C)
        \r → carriage return (0x0D)
        \\ → backslash
        \' → single quote
        \" → double quote (as char)
        \xNN → hex escape (2 hex digits)
        \oNNN → octal escape (3 octal digits)
        \u{N} or \u{U+N} → unicode escape (1-8 hex digits)
      '"' → end of string:
        if buffer empty → Token::String(utf8) or Token::BString(bytes)
        else → String::from_utf8(buffer) → Token::String or Token::BString
      EOF → error
      other → append to buffer
```

The lexer uses a "lazy allocation" optimization: if no escape sequences are encountered, the string is returned as a borrow into the source bytes. If any escape is encountered, a dynamic buffer is allocated.

### Comment Parsing

Three comment forms are supported:

1. **Line comments** (`#` or `//`): From the marker to the end of the line (or to `\r` before newline)
2. **Block comments** (`/* */`): Nested-aware parsing; tracks `/*` and matches with `*/`

Line comments use the `#` character (similar to Python and shell scripts), with `//` as an alternative (familiar from C/Rust/JavaScript).

Block comments handle arbitrary nesting by counting `/*` entries and `*/` exits, ensuring proper matching even with embedded `/*` in comment text.

### Operator/Punctuation Parsing

`parse_single_byte()` handles all single-character tokens. Each valid byte maps directly to a token variant. Invalid bytes produce a diagnostic: "The token `X` is not valid."

The slash `/` has special handling: it may be a division operator, a line comment (`//`), or the start of a block comment (`/*`). The `parse_slash_or_comment()` function peeks ahead to determine which case applies.

## Trivia Management

The lexer supports two modes:

- **Trivia enabled** (`enable_trivia()`): Whitespace, newlines, and comments are emitted as tokens. Used for pretty-printing, source formatting tools, and LSP features.
- **Trivia disabled** (`disable_trivia()`): Whitespace and comments are silently skipped. Used during normal parsing. This is the default after construction.

When trivia is disabled, `parse_next_token()` recursively skips any trivia tokens it encounters, effectively filtering them from the stream.

## Lexer API

### Core Methods

- **`next_tok()`**: Returns the next token and advances the lexer position
- **`peek_tok()`**: Returns the next token without consuming it (one-token lookahead)
- **`skip_tok()`**: Advances past the next token without returning it
- **`rewind(pos)`**: Resets the lexer to a previous `SourcePosition`
- **`is_eof()`**: Returns true if the next token is `Token::Eof`

### Convenience Methods

- **`next_is(token)`**: Check if the next token matches without consuming
- **`skip_if(token)`**: Skip the next token if it matches, return boolean
- **`skip_while(not)`**: Skip tokens until a specific token is found
- **`next_if_name()`**: Return the name string if the next token is `Name`, `SelfType`, or `SelfKeyword`
- **`current_pos()`**: Return the position after the last consumed token
- **`peek_pos()`**: Return the position of the next token

### Iterator Interface

The `LexerIterator` wrapper implements `Iterator<Item = AnnotatedToken>`, stopping before `Token::Eof`. This enables idiomatic Rust iteration:

```rust
let lexer = Lexer::new(source, fileid)?;
for token in LexerIterator::new(lexer) {
    // process token
}
```

## Source Size Limit

The lexer enforces a maximum source size of 4 GiB (u32::MAX bytes). This limit is derived from the `SourcePosition::offset` field being a `u32`. In test configurations, the limit is reduced to 4096 bytes to catch testing mistakes early.

## Reserved Prefix

Identifiers starting with `⚙️` (U+2699 U+FE0F) are reserved for compiler-generated names. If user code attempts to use such identifiers, the lexer produces error codes L0002. This reserved space ensures no collisions between user code and compiler-internal symbols.

## Error Handling and Recovery

The lexer uses `log::error!` macros to record diagnostics but does not abort on errors. Instead, it produces `Token::Eof` for invalid tokens and continues scanning. The error codes follow a systematic numbering:

- **L0000-L0099**: General lexer errors (atypical identifiers, reserved prefixes)
- **L0100-L0199**: Identifier errors (invalid UTF-8)
- **L0200-L0299**: Float literal errors
- **L0300-L0399**: Integer literal errors
- **L0400-L0499**: String literal errors
- **L0600-L0699**: Comment errors
- **L0700-L0799**: Invalid token errors

## Design Decisions

### Why `u8` Slice Instead of `char` Iterator?

The lexer operates on raw bytes (`&[u8]`) rather than decoded `char`s to:

1. **Handle invalid UTF-8 gracefully**: Byte strings and certain literals may contain non-UTF-8 sequences
2. **Maximum performance**: Byte operations are simpler and faster than Unicode-aware iteration
3. **Flexible encoding**: The lexer doesn't assume UTF-8 for the entire file (though identifiers are validated as UTF-8)

### Why Explicit Trivia Tokens?

Rather than stripping trivia during lexing (the traditional approach), Nitrate's lexer can optionally emit trivia as explicit tokens. This enables:

- **Pretty-printing**: Whitespace and comments can be faithfully reproduced
- **LSP features**: Whitespace-aware code actions and formatting
- **Source-level tooling**: The token stream preserves all source information

### Why Both `#` and `//` Comments?

Supporting both comment styles serves different communities:

- `#` is familiar to Python, shell script, and Ruby programmers
- `//` and `/* */` are familiar to C, Rust, JavaScript, and Java programmers
- `/* */` block comments enable multi-line commenting and temporary code disabling

### Why `AnnotatedToken` Instead of Simple Token + Position Pair?

Each `AnnotatedToken` carries both start and end positions, enabling:

- **Precise error ranges**: Errors can underline the exact offending tokens
- **Automatic code fixes**: The LSP can replace exact token ranges
- **Source mapping**: Debug information maps generated code to source ranges
