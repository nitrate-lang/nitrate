use super::token::{AnnotatedToken, Comment, CommentKind, Integer, IntegerKind, Token};
use crate::SourcePosition;
use log::error;
use nitrate_diagnosis::FileId;
use ordered_float::NotNan;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LexerError {
    SourceTooBig,
}

#[derive(Debug)]
pub struct Lexer<'a> {
    source: &'a [u8],
    internal_getc_pos: SourcePosition,
    current_pos: SourcePosition,
    preread_token: Option<AnnotatedToken>,
}

enum StringEscape {
    Char(char),
    Byte(u8),
}

// Must not be increased beyond u32::MAX, as the lexer/compiler pipeline
// assumes that offsets are representable as u32 values. However, it is
// acceptable to decrease this value.
#[cfg(not(test))]
const MAX_SOURCE_SIZE: usize = u32::MAX as usize;
#[cfg(test)]
const MAX_SOURCE_SIZE: usize = 4096 as usize;

impl<'a> Lexer<'a> {
    /// Creates a new lexer instance from the given source code and filename.
    /// The source code must not exceed 4 GiB in size.
    /// # Errors
    /// Returns `LexerError::SourceTooBig` if the source code exceeds 4 GiB in size.
    pub fn new(src: &'a [u8], fileid: Option<FileId>) -> Result<Self, LexerError> {
        if src.len() > MAX_SOURCE_SIZE {
            Err(LexerError::SourceTooBig)
        } else {
            Ok(Lexer {
                source: src,
                internal_getc_pos: SourcePosition {
                    line: 0,
                    column: 0,
                    offset: 0,
                    fileid: fileid.clone(),
                },
                current_pos: SourcePosition {
                    line: 0,
                    column: 0,
                    offset: 0,
                    fileid: fileid.clone(),
                },
                preread_token: None,
            })
        }
    }

    pub fn next_tok(&mut self) -> AnnotatedToken {
        if let Some(peeked) = self.preread_token.take() {
            self.current_pos = self.internal_getc_pos.clone();
            return peeked;
        }

        let token = self.parse_next_token();
        self.current_pos = self.internal_getc_pos.clone();

        token
    }

    pub fn peek_tok(&mut self) -> AnnotatedToken {
        if let Some(peeked) = self.preread_token.clone() {
            return peeked;
        }

        let peeked = self.parse_next_token();
        self.preread_token = Some(peeked.clone());

        peeked
    }

    pub fn skip_tok(&mut self) {
        if self.preread_token.is_some() {
            self.preread_token = None;
        } else {
            self.parse_next_token();
        }

        self.current_pos = self.internal_getc_pos.clone();
    }

    pub fn modify_next_tok(&mut self, token: Token) {
        let mut peeked = self.peek_tok();
        peeked.token = token;

        self.preread_token = Some(peeked);
    }

    #[inline(always)]
    pub fn next_t(&mut self) -> Token {
        self.next_tok().into_token()
    }

    #[inline(always)]
    #[must_use]
    pub fn peek_t(&mut self) -> Token {
        self.peek_tok().into_token()
    }

    #[inline(always)]
    #[must_use]
    pub fn next_is(&mut self, matches: &Token) -> bool {
        &self.peek_t() == matches
    }

    #[inline(always)]
    pub fn skip_if(&mut self, matches: &Token) -> bool {
        if &self.peek_t() == matches {
            self.skip_tok();
            true
        } else {
            false
        }
    }

    pub fn skip_while(&mut self, not: &Token) {
        while !self.is_eof() && &self.next_t() != not {}
    }

    /// Returns the end of the last consumed token
    #[inline(always)]
    #[must_use]
    pub fn current_pos(&self) -> SourcePosition {
        self.current_pos.clone()
    }

    #[inline(always)]
    #[must_use]
    pub fn peek_pos(&mut self) -> SourcePosition {
        self.peek_tok().start()
    }

    #[inline(always)]
    #[must_use]
    pub fn is_eof(&mut self) -> bool {
        self.peek_t() == Token::Eof
    }

    #[inline(always)]
    pub fn rewind(&mut self, pos: SourcePosition) {
        self.internal_getc_pos = pos.clone();
        self.current_pos = pos;
        self.preread_token = None;
    }

    #[inline(always)]
    pub fn next_if_string(&mut self) -> Option<String> {
        if let Token::String(string_data) = self.peek_t() {
            self.skip_tok();
            Some(string_data)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_bstring(&mut self) -> Option<Vec<u8>> {
        if let Token::BString(bstring_data) = self.peek_t() {
            self.skip_tok();
            Some(bstring_data)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_name(&mut self) -> Option<String> {
        if let Token::Name(name) = self.peek_t() {
            self.skip_tok();
            Some(name)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_integer(&mut self) -> Option<Integer> {
        if let Token::Integer(integer) = self.peek_t() {
            self.skip_tok();
            Some(integer)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_float(&mut self) -> Option<NotNan<f64>> {
        if let Token::Float(float) = self.peek_t() {
            self.skip_tok();
            Some(float)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_comment(&mut self) -> Option<Comment> {
        if let Token::Comment(comment) = self.peek_t() {
            self.skip_tok();
            Some(comment)
        } else {
            None
        }
    }

    #[inline(always)]
    fn advance(&mut self, byte: u8) -> u8 {
        let current = self.internal_getc_pos.clone();

        if byte == b'\n' {
            self.internal_getc_pos = SourcePosition {
                line: current.line + 1,
                column: 0,
                offset: current.offset + 1,
                fileid: current.fileid.clone(),
            };
        } else {
            self.internal_getc_pos = SourcePosition {
                line: current.line,
                column: current.column + 1,
                offset: current.offset + 1,
                fileid: current.fileid.clone(),
            };
        }

        byte
    }

    #[inline(always)]
    #[must_use]
    fn peek_byte(&self) -> Result<u8, ()> {
        self.source
            .get(self.internal_getc_pos.offset as usize)
            .copied()
            .ok_or(())
    }

    #[inline(always)]
    fn read_while<F>(&mut self, mut condition: F) -> &'a [u8]
    where
        F: FnMut(u8) -> bool,
    {
        let start_offset = self.internal_getc_pos.offset;
        let mut end_offset = start_offset;

        while let Some(b) = self.source.get(end_offset as usize) {
            if condition(*b) {
                self.advance(*b);
                end_offset += 1;
            } else {
                break;
            }
        }

        &self.source[start_offset as usize..end_offset as usize]
    }

    #[inline(always)]
    fn parse_atypical_identifier(&mut self) -> Result<Token, ()> {
        let start_pos = self.internal_getc_pos.clone();

        assert!(self.peek_byte().expect("Failed to peek byte") == b'`');
        self.advance(b'`');

        let identifier = self.read_while(|b| b != b'`');

        if let Ok(b'`') = self.peek_byte() {
            self.advance(b'`');
        } else {
            error!(
                "[L0000]: Unterminated atypical identifier. Did you forget the '`' terminator?\n--> {start_pos}"
            );
            return Err(());
        }

        if let Ok(identifier) = str::from_utf8(identifier) {
            Ok(Token::Name(identifier.to_string()))
        } else {
            error!("[L0001]: Identifier contains some invalid utf-8 bytes\n--> {start_pos}");

            Err(())
        }
    }

    #[inline(always)]
    fn parse_typical_identifier(&mut self) -> Result<Token, ()> {
        let start_pos = self.internal_getc_pos.clone();

        let name = self.read_while(|b| b.is_ascii_alphanumeric() || b == b'_' || !b.is_ascii());
        assert!(!name.is_empty(), "Identifier should not be empty");

        if let Some(keyword) = match name {
            b"let" => Some(Token::Let),
            b"var" => Some(Token::Var),
            b"fn" => Some(Token::Fn),
            b"enum" => Some(Token::Enum),
            b"struct" => Some(Token::Struct),
            b"class" => Some(Token::Class),
            b"union" => Some(Token::Union),
            b"contract" => Some(Token::Contract),
            b"trait" => Some(Token::Trait),
            b"impl" => Some(Token::Impl),
            b"type" => Some(Token::Type),
            b"scope" => Some(Token::Scope),
            b"import" => Some(Token::Import),
            b"mod" => Some(Token::Mod),
            b"safe" => Some(Token::Safe),
            b"unsafe" => Some(Token::Unsafe),
            b"promise" => Some(Token::Promise),
            b"static" => Some(Token::Static),
            b"mut" => Some(Token::Mut),
            b"const" => Some(Token::Const),
            b"poly" => Some(Token::Poly),
            b"iso" => Some(Token::Iso),
            b"pub" => Some(Token::Pub),
            b"sec" => Some(Token::Sec),
            b"pro" => Some(Token::Pro),
            b"if" => Some(Token::If),
            b"else" => Some(Token::Else),
            b"for" => Some(Token::For),
            b"in" => Some(Token::In),
            b"while" => Some(Token::While),
            b"do" => Some(Token::Do),
            b"switch" => Some(Token::Switch),
            b"break" => Some(Token::Break),
            b"continue" => Some(Token::Continue),
            b"ret" => Some(Token::Ret),
            b"async" => Some(Token::Async),
            b"await" => Some(Token::Await),
            b"asm" => Some(Token::Asm),
            b"null" => Some(Token::Null),
            b"true" => Some(Token::True),
            b"false" => Some(Token::False),
            b"bool" => Some(Token::Bool),
            b"u8" => Some(Token::U8),
            b"u16" => Some(Token::U16),
            b"u32" => Some(Token::U32),
            b"u64" => Some(Token::U64),
            b"u128" => Some(Token::U128),
            b"i8" => Some(Token::I8),
            b"i16" => Some(Token::I16),
            b"i32" => Some(Token::I32),
            b"i64" => Some(Token::I64),
            b"i128" => Some(Token::I128),
            b"f8" => Some(Token::F8),
            b"f16" => Some(Token::F16),
            b"f32" => Some(Token::F32),
            b"f64" => Some(Token::F64),
            b"f128" => Some(Token::F128),
            b"opaque" => Some(Token::Opaque),
            b"as" => Some(Token::As),
            b"typeof" => Some(Token::Typeof),

            _ => None,
        } {
            Ok(keyword)
        } else if let Ok(identifier) = str::from_utf8(name) {
            Ok(Token::Name(identifier.to_string()))
        } else {
            error!("[L0100]: Identifier contains some invalid utf-8 bytes\n--> {start_pos}");

            Err(())
        }
    }

    #[inline(always)]
    fn convert_float_repr(str_bytes: &str) -> Result<NotNan<f64>, ()> {
        match str_bytes.replace('_', "").parse::<f64>() {
            Ok(value) => Ok(NotNan::new(value).unwrap()),
            Err(e) => {
                error!("[L0200]: Invalid float literal: {e}");
                Err(())
            }
        }
    }

    #[inline(always)]
    fn parse_float(&mut self, start_pos: &SourcePosition) -> Result<Token, ()> {
        if let Ok(b'.') = self.peek_byte() {
            let rewind_pos = self.internal_getc_pos.clone();
            self.advance(b'.');

            match self.peek_byte() {
                Ok(b) if b.is_ascii_digit() => {
                    self.read_while(|b| b.is_ascii_digit() || b == b'_');

                    let literal = str::from_utf8(
                        &self.source
                            [start_pos.offset as usize..self.internal_getc_pos.offset as usize],
                    )
                    .expect("Failed to convert float literal to str");

                    if let Ok(result) = Self::convert_float_repr(literal) {
                        return Ok(Token::Float(result));
                    }
                }
                _ => {
                    self.rewind(rewind_pos);
                }
            }
        }

        Err(())
    }

    #[inline(always)]
    fn radix_decode(digits: &[u8], base: u32, start_pos: &SourcePosition) -> Result<u128, ()> {
        let mut number = 0u128;

        for digit in digits {
            if digit == &b'_' {
                continue;
            }

            if let Ok(digit) = u128::from_str_radix(
                str::from_utf8(&[*digit]).expect("Unexpected non-utf8 digit"),
                base,
            ) {
                if let Some(y) = number.checked_mul(u128::from(base)) {
                    if let Some(sum) = y.checked_add(digit) {
                        number = sum;
                        continue;
                    }
                }
            }

            error!("[L0300]: Integer literal is too large to fit in u128\n--> {start_pos}");
            return Err(());
        }

        Ok(number)
    }

    #[inline(always)]
    fn parse_number(&mut self) -> Result<Token, ()> {
        let start_pos = self.internal_getc_pos.clone();

        let mut base_prefix = None;
        let mut literal = self.read_while(|b| b.is_ascii_digit() || b == b'_');
        assert!(!literal.is_empty(), "Number should not be empty");

        if literal == b"0" {
            match self.peek_byte() {
                Ok(b'b') => {
                    self.advance(b'b');
                    base_prefix = Some(2);

                    literal = self.read_while(|b| b == b'0' || b == b'1' || b == b'_');
                    if literal.is_empty() {
                        error!(
                            "[L0301]: Binary integer literal must contain at least one digit after '0b'\n--> {start_pos}"
                        );
                        return Err(());
                    }
                }

                Ok(b'o') => {
                    self.advance(b'o');
                    base_prefix = Some(8);

                    literal = self.read_while(|b| (b'0'..=b'7').contains(&b) || b == b'_');
                    if literal.is_empty() {
                        error!(
                            "[L0302]: Octal literal must contain at least one digit after '0o'\n--> {start_pos}"
                        );
                        return Err(());
                    }
                }

                Ok(b'd') => {
                    self.advance(b'd');
                    base_prefix = Some(10);

                    literal = self.read_while(|b| b.is_ascii_digit() || b == b'_');
                    if literal.is_empty() {
                        error!(
                            "[L0303]: Decimal literal must contain at least one digit after '0d'\n--> {start_pos}"
                        );
                        return Err(());
                    }
                }

                Ok(b'x') => {
                    self.advance(b'x');
                    base_prefix = Some(16);

                    literal = self.read_while(|b| b.is_ascii_hexdigit() || b == b'_');
                    if literal.is_empty() {
                        error!(
                            "[L0304]: Hexadecimal literal must contain at least one digit after '0x'\n--> {start_pos}"
                        );
                        return Err(());
                    }
                }

                _ => {}
            }
        }

        if base_prefix.is_none() {
            if let Ok(float) = self.parse_float(&start_pos) {
                return Ok(float);
            }
        }

        let number = Self::radix_decode(literal, base_prefix.unwrap_or(10u32), &start_pos)?;

        Ok(Token::Integer(Integer::new(
            number,
            match base_prefix {
                Some(2) => IntegerKind::Bin,
                Some(8) => IntegerKind::Oct,
                Some(16) => IntegerKind::Hex,
                Some(10) | None => IntegerKind::Dec,

                _ => unreachable!(),
            },
        )))
    }

    #[inline(always)]
    fn parse_string_hex_escape(&mut self, start_pos: &SourcePosition) -> Result<StringEscape, ()> {
        let mut digits = [0u8; 2];

        for i in 0..2 {
            let byte = self.peek_byte()?;

            if byte.is_ascii_hexdigit() {
                self.advance(byte);
                digits[i] = byte;
            } else {
                error!(
                    "[L0400]: Invalid hex escape sequence '\\x{}' in string literal. Expected two hex digits (0-9, a-f, A-F) after '\\x'.\n--> {}",
                    str::from_utf8(&digits[..=i]).unwrap_or("<invalid utf-8>"),
                    start_pos
                );

                return Err(());
            }
        }

        let mut value = 0u8;
        for digit in digits {
            let digit = digit.to_ascii_lowercase();

            if digit.is_ascii_digit() {
                value = (value << 4) | (digit - b'0');
            } else {
                value = (value << 4) | (digit - b'a' + 10);
            }
        }

        Ok(StringEscape::Byte(value))
    }

    #[inline(always)]
    fn parse_string_octal_escape(
        &mut self,
        start_pos: &SourcePosition,
    ) -> Result<StringEscape, ()> {
        let mut digits = [0u8; 3];

        for i in 0..3 {
            let byte = self.peek_byte()?;

            if (b'0'..=b'7').contains(&byte) {
                self.advance(byte);
                digits[i] = byte;
            } else {
                error!(
                    "[L0401]: Invalid octal escape sequence '\\o{}' in string literal. Expected three octal digits (0-7) after '\\o'.\n--> {}",
                    str::from_utf8(&digits).unwrap_or("<invalid utf-8>"),
                    start_pos
                );

                return Err(());
            }
        }

        let mut value = 0u8;
        for &digit in &digits {
            value = (value << 3) | (digit - b'0');
        }

        Ok(StringEscape::Byte(value))
    }

    #[inline(always)]
    fn parse_string_unicode_escape(
        &mut self,
        start_pos: &SourcePosition,
    ) -> Result<StringEscape, ()> {
        if self.peek_byte()? != b'{' {
            error!(
                "[L0402]: Invalid unicode escape in string literal. Expected '{{' after '\\u'.\n--> {start_pos}"
            );
            return Err(());
        }
        self.advance(b'{');

        if self.peek_byte()? == b'U' {
            self.advance(b'U');

            if self.peek_byte()? == b'+' {
                self.advance(b'+');
            } else {
                error!(
                    "[L0403]: Invalid unicode escape in string literal. Expected '+' after '\\uU'.\n--> {start_pos}"
                );
                return Err(());
            }
        }

        let digits = self.read_while(|b| b.is_ascii_hexdigit());
        if digits.is_empty() {
            error!(
                "[L0404]: Invalid unicode escape in string literal. Expected at least one hex digit after '\\u{{'.\n--> {start_pos}"
            );
            return Err(());
        }

        if digits.len() > 8 {
            error!(
                "[L0405]: Unicode escape codepoint in string literal is too large: '\\u{{{}}}'.\n--> {}",
                str::from_utf8(digits).unwrap_or("<invalid utf-8>"),
                start_pos
            );
            return Err(());
        }

        let mut value = 0u32;
        for &digit in digits {
            let digit = digit.to_ascii_lowercase();

            if digit.is_ascii_digit() {
                value = (value << 4) | u32::from(digit - b'0');
            } else {
                value = (value << 4) | u32::from(digit - b'a' + 10);
            }
        }

        let codepoint = char::from_u32(value);
        if codepoint.is_none() {
            error!(
                "[L0405]: Unicode escape codepoint in string literal is too large: '\\u{{{}}}'.\n--> {}",
                str::from_utf8(digits).unwrap_or("<invalid utf-8>"),
                start_pos
            );

            return Err(());
        }

        if self.peek_byte()? != b'}' {
            error!(
                "[L0406]: Invalid unicode escape in string literal. Expected '}}' after '\\u{{'.\n--> {start_pos}"
            );
            return Err(());
        }
        self.advance(b'}');

        codepoint.map(StringEscape::Char).ok_or(())
    }

    #[inline(always)]
    fn parse_string_escape(&mut self, start_pos: &SourcePosition) -> Result<StringEscape, ()> {
        match self.peek_byte() {
            Ok(b'0') => {
                self.advance(b'0');
                Ok(StringEscape::Byte(b'\0'))
            }
            Ok(b'a') => {
                self.advance(b'a');
                Ok(StringEscape::Byte(b'\x07'))
            }
            Ok(b'b') => {
                self.advance(b'b');
                Ok(StringEscape::Byte(b'\x08'))
            }
            Ok(b't') => {
                self.advance(b't');
                Ok(StringEscape::Byte(b'\t'))
            }
            Ok(b'n') => {
                self.advance(b'n');
                Ok(StringEscape::Byte(b'\n'))
            }
            Ok(b'v') => {
                self.advance(b'v');
                Ok(StringEscape::Byte(b'\x0b'))
            }
            Ok(b'f') => {
                self.advance(b'f');
                Ok(StringEscape::Byte(b'\x0c'))
            }
            Ok(b'r') => {
                self.advance(b'r');
                Ok(StringEscape::Byte(b'\r'))
            }
            Ok(b'\\') => {
                self.advance(b'\\');
                Ok(StringEscape::Byte(b'\\'))
            }
            Ok(b'\'') => {
                self.advance(b'\'');
                Ok(StringEscape::Byte(b'\''))
            }
            Ok(b'"') => {
                self.advance(b'"');
                Ok(StringEscape::Char('"'))
            }

            Ok(b'x') => {
                self.advance(b'x');
                self.parse_string_hex_escape(start_pos)
            }

            Ok(b'o') => {
                self.advance(b'o');
                self.parse_string_octal_escape(start_pos)
            }

            Ok(b'u') => {
                self.advance(b'u');
                self.parse_string_unicode_escape(start_pos)
            }

            Ok(b) => {
                error!(
                    "[L0407]: Invalid escape sequence '\\{}' in string literal\n--> {}",
                    b as char, start_pos
                );

                Err(())
            }

            Err(()) => {
                error!(
                    "[L0408]: Unexpected end of input while parsing string literal\n--> {start_pos}"
                );
                Err(())
            }
        }
    }

    #[inline(always)]
    fn parse_string(&mut self) -> Result<Token, ()> {
        let start_pos = self.internal_getc_pos.clone();

        assert!(self.peek_byte().expect("Failed to peek byte") == b'"');
        self.advance(b'"');

        let start_offset = self.internal_getc_pos.offset;
        let mut end_offset = start_offset;
        let mut storage = Vec::new();

        loop {
            match self.peek_byte() {
                Ok(b'\\') => {
                    self.advance(b'\\');

                    if storage.is_empty() {
                        storage.extend_from_slice(
                            &self.source[start_offset as usize..end_offset as usize],
                        );
                    }

                    match self.parse_string_escape(&start_pos) {
                        Ok(StringEscape::Char(c)) => {
                            storage.extend_from_slice(c.to_string().as_bytes());
                        }

                        Ok(StringEscape::Byte(b)) => {
                            storage.push(b);
                        }

                        Err(()) => {
                            return Err(());
                        }
                    }

                    assert!(
                        !storage.is_empty(),
                        "Dynamic string buffer should not be empty after parsing escape sequence"
                    );
                }

                Ok(b'"') => {
                    self.advance(b'"');

                    if storage.is_empty() {
                        let buffer = &self.source[start_offset as usize..end_offset as usize];

                        if let Ok(utf8_str) = str::from_utf8(buffer) {
                            return Ok(Token::String(utf8_str.to_string()));
                        }
                        return Ok(Token::BString(buffer.to_vec()));
                    } else if let Ok(utf8_str) = String::from_utf8(storage.to_vec()) {
                        return Ok(Token::String(utf8_str.to_string()));
                    }
                    return Ok(Token::BString(storage.to_vec()));
                }

                Ok(b) => {
                    self.advance(b);
                    if storage.is_empty() {
                        end_offset += 1;
                    } else {
                        storage.push(b);
                    }
                }

                Err(()) => {
                    error!(
                        "[L0408]: Unexpected end of input while parsing string literal\n--> {start_pos}"
                    );
                    return Err(());
                }
            }
        }
    }

    #[inline(always)]
    fn parse_comment(&mut self) -> Result<Token, ()> {
        let start_pos = self.internal_getc_pos.clone();
        let mut comment_bytes = self.read_while(|b| b != b'\n');

        if comment_bytes.ends_with(b"\r") {
            comment_bytes = &comment_bytes[..comment_bytes.len() - 1];
        }

        if let Ok(comment) = str::from_utf8(comment_bytes) {
            Ok(Token::Comment(Comment::new(
                comment.to_string(),
                CommentKind::SingleLine,
            )))
        } else {
            error!(
                "[L0600]: Single-line comment contains some invalid utf-8 bytes\n--> {start_pos}"
            );

            Err(())
        }
    }

    #[inline(always)]
    fn parse_operator_or_punctuation(&mut self) -> Result<Token, ()> {
        let start_pos = self.internal_getc_pos.clone();

        let b = self.peek_byte()?;
        let token = match b {
            b'\'' => Some(Token::SingleQuote),
            b';' => Some(Token::Semi),
            b',' => Some(Token::Comma),
            b'.' => Some(Token::Dot),
            b'(' => Some(Token::OpenParen),
            b')' => Some(Token::CloseParen),
            b'{' => Some(Token::OpenBrace),
            b'}' => Some(Token::CloseBrace),
            b'[' => Some(Token::OpenBracket),
            b']' => Some(Token::CloseBracket),
            b'@' => Some(Token::At),
            b'~' => Some(Token::Tilde),
            b'?' => Some(Token::Question),
            b':' => Some(Token::Colon),
            b'$' => Some(Token::Dollar),
            b'=' => Some(Token::Eq),
            b'!' => Some(Token::Bang),
            b'<' => Some(Token::Lt),
            b'>' => Some(Token::Gt),
            b'-' => Some(Token::Minus),
            b'&' => Some(Token::And),
            b'|' => Some(Token::Or),
            b'+' => Some(Token::Plus),
            b'*' => Some(Token::Star),
            b'/' => Some(Token::Slash),
            b'^' => Some(Token::Caret),
            b'%' => Some(Token::Percent),
            _ => None,
        };

        if let Some(token) = token {
            self.advance(self.peek_byte()?);
            return Ok(token);
        }

        error!(
            "[L0700]: The token `{}` is not valid. Did you mistype an operator or forget some whitespace?\n--> {}",
            str::from_utf8(&[b]).unwrap_or("<invalid utf-8>"),
            start_pos
        );

        Err(())
    }

    #[inline(always)]
    fn parse_next_token(&mut self) -> AnnotatedToken {
        self.read_while(|b| b.is_ascii_whitespace());

        let start_pos = self.internal_getc_pos.clone();

        let token = match self.peek_byte() {
            Err(()) => {
              Ok(Token::Eof)
            },
            Ok(b) => match b {
                b'`' => self.parse_atypical_identifier(),
                b if b.is_ascii_alphabetic() || b == b'_' || !b.is_ascii() /* Support UTF-8 identifiers */ => {
                    self.parse_typical_identifier()
                }
                b if b.is_ascii_digit() => self.parse_number(),
                b'"' => self.parse_string(),
                b'#' => self.parse_comment(),
                _ => self.parse_operator_or_punctuation(),
            },
        }
        .unwrap_or(Token::Eof);

        let end_pos = self.internal_getc_pos.clone();

        AnnotatedToken::new(token, start_pos, end_pos)
    }
}
