use log::error;
use nitrate_diagnosis::FileId;
use nitrate_token::{AnnotatedToken, Comment, CommentKind, Integer, IntegerKind, LexPos, Token};
use ordered_float::NotNan;

const RESERVED_PREFIX: &str = "⚙️";

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum LexerError {
    SourceTooBig,
}

#[derive(Debug)]
pub struct Lexer<'a> {
    pub source: &'a [u8],
    internal_getc_pos: LexPos,
    current_pos: LexPos,
    preread_token: Option<AnnotatedToken>,
    trivia_enabled: bool,
}

enum StringEscape {
    Char(char),
    Byte(u8),
}

#[cfg(not(test))]
const MAX_SOURCE_SIZE: usize = u32::MAX as usize;
#[cfg(test)]
const MAX_SOURCE_SIZE: usize = 4096;

impl<'a> Lexer<'a> {
    pub fn new(src: &'a [u8], fileid: Option<FileId>) -> Result<Self, LexerError> {
        if src.len() > MAX_SOURCE_SIZE {
            Err(LexerError::SourceTooBig)
        } else {
            Ok(Lexer {
                source: src,
                internal_getc_pos: LexPos {
                    fileid: fileid.clone(),
                    line: 0,
                    column: 0,
                    offset: 0,
                },
                current_pos: LexPos {
                    fileid: fileid.clone(),
                    line: 0,
                    column: 0,
                    offset: 0,
                },
                preread_token: None,
                trivia_enabled: true,
            })
        }
    }

    pub fn enable_trivia(&mut self) {
        self.trivia_enabled = true;
    }
    pub fn disable_trivia(&mut self) {
        self.trivia_enabled = false;
        // If a trivia token (comment/whitespace/newline) was already peeked
        // while trivia was enabled, drop it. Otherwise the next peek would
        // return the stale trivia token instead of re-scanning with trivia
        // skipping active (e.g. `parse_source` peeks to compute the module
        // span before disabling trivia).
        if let Some(peeked) = &self.preread_token
            && peeked.token.is_trivia()
        {
            self.preread_token = None;
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

    #[inline(always)]
    #[must_use]
    pub fn next_is(&mut self, matches: &Token) -> bool {
        &self.peek_tok().token == matches
    }

    #[inline(always)]
    pub fn skip_if(&mut self, matches: &Token) -> bool {
        if &self.peek_tok().token == matches {
            self.skip_tok();
            true
        } else {
            false
        }
    }

    pub fn skip_while(&mut self, not: &Token) {
        while !self.is_eof() && &self.next_tok().token != not {}
    }

    #[inline(always)]
    #[must_use]
    pub fn current_pos(&self) -> LexPos {
        self.current_pos.clone()
    }

    /// Returns the start position of the next token without consuming it.
    #[inline(always)]
    #[must_use]
    pub fn peek_pos(&mut self) -> LexPos {
        let tok = self.peek_tok();
        LexPos {
            fileid: tok.fileid,
            line: tok.start_line,
            column: tok.start_column,
            offset: tok.start_offset,
        }
    }

    #[inline(always)]
    #[must_use]
    pub fn is_eof(&mut self) -> bool {
        self.peek_tok().token == Token::Eof
    }

    /// Rewind the lexer to a previously saved position.
    #[inline(always)]
    pub fn rewind(&mut self, pos: LexPos) {
        self.internal_getc_pos = pos.clone();
        self.current_pos = pos;
        self.preread_token = None;
    }

    /// Rewind to a position specified by raw tuple (fileid, line, column, offset).
    #[inline(always)]
    pub fn rewind_raw(&mut self, (fileid, line, column, offset): (Option<FileId>, u16, u8, u32)) {
        self.rewind(LexPos {
            fileid,
            line,
            column,
            offset,
        });
    }

    /// Rewind to a previously saved lexer state. The saved value is a
    /// `(fileid, line, column, offset)` tuple from `current_pos` or `peek_pos`.
    #[inline(always)]
    pub fn rewind_saved(&mut self, saved: (Option<FileId>, u16, u8, u32)) {
        self.rewind_raw(saved);
    }

    /// Update the internal rewind functions.
    fn rewind_to_internal(&mut self, fileid: Option<FileId>, line: u16, column: u8, offset: u32) {
        self.internal_getc_pos = LexPos {
            fileid: fileid.clone(),
            line,
            column,
            offset,
        };
        self.current_pos = LexPos {
            fileid,
            line,
            column,
            offset,
        };
        self.preread_token = None;
    }

    #[inline(always)]
    pub fn next_if_name(&mut self) -> Option<String> {
        match &self.peek_tok().token {
            Token::Name(name) => {
                self.skip_tok();
                Some(name.clone())
            }
            Token::SelfType => {
                self.skip_tok();
                Some("Self".to_string())
            }
            Token::SelfKeyword => {
                self.skip_tok();
                Some("self".to_string())
            }
            Token::Super => {
                self.skip_tok();
                Some("super".to_string())
            }
            Token::Crate => {
                self.skip_tok();
                Some("crate".to_string())
            }
            _ => None,
        }
    }

    #[inline(always)]
    fn advance(&mut self, byte: u8) -> u8 {
        let current = self.internal_getc_pos.clone();
        if byte == b'\n' {
            self.internal_getc_pos = LexPos {
                line: current.line.saturating_add(1),
                column: 0,
                offset: current.offset.saturating_add(1),
                fileid: current.fileid,
            };
        } else {
            let utf8_end = (byte & 0x80) == 0 || (byte & 0xC0) == 0xC0;
            self.internal_getc_pos = LexPos {
                line: current.line,
                column: if utf8_end {
                    current.column.saturating_add(1)
                } else {
                    current.column
                },
                offset: current.offset.saturating_add(1),
                fileid: current.fileid,
            };
        }
        byte
    }

    #[inline(always)]
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
            error!("[L0000]: Unterminated atypical identifier.");
            return Err(());
        }
        if let Ok(identifier) = str::from_utf8(identifier) {
            if identifier.starts_with(RESERVED_PREFIX) {
                error!("[L0002]: Identifiers starting with '{RESERVED_PREFIX}' are reserved.");
                return Err(());
            }
            Ok(Token::Name(identifier.to_string()))
        } else {
            error!("[L0001]: Identifier contains invalid utf-8");
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
            b"use" => Some(Token::Use),
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
            b"match" => Some(Token::Match),
            b"break" => Some(Token::Break),
            b"continue" => Some(Token::Continue),
            b"ret" => Some(Token::Ret),
            b"async" => Some(Token::Async),
            b"await" => Some(Token::Await),
            b"extern" => Some(Token::Extern),
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
            b"usize" => Some(Token::USize),
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
            b"Self" => Some(Token::SelfType),
            b"self" => Some(Token::SelfKeyword),
            b"super" => Some(Token::Super),
            b"crate" => Some(Token::Crate),
            _ => None,
        } {
            Ok(keyword)
        } else if let Ok(identifier) = str::from_utf8(name) {
            if identifier.starts_with(RESERVED_PREFIX) {
                error!("[L0002]: Reserved prefix.");
                Err(())
            } else {
                Ok(Token::Name(identifier.to_string()))
            }
        } else {
            error!("[L0100]: Identifier invalid utf-8.");
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
    fn parse_float(&mut self, whole_start: u32) -> Result<Token, ()> {
        if let Ok(b'.') = self.peek_byte() {
            let rewind = self.internal_getc_pos.clone();
            self.advance(b'.');
            match self.peek_byte() {
                Ok(b) if b.is_ascii_digit() => {
                    self.read_while(|b| b.is_ascii_digit() || b == b'_');
                    // The integer part (before the `.`) was already consumed by
                    // `parse_number`, so the literal must span from the start of
                    // the whole number, otherwise `123.456` would be lexed as
                    // just `Float(0.456)` and lose the `123` prefix.
                    let literal =
                        str::from_utf8(&self.source[whole_start as usize..self.internal_getc_pos.offset as usize])
                            .expect("Failed to convert");
                    if let Ok(result) = Self::convert_float_repr(literal) {
                        return Ok(Token::Float(result));
                    }
                }
                _ => {
                    // Restore ONLY the internal read position here. Calling
                    // `rewind`/`rewind_raw` would also reset `current_pos` and
                    // clear `preread_token`, corrupting the lexer's outer state
                    // (e.g. after an integer was already peeked). This is what
                    // made `5..10` lex incorrectly: the integer token was dropped
                    // from the preread cache, so the parser then saw a stray Dot.
                    self.internal_getc_pos = rewind;
                }
            }
        }
        Err(())
    }

    #[inline(always)]
    fn radix_decode(digits: &[u8], base: u32) -> Result<u128, ()> {
        let mut number = 0u128;
        for digit in digits {
            if digit == &b'_' {
                continue;
            }
            if let Ok(d) = u128::from_str_radix(str::from_utf8(&[*digit]).expect("non-utf8 digit"), base)
                && let Some(y) = number.checked_mul(u128::from(base))
                && let Some(sum) = y.checked_add(d)
            {
                number = sum;
                continue;
            }
            error!("[L0300]: Integer literal too large");
            return Err(());
        }
        Ok(number)
    }

    #[inline(always)]
    fn parse_number(&mut self) -> Result<Token, ()> {
        let mut base_prefix = None;
        let number_start = self.internal_getc_pos.offset;
        let mut literal = self.read_while(|b| b.is_ascii_digit() || b == b'_');
        assert!(!literal.is_empty());
        if literal == b"0" {
            match self.peek_byte() {
                Ok(b'b') => {
                    self.advance(b'b');
                    base_prefix = Some(2);
                    literal = self.read_while(|b| b == b'0' || b == b'1' || b == b'_');
                    if literal.is_empty() {
                        error!("[L0301]: Binary literal must have digits");
                        return Err(());
                    }
                }
                Ok(b'o') => {
                    self.advance(b'o');
                    base_prefix = Some(8);
                    literal = self.read_while(|b| (b'0'..=b'7').contains(&b) || b == b'_');
                    if literal.is_empty() {
                        error!("[L0302]: Octal literal must have digits");
                        return Err(());
                    }
                }
                Ok(b'd') => {
                    self.advance(b'd');
                    base_prefix = Some(10);
                    literal = self.read_while(|b| b.is_ascii_digit() || b == b'_');
                    if literal.is_empty() {
                        error!("[L0303]: Decimal literal must have digits");
                        return Err(());
                    }
                }
                Ok(b'x') => {
                    self.advance(b'x');
                    base_prefix = Some(16);
                    literal = self.read_while(|b| b.is_ascii_hexdigit() || b == b'_');
                    if literal.is_empty() {
                        error!("[L0304]: Hex literal must have digits");
                        return Err(());
                    }
                }
                _ => {}
            }
        }
        if base_prefix.is_none()
            && let Ok(float) = self.parse_float(number_start)
        {
            return Ok(float);
        }
        let number = Self::radix_decode(literal, base_prefix.unwrap_or(10u32))?;
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
    fn parse_string_hex_escape(&mut self) -> Result<StringEscape, ()> {
        let mut digits = [0u8; 2];
        for i in 0..2 {
            let byte = self.peek_byte()?;
            if byte.is_ascii_hexdigit() {
                self.advance(byte);
                digits[i] = byte;
            } else {
                error!("[L0400]: Invalid hex escape");
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
    fn parse_string_octal_escape(&mut self) -> Result<StringEscape, ()> {
        let mut digits = [0u8; 3];
        for i in 0..3 {
            let byte = self.peek_byte()?;
            if (b'0'..=b'7').contains(&byte) {
                self.advance(byte);
                digits[i] = byte;
            } else {
                error!("[L0401]: Invalid octal escape");
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
    fn parse_string_unicode_escape(&mut self) -> Result<StringEscape, ()> {
        if self.peek_byte()? != b'{' {
            error!("[L0402]: Expected '{{'");
            return Err(());
        }
        self.advance(b'{');
        if self.peek_byte()? == b'U' {
            self.advance(b'U');
            if self.peek_byte()? == b'+' {
                self.advance(b'+');
            } else {
                error!("[L0403]: Expected '+'");
                return Err(());
            }
        }
        let digits = self.read_while(|b| b.is_ascii_hexdigit());
        if digits.is_empty() {
            error!("[L0404]: Expected hex digits");
            return Err(());
        }
        if digits.len() > 8 {
            error!("[L0405]: Codepoint too large");
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
        let codepoint = char::from_u32(value).ok_or_else(|| {
            error!("[L0405]: Invalid codepoint");
        })?;
        if self.peek_byte()? != b'}' {
            error!("[L0406]: Expected '}}'");
            return Err(());
        }
        self.advance(b'}');
        Ok(StringEscape::Char(codepoint))
    }

    fn parse_string_escape(&mut self) -> Result<StringEscape, ()> {
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
                self.parse_string_hex_escape()
            }
            Ok(b'o') => {
                self.advance(b'o');
                self.parse_string_octal_escape()
            }
            Ok(b'u') => {
                self.advance(b'u');
                self.parse_string_unicode_escape()
            }
            Ok(b) => {
                error!("[L0407]: Invalid escape \\{}", b as char);
                Err(())
            }
            Err(()) => {
                error!("[L0408]: Unexpected EOF in string");
                Err(())
            }
        }
    }

    fn parse_string(&mut self) -> Result<Token, ()> {
        assert!(self.peek_byte().expect("peek") == b'"');
        self.advance(b'"');
        let start_offset = self.internal_getc_pos.offset;
        let mut end_offset = start_offset;
        let mut storage = Vec::new();
        loop {
            match self.peek_byte() {
                Ok(b'\\') => {
                    self.advance(b'\\');
                    if storage.is_empty() {
                        storage.extend_from_slice(&self.source[start_offset as usize..end_offset as usize]);
                    }
                    match self.parse_string_escape()? {
                        StringEscape::Char(c) => {
                            storage.extend_from_slice(c.to_string().as_bytes());
                        }
                        StringEscape::Byte(b) => {
                            storage.push(b);
                        }
                    }
                }
                Ok(b'"') => {
                    self.advance(b'"');
                    if storage.is_empty() {
                        let buffer = &self.source[start_offset as usize..end_offset as usize];
                        if let Ok(s) = str::from_utf8(buffer) {
                            return Ok(Token::String(s.to_string()));
                        }
                        return Ok(Token::BString(buffer.to_vec()));
                    }
                    return if let Ok(s) = String::from_utf8(storage.clone()) {
                        Ok(Token::String(s))
                    } else {
                        Ok(Token::BString(storage.clone()))
                    };
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
                    error!("[L0408]: Unexpected EOF in string");
                    return Err(());
                }
            }
        }
    }

    fn parse_comment(&mut self) -> Result<Token, ()> {
        let mut bytes = self.read_while(|b| b != b'\n');
        if bytes.ends_with(b"\r") {
            bytes = &bytes[..bytes.len() - 1];
        }
        if let Ok(s) = str::from_utf8(bytes) {
            Ok(Token::Comment(Comment::new(s.to_string(), CommentKind::SingleLine)))
        } else {
            error!("[L0600]: Invalid utf-8 in comment");
            Err(())
        }
    }

    fn parse_slash_comment(&mut self) -> Result<Token, ()> {
        self.advance(b'/');
        let mut bytes = self.read_while(|b| b != b'\n');
        if bytes.ends_with(b"\r") {
            bytes = &bytes[..bytes.len() - 1];
        }
        if let Ok(s) = str::from_utf8(bytes) {
            Ok(Token::Comment(Comment::new(format!("//{s}"), CommentKind::SingleLine)))
        } else {
            error!("[L0600]: Invalid utf-8");
            Err(())
        }
    }

    fn parse_block_comment(&mut self) -> Result<Token, ()> {
        self.advance(b'*');
        let mut bytes = vec![b'/', b'*'];
        loop {
            match self.peek_byte() {
                Ok(b'*') => {
                    self.advance(b'*');
                    if let Ok(b'/') = self.peek_byte() {
                        self.advance(b'/');
                        bytes.push(b'*');
                        bytes.push(b'/');
                        break;
                    }
                    bytes.push(b'*');
                }
                Ok(b) => {
                    self.advance(b);
                    bytes.push(b);
                }
                Err(()) => {
                    error!("[L0601]: Unterminated block comment");
                    return Err(());
                }
            }
        }
        if let Ok(s) = String::from_utf8(bytes) {
            Ok(Token::Comment(Comment::new(s, CommentKind::MultiLine)))
        } else {
            error!("[L0602]: Invalid utf-8");
            Err(())
        }
    }

    fn parse_slash_or_comment(&mut self) -> Result<Token, ()> {
        self.advance(b'/');
        match self.peek_byte() {
            Ok(b'/') => self.parse_slash_comment(),
            Ok(b'*') => self.parse_block_comment(),
            _ => Ok(Token::Slash),
        }
    }

    fn parse_single_byte(&mut self) -> Result<Token, ()> {
        let b = self.peek_byte()?;
        let tok = match b {
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
            b'^' => Some(Token::Caret),
            b'%' => Some(Token::Percent),
            b'\t' => Some(Token::HorizontalTab),
            b'\n' => Some(Token::NewLine),
            b'\x0b' => Some(Token::VerticalTab),
            b'\x0c' => Some(Token::FormFeed),
            b'\r' => Some(Token::CarriageReturn),
            b' ' => Some(Token::Space),
            _ => None,
        };
        if let Some(t) = tok {
            self.advance(self.peek_byte()?);
            Ok(t)
        } else {
            error!("[L0700]: Invalid token `{}`", str::from_utf8(&[b]).unwrap_or("?"));
            Err(())
        }
    }

    fn parse_next_token(&mut self) -> AnnotatedToken {
        let start = self.internal_getc_pos.clone();
        let token = match self.peek_byte() {
            Err(()) => Ok(Token::Eof),
            Ok(b) => match b {
                b'`' => self.parse_atypical_identifier(),
                b if b.is_ascii_alphabetic() || b == b'_' || !b.is_ascii() => self.parse_typical_identifier(),
                b if b.is_ascii_digit() => self.parse_number(),
                b'"' => self.parse_string(),
                b'#' => self.parse_comment(),
                b'/' => self.parse_slash_or_comment(),
                _ => self.parse_single_byte(),
            },
        }
        .unwrap_or(Token::Eof);

        if !self.trivia_enabled && token.is_trivia() {
            return self.parse_next_token();
        }

        let end = self.internal_getc_pos.clone();
        AnnotatedToken::new_raw(
            token,
            start.fileid,
            start.line,
            start.column,
            start.offset,
            end.line,
            end.column,
            end.offset,
        )
    }
}

pub struct LexerIterator<'a> {
    lexer: Lexer<'a>,
}

impl<'a> LexerIterator<'a> {
    pub fn new(lexer: Lexer<'a>) -> Self {
        LexerIterator { lexer }
    }
}

impl<'a> Iterator for LexerIterator<'a> {
    type Item = AnnotatedToken;
    fn next(&mut self) -> Option<Self::Item> {
        let token = self.lexer.next_tok();
        if token.token == Token::Eof { None } else { Some(token) }
    }
}
