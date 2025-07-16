use smallvec::SmallVec;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum IdentifierKind {
    Typical,
    Atypical,
}

#[derive(Debug, Clone, PartialEq)]
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

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum IntegerKind {
    Binary,
    Octal,
    Decimal,
    Hexadecimal,
}

#[derive(Debug, Clone, PartialEq)]
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

#[derive(Debug, Clone, PartialEq)]
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

#[derive(Debug, Clone, Copy, PartialEq)]
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

#[derive(Debug, Clone, Copy, PartialEq)]
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

#[derive(Debug, Clone, Copy, PartialEq)]
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

#[derive(Debug, Clone, PartialEq)]
enum StringData<'a> {
    RefString(&'a [u8]),
    DynString(SmallVec<[u8; 64]>),
}

#[derive(Clone, PartialEq)]
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

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum CommentKind {
    SingleLine,
    MultiLine,
}

#[derive(Debug, Clone, PartialEq)]
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

#[derive(Debug, Clone, PartialEq)]
pub enum Token<'a> {
    Identifier(Identifier<'a>),
    Integer(Integer),
    Float(Float),
    Keyword(Keyword),
    String(StringLit<'a>),
    Char(char),
    Punctuation(Punctuation),
    Operator(Operator),
    Comment(Comment<'a>),
    Eof,
    Illegal,
}

#[derive(Debug, Clone, PartialEq)]
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

#[derive(Debug, Clone, PartialEq)]
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
