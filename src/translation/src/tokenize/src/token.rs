use std::ops::Deref;

use enum_iterator::Sequence;
use nitrate_diagnosis::FileId;
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

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
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

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct SourcePosition {
    pub line: u32,
    pub column: u32,
    pub offset: u32,
    pub fileid: Option<FileId>,
}

impl std::fmt::Display for SourcePosition {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}:{}:{}",
            self.fileid.as_ref().map_or("???", |fid| fid.deref()),
            self.line + 1,
            self.column + 1
        )
    }
}

impl From<SourcePosition> for nitrate_diagnosis::SourcePosition {
    fn from(pos: SourcePosition) -> Self {
        nitrate_diagnosis::SourcePosition {
            line: pos.line,
            column: pos.column,
            offset: pos.offset,
            fileid: pos.fileid,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct AnnotatedToken {
    pub(crate) token: Token,

    start_line: u32,
    start_column: u32,
    start_offset: u32,

    end_line: u32,
    end_column: u32,
    end_offset: u32,

    fileid: Option<FileId>,
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
            fileid: start.fileid,
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
            fileid: self.fileid.clone(),
        }
    }

    #[must_use]
    pub fn end(&self) -> SourcePosition {
        SourcePosition {
            line: self.end_line,
            column: self.end_column,
            offset: self.end_offset,
            fileid: self.fileid.clone(),
        }
    }

    #[must_use]
    pub fn range(&self) -> (SourcePosition, SourcePosition) {
        (self.start(), self.end())
    }
}

#[cfg(test)]
mod tests {
    use nitrate_diagnosis::get_or_create_file_id;

    use super::*;

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
            let comment = Comment::new(text.to_string(), kind);
            assert_eq!(comment.text(), &text);
            assert_eq!(comment.kind(), kind);
            assert_eq!(format!("{}", comment), expected_str);
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
                    " This is a comment".to_string(),
                    CommentKind::SingleLine,
                )),
                "# This is a comment",
            ),
            (Token::Let, "let"),
            (Token::OpenParen, "("),
            (Token::Plus, "+"),
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
        let filename = "file.txt";

        let position = SourcePosition {
            line,
            column,
            offset,
            fileid: get_or_create_file_id(filename),
        };

        assert_eq!(
            format!("{}", position),
            format!(
                "{}:{}:{}",
                position.fileid.unwrap().deref(),
                line + 1,
                column + 1
            )
        );
    }

    #[test]
    fn test_annotated_token_parsetree() {
        let filename = get_or_create_file_id("file.txt");

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
                    " This is a comment".into(),
                    CommentKind::SingleLine,
                )),
                "# This is a comment",
            ),
            (Token::Let, "let"),
            (Token::OpenParen, "("),
            (Token::Plus, "+"),
            (Token::Eof, ""),
        ];

        for (token, expected_str) in test_vectors {
            let start = SourcePosition {
                line: 1,
                column: 2,
                offset: 3,
                fileid: filename.clone(),
            };
            let end = SourcePosition {
                line: 4,
                column: 5,
                offset: 6,
                fileid: filename.clone(),
            };

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
