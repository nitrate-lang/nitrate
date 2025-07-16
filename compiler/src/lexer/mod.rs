use hashbrown::hash_set::HashSet;
use log::error;
use smallvec::SmallVec;
use stackvector::StackVec;
use std::marker::PhantomData;

// Must not be increased beyond u32::MAX, as the lexer/compiler pipeline
// assumes that offsets are representable as u32 values. However, it is
// acceptable to decrease this value (for whatever reason?).
const MAX_SOURCE_SIZE: usize = u32::MAX as usize;

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum IdentifierKind {
    Typical,
    Atypical,
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct Identifier<'a> {
    name: &'a str,
    kind: IdentifierKind,
}

impl<'a> Identifier<'a> {
    pub const fn new(name: &'a str, kind: IdentifierKind) -> Self {
        Identifier { name, kind }
    }

    pub const fn name(&self) -> &str {
        self.name
    }

    pub const fn kind(&self) -> IdentifierKind {
        self.kind
    }
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum IntegerKind {
    Binary,
    Octal,
    Decimal,
    Hexadecimal,
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct Integer<'a> {
    value: u128,
    origin: &'a str,
    kind: IntegerKind,
}

impl<'a> Integer<'a> {
    pub const fn new(value: u128, origin: &'a str, kind: IntegerKind) -> Self {
        Integer {
            value,
            origin,
            kind,
        }
    }

    pub const fn value(&self) -> u128 {
        self.value
    }

    pub const fn origin(&self) -> &str {
        self.origin
    }

    pub const fn kind(&self) -> IntegerKind {
        self.kind
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct Float<'a> {
    value: f64,
    origin: &'a str,
}

impl<'a> Float<'a> {
    pub const fn new(value: f64, origin: &'a str) -> Self {
        Float { value, origin }
    }

    pub const fn value(&self) -> f64 {
        self.value
    }

    pub const fn origin(&self) -> &str {
        self.origin
    }
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum Keyword {
    /* Storage */
    Let,       /* 'let' */
    Var,       /* 'var' */
    Fn,        /* 'fn' */
    Enum,      /* 'enum' */
    Struct,    /* 'struct' */
    Class,     /* 'class' */
    Union,     /* 'union' */
    Interface, /* 'interface' */
    Trait,     /* 'trait' */
    Type,      /* 'type' */
    Opaque,    /* 'opaque' */
    Scope,     /* 'scope' */
    Import,    /* 'import' */
    UnitTest,  /* 'unit_test' */

    /* Modifiers */
    Safe,    /* 'safe' */
    Unsafe,  /* 'unsafe' */
    Promise, /* 'promise' */
    Static,  /* 'static' */
    Mut,     /* 'mut' */
    Const,   /* 'const' */
    Pub,     /* 'pub' */
    Sec,     /* 'sec' */
    Pro,     /* 'pro' */

    /* Control Flow */
    If,       /* 'if' */
    Else,     /* 'else' */
    For,      /* 'for' */
    While,    /* 'while' */
    Do,       /* 'do' */
    Switch,   /* 'switch' */
    Break,    /* 'break' */
    Continue, /* 'continue' */
    Ret,      /* 'ret' */
    Foreach,  /* 'foreach' */
    Try,      /* 'try' */
    Catch,    /* 'catch' */
    Throw,    /* 'throw' */
    Async,    /* 'async' */
    Await,    /* 'await' */
    Asm,      /* 'asm' */

    /* Literals */
    Null,  /* 'null' */
    True,  /* 'true' */
    False, /* 'false' */
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum Punctuation {
    LeftParenthesis,  /* '(' */
    RightParenthesis, /* ')' */
    LeftBracket,      /* '[' */
    RightBracket,     /* ']' */
    LeftBrace,        /* '{' */
    RightBrace,       /* '}' */
    Comma,            /* ',' */
    Semicolon,        /* ';' */
    Colon,            /* ':' */
    AtSign,           /* '@' */
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum Operator {
    /*----------------------------------------------------------------*
     * Arithmetic Operators                                           *
     *----------------------------------------------------------------*/
    Add, /* '+': "Addition Operator" */
    Sub, /* '-': "Subtraction Operator" */
    Mul, /* '*': "Multiplication Operator" */
    Div, /* '/': "Division Operator" */
    Mod, /* '%': "Modulus Operator" */

    /*----------------------------------------------------------------*
     * Bitwise Operators                                              *
     *----------------------------------------------------------------*/
    BitAnd,  /* '&':   "Bitwise AND Operator" */
    BitOr,   /* '|':   "Bitwise OR Operator" */
    BitXor,  /* '^':   "Bitwise XOR Operator" */
    BitNot,  /* '~':   "Bitwise NOT Operator" */
    BitShl,  /* '<<':  "Bitwise Left-Shift Operator" */
    BitShr,  /* '>>':  "Bitwise Right-Shift Operator" */
    BitRotl, /* '<<<': "Bitwise Left-Rotate Operator" */
    BitRotr, /* '>>>': "Bitwise Right-Rotate Operator" */

    /*----------------------------------------------------------------*
     * Logical Operators                                              *
     *----------------------------------------------------------------*/
    LogicAnd, /* '&&': "Logical AND Operator" */
    LogicOr,  /* '||': "Logical OR Operator" */
    LogicXor, /* '^^': "Logical XOR Operator" */
    LogicNot, /* '!':  "Logical NOT Operator" */
    LogicLt,  /* '<':  "Logical Less-Than Operator" */
    LogicGt,  /* '>':  "Logical Greater-Than Operator" */
    LogicLe,  /* '<=': "Logical Less-Than or Equal-To Operator" */
    LogicGe,  /* '>=': "Logical Greater-Than or Equal-To Operator" */
    LogicEq,  /* '==': "Logical Equal-To Operator" */
    LogicNe,  /* '!=': "Logical Not Equal-To Operator" */

    /*----------------------------------------------------------------*
     * Assignment Operators                                           *
     *----------------------------------------------------------------*/
    Set,         /* '=':    "Assignment Operator" */
    SetPlus,     /* '+=':   "Addition Assignment Operator" */
    SetMinus,    /* '-=':   "Subtraction Assignment Operator" */
    SetTimes,    /* '*=':   "Multiplication Assignment Operator" */
    SetSlash,    /* '/=':   "Division Assignment Operator" */
    SetPercent,  /* '%=':   "Modulus Assignment Operator" */
    SetBitAnd,   /* '&=':   "Bitwise AND Assignment Operator" */
    SetBitOr,    /* '|=':   "Bitwise OR Assignment Operator" */
    SetBitXor,   /* '^=':   "Bitwise XOR Assignment Operator" */
    SetBitShl,   /* '<<=':  "Bitwise Left-Shift Assignment Operator" */
    SetBitShr,   /* '>>=':  "Bitwise Right-Shift Assignment Operator" */
    SetBitRotl,  /* '<<<=': "Bitwise Rotate-Left Assignment Operator" */
    SetBitRotr,  /* '>>>=': "Bitwise Rotate-Right Assignment Operator" */
    SetLogicAnd, /* '&&=':  "Logical AND Assignment Operator" */
    SetLogicOr,  /* '||=':  "Logical OR Assignment Operator" */
    SetLogicXor, /* '^^=':  "Logical XOR Assignment Operator" */
    Inc,         /* '++':   "Increment Operator" */
    Dec,         /* '--':   "Decrement Operator" */

    /*----------------------------------------------------------------*
     * Type System Operators                                          *
     *----------------------------------------------------------------*/
    As,        /* 'as':         "Type Cast Operator" */
    BitcastAs, /* 'bitcast_as': "Bitcast Operator" */
    Sizeof,    /* 'sizeof':     "Size Of Operator" */
    Alignof,   /* 'alignof':    "Alignment Of Operator" */
    Typeof,    /* 'typeof':     "Type Of Operator" */

    /*----------------------------------------------------------------*
     * Syntactic Operators                                            *
     *----------------------------------------------------------------*/
    Dot,        /* '.':          "Dot Operator" */
    Ellipsis,   /* '...':        "Ellipsis Operator" */
    Scope,      /* '::':         "Scope Resolution Operator" */
    Arrow,      /* '->':         "Arrow Operator" */
    BlockArrow, /* '=>':         "Block Arrow Operator" */

    /*----------------------------------------------------------------*
     * Special Operators                                              *
     *----------------------------------------------------------------*/
    Range,     /* '..':         "Range Operator" */
    Question,  /* '?':          "Ternary Operator" */
    Spaceship, /* '<=>':        "Spaceship Operator" */
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum CommentKind {
    SingleLine,
    MultiLine,
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct Comment<'a> {
    text: &'a str,
    kind: CommentKind,
}

impl<'a> Comment<'a> {
    pub const fn new(text: &'a str, kind: CommentKind) -> Self {
        Comment { text, kind }
    }

    pub const fn text(&self) -> &str {
        self.text
    }

    pub const fn kind(&self) -> CommentKind {
        self.kind
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub enum Token<'a, 'b> {
    Identifier(Identifier<'a>),
    Integer(Integer<'a>),
    Float(Float<'a>),
    Keyword(Keyword),
    String(&'b str),
    BinaryString(&'b [u8]),
    Char(char),
    Punctuation(Punctuation),
    Operator(Operator),
    Comment(Comment<'a>),
    Eof,
    Illegal,
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct SourcePosition<'a> {
    line: u32,   // zero-based unicode-aware line number
    column: u32, // zero-based unicode-aware column number
    offset: u32, // zero-based raw byte offset number
    filename: &'a str,
}

impl<'a> SourcePosition<'a> {
    pub const fn new(line: u32, column: u32, offset: u32, filename: &'a str) -> Self {
        SourcePosition {
            line,
            column,
            offset,
            filename,
        }
    }

    pub const fn line(&self) -> u32 {
        self.line
    }

    pub const fn column(&self) -> u32 {
        self.column
    }

    pub const fn offset(&self) -> usize {
        self.offset as usize
    }

    pub const fn filename(&self) -> &'a str {
        self.filename
    }
}

impl std::fmt::Display for SourcePosition<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}:{}:{}", self.filename, self.line + 1, self.column + 1)
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct AnnotatedToken<'a, 'b> {
    token: Token<'a, 'b>,

    start_line: u32,
    start_column: u32,
    start_offset: u32,

    end_line: u32,
    end_column: u32,
    end_offset: u32,

    filename: &'a str,
}

impl<'a, 'b> AnnotatedToken<'a, 'b> {
    pub const fn new(
        token: Token<'a, 'b>,
        start: SourcePosition<'a>,
        end: SourcePosition<'a>,
    ) -> Self {
        AnnotatedToken {
            token,
            start_line: start.line(),
            start_column: start.column(),
            start_offset: start.offset,
            end_line: end.line(),
            end_column: end.column(),
            end_offset: end.offset,
            filename: start.filename(),
        }
    }

    pub const fn token(&self) -> &Token {
        &self.token
    }

    pub const fn start(&self) -> SourcePosition<'a> {
        SourcePosition::new(
            self.start_line,
            self.start_column,
            self.start_offset,
            self.filename,
        )
    }

    pub const fn end(&self) -> SourcePosition<'a> {
        SourcePosition::new(
            self.end_line,
            self.end_column,
            self.end_offset,
            self.filename,
        )
    }

    pub const fn range(&self) -> (SourcePosition<'a>, SourcePosition<'a>) {
        (self.start(), self.end())
    }
}

#[derive(Debug, Default)]
pub struct StringStorage<'b> {
    strings: HashSet<SmallVec<[u8; 32]>>,
    binary_strings: HashSet<SmallVec<[u8; 32]>>,
    _data: PhantomData<&'b ()>,
}

impl<'b> StringStorage<'b> {
    pub fn new() -> Self {
        StringStorage {
            strings: HashSet::new(),
            binary_strings: HashSet::new(),
            _data: PhantomData,
        }
    }

    fn get_or_intern_str(&mut self, str: SmallVec<[u8; 32]>) -> &'b str {
        let bytes = self.strings.get_or_insert(str);
        let string = unsafe { str::from_utf8_unchecked(&bytes) };

        /*
         * SAFETY: The lifetime `b` is the same as the lifetime of
         * Self, which owns the string. Therefore, it is safe to
         * transmute the reference, Probably..
         */
        return unsafe { std::mem::transmute::<&str, &'b str>(string) };
    }

    fn get_or_intern_bytes(&mut self, bytes: SmallVec<[u8; 32]>) -> &'b [u8] {
        let bytes = self.binary_strings.get_or_insert(bytes);

        /*
         * SAFETY: The lifetime `b` is the same as the lifetime of
         * Self, which owns the byte vector. Therefore, it is safe to
         * transmute the reference, Probably..
         */
        return unsafe { std::mem::transmute::<&[u8], &'b [u8]>(bytes) };
    }
}

#[derive(Debug)]
pub struct Lexer<'a, 'b>
where
    'a: 'b,
{
    source: &'a [u8],
    read_pos: SourcePosition<'a>,
    current: Option<AnnotatedToken<'a, 'b>>,
    storage: &'b mut StringStorage<'b>,
}

#[derive(Debug, Clone, Copy)]
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

enum StringEscape {
    Char(char),
    Byte(u8),
}

impl<'a, 'b> Lexer<'a, 'b> {
    pub fn new(
        src: &'a [u8],
        filename: &'a str,
        storage: &'b mut StringStorage<'b>,
    ) -> Result<Self, LexerConstructionError> {
        if src.len() > MAX_SOURCE_SIZE {
            Err(LexerConstructionError::SourceTooBig)
        } else {
            Ok(Lexer {
                source: src,
                read_pos: SourcePosition::new(0, 0, 0, filename),
                current: None,
                storage: storage,
            })
        }
    }

    pub fn skip_token(&mut self) {
        if self.current.is_some() {
            self.current = None;
        } else {
            self.parse_next_token(); // Discard the token
        }
    }

    pub fn next_token(&mut self) -> AnnotatedToken<'a, 'b> {
        self.current
            .take()
            .unwrap_or_else(|| self.parse_next_token())
    }

    pub fn peek_token(&mut self) -> AnnotatedToken<'a, 'b> {
        let token = self
            .current
            .take()
            .unwrap_or_else(|| self.parse_next_token());
        self.current = Some(token.clone());

        token
    }

    pub fn current_position(&self) -> SourcePosition<'a> {
        self.read_pos.clone()
    }

    pub fn set_position(&mut self, pos: SourcePosition<'a>) {
        self.read_pos = pos;
    }

    const fn advance(&mut self, b: u8) -> u8 {
        self.read_pos.offset += 1;

        if b == b'\n' {
            // The line number can never overflow, because
            // the maximum source size is limited to u32::MAX,
            // which is the maximum value for a line number.
            self.read_pos.line += 1;
            self.read_pos.column = 0;
        } else {
            // The column number can never overflow, because
            // the maximum source size is limited to u32::MAX,
            // which is the maximum value for a column number.
            self.read_pos.column += 1;
        }

        b
    }

    fn peek_byte(&self) -> Result<u8, ()> {
        self.source.get(self.read_pos.offset()).copied().ok_or(())
    }

    fn read_while<F>(&mut self, mut condition: F) -> &'a [u8]
    where
        F: FnMut(u8) -> bool,
    {
        let start_offset = self.read_pos.offset();
        let mut end_offset = start_offset;

        while let Some(b) = self.source.get(end_offset) {
            if condition(*b) {
                self.advance(*b);
                end_offset += 1;
            } else {
                break;
            }
        }

        &self.source[start_offset..end_offset]
    }

    fn parse_atypical_identifier(&mut self) -> Result<Token<'a, 'b>, ()> {
        let start_pos = self.current_position();

        assert!(self.peek_byte().unwrap() == b'`');
        self.advance(b'`');

        let identifier = self.read_while(|b| b != b'`');

        match self.peek_byte() {
            Ok(b'`') => {
                self.advance(b'`');
            }
            _ => {
                error!(
                    "error[L0001]: Unterminated atypical identifier. Did you forget the '`' terminator?\n--> {}",
                    start_pos
                );
                return Err(());
            }
        }

        if let Ok(identifier) = str::from_utf8(identifier) {
            Ok(Token::Identifier(Identifier::new(
                identifier,
                IdentifierKind::Atypical,
            )))
        } else {
            error!(
                "error[L0003]: Identifier contains some invalid utf-8 bytes\n--> {}",
                start_pos
            );

            Err(())
        }
    }

    fn parse_typical_identifier(&mut self) -> Result<Token<'a, 'b>, ()> {
        let start_pos = self.current_position();

        let name = self.read_while(|b| b.is_ascii_alphanumeric() || b == b'_' || !b.is_ascii());
        assert!(!name.is_empty(), "Identifier should not be empty");

        if let Some(word_like_operator) = match name {
            b"as" => Some(Operator::As),
            b"bitcast_as" => Some(Operator::BitcastAs),
            b"sizeof" => Some(Operator::Sizeof),
            b"alignof" => Some(Operator::Alignof),
            b"typeof" => Some(Operator::Typeof),
            _ => None,
        } {
            Ok(Token::Operator(word_like_operator))
        } else if let Some(keyword) = match name {
            b"let" => Some(Keyword::Let),
            b"var" => Some(Keyword::Var),
            b"fn" => Some(Keyword::Fn),
            b"enum" => Some(Keyword::Enum),
            b"struct" => Some(Keyword::Struct),
            b"class" => Some(Keyword::Class),
            b"union" => Some(Keyword::Union),
            b"interface" => Some(Keyword::Interface),
            b"trait" => Some(Keyword::Trait),
            b"type" => Some(Keyword::Type),
            b"opaque" => Some(Keyword::Opaque),
            b"scope" => Some(Keyword::Scope),
            b"import" => Some(Keyword::Import),
            b"unit_test" => Some(Keyword::UnitTest),

            b"safe" => Some(Keyword::Safe),
            b"unsafe" => Some(Keyword::Unsafe),
            b"promise" => Some(Keyword::Promise),
            b"static" => Some(Keyword::Static),
            b"mut" => Some(Keyword::Mut),
            b"const" => Some(Keyword::Const),
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
            b"try" => Some(Keyword::Try),
            b"catch" => Some(Keyword::Catch),
            b"throw" => Some(Keyword::Throw),
            b"async" => Some(Keyword::Async),
            b"await" => Some(Keyword::Await),
            b"asm" => Some(Keyword::Asm),

            b"null" => Some(Keyword::Null),
            b"true" => Some(Keyword::True),
            b"false" => Some(Keyword::False),

            _ => None,
        } {
            Ok(Token::Keyword(keyword))
        } else if let Ok(identifier) = str::from_utf8(name) {
            Ok(Token::Identifier(Identifier::new(
                identifier,
                IdentifierKind::Typical,
            )))
        } else {
            error!(
                "error[L0003]: Identifier contains some invalid utf-8 bytes\n--> {}",
                start_pos
            );

            Err(())
        }
    }

    fn convert_float_repr(&self, str_bytes: &str) -> Result<f64, ()> {
        match str_bytes.replace("_", "").parse::<f64>() {
            Ok(value) => Ok(value),
            Err(e) => {
                error!("error[L0058]: Invalid float literal: {}", e);
                Err(())
            }
        }
    }

    fn parse_float(&mut self, start_pos: &SourcePosition) -> Result<Token<'a, 'b>, ()> {
        match self.peek_byte() {
            Ok(b'.') => {
                let rewind_pos = self.current_position();
                self.advance(b'.');

                match self.peek_byte() {
                    Ok(b) if b.is_ascii_digit() => {
                        self.read_while(|b| b.is_ascii_digit() || b == b'_');

                        let literal = str::from_utf8(
                            &self.source[start_pos.offset()..self.current_position().offset()],
                        )
                        .unwrap();

                        if let Ok(result) = self.convert_float_repr(&literal) {
                            return Ok(Token::Float(Float::new(result, literal)));
                        }
                    }
                    _ => {
                        self.set_position(rewind_pos);
                    }
                }
            }

            _ => {}
        }

        Err(())
    }

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
                if let Some(y) = number.checked_mul(base as u128) {
                    if let Some(sum) = y.checked_add(digit) {
                        number = sum;
                        continue;
                    }
                }
            }

            error!(
                "error[L0050]: Integer literal is too large to fit in u128\n--> {}",
                start_pos
            );
            return Err(());
        }

        Ok(number)
    }

    fn parse_number(&mut self) -> Result<Token<'a, 'b>, ()> {
        let start_pos = self.current_position();

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
                            "error[L0051]: Binary literal must contain at least one digit after '0b'\n--> {}",
                            start_pos
                        );
                        return Err(());
                    }
                }

                Ok(b'o') => {
                    self.advance(b'o');
                    base_prefix = Some(8);

                    literal = self.read_while(|b| (b >= b'0' && b <= b'7') || b == b'_');
                    if literal.is_empty() {
                        error!(
                            "error[L0052]: Octal literal must contain at least one digit after '0o'\n--> {}",
                            start_pos
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
                            "error[L0053]: Decimal literal must contain at least one digit after '0d'\n--> {}",
                            start_pos
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
                            "error[L0054]: Hexadecimal literal must contain at least one digit after '0x'\n--> {}",
                            start_pos
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
        literal = &self.source[start_pos.offset()..self.current_position().offset()];

        Ok(Token::Integer(Integer::new(
            number,
            str::from_utf8(&literal).expect("Unexpected non-utf8 digit"),
            match base_prefix {
                None => IntegerKind::Decimal,
                Some(2) => IntegerKind::Binary,
                Some(8) => IntegerKind::Octal,
                Some(10) => IntegerKind::Decimal,
                Some(16) => IntegerKind::Hexadecimal,
                _ => unreachable!(),
            },
        )))
    }

    fn parse_string_hex_escape(&mut self, start_pos: &SourcePosition) -> Result<StringEscape, ()> {
        let mut digits = [0u8; 2];

        for i in 0..2 {
            let byte = self.peek_byte()?;

            if (byte >= b'0' && byte <= b'9')
                || (byte >= b'a' && byte <= b'f')
                || (byte >= b'A' && byte <= b'F')
            {
                self.advance(byte);
                digits[i] = byte;
            } else {
                error!(
                    "error[L0043]: Invalid hex escape sequence '\\x{}' in string literal. Expected two hex digits (0-9, a-f, A-F) after '\\x'.\n--> {}",
                    str::from_utf8(&digits[..i + 1]).unwrap_or("<invalid utf-8>"),
                    start_pos
                );

                return Err(());
            }
        }

        let mut value = 0u8;
        for digit in digits {
            let digit = digit.to_ascii_lowercase();

            if digit >= b'0' && digit <= b'9' {
                value = (value << 4) | (digit - b'0');
            } else {
                value = (value << 4) | (digit - b'a' + 10);
            }
        }

        Ok(StringEscape::Byte(value))
    }

    fn parse_string_octal_escape(
        &mut self,
        start_pos: &SourcePosition,
    ) -> Result<StringEscape, ()> {
        let mut digits = [0u8; 3];

        for i in 0..3 {
            let byte = self.peek_byte()?;

            if byte >= b'0' && byte <= b'7' {
                self.advance(byte);
                digits[i] = byte;
            } else {
                error!(
                    "error[L0044]: Invalid octal escape sequence '\\o{}' in string literal. Expected three octal digits (0-7) after '\\o'.\n--> {}",
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

    fn parse_string_unicode_escape(
        &mut self,
        start_pos: &SourcePosition,
    ) -> Result<StringEscape, ()> {
        if self.peek_byte()? != b'{' {
            error!(
                "error[L0047]: Invalid unicode escape in string literal. Expected '{{' after '\\u'.\n--> {}",
                start_pos
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
                    "error[L0049]: Invalid unicode escape in string literal. Expected '+' after '\\uU'.\n--> {}",
                    start_pos
                );
                return Err(());
            }
        }

        let codepoint = (|| {
            let digits = self.read_while(|b| b.is_ascii_hexdigit());
            if digits.is_empty() {
                error!(
                    "error[L0045]: Invalid unicode escape in string literal. Expected at least one hex digit after '\\u{{'.\n--> {}",
                    start_pos
                );
                return None;
            }

            if digits.len() > 8 {
                error!(
                    "error[L0048]: Unicode escape codepoint in string literal is too large: '\\u{{{}}}'.\n--> {}",
                    str::from_utf8(&digits).unwrap_or("<invalid utf-8>"),
                    start_pos
                );
                return None;
            }

            let mut value = 0u32;
            for &digit in digits {
                let digit = digit.to_ascii_lowercase();

                if digit >= b'0' && digit <= b'9' {
                    value = (value << 4) | (digit - b'0') as u32
                } else {
                    value = (value << 4) | (digit - b'a' + 10) as u32
                }
            }

            let codepoint = char::from_u32(value);
            if codepoint.is_none() {
                error!(
                    "error[L0048]: Unicode escape codepoint in string literal is too large: '\\u{{{}}}'.\n--> {}",
                    str::from_utf8(&digits).unwrap_or("<invalid utf-8>"),
                    start_pos
                );

                return None;
            }

            codepoint
        })();

        if self.peek_byte()? != b'}' {
            error!(
                "error[L0046]: Invalid unicode escape in string literal. Expected '}}' after '\\u{{'.\n--> {}",
                start_pos
            );
            return Err(());
        }
        self.advance(b'}');

        codepoint.map_or_else(|| Err(()), |c| Ok(StringEscape::Char(c)))
    }

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
                    "error[L0040]: Invalid escape sequence '\\{}' in string literal\n--> {}",
                    b as char, start_pos
                );

                Err(())
            }

            Err(()) => {
                error!(
                    "error[L0041]: Unexpected end of input while parsing string literal\n--> {}",
                    start_pos
                );
                Err(())
            }
        }
    }

    fn parse_string(&mut self) -> Result<Token<'a, 'b>, ()> {
        let start_pos = self.current_position();

        assert!(self.peek_byte().unwrap() == b'"');
        self.advance(b'"');

        let start_offset = self.current_position().offset();
        let mut end_offset = start_offset;
        let mut storage = SmallVec::<[u8; 32]>::new();

        loop {
            match self.peek_byte() {
                Ok(b'\\') => {
                    self.advance(b'\\');

                    if storage.is_empty() {
                        storage.extend_from_slice(&self.source[start_offset..end_offset]);
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
                        let buffer = &self.source[start_offset..end_offset];

                        if let Ok(string) = str::from_utf8(&buffer) {
                            return Ok(Token::String(string));
                        } else {
                            return Ok(Token::BinaryString(buffer));
                        }
                    } else {
                        if str::from_utf8(&storage).is_ok() {
                            return Ok(Token::String(self.storage.get_or_intern_str(storage)));
                        } else {
                            return Ok(Token::BinaryString(
                                self.storage.get_or_intern_bytes(storage),
                            ));
                        }
                    }
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
                        "error[L0041]: Unexpected end of input while parsing string literal\n--> {}",
                        start_pos
                    );
                    return Err(());
                }
            }
        }
    }

    fn parse_char_escape(&mut self, start_pos: &SourcePosition) -> Result<u8, ()> {
        match self.peek_byte() {
            Ok(b'0') => {
                self.advance(b'0');
                Ok(b'\0')
            }
            Ok(b'a') => {
                self.advance(b'a');
                Ok(b'\x07')
            }
            Ok(b'b') => {
                self.advance(b'b');
                Ok(b'\x08')
            }
            Ok(b't') => {
                self.advance(b't');
                Ok(b'\t')
            }
            Ok(b'n') => {
                self.advance(b'n');
                Ok(b'\n')
            }
            Ok(b'v') => {
                self.advance(b'v');
                Ok(b'\x0b')
            }
            Ok(b'f') => {
                self.advance(b'f');
                Ok(b'\x0c')
            }
            Ok(b'r') => {
                self.advance(b'r');
                Ok(b'\r')
            }
            Ok(b'\\') => {
                self.advance(b'\\');
                Ok(b'\\')
            }
            Ok(b'\'') => {
                self.advance(b'\'');
                Ok(b'\'')
            }
            Ok(b) => {
                error!(
                    "error[L0013]: Invalid escape sequence '\\{}' in character literal\n--> {}",
                    b as char, start_pos
                );

                Err(())
            }

            Err(()) => {
                error!(
                    "error[L0015]: Unexpected end of input while parsing character literal\n--> {}",
                    start_pos
                );
                Err(())
            }
        }
    }

    fn parse_char(&mut self) -> Result<Token<'a, 'b>, ()> {
        let start_pos = self.current_position();

        assert!(self.peek_byte().unwrap() == b'\'');
        self.advance(b'\'');

        // Lets use more than 4 bytes for debugability of misuses of single quotes.
        let mut char_buffer = StackVec::<[u8; 32]>::new();

        loop {
            if char_buffer.len() >= char_buffer.capacity() {
                error!(
                    "error[L0012]: Character literal '{}' is too long. Did you mean to use a string literal?\n--> {}",
                    str::from_utf8(&char_buffer).unwrap_or("<invalid utf-8>"),
                    start_pos
                );

                return Err(());
            }

            match self.peek_byte() {
                Ok(b'\\') => {
                    self.advance(b'\\');

                    if let Ok(escaped_char) = self.parse_char_escape(&start_pos) {
                        char_buffer.push(escaped_char);
                    } else {
                        return Err(());
                    }
                }

                Ok(b'\'') => {
                    self.advance(b'\'');

                    if let Ok(chars_buffer) = str::from_utf8(&char_buffer) {
                        if chars_buffer.is_empty() {
                            error!(
                                "error[L0011]: Character literal is empty. Did you forget to specify the character?\n--> {}",
                                start_pos
                            );

                            return Err(());
                        }

                        let mut chars_iter = chars_buffer.chars();
                        let character = chars_iter
                            .next()
                            .expect("Character literal should not be empty");

                        if chars_iter.next().is_some() {
                            error!(
                                "error[L0010]: Character literal '{}' contains more than one character. Did you mean to use a string literal?\n--> {}",
                                str::from_utf8(&char_buffer).unwrap_or("<invalid utf-8>"),
                                start_pos
                            );

                            return Err(());
                        }

                        return Ok(Token::Char(character));
                    } else {
                        error!(
                            "error[L0012]: Character literal '{:?}' contains some invalid utf-8 bytes\n--> {}",
                            char_buffer.as_slice() as &[u8],
                            start_pos
                        );

                        return Err(());
                    }
                }

                Ok(b) => {
                    self.advance(b);
                    char_buffer.push(b);
                }

                Err(()) => {
                    error!(
                        "error[L0015]: Unexpected end of input while parsing character literal\n--> {}",
                        start_pos
                    );

                    return Err(());
                }
            }
        }
    }

    fn parse_comment(&mut self) -> Result<Token<'a, 'b>, ()> {
        let start_pos = self.current_position();
        let mut comment_bytes = self.read_while(|b| b != b'\n');

        // CRLF is dumb
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
                "error[L0020]: Single-line comment contains some invalid utf-8 bytes\n--> {}",
                start_pos
            );

            Err(())
        }
    }

    fn parse_operator_or_punctuation(&mut self) -> Result<Token<'a, 'b>, ()> {
        /*
         * The word-like operators are not handled here, as they are ambiguous with identifiers.
         * They are handled in `parse_typical_identifier`.
         */

        let start_pos = self.current_position();
        let b = self.peek_byte()?;

        match b {
            b'(' | b')' | b'[' | b']' | b'{' | b'}' | b',' | b';' | b'@' => {
                match self.advance(b) {
                    b'(' => Ok(Token::Punctuation(Punctuation::LeftParenthesis)),
                    b')' => Ok(Token::Punctuation(Punctuation::RightParenthesis)),
                    b'[' => Ok(Token::Punctuation(Punctuation::LeftBracket)),
                    b']' => Ok(Token::Punctuation(Punctuation::RightBracket)),
                    b'{' => Ok(Token::Punctuation(Punctuation::LeftBrace)),
                    b'}' => Ok(Token::Punctuation(Punctuation::RightBrace)),
                    b',' => Ok(Token::Punctuation(Punctuation::Comma)),
                    b';' => Ok(Token::Punctuation(Punctuation::Semicolon)),
                    b'@' => Ok(Token::Punctuation(Punctuation::AtSign)),
                    _ => unreachable!(), // All cases are handled above
                }
            }

            b':' => {
                self.advance(b':');
                match self.peek_byte() {
                    Ok(b':') => {
                        self.advance(b':');
                        Ok(Token::Operator(Operator::Scope))
                    }
                    _ => Ok(Token::Punctuation(Punctuation::Colon)),
                }
            }

            b'?' => {
                self.advance(b'?');
                Ok(Token::Operator(Operator::Question))
            }
            b'~' => {
                self.advance(b'~');
                Ok(Token::Operator(Operator::BitNot))
            }
            b'+' => {
                self.advance(b'+');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::SetPlus))
                    }
                    Ok(b'+') => {
                        self.advance(b'+');
                        Ok(Token::Operator(Operator::Inc))
                    }
                    _ => Ok(Token::Operator(Operator::Add)),
                }
            }
            b'-' => {
                self.advance(b'-');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::SetMinus))
                    }
                    Ok(b'-') => {
                        self.advance(b'-');
                        Ok(Token::Operator(Operator::Dec))
                    }
                    Ok(b'>') => {
                        self.advance(b'>');
                        Ok(Token::Operator(Operator::Arrow))
                    }
                    _ => Ok(Token::Operator(Operator::Sub)),
                }
            }
            b'*' => {
                self.advance(b'*');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::SetTimes))
                    }
                    _ => Ok(Token::Operator(Operator::Mul)),
                }
            }
            b'/' => {
                self.advance(b'/');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::SetSlash))
                    }
                    _ => Ok(Token::Operator(Operator::Div)),
                }
            }
            b'%' => {
                self.advance(b'%');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::SetPercent))
                    }
                    _ => Ok(Token::Operator(Operator::Mod)),
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
                                Ok(Token::Operator(Operator::SetLogicAnd))
                            }
                            _ => Ok(Token::Operator(Operator::LogicAnd)),
                        }
                    }
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::SetBitAnd))
                    }
                    _ => Ok(Token::Operator(Operator::BitAnd)),
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
                                Ok(Token::Operator(Operator::SetLogicOr))
                            }
                            _ => Ok(Token::Operator(Operator::LogicOr)),
                        }
                    }
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::SetBitOr))
                    }
                    _ => Ok(Token::Operator(Operator::BitOr)),
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
                                Ok(Token::Operator(Operator::SetLogicXor))
                            }
                            _ => Ok(Token::Operator(Operator::LogicXor)),
                        }
                    }
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::SetBitXor))
                    }
                    _ => Ok(Token::Operator(Operator::BitXor)),
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
                                Ok(Token::Operator(Operator::Spaceship))
                            }
                            _ => Ok(Token::Operator(Operator::LogicLe)),
                        }
                    }
                    Ok(b'<') => {
                        self.advance(b'<');
                        match self.peek_byte() {
                            Ok(b'=') => {
                                self.advance(b'=');
                                Ok(Token::Operator(Operator::SetBitShl))
                            }
                            Ok(b'<') => {
                                self.advance(b'<');
                                match self.peek_byte() {
                                    Ok(b'=') => {
                                        self.advance(b'=');
                                        Ok(Token::Operator(Operator::SetBitRotl))
                                    }
                                    _ => Ok(Token::Operator(Operator::BitRotl)),
                                }
                            }
                            _ => Ok(Token::Operator(Operator::BitShl)),
                        }
                    }
                    _ => Ok(Token::Operator(Operator::LogicLt)),
                }
            }
            b'>' => {
                self.advance(b'>');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::LogicGe))
                    }
                    Ok(b'>') => {
                        self.advance(b'>');
                        match self.peek_byte() {
                            Ok(b'=') => {
                                self.advance(b'=');
                                Ok(Token::Operator(Operator::SetBitShr))
                            }
                            Ok(b'>') => {
                                self.advance(b'>');
                                match self.peek_byte() {
                                    Ok(b'=') => {
                                        self.advance(b'=');
                                        Ok(Token::Operator(Operator::SetBitRotr))
                                    }
                                    _ => Ok(Token::Operator(Operator::BitRotr)),
                                }
                            }
                            _ => Ok(Token::Operator(Operator::BitShr)),
                        }
                    }
                    _ => Ok(Token::Operator(Operator::LogicGt)),
                }
            }
            b'!' => {
                self.advance(b'!');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::LogicNe))
                    }
                    _ => Ok(Token::Operator(Operator::LogicNot)),
                }
            }
            b'=' => {
                self.advance(b'=');
                match self.peek_byte() {
                    Ok(b'=') => {
                        self.advance(b'=');
                        Ok(Token::Operator(Operator::LogicEq))
                    }
                    Ok(b'>') => {
                        self.advance(b'>');
                        Ok(Token::Operator(Operator::BlockArrow))
                    }
                    _ => Ok(Token::Operator(Operator::Set)),
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
                                Ok(Token::Operator(Operator::Ellipsis))
                            }
                            _ => Ok(Token::Operator(Operator::Range)),
                        }
                    }
                    _ => Ok(Token::Operator(Operator::Dot)),
                }
            }

            _ => {
                error!(
                    "error[L0030]: The token `{}` is not valid. Did you mistype an operator or forget some whitespace?\n--> {}",
                    str::from_utf8(&[b]).unwrap_or("<invalid utf-8>"),
                    start_pos
                );

                Err(())
            }
        }
    }

    fn parse_next_token(&mut self) -> AnnotatedToken<'a, 'b> {
        self.read_while(|b| b.is_ascii_whitespace());

        let start_pos = self.current_position();

        let token = match self.peek_byte() {
            Err(()) => Ok(Token::Eof),
            Ok(b) => match b {
                b'`' => self.parse_atypical_identifier(),
                b if b.is_ascii_alphabetic() || b == b'_' || !b.is_ascii() /* Support UTF-8 identifiers */ => {
                    self.parse_typical_identifier()
                }
                b if b.is_ascii_digit() => self.parse_number(),
                b'"' => self.parse_string(),
                b'\'' => self.parse_char(),
                b'#' => self.parse_comment(),
                _ => self.parse_operator_or_punctuation(),
            },
        }
        .unwrap_or(Token::Illegal);

        let end_pos = self.current_position();

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

    let mut storage = StringStorage::new();
    let mut lexer =
        Lexer::new(&test_vector, "test_file", &mut storage).expect("Failed to create lexer");

    match lexer.next_token().token() {
        Token::String(s) => {
            assert_eq!(
                s.as_bytes(),
                expected.as_bytes(),
                "Parsed string does not match expected"
            );
        }
        _ => panic!("Expected a string token"),
    }
}
