use enum_iterator::Sequence;
use interned_string::IString;
pub use ordered_float::NotNan;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence)]
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
    pub fn new(name: &'a str) -> Option<Self> {
        Some(Name {
            name,
            kind: Name::get_kind(name)?,
        })
    }

    #[must_use]
    pub fn new_typical(name: &'a str) -> Option<Self> {
        Name::is_typical(name).then_some(Name {
            name,
            kind: IdentifierKind::Typical,
        })
    }

    #[must_use]
    pub fn new_atypical(name: &'a str) -> Option<Self> {
        Name::is_atypical(name).then_some(Name {
            name,
            kind: IdentifierKind::Atypical,
        })
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
        !name.contains('`')
    }

    #[must_use]
    pub fn get_kind(name: &str) -> Option<IdentifierKind> {
        if Self::is_typical(name) {
            Some(IdentifierKind::Typical)
        } else if Self::is_atypical(name) {
            Some(IdentifierKind::Atypical)
        } else {
            None
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence)]
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence)]
pub enum CommentKind {
    SingleLine,
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
    pub const fn kind(&self) -> CommentKind {
        self.kind
    }
}

impl std::fmt::Display for Comment<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.kind() {
            CommentKind::SingleLine => write!(f, "#{}", self.text()),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence)]
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
    Poly,    /* 'poly' */
    Iso,     /* 'iso' */
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
            Keyword::Poly => write!(f, "poly"),
            Keyword::Iso => write!(f, "iso"),
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence)]
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
    SingleQuote,  /* ''' */
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
            Punct::SingleQuote => write!(f, "'"),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence)]
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
    BitAnd, /* '&':   "Bitwise AND Operator" */
    BitOr,  /* '|':   "Bitwise OR Operator" */
    BitXor, /* '^':   "Bitwise XOR Operator" */
    BitNot, /* '~':   "Bitwise NOT Operator" */
    BitShl, /* '<<':  "Bitwise Left-Shift Operator" */
    BitShr, /* '>>':  "Bitwise Right-Shift Operator" */
    BitRol, /* '<<<': "Bitwise Left-Rotate Operator" */
    BitRor, /* '>>>': "Bitwise Right-Rotate Operator" */

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
    Range, /* '..':         "Range Operator" */
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
            Op::BitRol => write!(f, "<<<"),
            Op::BitRor => write!(f, ">>>"),

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
        }
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub enum Token<'a> {
    Name(Name<'a>),
    Integer(Integer),
    Float(NotNan<f64>),
    String(IString),
    BString(Vec<u8>),
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
            Token::BString(s) => write!(f, "{s:?}"),
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
    pub const fn offset(&self) -> u32 {
        self.offset
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

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_name_token_structure() {
        assert_eq!(
            enum_iterator::all::<IdentifierKind>().collect::<Vec<_>>(),
            vec![IdentifierKind::Typical, IdentifierKind::Atypical]
        );

        let test_vectors = [
            ("foo", Some(IdentifierKind::Typical)),
            ("_bar", Some(IdentifierKind::Typical)),
            ("baz123", Some(IdentifierKind::Typical)),
            ("0wierd", Some(IdentifierKind::Atypical)),
            ("123", Some(IdentifierKind::Atypical)),
            ("!@#$%", Some(IdentifierKind::Atypical)),
            ("λ", Some(IdentifierKind::Typical)),
            ("变量", Some(IdentifierKind::Typical)),
            ("🔥", Some(IdentifierKind::Typical)),
            ("1🔥", Some(IdentifierKind::Atypical)),
            ("🔥`", None),
            ("with space", Some(IdentifierKind::Atypical)),
            (" ", Some(IdentifierKind::Atypical)),
            ("\n", Some(IdentifierKind::Atypical)),
            ("", Some(IdentifierKind::Atypical)),
            ("`", None),
        ];

        for (input, expected) in test_vectors {
            let name = Name::new(input);

            if let Some(expected_kind) = expected {
                let name = name.expect(&format!("input: {:?}", input));
                assert_eq!(name.kind(), expected_kind, "input: {:?}", input);
                assert_eq!(name.name(), input, "input: {:?}", input);

                let expected_display_str = match expected_kind {
                    IdentifierKind::Atypical => format!("`{}`", input),
                    IdentifierKind::Typical => format!("{}", input),
                };

                assert_eq!(format!("{}", name), expected_display_str);
                assert_eq!(name.into_name(), input);
            } else {
                assert!(name.is_none(), "input: {:?}", input);
            }

            assert!(expected == None || Name::new_atypical(input).is_some());
            assert_eq!(
                Name::new_typical(input).is_some(),
                expected == Some(IdentifierKind::Typical),
            );
        }
    }

    #[test]
    fn test_integer_token_structure() {
        assert_eq!(
            enum_iterator::all::<IntegerKind>().collect::<Vec<_>>(),
            vec![
                IntegerKind::Bin,
                IntegerKind::Oct,
                IntegerKind::Dec,
                IntegerKind::Hex
            ]
        );

        let prime_u128 = 0xa8b437b5f0bd41f1e97765f63699f65d_u128;

        let test_vectors = [
            (0_u128, IntegerKind::Bin, "0b0"),
            (0_u128, IntegerKind::Oct, "0o0"),
            (0_u128, IntegerKind::Dec, "0"),
            (0_u128, IntegerKind::Hex, "0x0"),
            (
                prime_u128,
                IntegerKind::Bin,
                "0b10101000101101000011011110110101111100001011110101000001111100011110100101110111011001011111011000110110100110011111011001011101",
            ),
            (
                prime_u128,
                IntegerKind::Oct,
                "0o2505503366574136501743645673137306646373135",
            ),
            (
                prime_u128,
                IntegerKind::Dec,
                "224246046673732952298033213736759195229",
            ),
            (
                prime_u128,
                IntegerKind::Hex,
                "0xa8b437b5f0bd41f1e97765f63699f65d",
            ),
            (
                u128::MAX,
                IntegerKind::Bin,
                "0b11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111",
            ),
            (
                u128::MAX,
                IntegerKind::Oct,
                "0o3777777777777777777777777777777777777777777",
            ),
            (
                u128::MAX,
                IntegerKind::Dec,
                "340282366920938463463374607431768211455",
            ),
            (
                u128::MAX,
                IntegerKind::Hex,
                "0xffffffffffffffffffffffffffffffff",
            ),
        ];

        for (value, kind, expected_str) in test_vectors {
            let integer = Integer::new(value, kind);
            assert_eq!(integer.value(), value);
            assert_eq!(integer.kind(), kind);
            assert_eq!(format!("{}", integer), expected_str);
        }
    }

    #[test]
    fn test_string_data_structure() {
        assert_eq!(StringData::from_ref("hello").get(), "hello");
        assert_eq!(StringData::from_dyn("world".to_string()).get(), "world");
        assert_eq!(format!("{}", StringData::from_ref("test")), "\"test\"");
        assert_eq!(
            format!("{:?}", StringData::from_ref("test2")),
            "StringData(\"test2\")",
        );
    }

    #[test]
    fn test_binary_data_structure() {
        assert_eq!(BStringData::from_ref(b"hello").get(), b"hello");
        assert_eq!(BStringData::from_dyn(b"world".to_vec()).get(), b"world");
        assert_eq!(
            format!("{}", BStringData::from_ref(b"test")),
            "[116, 101, 115, 116]"
        );

        assert_eq!(
            format!("{:?}", BStringData::from_ref(b"test2")),
            "BStringData([116, 101, 115, 116, 50])"
        );
    }

    #[test]
    fn test_comment_token_structure() {
        assert_eq!(
            enum_iterator::all::<CommentKind>().collect::<Vec<_>>(),
            vec![CommentKind::SingleLine]
        );

        let test_vectors = [
            (
                " This is a single-line comment",
                CommentKind::SingleLine,
                "# This is a single-line comment",
            ),
            (
                "This is another single-line comment",
                CommentKind::SingleLine,
                "#This is another single-line comment",
            ),
        ];

        for (text, kind, expected_str) in test_vectors {
            let comment = Comment::new(text, kind);
            assert_eq!(comment.text(), text);
            assert_eq!(comment.kind(), kind);
            assert_eq!(format!("{}", comment), expected_str);
        }
    }

    #[test]
    fn test_keyword_structure() {
        for keyword in enum_iterator::all::<Keyword>() {
            let keyword_str = match keyword {
                Keyword::Let => "let",
                Keyword::Var => "var",
                Keyword::Fn => "fn",
                Keyword::Enum => "enum",
                Keyword::Struct => "struct",
                Keyword::Class => "class",
                Keyword::Contract => "contract",
                Keyword::Trait => "trait",
                Keyword::Impl => "impl",
                Keyword::Type => "type",
                Keyword::Scope => "scope",
                Keyword::Import => "import",
                Keyword::Safe => "safe",
                Keyword::Unsafe => "unsafe",
                Keyword::Promise => "promise",
                Keyword::Static => "static",
                Keyword::Mut => "mut",
                Keyword::Const => "const",
                Keyword::Poly => "poly",
                Keyword::Iso => "iso",
                Keyword::Pub => "pub",
                Keyword::Sec => "sec",
                Keyword::Pro => "pro",
                Keyword::If => "if",
                Keyword::Else => "else",
                Keyword::For => "for",
                Keyword::While => "while",
                Keyword::Do => "do",
                Keyword::Switch => "switch",
                Keyword::Break => "break",
                Keyword::Continue => "continue",
                Keyword::Ret => "ret",
                Keyword::Foreach => "foreach",
                Keyword::Async => "async",
                Keyword::Await => "await",
                Keyword::Asm => "asm",
                Keyword::Assert => "assert",
                Keyword::Null => "null",
                Keyword::True => "true",
                Keyword::False => "false",
                Keyword::Bool => "bool",
                Keyword::U8 => "u8",
                Keyword::U16 => "u16",
                Keyword::U32 => "u32",
                Keyword::U64 => "u64",
                Keyword::U128 => "u128",
                Keyword::I8 => "i8",
                Keyword::I16 => "i16",
                Keyword::I32 => "i32",
                Keyword::I64 => "i64",
                Keyword::I128 => "i128",
                Keyword::F8 => "f8",
                Keyword::F16 => "f16",
                Keyword::F32 => "f32",
                Keyword::F64 => "f64",
                Keyword::F128 => "f128",
                Keyword::Opaque => "opaque",
            };

            assert_eq!(format!("{}", keyword), keyword_str);
        }
    }

    #[test]
    fn test_punct_structure() {
        for punct in enum_iterator::all::<Punct>() {
            let punct_str = match punct {
                Punct::LeftParen => "(",
                Punct::RightParen => ")",
                Punct::LeftBracket => "[",
                Punct::RightBracket => "]",
                Punct::LeftBrace => "{",
                Punct::RightBrace => "}",
                Punct::Comma => ",",
                Punct::Semicolon => ";",
                Punct::Colon => ":",
                Punct::AtSign => "@",
                Punct::SingleQuote => "'",
            };

            assert_eq!(format!("{}", punct), punct_str);
        }
    }

    #[test]
    fn test_operator_structure() {
        for operator in enum_iterator::all::<Op>() {
            let operator_str = match operator {
                Op::Add => "+",
                Op::Sub => "-",
                Op::Mul => "*",
                Op::Div => "/",
                Op::Mod => "%",
                Op::BitAnd => "&",
                Op::BitOr => "|",
                Op::BitXor => "^",
                Op::BitNot => "~",
                Op::BitShl => "<<",
                Op::BitShr => ">>",
                Op::BitRol => "<<<",
                Op::BitRor => ">>>",
                Op::LogicAnd => "&&",
                Op::LogicOr => "||",
                Op::LogicXor => "^^",
                Op::LogicNot => "!",
                Op::LogicLt => "<",
                Op::LogicGt => ">",
                Op::LogicLe => "<=",
                Op::LogicGe => ">=",
                Op::LogicEq => "==",
                Op::LogicNe => "!=",
                Op::Set => "=",
                Op::SetPlus => "+=",
                Op::SetMinus => "-=",
                Op::SetTimes => "*=",
                Op::SetSlash => "/=",
                Op::SetPercent => "%=",
                Op::SetBitAnd => "&=",
                Op::SetBitOr => "|=",
                Op::SetBitXor => "^=",
                Op::SetBitShl => "<<=",
                Op::SetBitShr => ">>=",
                Op::SetBitRotl => "<<<=",
                Op::SetBitRotr => ">>>=",
                Op::SetLogicAnd => "&&=",
                Op::SetLogicOr => "||=",
                Op::SetLogicXor => "^^=",
                Op::As => "as",
                Op::BitcastAs => "bitcast_as",
                Op::Sizeof => "sizeof",
                Op::Alignof => "alignof",
                Op::Typeof => "typeof",
                Op::Dot => ".",
                Op::Ellipsis => "...",
                Op::Scope => "::",
                Op::Arrow => "->",
                Op::BlockArrow => "=>",
                Op::Range => "..",
            };

            assert_eq!(format!("{}", operator), operator_str);
        }
    }

    #[test]
    fn test_token_structure() {
        let test_vectors = [
            (Token::Name(Name::new("example").unwrap()), "example"),
            (Token::Integer(Integer::new(42, IntegerKind::Dec)), "42"),
            (Token::Float(NotNan::new(3.14).unwrap()), "3.14"),
            (Token::String(StringData::from_ref("hello")), "\"hello\""),
            (
                Token::BString(BStringData::from_ref(b"world")),
                "[119, 111, 114, 108, 100]",
            ),
            (
                Token::Comment(Comment::new(" This is a comment", CommentKind::SingleLine)),
                "# This is a comment",
            ),
            (Token::Keyword(Keyword::Let), "let"),
            (Token::Punct(Punct::LeftParen), "("),
            (Token::Op(Op::Add), "+"),
            (Token::Eof, ""),
            (Token::Illegal, "<illegal>"),
        ];

        for (token, expected_str) in test_vectors {
            assert_eq!(format!("{}", token), expected_str);
        }
    }

    #[test]
    fn test_source_position_structure() {
        let line = 2_u32;
        let column = 5_u32;
        let offset = 15_u32;
        let filename = "test_file.txt";

        let position = SourcePosition::new(line, column, offset, filename);
        assert_eq!(position.line(), line);
        assert_eq!(position.column(), column);
        assert_eq!(position.offset(), offset);
        assert_eq!(position.filename(), filename);

        assert_eq!(
            format!("{}", position),
            format!("{}:{}:{}", filename, line + 1, column + 1)
        );
    }

    #[test]
    fn test_annotated_token_structure() {
        let test_vectors = [
            (
                Token::Name(Name::new("example").unwrap()),
                SourcePosition::new(1, 0, 10, "file.txt"),
                SourcePosition::new(1, 7, 17, "file.txt"),
                "example",
            ),
            (
                Token::Integer(Integer::new(42, IntegerKind::Dec)),
                SourcePosition::new(2, 5, 25, "file.txt"),
                SourcePosition::new(2, 7, 27, "file.txt"),
                "42",
            ),
            (
                Token::Float(NotNan::new(3.14).unwrap()),
                SourcePosition::new(3, 0, 30, "file.txt"),
                SourcePosition::new(3, 5, 35, "file.txt"),
                "3.14",
            ),
            (
                Token::String(StringData::from_ref("hello")),
                SourcePosition::new(4, 0, 40, "file.txt"),
                SourcePosition::new(4, 6, 46, "file.txt"),
                "\"hello\"",
            ),
            (
                Token::BString(BStringData::from_ref(b"world")),
                SourcePosition::new(5, 0, 50, "file.txt"),
                SourcePosition::new(5, 6, 56, "file.txt"),
                "[119, 111, 114, 108, 100]",
            ),
            (
                Token::Comment(Comment::new(" This is a comment", CommentKind::SingleLine)),
                SourcePosition::new(6, 0, 60, "file.txt"),
                SourcePosition::new(6, 7, 67, "file.txt"),
                "# This is a comment",
            ),
            (
                Token::Keyword(Keyword::Let),
                SourcePosition::new(7, 0, 70, "file.txt"),
                SourcePosition::new(7, 3, 73, "file.txt"),
                "let",
            ),
            (
                Token::Punct(Punct::LeftParen),
                SourcePosition::new(8, 0, 80, "file.txt"),
                SourcePosition::new(8, 1, 81, "file.txt"),
                "(",
            ),
            (
                Token::Op(Op::Add),
                SourcePosition::new(9, 0, 90, "file.txt"),
                SourcePosition::new(9, 1, 91, "file.txt"),
                "+",
            ),
            (
                Token::Eof,
                SourcePosition::new(10, 0, 100, "file.txt"),
                SourcePosition::new(10, 0, 100, "file.txt"),
                "",
            ),
            (
                Token::Illegal,
                SourcePosition::new(11, 0, 110, "file.txt"),
                SourcePosition::new(11, 8, 118, "file.txt"),
                "<illegal>",
            ),
        ];

        for (token, start, end, expected_str) in test_vectors {
            let annotated_token = AnnotatedToken::new(token.clone(), start, end);
            assert_eq!(annotated_token.token(), &token);
            assert_eq!(annotated_token.start(), start);
            assert_eq!(annotated_token.end(), end);
            assert_eq!(annotated_token.range(), (start, end));
            assert_eq!(format!("{}", annotated_token.token()), expected_str);
            assert_eq!(format!("{}", annotated_token.into_token()), expected_str);
        }
    }
}
