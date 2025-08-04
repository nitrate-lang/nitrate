use smallvec::SmallVec;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum IdentifierKind {
    Typical,
    Atypical,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Identifier<'a> {
    name: &'a str,
    kind: IdentifierKind,
}

impl<'a> Identifier<'a> {
    pub fn new(name: &'a str) -> Self {
        Identifier {
            name,
            kind: Identifier::get_kind(name),
        }
    }

    pub fn new_typical(name: &'a str) -> Self {
        assert!(
            Identifier::is_typical(name),
            "Expected a typical identifier"
        );
        Identifier {
            name,
            kind: IdentifierKind::Typical,
        }
    }

    pub fn new_atypical(name: &'a str) -> Self {
        assert!(
            Identifier::is_atypical(name),
            "Expected an atypical identifier"
        );
        Identifier {
            name,
            kind: IdentifierKind::Atypical,
        }
    }

    pub const fn name(&self) -> &'a str {
        self.name
    }

    pub const fn kind(&self) -> IdentifierKind {
        self.kind
    }

    pub fn is_typical(name: &str) -> bool {
        let mut iter = name.chars();

        if let Some(first) = iter.next() {
            if first.is_ascii_alphabetic() || first == '_' || !first.is_ascii() {
                return iter.all(|c| c.is_ascii_alphanumeric() || c == '_' || !c.is_ascii());
            }
        }

        false
    }

    pub fn is_atypical(name: &str) -> bool {
        !Self::is_typical(name)
    }

    pub fn get_kind(name: &str) -> IdentifierKind {
        if Self::is_typical(name) {
            IdentifierKind::Typical
        } else {
            IdentifierKind::Atypical
        }
    }
}

impl<'a> std::fmt::Display for Identifier<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.kind() {
            IdentifierKind::Typical => write!(f, "{}", self.name()),
            IdentifierKind::Atypical => write!(f, "`{}`", self.name()),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum IntegerKind {
    Binary,
    Octal,
    Decimal,
    Hexadecimal,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Integer {
    value: u128,
    kind: IntegerKind,
}

impl Integer {
    pub const fn new(value: u128, kind: IntegerKind) -> Self {
        Integer { value, kind }
    }

    pub const fn value(&self) -> u128 {
        self.value
    }

    pub const fn kind(&self) -> IntegerKind {
        self.kind
    }
}

impl std::fmt::Display for Integer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.kind() {
            IntegerKind::Binary => write!(f, "0b{:b}", self.value()),
            IntegerKind::Octal => write!(f, "0o{:o}", self.value()),
            IntegerKind::Decimal => write!(f, "{}", self.value()),
            IntegerKind::Hexadecimal => write!(f, "0x{:x}", self.value()),
        }
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct Float {
    value: f64,
}

impl Float {
    pub const fn new(value: f64) -> Self {
        Float { value }
    }

    pub const fn value(&self) -> f64 {
        self.value
    }
}

impl std::hash::Hash for Float {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.value.to_bits().hash(state);
    }
}

impl std::cmp::Eq for Float {}

impl std::fmt::Display for Float {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.value())
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
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

impl std::fmt::Display for Keyword {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Keyword::Let => write!(f, "let"),
            Keyword::Var => write!(f, "var"),
            Keyword::Fn => write!(f, "fn"),
            Keyword::Enum => write!(f, "enum"),
            Keyword::Struct => write!(f, "struct"),
            Keyword::Class => write!(f, "class"),
            Keyword::Union => write!(f, "union"),
            Keyword::Interface => write!(f, "interface"),
            Keyword::Trait => write!(f, "trait"),
            Keyword::Type => write!(f, "type"),
            Keyword::Opaque => write!(f, "opaque"),
            Keyword::Scope => write!(f, "scope"),
            Keyword::Import => write!(f, "import"),
            Keyword::UnitTest => write!(f, "unit_test"),

            Keyword::Safe => write!(f, "safe"),
            Keyword::Unsafe => write!(f, "unsafe"),
            Keyword::Promise => write!(f, "promise"),
            Keyword::Static => write!(f, "static"),
            Keyword::Mut => write!(f, "mut"),
            Keyword::Const => write!(f, "const"),
            Keyword::Pub => write!(f, "pub"),
            Keyword::Sec => write!(f, "sec"),
            Keyword::Pro => write!(f, "pro"),

            Keyword::If => write!(f, "if"),
            Keyword::Else => write!(f, "else"),
            Keyword::For => write!(f, "for"),
            Keyword::While => write!(f, "while"),
            Keyword::Do => write!(f, "do"),
            Keyword::Switch => write!(f, "switch"),
            Keyword::Break => write!(f, "break"),
            Keyword::Continue => write!(f, "continue"),
            Keyword::Ret => write!(f, "ret"),
            Keyword::Foreach => write!(f, "foreach"),
            Keyword::Try => write!(f, "try"),
            Keyword::Catch => write!(f, "catch"),
            Keyword::Throw => write!(f, "throw"),
            Keyword::Async => write!(f, "async"),
            Keyword::Await => write!(f, "await"),
            Keyword::Asm => write!(f, "asm"),

            // Literals
            Keyword::Null => write!(f, "null"),
            Keyword::True => write!(f, "true"),
            Keyword::False => write!(f, "false"),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum Punct {
    LeftParen,    /* '(' */
    RightParen,   /* ')' */
    LeftBracket,  /* '[' */
    RightBracket, /* ']' */
    LeftBrace,    /* '{' */
    RightBrace,   /* '}' */
    Comma,        /* ',' */
    Semicolon,    /* ';' */
    Colon,        /* ':' */
    AtSign,       /* '@' */
}

impl std::fmt::Display for Punct {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Punct::LeftParen => write!(f, "("),
            Punct::RightParen => write!(f, ")"),
            Punct::LeftBracket => write!(f, "["),
            Punct::RightBracket => write!(f, "]"),
            Punct::LeftBrace => write!(f, "{{"),
            Punct::RightBrace => write!(f, "}}"),
            Punct::Comma => write!(f, ","),
            Punct::Semicolon => write!(f, ";"),
            Punct::Colon => write!(f, ":"),
            Punct::AtSign => write!(f, "@"),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
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

impl std::fmt::Display for Operator {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Operator::Add => write!(f, "+"),
            Operator::Sub => write!(f, "-"),
            Operator::Mul => write!(f, "*"),
            Operator::Div => write!(f, "/"),
            Operator::Mod => write!(f, "%"),

            Operator::BitAnd => write!(f, "&"),
            Operator::BitOr => write!(f, "|"),
            Operator::BitXor => write!(f, "^"),
            Operator::BitNot => write!(f, "~"),
            Operator::BitShl => write!(f, "<<"),
            Operator::BitShr => write!(f, ">>"),
            Operator::BitRotl => write!(f, "<<<"),
            Operator::BitRotr => write!(f, ">>>"),

            Operator::LogicAnd => write!(f, "&&"),
            Operator::LogicOr => write!(f, "||"),
            Operator::LogicXor => write!(f, "^^"),
            Operator::LogicNot => write!(f, "!"),
            Operator::LogicLt => write!(f, "<"),
            Operator::LogicGt => write!(f, ">"),
            Operator::LogicLe => write!(f, "<="),
            Operator::LogicGe => write!(f, ">="),
            Operator::LogicEq => write!(f, "=="),
            Operator::LogicNe => write!(f, "!="),

            Operator::Set => write!(f, "="),
            Operator::SetPlus => write!(f, "+="),
            Operator::SetMinus => write!(f, "-="),
            Operator::SetTimes => write!(f, "*="),
            Operator::SetSlash => write!(f, "/="),
            Operator::SetPercent => write!(f, "%="),
            Operator::SetBitAnd => write!(f, "&="),
            Operator::SetBitOr => write!(f, "|="),
            Operator::SetBitXor => write!(f, "^="),
            Operator::SetBitShl => write!(f, "<<="),
            Operator::SetBitShr => write!(f, ">>="),
            Operator::SetBitRotl => write!(f, "<<<="),
            Operator::SetBitRotr => write!(f, ">>>="),
            Operator::SetLogicAnd => write!(f, "&&="),
            Operator::SetLogicOr => write!(f, "||="),
            Operator::SetLogicXor => write!(f, "^^="),
            Operator::Inc => write!(f, "++"),
            Operator::Dec => write!(f, "--"),

            Operator::As => write!(f, "as"),
            Operator::BitcastAs => write!(f, "bitcast_as"),
            Operator::Sizeof => write!(f, "sizeof"),
            Operator::Alignof => write!(f, "alignof"),
            Operator::Typeof => write!(f, "typeof"),

            Operator::Dot => write!(f, "."),
            Operator::Ellipsis => write!(f, "..."),
            Operator::Scope => write!(f, "::"),
            Operator::Arrow => write!(f, "->"),
            Operator::BlockArrow => write!(f, "=>"),

            Operator::Range => write!(f, ".."),
            Operator::Question => write!(f, "?"),
            Operator::Spaceship => write!(f, "<=>"),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
enum StringData<'a> {
    RefString(&'a [u8]),
    DynString(SmallVec<[u8; 64]>),
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StringLit<'a> {
    data: StringData<'a>,
    is_utf8: bool,
}

impl<'a> StringLit<'a> {
    pub const fn from_ref(data: &'a [u8]) -> Self {
        StringLit {
            data: StringData::RefString(data),
            is_utf8: str::from_utf8(data).is_ok(),
        }
    }

    pub fn from_dyn(data: SmallVec<[u8; 64]>) -> Self {
        StringLit {
            is_utf8: str::from_utf8(data.as_slice()).is_ok(),
            data: StringData::DynString(data),
        }
    }

    pub fn data(&self) -> &[u8] {
        match &self.data {
            StringData::RefString(s) => s,
            StringData::DynString(s) => s.as_slice(),
        }
    }
}

impl<'a> std::ops::Deref for StringLit<'a> {
    type Target = [u8];

    fn deref(&self) -> &Self::Target {
        self.data()
    }
}

impl<'a> std::fmt::Debug for StringLit<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if self.is_utf8 {
            write!(f, "StringLit({:?})", unsafe {
                str::from_utf8_unchecked(self.data())
            })
        } else {
            write!(f, "StringLit({:?})", self.data())
        }
    }
}

impl<'a> std::fmt::Display for StringLit<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        // TODO: Implement display for StringLit
        Ok(())
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum CommentKind {
    SingleLine,
    MultiLine,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
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

impl<'a> std::fmt::Display for Comment<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.kind() {
            CommentKind::SingleLine => write!(f, "#{}", self.text()),
            CommentKind::MultiLine => unimplemented!(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub enum Token<'a> {
    Name(Identifier<'a>),
    Integer(Integer),
    Float(Float),
    Keyword(Keyword),
    String(StringLit<'a>),
    Char(char),
    Punct(Punct),
    Op(Operator),
    Comment(Comment<'a>),
    Eof,
    Illegal,
}

impl<'a> std::fmt::Display for Token<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Token::Name(id) => write!(f, "{}", id),
            Token::Integer(int) => write!(f, "{}", int),
            Token::Float(float) => write!(f, "{}", float),
            Token::Keyword(kw) => write!(f, "{}", kw),
            Token::String(s) => write!(f, "{}", s),
            Token::Char(c) => write!(f, "'{}'", c),
            Token::Punct(p) => write!(f, "{}", p),
            Token::Op(op) => write!(f, "{}", op),
            Token::Comment(c) => write!(f, "{}", c),
            Token::Eof => write!(f, ""),
            Token::Illegal => write!(f, "<illegal>"),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
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

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct AnnotatedToken<'a> {
    token: Token<'a>,

    start_line: u32,
    start_column: u32,
    start_offset: u32,

    end_line: u32,
    end_column: u32,
    end_offset: u32,

    filename: &'a str,
}

impl<'a> AnnotatedToken<'a> {
    pub const fn new(token: Token<'a>, start: SourcePosition<'a>, end: SourcePosition<'a>) -> Self {
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

    pub fn into_token(self) -> Token<'a> {
        self.token
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
