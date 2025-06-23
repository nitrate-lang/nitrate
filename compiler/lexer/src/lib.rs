#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum IdentifierKind {
    Typical,
    Atypical,
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub struct Identifier<'input> {
    name: &'input str,
    kind: IdentifierKind,
}

impl<'input> Identifier<'input> {
    pub fn new(name: &'input str, kind: IdentifierKind) -> Self {
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

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub struct Integer<'input> {
    value: u128,
    original_text: &'input str,
    kind: IntegerKind,
}

impl<'input> Integer<'input> {
    pub fn new(value: u128, original_text: &'input str, kind: IntegerKind) -> Self {
        Integer {
            value,
            original_text,
            kind,
        }
    }

    pub fn value(&self) -> u128 {
        self.value
    }

    pub fn original_text(&self) -> &str {
        self.original_text
    }

    pub fn kind(&self) -> IntegerKind {
        self.kind
    }
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub struct Float<'input> {
    value: f64,
    original_text: &'input str,
}

impl<'input> Float<'input> {
    pub fn new(value: f64, original_text: &'input str) -> Self {
        Float {
            value,
            original_text,
        }
    }

    pub fn value(&self) -> f64 {
        self.value
    }

    pub fn original_text(&self) -> &str {
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

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub struct Comment<'input> {
    text: &'input str,
    kind: CommentKind,
}

impl<'input> Comment<'input> {
    pub fn new(text: &'input str, kind: CommentKind) -> Self {
        Comment { text, kind }
    }

    pub fn text(&self) -> &str {
        self.text
    }

    pub fn kind(&self) -> CommentKind {
        self.kind
    }
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub enum Token<'input> {
    Identifier(Identifier<'input>),
    Integer(Integer<'input>),
    Float(Float<'input>),
    Keyword(Keyword),
    String(&'input str),
    Char(char),
    Punctuation(Punctuation),
    Operator(Operator),
    Comment(Comment<'input>),
    Eof,
    Illegal,
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
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

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
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

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
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

#[derive(Debug)]
pub struct Lexer<'input> {
    src: &'input [u8],
}

impl<'input> Lexer<'input> {
    pub fn new(src: &'input [u8]) -> Self {
        Lexer { src }
    }

    pub fn save_state(&self) -> Lexer<'input> {
        // TODO: Implement saving the current position
        Lexer { src: self.src }
    }

    pub fn assume_state(&mut self, _state: Lexer<'input>) {

        // TODO: Implement rewinding the position
    }

    pub fn lookahead_token(&mut self, _lookahead: u32) -> AnnotatedToken<'input> {
        // TODO: Implement looking ahead for a specific number of tokens
        AnnotatedToken::new(Token::Illegal, SourceRange::invalid())
    }

    pub fn skip_token(&mut self) {
        // TODO: Implement skipping the next token
    }

    pub fn next_token(&mut self) -> AnnotatedToken<'input> {
        // TODO: Implement the logic to return the next token
        AnnotatedToken::new(Token::Illegal, SourceRange::invalid())
    }

    pub fn peek_token(&mut self) -> AnnotatedToken<'input> {
        // TODO: Implement peeking the next token without consuming it
        AnnotatedToken::new(Token::Illegal, SourceRange::invalid())
    }
}
