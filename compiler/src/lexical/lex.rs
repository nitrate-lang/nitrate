use super::token::{
    AnnotatedToken, BStringData, Comment, CommentKind, Integer, IntegerKind, Keyword, Name, Op,
    Punct, SourcePosition, StringData, Token,
};
use log::error;
use ordered_float::NotNan;
use smallvec::SmallVec;

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd, Hash)]
pub enum LexerConstructionError {
    SourceTooBig,
}

impl std::fmt::Display for LexerConstructionError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            LexerConstructionError::SourceTooBig => write!(f, "Source code is too big"),
        }
    }
}

#[derive(Debug)]
pub struct Lexer<'a> {
    source: &'a [u8],
    current_peek_pos: SourcePosition<'a>,
    sync_pos: SourcePosition<'a>,
    current: Option<AnnotatedToken<'a>>,
}

enum StringEscape {
    Char(char),
    Byte(u8),
}

impl<'a> Lexer<'a> {
    pub fn new(src: &'a [u8], filename: &'a str) -> Result<Self, LexerConstructionError> {
        // Must not be increased beyond u32::MAX, as the lexer/compiler pipeline
        // assumes that offsets are representable as u32 values. However, it is
        // acceptable to decrease this value.
        const MAX_SOURCE_SIZE: usize = u32::MAX as usize;

        if src.len() > MAX_SOURCE_SIZE {
            Err(LexerConstructionError::SourceTooBig)
        } else {
            Ok(Lexer {
                source: src,
                current_peek_pos: SourcePosition::new(0, 0, 0, filename),
                sync_pos: SourcePosition::new(0, 0, 0, filename),
                current: None,
            })
        }
    }

    pub fn next_tok(&mut self) -> AnnotatedToken<'a> {
        let token = self
            .current
            .take()
            .unwrap_or_else(|| self.parse_next_token());

        self.sync_pos = self.reader_position();

        token
    }

    pub fn peek_tok(&mut self) -> AnnotatedToken<'a> {
        let token = self
            .current
            .take()
            .unwrap_or_else(|| self.parse_next_token());
        self.current = Some(token.clone());

        token
    }

    pub fn skip_tok(&mut self) {
        if self.current.is_some() {
            self.current = None;
        } else {
            self.parse_next_token(); // Discard the token
        }

        self.sync_pos = self.reader_position();
    }

    #[inline(always)]
    pub fn next_t(&mut self) -> Token<'a> {
        self.next_tok().into_token()
    }

    #[inline(always)]
    pub fn peek_t(&mut self) -> Token<'a> {
        self.peek_tok().into_token()
    }

    #[inline(always)]
    pub fn next_is(&mut self, matches: &Token<'a>) -> bool {
        &self.peek_t() == matches
    }

    #[inline(always)]
    pub fn skip_if(&mut self, matches: &Token<'a>) -> bool {
        if &self.peek_t() == matches {
            self.skip_tok();
            true
        } else {
            false
        }
    }

    #[inline(always)]
    #[must_use]
    pub fn sync_position(&self) -> SourcePosition<'a> {
        self.sync_pos
    }

    #[inline(always)]
    pub fn is_eof(&mut self) -> bool {
        self.peek_t() == Token::Eof
    }

    #[inline(always)]
    pub fn rewind(&mut self, pos: SourcePosition<'a>) {
        self.current_peek_pos = pos;
        self.sync_pos = pos;
        self.current = None;
    }

    #[inline(always)]
    pub fn next_if_string(&mut self) -> Option<StringData<'a>> {
        if let Token::String(string_data) = self.peek_t() {
            self.skip_tok();
            Some(string_data)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_bstring(&mut self) -> Option<BStringData<'a>> {
        if let Token::BString(bstring_data) = self.peek_t() {
            self.skip_tok();
            Some(bstring_data)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_name(&mut self) -> Option<Name<'a>> {
        if let Token::Name(name) = self.peek_t() {
            self.skip_tok();
            Some(name)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_keyword(&mut self) -> Option<Keyword> {
        if let Token::Keyword(keyword) = self.peek_t() {
            self.skip_tok();
            Some(keyword)
        } else {
            None
        }
    }

    #[inline(always)]
    pub fn next_if_op(&mut self) -> Option<Op> {
        if let Token::Op(op) = self.peek_t() {
            self.skip_tok();
            Some(op)
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
    fn reader_position(&self) -> SourcePosition<'a> {
        self.current_peek_pos
    }

    #[inline(always)]
    fn advance(&mut self, byte: u8) -> u8 {
        let current = self.reader_position();

        if byte == b'\n' {
            self.current_peek_pos = SourcePosition::new(
                current.line() + 1,
                0,
                current.offset() + 1,
                current.filename(),
            );
        } else {
            self.current_peek_pos = SourcePosition::new(
                current.line(),
                current.column() + 1,
                current.offset() + 1,
                current.filename(),
            );
        }

        byte
    }

    #[inline(always)]
    fn peek_byte(&self) -> Result<u8, ()> {
        self.source
            .get(self.reader_position().offset() as usize)
            .copied()
            .ok_or(())
    }

    #[inline(always)]
    fn read_while<F>(&mut self, mut condition: F) -> &'a [u8]
    where
        F: FnMut(u8) -> bool,
    {
        let start_offset = self.reader_position().offset();
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
    fn parse_atypical_identifier(&mut self) -> Result<Token<'a>, ()> {
        let start_pos = self.reader_position();

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
            Ok(Token::Name(Name::new_atypical(identifier)))
        } else {
            error!("[L0001]: Identifier contains some invalid utf-8 bytes\n--> {start_pos}");

            Err(())
        }
    }

    #[inline(always)]
    fn parse_typical_identifier(&mut self) -> Result<Token<'a>, ()> {
        let start_pos = self.reader_position();

        let name = self.read_while(|b| b.is_ascii_alphanumeric() || b == b'_' || !b.is_ascii());
        assert!(!name.is_empty(), "Identifier should not be empty");

        if let Some(word_like_operator) = match name {
            b"as" => Some(Op::As),
            b"bitcast_as" => Some(Op::BitcastAs),
            b"sizeof" => Some(Op::Sizeof),
            b"alignof" => Some(Op::Alignof),
            b"typeof" => Some(Op::Typeof),
            _ => None,
        } {
            Ok(Token::Op(word_like_operator))
        } else if let Some(keyword) = match name {
            b"let" => Some(Keyword::Let),
            b"var" => Some(Keyword::Var),
            b"fn" => Some(Keyword::Fn),
            b"enum" => Some(Keyword::Enum),
            b"struct" => Some(Keyword::Struct),
            b"class" => Some(Keyword::Class),
            b"contract" => Some(Keyword::Contract),
            b"trait" => Some(Keyword::Trait),
            b"impl" => Some(Keyword::Impl),
            b"type" => Some(Keyword::Type),
            b"scope" => Some(Keyword::Scope),
            b"import" => Some(Keyword::Import),

            b"safe" => Some(Keyword::Safe),
            b"unsafe" => Some(Keyword::Unsafe),
            b"promise" => Some(Keyword::Promise),
            b"static" => Some(Keyword::Static),
            b"mut" => Some(Keyword::Mut),
            b"const" => Some(Keyword::Const),
            b"poly" => Some(Keyword::Poly),
            b"iso" => Some(Keyword::Iso),
            b"pub" => Some(Keyword::Pub),
            b"sec" => Some(Keyword::Sec),
            b"pro" => Some(Keyword::Pro),

            b"if" => Some(Keyword::If),
            b"else" => Some(Keyword::Else),
            b"for" => Some(Keyword::For),
            b"while" => Some(Keyword::While),
            b"do" => Some(Keyword::Do),
            b"switch" => Some(Keyword::Switch),
            b"break" => Some(Keyword::Break),
            b"continue" => Some(Keyword::Continue),
            b"ret" => Some(Keyword::Ret),
            b"foreach" => Some(Keyword::Foreach),
            b"async" => Some(Keyword::Async),
            b"await" => Some(Keyword::Await),
            b"asm" => Some(Keyword::Asm),
            b"assert" => Some(Keyword::Assert),

            b"null" => Some(Keyword::Null),
            b"true" => Some(Keyword::True),
            b"false" => Some(Keyword::False),

            b"bool" => Some(Keyword::Bool),
            b"u8" => Some(Keyword::U8),
            b"u16" => Some(Keyword::U16),
            b"u32" => Some(Keyword::U32),
            b"u64" => Some(Keyword::U64),
            b"u128" => Some(Keyword::U128),
            b"i8" => Some(Keyword::I8),
            b"i16" => Some(Keyword::I16),
            b"i32" => Some(Keyword::I32),
            b"i64" => Some(Keyword::I64),
            b"i128" => Some(Keyword::I128),
            b"f8" => Some(Keyword::F8),
            b"f16" => Some(Keyword::F16),
            b"f32" => Some(Keyword::F32),
            b"f64" => Some(Keyword::F64),
            b"f128" => Some(Keyword::F128),
            b"opaque" => Some(Keyword::Opaque),

            _ => None,
        } {
            Ok(Token::Keyword(keyword))
        } else if let Ok(identifier) = str::from_utf8(name) {
            Ok(Token::Name(Name::new_typical(identifier)))
        } else {
            error!("[L0100]: Identifier contains some invalid utf-8 bytes\n--> {start_pos}");

            Err(())
        }
    }

    #[inline(always)]
    fn convert_float_repr(&self, str_bytes: &str) -> Result<NotNan<f64>, ()> {
        match str_bytes.replace('_', "").parse::<f64>() {
            Ok(value) => Ok(NotNan::new(value).unwrap()),
            Err(e) => {
                error!("[L0200]: Invalid float literal: {e}");
                Err(())
            }
        }
    }

    #[inline(always)]
    fn parse_float(&mut self, start_pos: &SourcePosition) -> Result<Token<'a>, ()> {
        if let Ok(b'.') = self.peek_byte() {
            let rewind_pos = self.reader_position();
            self.advance(b'.');

            match self.peek_byte() {
                Ok(b) if b.is_ascii_digit() => {
                    self.read_while(|b| b.is_ascii_digit() || b == b'_');

                    let literal = str::from_utf8(
                        &self.source
                            [start_pos.offset() as usize..self.reader_position().offset() as usize],
                    )
                    .expect("Failed to convert float literal to str");

                    if let Ok(result) = self.convert_float_repr(literal) {
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
    fn radix_decode(
        &self,
        digits: &[u8],
        base: u32,
        start_pos: &SourcePosition,
    ) -> Result<u128, ()> {
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
    fn parse_number(&mut self) -> Result<Token<'a>, ()> {
        let start_pos = self.reader_position();

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

        let number = self.radix_decode(literal, base_prefix.unwrap_or(10u32), &start_pos)?;

        Ok(Token::Integer(Integer::new(
            number,
            match base_prefix {
                None => IntegerKind::Dec,
                Some(2) => IntegerKind::Bin,
                Some(8) => IntegerKind::Oct,
                Some(10) => IntegerKind::Dec,
                Some(16) => IntegerKind::Hex,
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
    fn parse_string(&mut self) -> Result<Token<'a>, ()> {
        let start_pos = self.reader_position();

        assert!(self.peek_byte().expect("Failed to peek byte") == b'"');
        self.advance(b'"');

        let start_offset = self.reader_position().offset();
        let mut end_offset = start_offset;
        let mut storage = SmallVec::<[u8; 64]>::new();

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
                            return Ok(Token::String(StringData::from_ref(utf8_str)));
                        }
                        return Ok(Token::BString(BStringData::from_ref(buffer)));
                    } else if let Ok(utf8_str) = String::from_utf8(storage.to_vec()) {
                        return Ok(Token::String(StringData::from_dyn(utf8_str)));
                    }
                    return Ok(Token::BString(BStringData::from_dyn(storage.to_vec())));
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
    fn parse_comment(&mut self) -> Result<Token<'a>, ()> {
        let start_pos = self.reader_position();
        let mut comment_bytes = self.read_while(|b| b != b'\n');

        if comment_bytes.ends_with(b"\r") {
            comment_bytes = &comment_bytes[..comment_bytes.len() - 1];
        }

        if let Ok(comment) = str::from_utf8(comment_bytes) {
            Ok(Token::Comment(Comment::new(
                comment,
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
    fn parse_operator_or_punctuation(&mut self) -> Result<Token<'a>, ()> {
        /*
         * The word-like operators are not handled here, as they are ambiguous with identifiers.
         * They are handled in `parse_typical_identifier`.
         */

        let start_pos = self.reader_position();
        let b = self.peek_byte()?;

        match b {
            b'(' | b')' | b'[' | b']' | b'{' | b'}' | b',' | b';' | b'@' | b'\'' => {
                match self.advance(b) {
                    b'(' => Ok(Token::Punct(Punct::LeftParen)),
                    b')' => Ok(Token::Punct(Punct::RightParen)),
                    b'[' => Ok(Token::Punct(Punct::LeftBracket)),
                    b']' => Ok(Token::Punct(Punct::RightBracket)),
                    b'{' => Ok(Token::Punct(Punct::LeftBrace)),
                    b'}' => Ok(Token::Punct(Punct::RightBrace)),
                    b',' => Ok(Token::Punct(Punct::Comma)),
                    b';' => Ok(Token::Punct(Punct::Semicolon)),
                    b'@' => Ok(Token::Punct(Punct::AtSign)),
                    b'\'' => Ok(Token::Punct(Punct::SingleQuote)),
                    _ => unreachable!(), // All cases are handled above
                }
            }

            b':' => {
                self.advance(b':');
                match self.peek_byte() {
                    Ok(b':') => {
                        self.advance(b':');
                        Ok(Token::Op(Op::Scope))
                    }
                    _ => Ok(Token::Punct(Punct::Colon)),
                }
            }

            b'?' => {
                self.advance(b'?');
                Ok(Token::Op(Op::Question))
            }
            b'~' => {
                self.advance(b'~');
                Ok(Token::Op(Op::BitNot))
            }
            b'+' => {
                self.advance(b'+');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::SetPlus))
                    }
                    Ok(b'+') => {
                        self.advance(b'+');
                        Ok(Token::Op(Op::Inc))
                    }
                    _ => Ok(Token::Op(Op::Add)),
                }
            }
            b'-' => {
                self.advance(b'-');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::SetMinus))
                    }
                    Ok(b'-') => {
                        self.advance(b'-');
                        Ok(Token::Op(Op::Dec))
                    }
                    Ok(b'>') => {
                        self.advance(b'>');
                        Ok(Token::Op(Op::Arrow))
                    }
                    _ => Ok(Token::Op(Op::Sub)),
                }
            }
            b'*' => {
                self.advance(b'*');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::SetTimes))
                    }
                    _ => Ok(Token::Op(Op::Mul)),
                }
            }
            b'/' => {
                self.advance(b'/');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::SetSlash))
                    }
                    _ => Ok(Token::Op(Op::Div)),
                }
            }
            b'%' => {
                self.advance(b'%');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::SetPercent))
                    }
                    _ => Ok(Token::Op(Op::Mod)),
                }
            }
            b'&' => {
                self.advance(b'&');
                match self.peek_byte() {
                    Ok(b'&') => {
                        self.advance(b'&');
                        match self.peek_byte() {
                            Ok(b'=') => {
                                self.advance(b'=');
                                Ok(Token::Op(Op::SetLogicAnd))
                            }
                            _ => Ok(Token::Op(Op::LogicAnd)),
                        }
                    }
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::SetBitAnd))
                    }
                    _ => Ok(Token::Op(Op::BitAnd)),
                }
            }
            b'|' => {
                self.advance(b'|');
                match self.peek_byte() {
                    Ok(b'|') => {
                        self.advance(b'|');
                        match self.peek_byte() {
                            Ok(b'=') => {
                                self.advance(b'=');
                                Ok(Token::Op(Op::SetLogicOr))
                            }
                            _ => Ok(Token::Op(Op::LogicOr)),
                        }
                    }
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::SetBitOr))
                    }
                    _ => Ok(Token::Op(Op::BitOr)),
                }
            }
            b'^' => {
                self.advance(b'^');
                match self.peek_byte() {
                    Ok(b'^') => {
                        self.advance(b'^');
                        match self.peek_byte() {
                            Ok(b'=') => {
                                self.advance(b'=');
                                Ok(Token::Op(Op::SetLogicXor))
                            }
                            _ => Ok(Token::Op(Op::LogicXor)),
                        }
                    }
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::SetBitXor))
                    }
                    _ => Ok(Token::Op(Op::BitXor)),
                }
            }
            b'<' => {
                self.advance(b'<');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        match self.peek_byte() {
                            Ok(b'>') => {
                                self.advance(b'>');
                                Ok(Token::Op(Op::Spaceship))
                            }
                            _ => Ok(Token::Op(Op::LogicLe)),
                        }
                    }
                    Ok(b'<') => {
                        self.advance(b'<');
                        match self.peek_byte() {
                            Ok(b'=') => {
                                self.advance(b'=');
                                Ok(Token::Op(Op::SetBitShl))
                            }
                            Ok(b'<') => {
                                self.advance(b'<');
                                match self.peek_byte() {
                                    Ok(b'=') => {
                                        self.advance(b'=');
                                        Ok(Token::Op(Op::SetBitRotl))
                                    }
                                    _ => Ok(Token::Op(Op::BitRotl)),
                                }
                            }
                            _ => Ok(Token::Op(Op::BitShl)),
                        }
                    }
                    _ => Ok(Token::Op(Op::LogicLt)),
                }
            }
            b'>' => {
                self.advance(b'>');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::LogicGe))
                    }
                    Ok(b'>') => {
                        self.advance(b'>');
                        match self.peek_byte() {
                            Ok(b'=') => {
                                self.advance(b'=');
                                Ok(Token::Op(Op::SetBitShr))
                            }
                            Ok(b'>') => {
                                self.advance(b'>');
                                match self.peek_byte() {
                                    Ok(b'=') => {
                                        self.advance(b'=');
                                        Ok(Token::Op(Op::SetBitRotr))
                                    }
                                    _ => Ok(Token::Op(Op::BitRotr)),
                                }
                            }
                            _ => Ok(Token::Op(Op::BitShr)),
                        }
                    }
                    _ => Ok(Token::Op(Op::LogicGt)),
                }
            }
            b'!' => {
                self.advance(b'!');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::LogicNe))
                    }
                    _ => Ok(Token::Op(Op::LogicNot)),
                }
            }
            b'=' => {
                self.advance(b'=');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Op(Op::LogicEq))
                    }
                    Ok(b'>') => {
                        self.advance(b'>');
                        Ok(Token::Op(Op::BlockArrow))
                    }
                    _ => Ok(Token::Op(Op::Set)),
                }
            }
            b'.' => {
                self.advance(b'.');
                match self.peek_byte() {
                    Ok(b'.') => {
                        self.advance(b'.');
                        match self.peek_byte() {
                            Ok(b'.') => {
                                self.advance(b'.');
                                Ok(Token::Op(Op::Ellipsis))
                            }
                            _ => Ok(Token::Op(Op::Range)),
                        }
                    }
                    _ => Ok(Token::Op(Op::Dot)),
                }
            }

            _ => {
                error!(
                    "[L0700]: The token `{}` is not valid. Did you mistype an operator or forget some whitespace?\n--> {}",
                    str::from_utf8(&[b]).unwrap_or("<invalid utf-8>"),
                    start_pos
                );

                Err(())
            }
        }
    }

    #[inline(always)]
    fn parse_next_token(&mut self) -> AnnotatedToken<'a> {
        self.read_while(|b| b.is_ascii_whitespace());

        let start_pos = self.reader_position();

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
        .unwrap_or(Token::Illegal);

        let end_pos = self.reader_position();

        AnnotatedToken::new(token, start_pos, end_pos)
    }
}

#[test]
fn test_parse_string_escape() {
    let test_vector = [
        // "👀 Hello, 🔥😂 \0\a\b\t\n\v\f\r\\\'\"\x38\x0fA\o0171"
        0x22, 0xf0, 0x9f, 0x91, 0x80, 0x20, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0xf0, 0x9f,
        0x94, 0xa5, 0xf0, 0x9f, 0x98, 0x82, 0x20, 0x5c, 0x30, 0x5c, 0x61, 0x5c, 0x62, 0x5c, 0x74,
        0x5c, 0x6e, 0x5c, 0x76, 0x5c, 0x66, 0x5c, 0x72, 0x5c, 0x5c, 0x5c, 0x27, 0x5c, 0x22, 0x5c,
        0x78, 0x33, 0x38, 0x5c, 0x78, 0x30, 0x66, 0x41, 0x5c, 0x6f, 0x30, 0x31, 0x37, 0x31, 0x22,
    ];

    let expected = "👀 Hello, 🔥😂 \0\u{7}\u{8}\t\n\u{b}\u{c}\r\\'\"8\u{f}A\u{f}1";

    let mut lexer = Lexer::new(&test_vector, "test_file").expect("Failed to create lexer");

    match lexer.next_tok().token() {
        Token::String(s) => {
            assert_eq!(s.get(), expected, "Parsed string does not match expected");
        }
        _ => panic!("Expected a string token"),
    }
}
