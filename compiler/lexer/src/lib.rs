use log::error;
use stackvector::StackVec;

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
pub struct Identifier<'src> {
    name: &'src str,
    kind: IdentifierKind,
}

impl<'src> Identifier<'src> {
    pub fn new(name: &'src str, kind: IdentifierKind) -> Self {
        Identifier { name, kind }
    }

    pub fn name(&self) -> &str {
        self.name
    }

    pub fn kind(&self) -> IdentifierKind {
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
pub struct Integer<'src> {
    value: u128,
    origin: &'src str,
    kind: IntegerKind,
}

impl<'src> Integer<'src> {
    pub fn new(value: u128, origin: &'src str, kind: IntegerKind) -> Self {
        Integer {
            value,
            origin,
            kind,
        }
    }

    pub fn value(&self) -> u128 {
        self.value
    }

    pub fn origin(&self) -> &str {
        self.origin
    }

    pub fn kind(&self) -> IntegerKind {
        self.kind
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct Float<'src> {
    value: f64,
    origin: &'src str,
}

impl<'src> Float<'src> {
    pub fn new(value: f64, origin: &'src str) -> Self {
        Float { value, origin }
    }

    pub fn value(&self) -> f64 {
        self.value
    }

    pub fn origin(&self) -> &str {
        self.origin
    }
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum Keyword {
    /* Storage */
    Let,      /* 'let' */
    Var,      /* 'var' */
    Fn,       /* 'fn' */
    Enum,     /* 'enum' */
    Struct,   /* 'struct' */
    Class,    /* 'class' */
    Union,    /* 'union' */
    Contract, /* 'interface' */
    Trait,    /* 'trait' */
    Type,     /* 'type' */
    Opaque,   /* 'opaque' */
    Scope,    /* 'scope' */
    Import,   /* 'import' */
    UnitTest, /* 'unit_test' */

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
    Return,   /* 'ret' */
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
pub struct Comment<'src> {
    text: &'src str,
    kind: CommentKind,
}

impl<'src> Comment<'src> {
    pub fn new(text: &'src str, kind: CommentKind) -> Self {
        Comment { text, kind }
    }

    pub fn text(&self) -> &str {
        self.text
    }

    pub fn kind(&self) -> CommentKind {
        self.kind
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub enum Token<'src> {
    Identifier(Identifier<'src>),
    Integer(Integer<'src>),
    Float(Float<'src>),
    Keyword(Keyword),
    String(&'src str),
    Char(char),
    Punctuation(Punctuation),
    Operator(Operator),
    Comment(Comment<'src>),
    Eof,
    Illegal,
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct SourcePosition {
    line: u32,   // zero-based unicode-aware line number
    column: u32, // zero-based unicode-aware column number
    offset: u32, // zero-based raw byte offset number
}

impl SourcePosition {
    pub fn new(line: u32, column: u32, offset: u32) -> Self {
        SourcePosition {
            line,
            column,
            offset,
        }
    }

    pub fn line(&self) -> u32 {
        self.line
    }

    pub fn column(&self) -> u32 {
        self.column
    }

    pub fn offset(&self) -> u32 {
        self.offset
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct SourceRange<'src> {
    start: SourcePosition,
    end: SourcePosition,
    filename: &'src str,
}

impl<'src> SourceRange<'src> {
    pub fn new(start: SourcePosition, end: SourcePosition, filename: &'src str) -> Self {
        SourceRange {
            start,
            end,
            filename,
        }
    }

    pub fn eof(pos: SourcePosition, filename: &'src str) -> Self {
        SourceRange {
            start: pos.clone(),
            end: pos.clone(),
            filename,
        }
    }

    pub fn invalid() -> Self {
        SourceRange {
            start: SourcePosition::new(0, 0, 0),
            end: SourcePosition::new(0, 0, 0),
            filename: "",
        }
    }

    pub fn start(&self) -> &SourcePosition {
        &self.start
    }

    pub fn end(&self) -> &SourcePosition {
        &self.end
    }

    pub fn filename(&self) -> &'src str {
        self.filename
    }

    pub fn is_valid(&self) -> bool {
        self.start.offset < self.end.offset
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct AnnotatedToken<'src> {
    token: Token<'src>,
    range: SourceRange<'src>,
}

impl<'src> AnnotatedToken<'src> {
    pub fn new(token: Token<'src>, range: SourceRange<'src>) -> Self {
        AnnotatedToken { token, range }
    }

    pub fn token(&self) -> &Token<'src> {
        &self.token
    }

    pub fn range(&self) -> &SourceRange<'src> {
        &self.range
    }
}

#[derive(Debug, Clone)]
pub struct Lexer<'src> {
    src: &'src str,
    filename: &'src str,
    read_pos: SourcePosition,
    current: Option<AnnotatedToken<'src>>,
}

#[derive(Debug, Clone, Copy)]
pub enum LexerConstructionError {
    SourceTooBig,
}

impl<'src> Lexer<'src> {
    pub fn new(src: &'src str, filename: &'src str) -> Result<Self, LexerConstructionError> {
        if src.len() > MAX_SOURCE_SIZE {
            Err(LexerConstructionError::SourceTooBig)
        } else {
            Ok(Lexer {
                src,
                filename,
                read_pos: SourcePosition::new(0, 0, 0),
                current: None,
            })
        }
    }

    pub fn skip_token(&mut self) {
        if self.current.is_some() {
            self.current = None;
        } else {
            self.get_next_token(); // Discard the token
        }
    }

    pub fn next_token(&mut self) -> AnnotatedToken<'src> {
        self.current.take().unwrap_or_else(|| self.get_next_token())
    }

    pub fn peek_token(&mut self) -> AnnotatedToken<'src> {
        let token = self.current.take().unwrap_or_else(|| self.get_next_token());
        self.current = Some(token.clone());

        token
    }

    fn advance(&mut self, b: u8) {
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
    }

    fn peek_byte(&self) -> Option<u8> {
        self.src
            .as_bytes()
            .get(self.read_pos.offset() as usize)
            .copied()
    }

    fn read_while<F>(&mut self, mut condition: F) -> &'src [u8]
    where
        F: FnMut(u8) -> bool,
    {
        let start_offset = self.read_pos.offset() as usize;
        let mut end_offset = start_offset;

        while let Some(b) = self.src.as_bytes().get(end_offset) {
            if condition(*b) {
                self.advance(*b);
                end_offset += 1;
            } else {
                break;
            }
        }

        &self.src.as_bytes()[start_offset..end_offset]
    }

    fn read_identifier_token(&mut self) -> Option<Token<'src>> {
        if self.peek_byte()? == b'`' {
            self.advance(b'`');

            let identifier = self.read_while(|b| b != b'`');

            return if self.peek_byte().unwrap_or_default() != b'`' {
                error!(
                    "Lexer error: Unterminated atypical identifier @ (row {}, col {}, off {})",
                    self.read_pos.line(),
                    self.read_pos.column(),
                    self.read_pos.offset()
                );

                None
            } else {
                self.advance(b'`');

                if identifier.is_empty() {
                    error!(
                        "Lexer error: Atypical identifier is empty @ (row {}, col {}, off {})",
                        self.read_pos.line(),
                        self.read_pos.column(),
                        self.read_pos.offset()
                    );

                    None
                } else if let Some(identifier) = str::from_utf8(identifier).ok() {
                    Some(Token::Identifier(Identifier::new(
                        identifier,
                        IdentifierKind::Atypical,
                    )))
                } else {
                    error!(
                        "Lexer error: Atypical identifier contains invalid utf-8 @ (row {}, col {}, off {})",
                        self.read_pos.line(),
                        self.read_pos.column(),
                        self.read_pos.offset()
                    );

                    None
                }
            };
        } else {
            let code = self.read_while(|b| b.is_ascii_alphanumeric() || b == b'_' || !b.is_ascii());
            assert!(!code.is_empty(), "Identifier should not be empty");

            // Check for a word-like operator
            return if let Some(operator) = match code {
                b"as" => Some(Operator::As),
                b"bitcast_as" => Some(Operator::BitcastAs),
                b"sizeof" => Some(Operator::Sizeof),
                b"alignof" => Some(Operator::Alignof),
                b"typeof" => Some(Operator::Typeof),
                _ => None,
            } {
                Some(Token::Operator(operator))
            } else if let Some(keyword) = match code {
                b"let" => Some(Keyword::Let),
                b"var" => Some(Keyword::Var),
                b"fn" => Some(Keyword::Fn),
                b"enum" => Some(Keyword::Enum),
                b"struct" => Some(Keyword::Struct),
                b"class" => Some(Keyword::Class),
                b"union" => Some(Keyword::Union),
                b"interface" => Some(Keyword::Contract),
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
                b"ret" => Some(Keyword::Return),
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
                Some(Token::Keyword(keyword))
            } else {
                if let Some(identifier) = str::from_utf8(code).ok() {
                    Some(Token::Identifier(Identifier::new(
                        identifier,
                        IdentifierKind::Typical,
                    )))
                } else {
                    error!(
                        "Lexer error: Typical identifier contains invalid utf-8 @ (row {}, col {}, off {})",
                        self.read_pos.line(),
                        self.read_pos.column(),
                        self.read_pos.offset()
                    );

                    None
                }
            };
        }
    }

    fn read_number_token(&mut self) -> Option<Token<'src>> {
        // TODO: Implement read of number token
        None
    }

    fn read_string_token(&mut self) -> Option<Token<'src>> {
        // TODO: Implement read of string token
        None
    }

    fn read_char_token(&mut self) -> Option<Token<'src>> {
        assert!(self.peek_byte()? == b'\'');
        self.advance(b'\'');

        let mut char_buffer = StackVec::<[u8; 4]>::new();

        loop {
            match self.peek_byte()? {
                b if b == b'\\' => {
                    // TODO: Implement escape sequence handling
                }

                b'\'' => {
                    self.advance(b'\'');
                    let mut chars_iter = str::from_utf8(&char_buffer).ok()?.chars();
                    let character = chars_iter.next()?;

                    if chars_iter.next().is_some() {
                        return None;
                    } else {
                        return Some(Token::Char(character));
                    }
                }

                b => {
                    if char_buffer.len() >= char_buffer.capacity() {
                        return None;
                    }

                    char_buffer.push(b);
                    self.advance(b);
                }
            }
        }
    }

    fn read_comment_token(&mut self) -> Option<Token<'src>> {
        if let Some(comment) = str::from_utf8(self.read_while(|b| b != b'\n')).ok() {
            Some(Token::Comment(Comment::new(
                comment,
                CommentKind::SingleLine,
            )))
        } else {
            error!(
                "Lexer error: Single-line comment contains invalid utf-8 @ (row {}, col {}, off {})",
                self.read_pos.line(),
                self.read_pos.column(),
                self.read_pos.offset()
            );

            None
        }
    }

    fn read_punctuation_token(&mut self) -> Option<Token<'src>> {
        /*
         * The colon punctuator is not handled here, as it is ambiguous with the scope
         * operator "::". See `read_operator_token` for the handling the colon punctuator.
         */

        let b = self.peek_byte()?;
        self.advance(b);

        Some(Token::Punctuation(match b {
            b'(' => Punctuation::LeftParenthesis,
            b')' => Punctuation::RightParenthesis,
            b'[' => Punctuation::LeftBracket,
            b']' => Punctuation::RightBracket,
            b'{' => Punctuation::LeftBrace,
            b'}' => Punctuation::RightBrace,
            b',' => Punctuation::Comma,
            b';' => Punctuation::Semicolon,
            b'@' => Punctuation::AtSign,

            _ => {
                error!(
                    "Lexer error: Invalid punctuation character '{}' @ (row {}, col {}, off {})",
                    b as char,
                    self.read_pos.line(),
                    self.read_pos.column(),
                    self.read_pos.offset()
                );

                return None;
            }
        }))
    }

    fn read_operator_token(&mut self) -> Option<Token<'src>> {
        /*
         * The word-like operators are not handled here, as they are ambiguous with identifiers.
         * They are handled in `read_identifier_token`.
         */

        let code = self.read_while(|b| {
            match b {
                b if b.is_ascii_whitespace() => false,           // whitespace
                b if b.is_ascii_digit() => false,                // number
                b'_' | b'`' if b.is_ascii_alphabetic() => false, // identifier
                b'"' | b'\'' => false,                           // string and char literals
                b'#' => false,                                   // comments
                b'(' | b')' | b'[' | b']' | b'{' | b'}' | b',' | b';' | b'@' => false, // punctuation

                _ => true, // operators and colon punctuator
            }
        });

        // Handle the colon punctuator because it is ambiguous with the scope operator "::".
        if code == b":" {
            return Some(Token::Punctuation(Punctuation::Colon));
        }

        Some(Token::Operator(match code {
            b"+" => Operator::Add,
            b"-" => Operator::Sub,
            b"*" => Operator::Mul,
            b"/" => Operator::Div,
            b"%" => Operator::Mod,

            b"&" => Operator::BitAnd,
            b"|" => Operator::BitOr,
            b"^" => Operator::BitXor,
            b"~" => Operator::BitNot,
            b"<<" => Operator::BitShl,
            b">>" => Operator::BitShr,
            b"<<<" => Operator::BitRotl,
            b">>>" => Operator::BitRotr,

            b"&&" => Operator::LogicAnd,
            b"||" => Operator::LogicOr,
            b"^^" => Operator::LogicXor,
            b"!" => Operator::LogicNot,
            b"<" => Operator::LogicLt,
            b">" => Operator::LogicGt,
            b"<=" => Operator::LogicLe,
            b">=" => Operator::LogicGe,
            b"==" => Operator::LogicEq,
            b"!=" => Operator::LogicNe,

            b"=" => Operator::Set,
            b"+=" => Operator::SetPlus,
            b"-=" => Operator::SetMinus,
            b"*=" => Operator::SetTimes,
            b"/=" => Operator::SetSlash,
            b"%=" => Operator::SetPercent,
            b"&=" => Operator::SetBitAnd,
            b"|=" => Operator::SetBitOr,
            b"^=" => Operator::SetBitXor,
            b"<<=" => Operator::SetBitShl,
            b">>=" => Operator::SetBitShr,
            b"<<<=" => Operator::SetBitRotl,
            b">>>=" => Operator::SetBitRotr,
            b"&&=" => Operator::SetLogicAnd,
            b"||=" => Operator::SetLogicOr,
            b"^^=" => Operator::SetLogicXor,
            b"++" => Operator::Inc,
            b"--" => Operator::Dec,

            b"." => Operator::Dot,
            b"..." => Operator::Ellipsis,
            b"::" => Operator::Scope,
            b"->" => Operator::Arrow,
            b"=>" => Operator::BlockArrow,

            b".." => Operator::Range,
            b"?" => Operator::Question,
            b"<=>" => Operator::Spaceship,

            _ => {
                if let Some(utf8_op) = str::from_utf8(code).ok() {
                    error!(
                        "Lexer error: Invalid operator '{}' @ (row {}, col {}, off {})",
                        utf8_op,
                        self.read_pos.line(),
                        self.read_pos.column(),
                        self.read_pos.offset()
                    );
                } else {
                    error!(
                        "Lexer error: Invalid operator '{:?}' @ (row {}, col {}, off {})",
                        code,
                        self.read_pos.line(),
                        self.read_pos.column(),
                        self.read_pos.offset()
                    );
                }

                return None;
            }
        }))
    }

    fn get_next_token(&mut self) -> AnnotatedToken<'src> {
        self.read_while(|b| b.is_ascii_whitespace());

        let start_pos = self.read_pos.clone();

        let token = match self.peek_byte() {
            None => Some(Token::Eof),
            Some(b) => match b {
                b if b.is_ascii_alphabetic() || b == b'_' || b == b'`' || !b.is_ascii() => {
                    self.read_identifier_token()
                }
                b if b.is_ascii_digit() => self.read_number_token(),
                b'"' => self.read_string_token(),
                b'\'' => self.read_char_token(),
                b if b == b'('
                    || b == b')'
                    || b == b'['
                    || b == b']'
                    || b == b'{'
                    || b == b'}'
                    || b == b','
                    || b == b';'
                    || b == b'@' =>
                {
                    self.read_punctuation_token()
                }
                b'#' => self.read_comment_token(),

                _ => self.read_operator_token(),
            },
        }
        .unwrap_or(Token::Illegal);

        let end_pos = self.read_pos.clone();
        let range = SourceRange::new(start_pos, end_pos, self.filename);

        AnnotatedToken::new(token, range)
    }
}
