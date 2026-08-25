# Nitrate: Syntax Highlighting

Syntax highlighting for the [Nitrate](https://github.com/nitrate-lang/nitrate) programming language.

## Features

This extension provides TextMate grammar for `.nit` files that mirrors the complete lexical grammar of the `nitrate_token_lexer` crate:

### Comments

- Line comments: `# ...` and `// ...`
- Block comments: `/* ... */` (multi-line, nested-aware in the compiler)

### Literals

- **Strings**: `"..."` (multi-line capable) with full escape-sequence support:
  - `\0 \a \b \t \n \v \f \r \\ \' \"`
  - Hex: `\xNN` (exactly 2 hex digits, lowercase `x`)
  - Octal: `\oNNN` (exactly 3 octal digits, lowercase `o`)
  - Unicode: `\u{N}` and `\u{U+N}` (1–8 hex digits)
- **Integers**: decimal, binary `0b...`, octal `0o...`, hex `0x...`, and explicit decimal `0d...` — all with underscore digit separators
- **Floats**: `digits.digits` form (e.g. `123.456`, `1_234.567_89`) — note: scientific notation is _not_ a single lexer token

### Keywords

All 64 reserved words: 46 structural/semantic keywords — visibility (`pub`, `sec`, `pro`), modifiers (`safe`, `unsafe`, `promise`, `static`, `mut`, `const`, `poly`, `iso`), control flow (`if`, `else`, `for`, `in`, `while`, `do`, `match`, `break`, `continue`, `ret`, `async`, `await`), special forms (`asm`, `extern`, `use`, `mod`, `typeof`, `as`), and `Self`/`self` — plus 18 primitive type names (`bool`, `u8`–`u128`, `usize`, `i8`–`i128`, `f8`–`f128`, `opaque`).

### Identifiers

- Typical identifiers: Unicode-aware (`[A-Za-z_][A-Za-z0-9_]*` plus any non-ASCII codepoint), matching the lexer's `!is_ascii()` byte-level check
- Atypical identifiers: backtick-delimited `` `...` `` (may span lines and contain reserved words)

### Operators & Punctuation

All single-byte tokens (`' ; , . ( ) { } [ ] @ ~ ? : $ = ! < > - & | + * / ^ %`) plus the multi-token operator sequences recognized by the parser (`<<=`, `>>=`, `<<<`, `>>>`, `&&=`, `||=`, `&&`, `||`, `==`, `!=`, `<=`, `>=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<`, `>>`, `..`, `->`).

## Scope Reference

| Language construct                 | TextMate scope                                                      |
| ---------------------------------- | ------------------------------------------------------------------- |
| Comments                           | `comment.line.nit` / `comment.block.nit`                            |
| Strings                            | `string.quoted.double.nit`, escapes `constant.character.escape.nit` |
| Numbers                            | `constant.numeric.integer.nit` / `constant.numeric.float.nit`       |
| Declarations (`fn`, `struct`, ...) | `storage.type.declaration.nit`                                      |
| Variables (`let`, `var`)           | `storage.type.variable.nit`                                         |
| Modifiers                          | `storage.modifier.nit`                                              |
| Control flow                       | `keyword.control.nit`                                               |
| Primitive types                    | `storage.type.primitive.nit`                                        |
| `Self` / `self`                    | `entity.name.type.nit` / `variable.language.nit`                    |
| Operators                          | `keyword.operator.nit`                                              |
| Punctuation                        | `punctuation.*.nit`                                                 |
| Identifiers                        | `variable.other.nit` / `variable.other.atypical.nit`                |

## License

See [LICENSE.md](LICENSE.md).
