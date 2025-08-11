use smallvec::SmallVec;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum IdentifierKind {
    Typical,
    Atypical,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Name<'a> {
    name: &'a str,
    kind: IdentifierKind,
}

impl<'a> Name<'a> {
    #[must_use]
    pub fn new(name: &'a str) -> Self {
        Name {
            name,
            kind: Name::get_kind(name),
        }
    }

    #[must_use]
    pub fn new_typical(name: &'a str) -> Self {
        assert!(Name::is_typical(name), "Expected a typical identifier");
        Name {
            name,
            kind: IdentifierKind::Typical,
        }
    }

    #[must_use]
    pub fn new_atypical(name: &'a str) -> Self {
        assert!(Name::is_atypical(name), "Expected an atypical identifier");
        Name {
            name,
            kind: IdentifierKind::Atypical,
        }
    }

    #[must_use]
    pub const fn into_name(self) -> &'a str {
        self.name
    }

    #[must_use]
    pub const fn name(&self) -> &'a str {
        self.name
    }

    #[must_use]
    pub const fn kind(&self) -> IdentifierKind {
        self.kind
    }

    #[must_use]
    pub fn is_typical(name: &str) -> bool {
        let mut iter = name.chars();

        if let Some(first) = iter.next() {
            if first.is_ascii_alphabetic() || first == '_' || !first.is_ascii() {
                return iter.all(|c| c.is_ascii_alphanumeric() || c == '_' || !c.is_ascii());
            }
        }

        false
    }

    #[must_use]
    pub fn is_atypical(name: &str) -> bool {
        !Self::is_typical(name)
    }

    #[must_use]
    pub fn get_kind(name: &str) -> IdentifierKind {
        if Self::is_typical(name) {
            IdentifierKind::Typical
        } else {
            IdentifierKind::Atypical
        }
    }
}

impl std::fmt::Display for Name<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.kind() {
            IdentifierKind::Typical => write!(f, "{}", self.name()),
            IdentifierKind::Atypical => write!(f, "`{}`", self.name()),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum IntegerKind {
    Bin,
    Oct,
    Dec,
    Hex,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct Integer {
    value: u128,
    kind: IntegerKind,
}

impl Integer {
    #[must_use]
    pub const fn new(value: u128, kind: IntegerKind) -> Self {
        Integer { value, kind }
    }

    #[must_use]
    pub const fn value(&self) -> u128 {
        self.value
    }

    #[must_use]
    pub const fn into_value(self) -> u128 {
        self.value
    }

    #[must_use]
    pub const fn kind(&self) -> IntegerKind {
        self.kind
    }
}

impl std::fmt::Display for Integer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.kind() {
            IntegerKind::Bin => write!(f, "0b{:b}", self.value()),
            IntegerKind::Oct => write!(f, "0o{:o}", self.value()),
            IntegerKind::Dec => write!(f, "{}", self.value()),
            IntegerKind::Hex => write!(f, "0x{:x}", self.value()),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
enum StringDataStorage<'a> {
    RefString(&'a str),
    DynString(String),
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct StringData<'a> {
    data: StringDataStorage<'a>,
}

impl<'a> StringData<'a> {
    #[must_use]
    pub const fn from_ref(data: &'a str) -> Self {
        StringData {
            data: StringDataStorage::RefString(data),
        }
    }

    #[must_use]
    pub fn from_dyn(data: String) -> Self {
        StringData {
            data: StringDataStorage::DynString(data),
        }
    }

    #[must_use]
    pub fn get(&self) -> &str {
        match &self.data {
            StringDataStorage::RefString(s) => s,
            StringDataStorage::DynString(s) => s.as_str(),
        }
    }
}

impl std::fmt::Debug for StringData<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "StringData({:?})", self.get())
    }
}

impl std::fmt::Display for StringData<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "\"{}\"", self.get())
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash)]
enum BStringDataStorage<'a> {
    RefString(&'a [u8]),
    DynString(SmallVec<[u8; 64]>),
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Hash)]
pub struct BStringData<'a> {
    data: BStringDataStorage<'a>,
}

impl<'a> BStringData<'a> {
    #[must_use]
    pub const fn from_ref(data: &'a [u8]) -> Self {
        BStringData {
            data: BStringDataStorage::RefString(data),
        }
    }

    #[must_use]
    pub fn from_dyn(data: SmallVec<[u8; 64]>) -> Self {
        BStringData {
            data: BStringDataStorage::DynString(data),
        }
    }

    #[must_use]
    pub fn get(&self) -> &[u8] {
        match &self.data {
            BStringDataStorage::RefString(s) => s,
            BStringDataStorage::DynString(s) => s.as_slice(),
        }
    }
}

impl std::fmt::Debug for BStringData<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "BStringData({:?})", self.get())
    }
}

impl std::fmt::Display for BStringData<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for byte in self.get() {
            write!(f, "{byte:02x} ")?;
        }
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
    #[must_use]
    pub const fn new(text: &'a str, kind: CommentKind) -> Self {
        Comment { text, kind }
    }

    #[must_use]
    pub const fn text(&self) -> &str {
        self.text
    }

    #[must_use]
    pub const fn into_text(self) -> &'a str {
        self.text
    }

    #[must_use]
    pub const fn kind(&self) -> CommentKind {
        self.kind
    }
}

impl std::fmt::Display for Comment<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.kind() {
            CommentKind::SingleLine => write!(f, "#{}", self.text()),
            CommentKind::MultiLine => unimplemented!(),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub enum Keyword {
    /* Storage */
    Let,      /* 'let' */
    Var,      /* 'var' */
    Fn,       /* 'fn' */
    Enum,     /* 'enum' */
    Struct,   /* 'struct' */
    Class,    /* 'class' */
    Contract, /* 'contract' */
    Trait,    /* 'trait' */
    Impl,     /* 'impl' */
    Type,     /* 'type' */
    Scope,    /* 'scope' */
    Import,   /* 'import' */

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
    Async,    /* 'async' */
    Await,    /* 'await' */
    Asm,      /* 'asm' */
    Assert,   /* 'assert' */

    /* Literals */
    Null,  /* 'null' */
    True,  /* 'true' */
    False, /* 'false' */

    /* Type Keywords */
    Bool,   /* 'bool' */
    U8,     /* 'u8' */
    U16,    /* 'u16' */
    U32,    /* 'u32' */
    U64,    /* 'u64' */
    U128,   /* 'u128' */
    I8,     /* 'i8' */
    I16,    /* 'i16' */
    I32,    /* 'i32' */
    I64,    /* 'i64' */
    I128,   /* 'i128' */
    F8,     /* 'f8' */
    F16,    /* 'f16' */
    F32,    /* 'f32' */
    F64,    /* 'f64' */
    F128,   /* 'f128' */
    Opaque, /* 'opaque' */
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
            Keyword::Contract => write!(f, "contract"),
            Keyword::Trait => write!(f, "trait"),
            Keyword::Impl => write!(f, "impl"),
            Keyword::Type => write!(f, "type"),
            Keyword::Scope => write!(f, "scope"),
            Keyword::Import => write!(f, "import"),

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
            Keyword::Async => write!(f, "async"),
            Keyword::Await => write!(f, "await"),
            Keyword::Asm => write!(f, "asm"),
            Keyword::Assert => write!(f, "assert"),

            Keyword::Null => write!(f, "null"),
            Keyword::True => write!(f, "true"),
            Keyword::False => write!(f, "false"),

            Keyword::Bool => write!(f, "bool"),
            Keyword::U8 => write!(f, "u8"),
            Keyword::U16 => write!(f, "u16"),
            Keyword::U32 => write!(f, "u32"),
            Keyword::U64 => write!(f, "u64"),
            Keyword::U128 => write!(f, "u128"),
            Keyword::I8 => write!(f, "i8"),
            Keyword::I16 => write!(f, "i16"),
            Keyword::I32 => write!(f, "i32"),
            Keyword::I64 => write!(f, "i64"),
            Keyword::I128 => write!(f, "i128"),
            Keyword::F8 => write!(f, "f8"),
            Keyword::F16 => write!(f, "f16"),
            Keyword::F32 => write!(f, "f32"),
            Keyword::F64 => write!(f, "f64"),
            Keyword::F128 => write!(f, "f128"),
            Keyword::Opaque => write!(f, "opaque"),
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
pub enum Op {
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

impl std::fmt::Display for Op {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Op::Add => write!(f, "+"),
            Op::Sub => write!(f, "-"),
            Op::Mul => write!(f, "*"),
            Op::Div => write!(f, "/"),
            Op::Mod => write!(f, "%"),

            Op::BitAnd => write!(f, "&"),
            Op::BitOr => write!(f, "|"),
            Op::BitXor => write!(f, "^"),
            Op::BitNot => write!(f, "~"),
            Op::BitShl => write!(f, "<<"),
            Op::BitShr => write!(f, ">>"),
            Op::BitRotl => write!(f, "<<<"),
            Op::BitRotr => write!(f, ">>>"),

            Op::LogicAnd => write!(f, "&&"),
            Op::LogicOr => write!(f, "||"),
            Op::LogicXor => write!(f, "^^"),
            Op::LogicNot => write!(f, "!"),
            Op::LogicLt => write!(f, "<"),
            Op::LogicGt => write!(f, ">"),
            Op::LogicLe => write!(f, "<="),
            Op::LogicGe => write!(f, ">="),
            Op::LogicEq => write!(f, "=="),
            Op::LogicNe => write!(f, "!="),

            Op::Set => write!(f, "="),
            Op::SetPlus => write!(f, "+="),
            Op::SetMinus => write!(f, "-="),
            Op::SetTimes => write!(f, "*="),
            Op::SetSlash => write!(f, "/="),
            Op::SetPercent => write!(f, "%="),
            Op::SetBitAnd => write!(f, "&="),
            Op::SetBitOr => write!(f, "|="),
            Op::SetBitXor => write!(f, "^="),
            Op::SetBitShl => write!(f, "<<="),
            Op::SetBitShr => write!(f, ">>="),
            Op::SetBitRotl => write!(f, "<<<="),
            Op::SetBitRotr => write!(f, ">>>="),
            Op::SetLogicAnd => write!(f, "&&="),
            Op::SetLogicOr => write!(f, "||="),
            Op::SetLogicXor => write!(f, "^^="),
            Op::Inc => write!(f, "++"),
            Op::Dec => write!(f, "--"),

            Op::As => write!(f, "as"),
            Op::BitcastAs => write!(f, "bitcast_as"),
            Op::Sizeof => write!(f, "sizeof"),
            Op::Alignof => write!(f, "alignof"),
            Op::Typeof => write!(f, "typeof"),

            Op::Dot => write!(f, "."),
            Op::Ellipsis => write!(f, "..."),
            Op::Scope => write!(f, "::"),
            Op::Arrow => write!(f, "->"),
            Op::BlockArrow => write!(f, "=>"),

            Op::Range => write!(f, ".."),
            Op::Question => write!(f, "?"),
            Op::Spaceship => write!(f, "<=>"),
        }
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub enum Token<'a> {
    Name(Name<'a>),
    Integer(Integer),
    Float(f64),
    String(StringData<'a>),
    BString(BStringData<'a>),
    Char(char),
    Comment(Comment<'a>),
    Keyword(Keyword),
    Punct(Punct),
    Op(Op),
    Eof,
    Illegal,
}

impl std::fmt::Display for Token<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Token::Name(id) => write!(f, "{id}"),
            Token::Integer(int) => write!(f, "{int}"),
            Token::Float(float) => write!(f, "{float}"),
            Token::String(s) => write!(f, "{s}"),
            Token::BString(s) => write!(f, "{s}"),
            Token::Char(c) => write!(f, "'{c}'"),
            Token::Comment(c) => write!(f, "{c}"),
            Token::Keyword(kw) => write!(f, "{kw}"),
            Token::Punct(p) => write!(f, "{p}"),
            Token::Op(op) => write!(f, "{op}"),
            Token::Eof => write!(f, ""),
            Token::Illegal => write!(f, "<illegal>"),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash)]
pub struct SourcePosition<'a> {
    line: u32,   // zero-based unicode-aware line number
    column: u32, // zero-based unicode-aware column number
    offset: u32, // zero-based raw byte offset number
    filename: &'a str,
}

impl<'a> SourcePosition<'a> {
    #[must_use]
    pub const fn new(line: u32, column: u32, offset: u32, filename: &'a str) -> Self {
        SourcePosition {
            line,
            column,
            offset,
            filename,
        }
    }

    #[must_use]
    pub const fn line(&self) -> u32 {
        self.line
    }

    #[must_use]
    pub const fn column(&self) -> u32 {
        self.column
    }

    #[must_use]
    pub const fn offset(&self) -> usize {
        self.offset as usize
    }

    #[must_use]
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
    #[must_use]
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

    #[must_use]
    pub const fn token(&self) -> &Token {
        &self.token
    }

    #[must_use]
    pub fn into_token(self) -> Token<'a> {
        self.token
    }

    #[must_use]
    pub const fn start(&self) -> SourcePosition<'a> {
        SourcePosition::new(
            self.start_line,
            self.start_column,
            self.start_offset,
            self.filename,
        )
    }

    #[must_use]
    pub const fn end(&self) -> SourcePosition<'a> {
        SourcePosition::new(
            self.end_line,
            self.end_column,
            self.end_offset,
            self.filename,
        )
    }

    #[must_use]
    pub const fn range(&self) -> (SourcePosition<'a>, SourcePosition<'a>) {
        (self.start(), self.end())
    }
}
