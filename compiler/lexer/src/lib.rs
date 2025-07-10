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

pub struct LexerPosition<'src> {
    filename: &'src str,
    line: u32,
    column: u32,
}

impl<'src> LexerPosition<'src> {
    pub fn new(pos: SourcePosition, filename: &'src str) -> Self {
        LexerPosition {
            filename,
            line: pos.line(),
            column: pos.column(),
        }
    }
}

impl std::fmt::Display for LexerPosition<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}:{}:{}", self.filename, self.line + 1, self.column + 1)
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
    source: &'src [u8],
    filename: &'src str,
    read_pos: SourcePosition,
    current: Option<AnnotatedToken<'src>>,
}

#[derive(Debug, Clone, Copy)]
pub enum LexerConstructionError {
    SourceTooBig,
}

impl<'src> Lexer<'src> {
    pub fn new(src: &'src [u8], filename: &'src str) -> Result<Self, LexerConstructionError> {
        if src.len() > MAX_SOURCE_SIZE {
            Err(LexerConstructionError::SourceTooBig)
        } else {
            Ok(Lexer {
                source: src,
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

    pub fn current_position(&self) -> LexerPosition<'src> {
        LexerPosition::new(self.read_pos.clone(), self.filename)
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
        self.source.get(self.read_pos.offset() as usize).copied()
    }

    fn read_while<F>(&mut self, mut condition: F) -> &'src [u8]
    where
        F: FnMut(u8) -> bool,
    {
        let start_offset = self.read_pos.offset() as usize;
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

    fn parse_atypical_identifier(&mut self) -> Option<Token<'src>> {
        let start_pos = self.current_position();

        assert!(self.peek_byte()? == b'`');
        self.advance(b'`');

        let identifier = self.read_while(|b| b != b'`');

        if self.peek_byte().unwrap_or_default() == b'`' {
            self.advance(b'`');
        } else {
            error!(
                "error[L0001]: Unterminated atypical identifier. Did you forget the '`' terminator?\n--> {}",
                start_pos
            );

            return None;
        }

        if identifier.is_empty() {
            error!(
                "error[L0002]: Atypical identifier does not contain at least one character.\n--> {}",
                start_pos
            );

            None
        } else if let Ok(identifier) = str::from_utf8(identifier) {
            Some(Token::Identifier(Identifier::new(
                identifier,
                IdentifierKind::Atypical,
            )))
        } else {
            error!(
                "error[L0003]: Identifier contains some invalid utf-8 bytes\n--> {}",
                start_pos
            );

            None
        }
    }

    fn parse_typical_identifier(&mut self) -> Option<Token<'src>> {
        let start_pos = self.current_position();

        let code = self.read_while(|b| b.is_ascii_alphanumeric() || b == b'_' || !b.is_ascii());
        assert!(!code.is_empty(), "Identifier should not be empty");

        // Check for a word-like operator
        if let Some(operator) = match code {
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
        } else if let Ok(identifier) = str::from_utf8(code) {
            Some(Token::Identifier(Identifier::new(
                identifier,
                IdentifierKind::Typical,
            )))
        } else {
            error!(
                "error[L0003]: Identifier contains some invalid utf-8 bytes\n--> {}",
                start_pos
            );

            None
        }
    }

    fn parse_number(&mut self) -> Option<Token<'src>> {
        // TODO: Implement read of number token
        None
    }

    fn parse_string(&mut self) -> Option<Token<'src>> {
        // TODO: Implement read of string token
        None
    }

    fn parse_char(&mut self) -> Option<Token<'src>> {
        let start_pos = self.current_position();

        assert!(self.peek_byte()? == b'\'');
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

                return None;
            }

            match self.peek_byte()? {
                b'\\' => {
                    self.advance(b'\\');

                    match self.peek_byte()? {
                        b'0' => {
                            char_buffer.push(b'\0');
                            self.advance(b'0');
                        }
                        b'a' => {
                            char_buffer.push(b'\x07');
                            self.advance(b'a');
                        }
                        b'b' => {
                            char_buffer.push(b'\x08');
                            self.advance(b'b');
                        }
                        b't' => {
                            char_buffer.push(b'\t');
                            self.advance(b't');
                        }
                        b'n' => {
                            char_buffer.push(b'\n');
                            self.advance(b'n');
                        }
                        b'v' => {
                            char_buffer.push(b'\x0b');
                            self.advance(b'v');
                        }
                        b'f' => {
                            char_buffer.push(b'\x0c');
                            self.advance(b'f');
                        }
                        b'r' => {
                            char_buffer.push(b'\r');
                            self.advance(b'r');
                        }
                        b'\\' => {
                            char_buffer.push(b'\\');
                            self.advance(b'\\');
                        }
                        b'\'' => {
                            char_buffer.push(b'\'');
                            self.advance(b'\'');
                        }

                        b => {
                            error!(
                                "error[L0013]: Invalid escape sequence '\\{}' in character literal\n--> {}",
                                b as char, start_pos
                            );

                            return None;
                        }
                    }
                }

                b'\'' => {
                    self.advance(b'\'');

                    let chars_buffer = str::from_utf8(&char_buffer);
                    if chars_buffer.is_err() {
                        error!(
                            "error[L0012]: Character literal '{:?}' contains some invalid utf-8 bytes\n--> {}",
                            char_buffer.as_slice() as &[u8],
                            start_pos
                        );

                        return None;
                    }

                    let chars_buffer = chars_buffer.expect("Invalid UTF-8");
                    if chars_buffer.is_empty() {
                        error!(
                            "error[L0011]: Character literal is empty. Did you forget to specify the character?\n--> {}",
                            start_pos
                        );

                        return None;
                    }

                    let mut chars_iter = chars_buffer.chars();
                    let character = chars_iter.next()?;

                    if chars_iter.next().is_some() {
                        error!(
                            "error[L0010]: Character literal '{}' contains more than one character. Did you mean to use a string literal?\n--> {}",
                            str::from_utf8(&char_buffer).unwrap_or("<invalid utf-8>"),
                            start_pos
                        );

                        return None;
                    }

                    return Some(Token::Char(character));
                }

                b => {
                    char_buffer.push(b);
                    self.advance(b);
                }
            }
        }
    }

    fn parse_comment(&mut self) -> Option<Token<'src>> {
        let start_pos = self.current_position();
        let comment_bytes = self.read_while(|b| b != b'\n');

        if let Ok(comment) = str::from_utf8(&comment_bytes) {
            Some(Token::Comment(Comment::new(
                comment,
                CommentKind::SingleLine,
            )))
        } else {
            error!(
                "error[L0020]: Single-line comment contains some invalid utf-8 bytes\n--> {}",
                start_pos
            );

            None
        }
    }

    fn parse_punctuation(&mut self) -> Option<Token<'src>> {
        /*
         * The colon punctuator is not handled here, as it is ambiguous with the scope
         * operator "::". See `parse_operator` for the handling the colon punctuator.
         */

        let start_pos = self.current_position();

        let b = self.peek_byte()?;
        let punctor = match b {
            b'(' => Some(Punctuation::LeftParenthesis),
            b')' => Some(Punctuation::RightParenthesis),
            b'[' => Some(Punctuation::LeftBracket),
            b']' => Some(Punctuation::RightBracket),
            b'{' => Some(Punctuation::LeftBrace),
            b'}' => Some(Punctuation::RightBrace),
            b',' => Some(Punctuation::Comma),
            b';' => Some(Punctuation::Semicolon),
            b'@' => Some(Punctuation::AtSign),

            _ => {
                error!(
                    "error[L0030]: The token `{}` is not valid. Did you mistype an operator or forget some whitespace?\n--> {}",
                    b as char, start_pos
                );

                None
            }
        };

        if let Some(punctor) = punctor {
            self.advance(b);

            Some(Token::Punctuation(punctor))
        } else {
            None
        }
    }

    fn parse_operator(&mut self) -> Option<Token<'src>> {
        /*
         * The word-like operators are not handled here, as they are ambiguous with identifiers.
         * They are handled in `parse_identifier`.
         */

        let start_pos = self.current_position();

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
                error!(
                    "error[L0030]: The token `{}` is not valid. Did you mistype an operator or forget some whitespace?\n--> {}",
                    str::from_utf8(code).unwrap_or("<invalid utf-8>"),
                    start_pos
                );

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
                b'`' => self.parse_atypical_identifier(),
                b if b.is_ascii_alphabetic() || b == b'_' || !b.is_ascii() => {
                    self.parse_typical_identifier()
                }
                b if b.is_ascii_digit() => self.parse_number(),
                b'"' => self.parse_string(),
                b'\'' => self.parse_char(),
                b'(' | b')' | b'[' | b']' | b'{' | b'}' | b',' | b';' | b'@' => {
                    self.parse_punctuation()
                }
                b'#' => self.parse_comment(),

                _ => self.parse_operator(),
            },
        }
        .unwrap_or(Token::Illegal);

        let end_pos = self.read_pos.clone();
        let range = SourceRange::new(start_pos, end_pos, self.filename);

        AnnotatedToken::new(token, range)
    }
}
