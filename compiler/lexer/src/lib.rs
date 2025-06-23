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
pub struct Identifier<'input> {
    name: &'input [u8],
    kind: IdentifierKind,
}

impl<'input> Identifier<'input> {
    pub fn new(name: &'input [u8], kind: IdentifierKind) -> Self {
        Identifier { name, kind }
    }

    pub fn name(&self) -> &[u8] {
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
pub struct Integer<'input> {
    value: u128,
    original_text: &'input [u8],
    kind: IntegerKind,
}

impl<'input> Integer<'input> {
    pub fn new(value: u128, original_text: &'input [u8], kind: IntegerKind) -> Self {
        Integer {
            value,
            original_text,
            kind,
        }
    }

    pub fn value(&self) -> u128 {
        self.value
    }

    pub fn original_text(&self) -> &[u8] {
        self.original_text
    }

    pub fn kind(&self) -> IntegerKind {
        self.kind
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct Float<'input> {
    value: f64,
    original_text: &'input [u8],
}

impl<'input> Float<'input> {
    pub fn new(value: f64, original_text: &'input [u8]) -> Self {
        Float {
            value,
            original_text,
        }
    }

    pub fn value(&self) -> f64 {
        self.value
    }

    pub fn original_text(&self) -> &[u8] {
        self.original_text
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
    Plus,    /* '+': Addition */
    Minus,   /* '-': Subtraction */
    Times,   /* '*': Multiplication */
    Slash,   /* '/': Division */
    Percent, /* '%': Modulus */

    /*----------------------------------------------------------------*
     * Bitwise Operators                                              *
     *----------------------------------------------------------------*/
    BitAnd,  /* '&':   Bitwise AND */
    BitOr,   /* '|':   Bitwise OR */
    BitXor,  /* '^':   Bitwise XOR */
    BitNot,  /* '~':   Bitwise NOT */
    BitShl,  /* '<<':  Bitwise left shift */
    BitShr,  /* '>>':  Bitwise right shift */
    BitRotl, /* '<<<': Bitwise rotate left */
    BitRotr, /* '>>>': Bitwise rotate right */

    /*----------------------------------------------------------------*
     * Logical Operators                                              *
     *----------------------------------------------------------------*/
    LogicAnd, /* '&&': Logical AND */
    LogicOr,  /* '||': Logical OR */
    LogicXor, /* '^^': Logical XOR */
    LogicNot, /* '!':  Logical NOT */
    LogicLt,  /* '<':  Logical less than */
    LogicGt,  /* '>':  Logical greater than */
    LogicLe,  /* '<=': Logical less than or equal to */
    LogicGe,  /* '>=': Logical greater than or equal to */
    LogicEq,  /* '==': Logical equal to */
    LogicNe,  /* '!=': Logical not equal to */

    /*----------------------------------------------------------------*
     * Assignment Operators                                           *
     *----------------------------------------------------------------*/
    Set,         /* '=':    Assignment */
    SetPlus,     /* '+=':   Addition Assignment */
    SetMinus,    /* '-=':   Subtraction Assignment */
    SetTimes,    /* '*=':   Multiplication Assignment */
    SetSlash,    /* '/=':   Division Assignment */
    SetPercent,  /* '%=':   Modulus Assignment */
    SetBitAnd,   /* '&=':   Bitwise AND Assignment */
    SetBitOr,    /* '|=':   Bitwise OR Assignment */
    SetBitXor,   /* '^=':   Bitwise XOR Assignment */
    SetBitNot,   /* '~=':   Bitwise NOT Assignment */
    SetBitShl,   /* '<<=':  Bitwise left shift Assignment */
    SetBitShr,   /* '>>=':  Bitwise right shift Assignment */
    SetBitRotl,  /* '<<<=': Bitwise rotate left Assignment */
    SetBitRotr,  /* '>>>=': Bitwise rotate right Assignment */
    SetLogicAnd, /* '&&=':  Logical AND Assignment */
    SetLogicOr,  /* '||=':  Logical OR Assignment */
    SetLogicXor, /* '^^=':  Logical XOR Assignment */
    SetLogicNot, /* '!==':  Logical NOT Assignment */
    SetLogicLt,  /* '<==':  Logical less than Assignment */
    SetLogicGt,  /* '>==':  Logical greater than Assignment */
    SetLogicLe,  /* '<==':  Logical less than or equal to Assignment */
    SetLogicGe,  /* '>==':  Logical greater than or equal to Assignment */
    SetLogicEq,  /* '===':  Logical equal to Assignment */
    SetLogicNe,  /* '!==':  Logical not equal to Assignment */
    SetInc,      /* '++':   Increment */
    SetDec,      /* '--':   Decrement */

    /*----------------------------------------------------------------*
     * Type System Operators                                          *
     *----------------------------------------------------------------*/
    As,        /* 'as':         Type cast */
    BitcastAs, /* 'bitcast_as': Bitcast */
    Sizeof,    /* 'sizeof':     Size of */
    Alignof,   /* 'alignof':    Alignment of */
    Typeof,    /* 'typeof':     Type of */

    /*----------------------------------------------------------------*
     * Syntactic Operators                                            *
     *----------------------------------------------------------------*/
    Dot,        /* '.':          Dot */
    Ellipsis,   /* '...':        Ellipsis */
    Scope,      /* '::':         Scope resolution */
    Arrow,      /* '->':         Arrow operator */
    BlockArrow, /* '=>':         Block arrow operator */

    /*----------------------------------------------------------------*
     * Special Operators                                              *
     *----------------------------------------------------------------*/
    Range,     /* '..':         Range */
    Question,  /* '?':          Ternary operator (conditional) */
    Spaceship, /* '<=>':        Spaceship operator (three-way comparison) */
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum CommentKind {
    SingleLine,
    MultiLine,
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct Comment<'input> {
    text: &'input [u8],
    kind: CommentKind,
}

impl<'input> Comment<'input> {
    pub fn new(text: &'input [u8], kind: CommentKind) -> Self {
        Comment { text, kind }
    }

    pub fn text(&self) -> &[u8] {
        self.text
    }

    pub fn kind(&self) -> CommentKind {
        self.kind
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub enum Token<'input> {
    Identifier(Identifier<'input>),
    Integer(Integer<'input>),
    Float(Float<'input>),
    Keyword(Keyword),
    String(&'input [u8]),
    Char(char),
    Punctuation(Punctuation),
    Operator(Operator),
    Comment(Comment<'input>),
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
pub struct SourceRange<'input> {
    start: SourcePosition,
    end: SourcePosition,
    filename: &'input str,
}

impl<'input> SourceRange<'input> {
    pub fn new(start: SourcePosition, end: SourcePosition, filename: &'input str) -> Self {
        SourceRange {
            start,
            end,
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

    pub fn filename(&self) -> &'input str {
        self.filename
    }

    pub fn is_valid(&self) -> bool {
        self.start.offset < self.end.offset
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct AnnotatedToken<'input> {
    token: Token<'input>,
    range: SourceRange<'input>,
}

impl<'input> AnnotatedToken<'input> {
    pub fn new(token: Token<'input>, range: SourceRange<'input>) -> Self {
        AnnotatedToken { token, range }
    }

    pub fn token(&self) -> &Token<'input> {
        &self.token
    }

    pub fn range(&self) -> &SourceRange<'input> {
        &self.range
    }
}

#[derive(Debug, Clone)]
pub struct Lexer<'input> {
    src: &'input [u8],
    filename: &'input str,
    read_pos: SourcePosition,
    current: Option<AnnotatedToken<'input>>,
}

#[derive(Debug, Clone, Copy)]
pub enum LexerConstructionError {
    SourceTooBig,
}

impl<'input> Lexer<'input> {
    pub fn new(src: &'input [u8], filename: &'input str) -> Result<Self, LexerConstructionError> {
        if src.len() > MAX_SOURCE_SIZE {
            return Err(LexerConstructionError::SourceTooBig);
        }

        assert!(
            src.len() <= MAX_SOURCE_SIZE,
            "Source size exceeds maximum allowed size"
        );

        Ok(Lexer {
            src,
            filename,
            read_pos: SourcePosition::new(0, 0, 0),
            current: None,
        })
    }

    pub fn skip_token(&mut self) {
        if self.current.is_some() {
            self.current = None;
        } else {
            self.get_next_token(); // Discard the token
        }
    }

    pub fn next_token(&mut self) -> AnnotatedToken<'input> {
        self.current.take().unwrap_or_else(|| self.get_next_token())
    }

    pub fn peek_token(&mut self) -> AnnotatedToken<'input> {
        let token = self.current.take().unwrap_or_else(|| self.get_next_token());
        self.current = Some(token.clone());

        token
    }

    fn get_next_token(&mut self) -> AnnotatedToken<'input> {
        // TODO: Implement the logic to get the next token from the source
        AnnotatedToken::new(Token::Eof, SourceRange::invalid())
    }
}
