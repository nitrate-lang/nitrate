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

#[derive(Debug, Clone, PartialEq, PartialOrd)]
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

#[derive(Debug, Clone, PartialEq, PartialOrd)]
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

#[derive(Debug, Clone, PartialEq, PartialOrd)]
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

    pub fn eof(pos: SourcePosition, filename: &'input str) -> Self {
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
    src: &'input str,
    filename: &'input str,
    read_pos: SourcePosition,
    current: Option<AnnotatedToken<'input>>,
}

#[derive(Debug, Clone, Copy)]
pub enum LexerConstructionError {
    SourceTooBig,
}

impl<'input> Lexer<'input> {
    pub fn new(src: &'input str, filename: &'input str) -> Result<Self, LexerConstructionError> {
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

    pub fn next_token(&mut self) -> AnnotatedToken<'input> {
        self.current.take().unwrap_or_else(|| self.get_next_token())
    }

    pub fn peek_token(&mut self) -> AnnotatedToken<'input> {
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

    fn read_while<F>(&mut self, mut condition: F) -> &'input str
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

        &self.src[start_offset..end_offset]
    }

    fn read_identifier_token(&mut self) -> Option<Token<'input>> {
        if self.peek_byte()? == b'`' {
            self.advance(b'`');

            let identifier = self.read_while(|b| b != b'`');

            if self.peek_byte()? == b'`' {
                self.advance(b'`');

                Some(Token::Identifier(Identifier::new(
                    identifier,
                    IdentifierKind::Atypical,
                )))
            } else {
                Some(Token::Illegal) // Because of an unclosed backtick
            }
        } else {
            let identifier = self.read_while(|b| b.is_ascii_alphanumeric() || b == b'_');
            assert!(!identifier.is_empty(), "Identifier should not be empty");

            // Check if the identifier is word-like operator
            if let Some(operator) = match identifier {
                "as" => Some(Operator::As),
                "bitcast_as" => Some(Operator::BitcastAs),
                "sizeof" => Some(Operator::Sizeof),
                "alignof" => Some(Operator::Alignof),
                "typeof" => Some(Operator::Typeof),
                _ => None,
            } {
                return Some(Token::Operator(operator));
            }

            // Check if the identifier is a keyword
            if let Some(keyword) = match identifier {
                "let" => Some(Keyword::Let),
                "var" => Some(Keyword::Var),
                "fn" => Some(Keyword::Fn),
                "enum" => Some(Keyword::Enum),
                "struct" => Some(Keyword::Struct),
                "class" => Some(Keyword::Class),
                "union" => Some(Keyword::Union),
                "interface" => Some(Keyword::Contract),
                "trait" => Some(Keyword::Trait),
                "type" => Some(Keyword::Type),
                "opaque" => Some(Keyword::Opaque),
                "scope" => Some(Keyword::Scope),
                "import" => Some(Keyword::Import),
                "unit_test" => Some(Keyword::UnitTest),

                "safe" => Some(Keyword::Safe),
                "unsafe" => Some(Keyword::Unsafe),
                "promise" => Some(Keyword::Promise),
                "static" => Some(Keyword::Static),
                "mut" => Some(Keyword::Mut),
                "const" => Some(Keyword::Const),
                "pub" => Some(Keyword::Pub),
                "sec" => Some(Keyword::Sec),
                "pro" => Some(Keyword::Pro),

                "if" => Some(Keyword::If),
                "else" => Some(Keyword::Else),
                "for" => Some(Keyword::For),
                "while" => Some(Keyword::While),
                "do" => Some(Keyword::Do),
                "switch" => Some(Keyword::Switch),
                "break" => Some(Keyword::Break),
                "continue" => Some(Keyword::Continue),
                "ret" => Some(Keyword::Return),
                "foreach" => Some(Keyword::Foreach),
                "try" => Some(Keyword::Try),
                "catch" => Some(Keyword::Catch),
                "throw" => Some(Keyword::Throw),
                "async" => Some(Keyword::Async),
                "await" => Some(Keyword::Await),
                "asm" => Some(Keyword::Asm),

                "null" => Some(Keyword::Null),
                "true" => Some(Keyword::True),
                "false" => Some(Keyword::False),

                _ => None,
            } {
                Some(Token::Keyword(keyword))
            } else {
                Some(Token::Identifier(Identifier::new(
                    identifier,
                    IdentifierKind::Typical,
                )))
            }
        }
    }

    fn read_integer_token(&mut self) -> Option<Token<'input>> {
        // TODO: Implement the logic to read an integer token
        None
    }

    fn read_float_token(&mut self) -> Option<Token<'input>> {
        // TODO: Implement the logic to read a float token
        None
    }

    fn read_number_token(&mut self) -> Option<Token<'input>> {
        // TODO: Implement the logic to read a number token
        None
    }

    fn read_string_token(&mut self) -> Option<Token<'input>> {
        // TODO: Implement the logic to read a string token
        None
    }

    fn read_char_token(&mut self) -> Option<Token<'input>> {
        // TODO: Implement the logic to read a character token
        None
    }

    fn read_comment_token(&mut self) -> Option<Token<'input>> {
        let comment_text = self.read_while(|b| b != b'\n');

        Some(Token::Comment(Comment::new(
            comment_text,
            CommentKind::SingleLine,
        )))
    }

    fn read_punctuation_token(&mut self) -> Option<Token<'input>> {
        /*
         * The colon punctuator is not handled here, as it is ambiguous with the scope
         * operator "::". See `read_operator_token` for the handling the colon punctuator.
         */

        let b = self.peek_byte()?;
        self.advance(b);

        let punctuation = match b {
            b'(' => Punctuation::LeftParenthesis,
            b')' => Punctuation::RightParenthesis,
            b'[' => Punctuation::LeftBracket,
            b']' => Punctuation::RightBracket,
            b'{' => Punctuation::LeftBrace,
            b'}' => Punctuation::RightBrace,
            b',' => Punctuation::Comma,
            b';' => Punctuation::Semicolon,
            b'@' => Punctuation::AtSign,
            _ => return None, // Invalid punctuation
        };

        Some(Token::Punctuation(punctuation))
    }

    fn read_operator_token(&mut self) -> Option<Token<'input>> {
        /*
         * The word-like operators are not handled here, as they are ambiguous with identifiers.
         * They are handled in `read_identifier_token`.
         */

        let token_text = self.read_while(|b| {
            match b {
                // Whitespace is not an operator character.
                b if b.is_ascii_whitespace() => false,

                // Identifier characters are not operator characters.
                b if b.is_ascii_alphabetic() || b == b'_' || b == b'`' => false,

                // Numbers, Integers, and floats are not operator characters.
                b if b.is_ascii_digit() => false,

                // String and character delimiters are not operator characters.
                b'"' | b'\'' => false,

                // Comment delimiters are not operator characters.
                b if b == b'#' => false,

                // Punctuation characters are not operator characters.
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
                    false
                }

                // Anything else might be an operator character.
                _ => true,
            }
        });

        // Handle the colon punctuator because it is ambiguous with the scope operator "::".
        if token_text == ":" {
            return Some(Token::Punctuation(Punctuation::Colon));
        }

        let operator = match token_text {
            "+" => Operator::Add,
            "-" => Operator::Sub,
            "*" => Operator::Mul,
            "/" => Operator::Div,
            "%" => Operator::Mod,

            "&" => Operator::BitAnd,
            "|" => Operator::BitOr,
            "^" => Operator::BitXor,
            "~" => Operator::BitNot,
            "<<" => Operator::BitShl,
            ">>" => Operator::BitShr,
            "<<<" => Operator::BitRotl,
            ">>>" => Operator::BitRotr,

            "&&" => Operator::LogicAnd,
            "||" => Operator::LogicOr,
            "^^" => Operator::LogicXor,
            "!" => Operator::LogicNot,
            "<" => Operator::LogicLt,
            ">" => Operator::LogicGt,
            "<=" => Operator::LogicLe,
            ">=" => Operator::LogicGe,
            "==" => Operator::LogicEq,
            "!=" => Operator::LogicNe,

            "=" => Operator::Set,
            "+=" => Operator::SetPlus,
            "-=" => Operator::SetMinus,
            "*=" => Operator::SetTimes,
            "/=" => Operator::SetSlash,
            "%=" => Operator::SetPercent,
            "&=" => Operator::SetBitAnd,
            "|=" => Operator::SetBitOr,
            "^=" => Operator::SetBitXor,
            "<<=" => Operator::SetBitShl,
            ">>=" => Operator::SetBitShr,
            "<<<=" => Operator::SetBitRotl,
            ">>>=" => Operator::SetBitRotr,
            "&&=" => Operator::SetLogicAnd,
            "||=" => Operator::SetLogicOr,
            "^^=" => Operator::SetLogicXor,
            "++" => Operator::Inc,
            "--" => Operator::Dec,

            "." => Operator::Dot,
            "..." => Operator::Ellipsis,
            "::" => Operator::Scope,
            "->" => Operator::Arrow,
            "=>" => Operator::BlockArrow,

            ".." => Operator::Range,
            "?" => Operator::Question,
            "<=>" => Operator::Spaceship,

            _ => return None, // Invalid operator
        };

        Some(Token::Operator(operator))
    }

    fn get_next_token(&mut self) -> AnnotatedToken<'input> {
        // Skip whitespace
        self.read_while(|b| b.is_ascii_whitespace());

        let start_pos = self.read_pos.clone();

        let token = match self.peek_byte() {
            None => Some(Token::Eof),
            Some(b) => match b {
                b if b.is_ascii_alphabetic() || b == b'_' || b == b'`' => {
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
