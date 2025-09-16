use enum_iterator::Sequence;
use nitrate_diagnosis::SourcePosition;
pub use ordered_float::NotNan;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence, Serialize, Deserialize)]
pub enum IdentifierKind {
    Typical,
    Atypical,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence, Serialize, Deserialize)]
pub enum IntegerKind {
    Bin,
    Oct,
    Dec,
    Hex,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash, Serialize, Deserialize)]
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Hash, Sequence, Serialize, Deserialize)]
pub enum CommentKind {
    SingleLine,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Hash, Serialize, Deserialize)]
pub struct Comment {
    text: String,
    kind: CommentKind,
}

impl Comment {
    #[must_use]
    pub const fn new(text: String, kind: CommentKind) -> Self {
        Comment { text, kind }
    }

    #[must_use]
    pub fn text(&self) -> &String {
        &self.text
    }

    #[must_use]
    pub const fn kind(&self) -> CommentKind {
        self.kind
    }
}

impl std::fmt::Display for Comment {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.kind() {
            CommentKind::SingleLine => write!(f, "#{}", self.text()),
        }
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd, Serialize, Deserialize)]
pub enum Token {
    Name(String),
    Integer(Integer),
    Float(NotNan<f64>),
    String(String),
    BString(Vec<u8>),
    Comment(Comment),

    /// '''
    SingleQuote,
    /// ';'
    Semi,
    /// ','
    Comma,
    /// '.'
    Dot,
    /// '('
    OpenParen,
    /// ')'
    CloseParen,
    /// '{'
    OpenBrace,
    /// '}'
    CloseBrace,
    /// '['
    OpenBracket,
    /// ']'
    CloseBracket,
    /// '@'
    At,
    /// '~'
    Tilde,
    /// '?'
    Question,
    /// ':'
    Colon,
    /// '$'
    Dollar,
    /// `=`
    Eq,
    /// `!`
    Bang,
    /// `<`
    Lt,
    /// `>`
    Gt,
    /// `-`
    Minus,
    /// `&`
    And,
    /// `|`
    Or,
    /// `+`
    Plus,
    /// `*`
    Star,
    /// `/`
    Slash,
    /// `^`
    Caret,
    /// `%`
    Percent,

    /// 'let'
    Let,
    /// 'var'
    Var,
    /// 'fn'
    Fn,
    /// 'enum'
    Enum,
    /// 'struct'
    Struct,
    /// 'class'
    Class,
    /// 'union'
    Union,
    /// 'contract'
    Contract,
    /// 'trait'
    Trait,
    /// 'impl'
    Impl,
    /// 'type'
    Type,
    /// 'scope'
    Scope,
    /// 'import'
    Import,
    /// 'mod'
    Mod,
    /// 'safe'
    Safe,
    /// 'unsafe'
    Unsafe,
    /// 'promise'
    Promise,
    /// 'static'
    Static,
    /// 'mut'
    Mut,
    /// 'const'
    Const,
    /// 'poly'
    Poly,
    /// 'iso'
    Iso,
    /// 'pub'
    Pub,
    /// 'sec'
    Sec,
    /// 'pro'
    Pro,
    /// 'if'
    If,
    /// 'else'
    Else,
    /// 'for'
    For,
    /// 'in'
    In,
    /// 'while'
    While,
    /// 'do'
    Do,
    /// 'switch'
    Switch,
    /// 'break'
    Break,
    /// 'continue'
    Continue,
    /// 'ret'
    Ret,
    /// 'async'
    Async,
    /// 'await'
    Await,
    /// 'asm'
    Asm,
    /// 'null'
    Null,
    /// 'true'
    True,
    /// 'false'
    False,
    /// 'bool'
    Bool,
    /// 'u8'
    U8,
    /// 'u16'
    U16,
    /// 'u32'
    U32,
    /// 'u64'
    U64,
    /// 'u128'
    U128,
    /// 'i8'
    I8,
    /// 'i16'
    I16,
    /// 'i32'
    I32,
    /// 'i64'
    I64,
    /// 'i128'
    I128,
    /// 'f8'
    F8,
    /// 'f16'
    F16,
    /// 'f32'
    F32,
    /// 'f64'
    F64,
    /// 'f128'
    F128,
    /// 'opaque'
    Opaque,
    /// 'as'
    As,
    /// 'typeof'
    Typeof,

    Eof,
}

impl std::fmt::Display for Token {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Token::Name(id) => write!(f, "{id}"),
            Token::Integer(int) => write!(f, "{int}"),
            Token::Float(float) => write!(f, "{float}"),
            Token::String(s) => write!(f, "\"{s}\""),
            Token::BString(s) => write!(f, "{s:?}"),
            Token::Comment(c) => write!(f, "{c}"),

            Token::SingleQuote => write!(f, "'"),
            Token::Semi => write!(f, ";"),
            Token::Comma => write!(f, ","),
            Token::Dot => write!(f, "."),
            Token::OpenParen => write!(f, "("),
            Token::CloseParen => write!(f, ")"),
            Token::OpenBrace => write!(f, "{{"),
            Token::CloseBrace => write!(f, "}}"),
            Token::OpenBracket => write!(f, "["),
            Token::CloseBracket => write!(f, "]"),
            Token::At => write!(f, "@"),
            Token::Tilde => write!(f, "~"),
            Token::Question => write!(f, "?"),
            Token::Colon => write!(f, ":"),
            Token::Dollar => write!(f, "$"),
            Token::Eq => write!(f, "="),
            Token::Bang => write!(f, "!"),
            Token::Lt => write!(f, "<"),
            Token::Gt => write!(f, ">"),
            Token::Minus => write!(f, "-"),
            Token::And => write!(f, "&"),
            Token::Or => write!(f, "|"),
            Token::Plus => write!(f, "+"),
            Token::Star => write!(f, "*"),
            Token::Slash => write!(f, "/"),
            Token::Caret => write!(f, "^"),
            Token::Percent => write!(f, "%"),

            Token::Let => write!(f, "let"),
            Token::Var => write!(f, "var"),
            Token::Fn => write!(f, "fn"),
            Token::Enum => write!(f, "enum"),
            Token::Struct => write!(f, "struct"),
            Token::Class => write!(f, "class"),
            Token::Union => write!(f, "union"),
            Token::Contract => write!(f, "contract"),
            Token::Trait => write!(f, "trait"),
            Token::Impl => write!(f, "impl"),
            Token::Type => write!(f, "type"),
            Token::Scope => write!(f, "scope"),
            Token::Import => write!(f, "import"),
            Token::Mod => write!(f, "mod"),
            Token::Safe => write!(f, "safe"),
            Token::Unsafe => write!(f, "unsafe"),
            Token::Promise => write!(f, "promise"),
            Token::Static => write!(f, "static"),
            Token::Mut => write!(f, "mut"),
            Token::Const => write!(f, "const"),
            Token::Poly => write!(f, "poly"),
            Token::Iso => write!(f, "iso"),
            Token::Pub => write!(f, "pub"),
            Token::Sec => write!(f, "sec"),
            Token::Pro => write!(f, "pro"),
            Token::If => write!(f, "if"),
            Token::Else => write!(f, "else"),
            Token::For => write!(f, "for"),
            Token::In => write!(f, "in"),
            Token::While => write!(f, "while"),
            Token::Do => write!(f, "do"),
            Token::Switch => write!(f, "switch"),
            Token::Break => write!(f, "break"),
            Token::Continue => write!(f, "continue"),
            Token::Ret => write!(f, "ret"),
            Token::Async => write!(f, "async"),
            Token::Await => write!(f, "await"),
            Token::Asm => write!(f, "asm"),
            Token::Null => write!(f, "null"),
            Token::True => write!(f, "true"),
            Token::False => write!(f, "false"),
            Token::Bool => write!(f, "bool"),
            Token::U8 => write!(f, "u8"),
            Token::U16 => write!(f, "u16"),
            Token::U32 => write!(f, "u32"),
            Token::U64 => write!(f, "u64"),
            Token::U128 => write!(f, "u128"),
            Token::I8 => write!(f, "i8"),
            Token::I16 => write!(f, "i16"),
            Token::I32 => write!(f, "i32"),
            Token::I64 => write!(f, "i64"),
            Token::I128 => write!(f, "i128"),
            Token::F8 => write!(f, "f8"),
            Token::F16 => write!(f, "f16"),
            Token::F32 => write!(f, "f32"),
            Token::F64 => write!(f, "f64"),
            Token::F128 => write!(f, "f128"),
            Token::Opaque => write!(f, "opaque"),
            Token::As => write!(f, "as"),
            Token::Typeof => write!(f, "typeof"),

            Token::Eof => write!(f, ""),
        }
    }
}

#[derive(Debug, Clone, PartialEq, PartialOrd, Serialize, Deserialize)]
pub struct AnnotatedToken {
    pub(crate) token: Token,

    start_line: u32,
    start_column: u32,
    start_offset: u32,

    end_line: u32,
    end_column: u32,
    end_offset: u32,

    filename: String,
}

impl AnnotatedToken {
    #[must_use]
    pub fn new(token: Token, start: SourcePosition, end: SourcePosition) -> Self {
        AnnotatedToken {
            token,
            start_line: start.line,
            start_column: start.column,
            start_offset: start.offset,
            end_line: end.line,
            end_column: end.column,
            end_offset: end.offset,
            filename: start.filename,
        }
    }

    #[must_use]
    pub const fn token(&self) -> &Token {
        &self.token
    }

    #[must_use]
    pub fn into_token(self) -> Token {
        self.token
    }

    #[must_use]
    pub fn start(&self) -> SourcePosition {
        SourcePosition {
            line: self.start_line,
            column: self.start_column,
            offset: self.start_offset,
            filename: self.filename.clone(),
        }
    }

    #[must_use]
    pub fn end(&self) -> SourcePosition {
        SourcePosition {
            line: self.end_line,
            column: self.end_column,
            offset: self.end_offset,
            filename: self.filename.clone(),
        }
    }

    #[must_use]
    pub fn range(&self) -> (SourcePosition, SourcePosition) {
        (self.start(), self.end())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use interned_string::Intern;

    #[test]
    fn test_name_token_parsetree() {
        assert_eq!(
            enum_iterator::all::<IdentifierKind>().collect::<Vec<_>>(),
            vec![IdentifierKind::Typical, IdentifierKind::Atypical]
        );
    }

    #[test]
    fn test_integer_token_parsetree() {
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
    fn test_comment_token_parsetree() {
        assert_eq!(
            enum_iterator::all::<CommentKind>().collect::<Vec<_>>(),
            vec![CommentKind::SingleLine]
        );

        let test_vectors = [
            (
                " This is a single-line comment".intern(),
                CommentKind::SingleLine,
                "# This is a single-line comment",
            ),
            (
                "This is another single-line comment".intern(),
                CommentKind::SingleLine,
                "#This is another single-line comment",
            ),
        ];

        for (text, kind, expected_str) in test_vectors {
            let comment = Comment::new(text.clone(), kind);
            assert_eq!(comment.text(), &text);
            assert_eq!(comment.kind(), kind);
            assert_eq!(format!("{}", comment), expected_str);
        }
    }

    #[test]
    fn test_keyword_parsetree() {
        for keyword in enum_iterator::all::<Keyword>() {
            let keyword_str = match keyword {
                Keyword::Let => "let",
                Keyword::Var => "var",
                Keyword::Fn => "fn",
                Keyword::Enum => "enum",
                Keyword::Struct => "struct",
                Keyword::Class => "class",
                Keyword::Union => "union",
                Keyword::Contract => "contract",
                Keyword::Trait => "trait",
                Keyword::Impl => "impl",
                Keyword::Type => "type",
                Keyword::Scope => "scope",
                Keyword::Import => "import",
                Keyword::Mod => "mod",
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
                Keyword::In => "in",
                Keyword::While => "while",
                Keyword::Do => "do",
                Keyword::Switch => "switch",
                Keyword::Break => "break",
                Keyword::Continue => "continue",
                Keyword::Ret => "ret",
                Keyword::Async => "async",
                Keyword::Await => "await",
                Keyword::Asm => "asm",
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
                Keyword::As => "as",
                Keyword::Typeof => "typeof",
            };

            assert_eq!(format!("{}", keyword), keyword_str);
        }
    }

    #[test]
    fn test_punct_parsetree() {
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
    fn test_operator_parsetree() {
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
    fn test_token_parsetree() {
        let test_vectors = [
            (Token::Name("example".into()), "example"),
            (Token::Integer(Integer::new(42, IntegerKind::Dec)), "42"),
            (Token::Float(NotNan::new(3.14).unwrap()), "3.14"),
            (Token::String("hello".into()), "\"hello\""),
            (
                Token::BString(Vec::from(b"world")),
                "[119, 111, 114, 108, 100]",
            ),
            (
                Token::Comment(Comment::new(
                    " This is a comment".intern(),
                    CommentKind::SingleLine,
                )),
                "# This is a comment",
            ),
            (Token::Keyword(Keyword::Let), "let"),
            (Token::OpenParen, "("),
            (Token::Op(Op::Add), "+"),
            (Token::Eof, ""),
        ];

        for (token, expected_str) in test_vectors {
            assert_eq!(format!("{}", token), expected_str);
        }
    }

    #[test]
    fn test_source_position_parsetree() {
        let line = 2_u32;
        let column = 5_u32;
        let offset = 15_u32;
        let filename = "test_file.txt".intern();

        let position = SourcePosition::new(line, column, offset, filename.clone());

        assert_eq!(position.line(), line);
        assert_eq!(position.column(), column);
        assert_eq!(position.offset(), offset);
        assert_eq!(position.filename(), &filename);

        assert_eq!(
            format!("{}", position),
            format!("{}:{}:{}", filename, line + 1, column + 1)
        );
    }

    #[test]
    fn test_annotated_token_parsetree() {
        let filename = "file.txt".intern();

        let test_vectors = [
            (
                Token::Name("example".into()),
                SourcePosition::new(1, 0, 10, filename.clone()),
                SourcePosition::new(1, 7, 17, filename.clone()),
                "example",
            ),
            (
                Token::Integer(Integer::new(42, IntegerKind::Dec)),
                SourcePosition::new(2, 5, 25, filename.clone()),
                SourcePosition::new(2, 7, 27, filename.clone()),
                "42",
            ),
            (
                Token::Float(NotNan::new(3.14).unwrap()),
                SourcePosition::new(3, 0, 30, filename.clone()),
                SourcePosition::new(3, 5, 35, filename.clone()),
                "3.14",
            ),
            (
                Token::String("hello".into()),
                SourcePosition::new(4, 0, 40, filename.clone()),
                SourcePosition::new(4, 6, 46, filename.clone()),
                "\"hello\"",
            ),
            (
                Token::BString(Vec::from(b"world")),
                SourcePosition::new(5, 0, 50, filename.clone()),
                SourcePosition::new(5, 6, 56, filename.clone()),
                "[119, 111, 114, 108, 100]",
            ),
            (
                Token::Comment(Comment::new(
                    " This is a comment".into(),
                    CommentKind::SingleLine,
                )),
                SourcePosition::new(6, 0, 60, filename.clone()),
                SourcePosition::new(6, 7, 67, filename.clone()),
                "# This is a comment",
            ),
            (
                Token::Keyword(Keyword::Let),
                SourcePosition::new(7, 0, 70, filename.clone()),
                SourcePosition::new(7, 3, 73, filename.clone()),
                "let",
            ),
            (
                Token::OpenParen,
                SourcePosition::new(8, 0, 80, filename.clone()),
                SourcePosition::new(8, 1, 81, filename.clone()),
                "(",
            ),
            (
                Token::Op(Op::Add),
                SourcePosition::new(9, 0, 90, filename.clone()),
                SourcePosition::new(9, 1, 91, filename.clone()),
                "+",
            ),
            (
                Token::Eof,
                SourcePosition::new(10, 0, 100, filename.clone()),
                SourcePosition::new(10, 0, 100, filename.clone()),
                "",
            ),
        ];

        for (token, start, end, expected_str) in test_vectors {
            let annotated_token = AnnotatedToken::new(token.clone(), start.clone(), end.clone());

            assert_eq!(annotated_token.token(), &token);
            assert_eq!(annotated_token.start(), start);
            assert_eq!(annotated_token.end(), end);
            assert_eq!(annotated_token.range(), (start, end));
            assert_eq!(format!("{}", annotated_token.token()), expected_str);
            assert_eq!(format!("{}", annotated_token.into_token()), expected_str);
        }
    }
}
