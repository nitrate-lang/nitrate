#![cfg(test)]
mod tests {
    use crate::{Lexer, LexerIterator, lex};
    use nitrate_token::{AnnotatedToken, Comment, CommentKind, Token};

    fn lexical_equate(keyword: &str, token: Token) {
        let lexer = Lexer::new(keyword.as_bytes(), None).expect("source is too big");
        let mut iter = LexerIterator::new(lexer);
        let first_token = iter.next().expect("should have at least one token");

        assert_eq!(
            first_token,
            AnnotatedToken {
                token,
                start_line: 0,
                start_column: 0,
                start_offset: 0,
                end_line: 0,
                end_column: keyword.len() as u32,
                end_offset: keyword.len() as u32,
                fileid: None
            },
            "Keyword '{}' was not recognized correctly",
            keyword
        );
        assert!(iter.next().is_none(), "should be no more tokens");
    }

    #[test]
    fn test_lexer_iterator() {
        let source = "fn main() { }";
        let lexer = Lexer::new(source.as_bytes(), None).expect("source is too big");
        let iter = LexerIterator::new(lexer);
        let tokens: Vec<_> = iter.collect();
        assert!(!tokens.is_empty(), "LexerIterator should produce tokens");
    }

    #[test]
    fn test_lexer_iterator_empty_source() {
        let source = "";
        let lexer = Lexer::new(source.as_bytes(), None).expect("source is too big");
        let iter = LexerIterator::new(lexer);
        let tokens: Vec<_> = iter.collect();
        assert!(tokens.is_empty(), "LexerIterator should produce no tokens");
    }

    #[test]
    fn test_lexer_iterator_single_ascii_identifier() {
        let source = "identifier";
        let lexer = Lexer::new(source.as_bytes(), None).expect("source is too big");
        let mut iter = LexerIterator::new(lexer);
        let first_token = iter.next().expect("should have at least one token");

        assert_eq!(
            first_token,
            AnnotatedToken {
                token: Token::Name("identifier".into()),
                start_line: 0,
                start_column: 0,
                start_offset: 0,
                end_line: 0,
                end_column: 10,
                end_offset: 10,
                fileid: None
            }
        );
        assert!(iter.next().is_none(), "should be no more tokens");
    }

    #[test]
    fn test_lexer_iterator_single_utf8_identifier() {
        let source = "π_ρογρ🎄αμματισμός🔥";
        let lexer = Lexer::new(source.as_bytes(), None).expect("source is too big");
        let mut iter = LexerIterator::new(lexer);
        let first_token = iter.next().expect("should have at least one token");

        assert_eq!(
            first_token,
            AnnotatedToken {
                token: Token::Name("π_ρογρ🎄αμματισμός🔥".into()),
                start_line: 0,
                start_column: 0,
                start_offset: 0,
                end_line: 0,
                end_column: 18,
                end_offset: 39,
                fileid: None
            }
        );
        assert!(iter.next().is_none(), "should be no more tokens");
    }

    #[test]
    fn test_lexer_iterator_single_raw_identifier() {
        let source = "`π_ρ \\n \\0ο🎄αμ\nματι σς🔥`";
        let lexer = Lexer::new(source.as_bytes(), None).expect("source is too big");
        let mut iter = LexerIterator::new(lexer);
        let first_token = iter.next().expect("should have at least one token");

        assert_eq!(
            first_token,
            AnnotatedToken {
                token: Token::Name("π_ρ \\n \\0ο🎄αμ\nματι σς🔥".into()),
                start_line: 0,
                start_column: 0,
                start_offset: 0,
                end_line: 1,
                end_column: 9,
                end_offset: 41,
                fileid: None
            }
        );
        assert!(iter.next().is_none(), "should be no more tokens");
    }

    #[test]
    fn test_lexer_iterator_let() {
        lexical_equate("let", Token::Let);
    }

    #[test]
    fn test_lexer_iterator_var() {
        lexical_equate("var", Token::Var);
    }

    #[test]
    fn test_lexer_iterator_fn() {
        lexical_equate("fn", Token::Fn);
    }

    #[test]
    fn test_lexer_iterator_enum() {
        lexical_equate("enum", Token::Enum);
    }

    #[test]
    fn test_lexer_iterator_struct() {
        lexical_equate("struct", Token::Struct);
    }

    #[test]
    fn test_lexer_iterator_class() {
        lexical_equate("class", Token::Class);
    }

    #[test]
    fn test_lexer_iterator_union() {
        lexical_equate("union", Token::Union);
    }

    #[test]
    fn test_lexer_iterator_contract() {
        lexical_equate("contract", Token::Contract);
    }

    #[test]
    fn test_lexer_iterator_trait() {
        lexical_equate("trait", Token::Trait);
    }

    #[test]
    fn test_lexer_iterator_impl() {
        lexical_equate("impl", Token::Impl);
    }

    #[test]
    fn test_lexer_iterator_type() {
        lexical_equate("type", Token::Type);
    }

    #[test]
    fn test_lexer_iterator_scope() {
        lexical_equate("scope", Token::Scope);
    }

    #[test]
    fn test_lexer_iterator_use() {
        lexical_equate("use", Token::Use);
    }

    #[test]
    fn test_lexer_iterator_mod() {
        lexical_equate("mod", Token::Mod);
    }

    #[test]
    fn test_lexer_iterator_safe() {
        lexical_equate("safe", Token::Safe);
    }

    #[test]
    fn test_lexer_iterator_unsafe() {
        lexical_equate("unsafe", Token::Unsafe);
    }

    #[test]
    fn test_lexer_iterator_promise() {
        lexical_equate("promise", Token::Promise);
    }

    #[test]
    fn test_lexer_iterator_static() {
        lexical_equate("static", Token::Static);
    }

    #[test]
    fn test_lexer_iterator_mut() {
        lexical_equate("mut", Token::Mut);
    }

    #[test]
    fn test_lexer_iterator_const() {
        lexical_equate("const", Token::Const);
    }

    #[test]
    fn test_lexer_iterator_poly() {
        lexical_equate("poly", Token::Poly);
    }

    #[test]
    fn test_lexer_iterator_iso() {
        lexical_equate("iso", Token::Iso);
    }

    #[test]
    fn test_lexer_iterator_pub() {
        lexical_equate("pub", Token::Pub);
    }

    #[test]
    fn test_lexer_iterator_sec() {
        lexical_equate("sec", Token::Sec);
    }

    #[test]
    fn test_lexer_iterator_pro() {
        lexical_equate("pro", Token::Pro);
    }

    #[test]
    fn test_lexer_iterator_if() {
        lexical_equate("if", Token::If);
    }

    #[test]
    fn test_lexer_iterator_else() {
        lexical_equate("else", Token::Else);
    }

    #[test]
    fn test_lexer_iterator_for() {
        lexical_equate("for", Token::For);
    }

    #[test]
    fn test_lexer_iterator_in() {
        lexical_equate("in", Token::In);
    }

    #[test]
    fn test_lexer_iterator_while() {
        lexical_equate("while", Token::While);
    }

    #[test]
    fn test_lexer_iterator_do() {
        lexical_equate("do", Token::Do);
    }

    #[test]
    fn test_lexer_iterator_match() {
        lexical_equate("match", Token::Match);
    }

    #[test]
    fn test_lexer_iterator_break() {
        lexical_equate("break", Token::Break);
    }

    #[test]
    fn test_lexer_iterator_continue() {
        lexical_equate("continue", Token::Continue);
    }

    #[test]
    fn test_lexer_iterator_ret() {
        lexical_equate("ret", Token::Ret);
    }

    #[test]
    fn test_lexer_iterator_async() {
        lexical_equate("async", Token::Async);
    }

    #[test]
    fn test_lexer_iterator_await() {
        lexical_equate("await", Token::Await);
    }

    #[test]
    fn test_lexer_iterator_asm() {
        lexical_equate("asm", Token::Asm);
    }

    #[test]
    fn test_lexer_iterator_null() {
        lexical_equate("null", Token::Null);
    }

    #[test]
    fn test_lexer_iterator_true() {
        lexical_equate("true", Token::True);
    }

    #[test]
    fn test_lexer_iterator_false() {
        lexical_equate("false", Token::False);
    }

    #[test]
    fn test_lexer_iterator_bool() {
        lexical_equate("bool", Token::Bool);
    }

    #[test]
    fn test_lexer_iterator_u8() {
        lexical_equate("u8", Token::U8);
    }

    #[test]
    fn test_lexer_iterator_u16() {
        lexical_equate("u16", Token::U16);
    }

    #[test]
    fn test_lexer_iterator_u32() {
        lexical_equate("u32", Token::U32);
    }

    #[test]
    fn test_lexer_iterator_u64() {
        lexical_equate("u64", Token::U64);
    }

    #[test]
    fn test_lexer_iterator_u128() {
        lexical_equate("u128", Token::U128);
    }

    #[test]
    fn test_lexer_iterator_usize() {
        lexical_equate("usize", Token::USize);
    }

    #[test]
    fn test_lexer_iterator_i8() {
        lexical_equate("i8", Token::I8);
    }

    #[test]
    fn test_lexer_iterator_i16() {
        lexical_equate("i16", Token::I16);
    }

    #[test]
    fn test_lexer_iterator_i32() {
        lexical_equate("i32", Token::I32);
    }

    #[test]
    fn test_lexer_iterator_i64() {
        lexical_equate("i64", Token::I64);
    }

    #[test]
    fn test_lexer_iterator_i128() {
        lexical_equate("i128", Token::I128);
    }

    #[test]
    fn test_lexer_iterator_f8() {
        lexical_equate("f8", Token::F8);
    }

    #[test]
    fn test_lexer_iterator_f16() {
        lexical_equate("f16", Token::F16);
    }

    #[test]
    fn test_lexer_iterator_f32() {
        lexical_equate("f32", Token::F32);
    }

    #[test]
    fn test_lexer_iterator_f64() {
        lexical_equate("f64", Token::F64);
    }

    #[test]
    fn test_lexer_iterator_f128() {
        lexical_equate("f128", Token::F128);
    }

    #[test]
    fn test_lexer_iterator_opaque() {
        lexical_equate("opaque", Token::Opaque);
    }

    #[test]
    fn test_lexer_iterator_as() {
        lexical_equate("as", Token::As);
    }

    #[test]
    fn test_lexer_iterator_typeof() {
        lexical_equate("typeof", Token::Typeof);
    }

    #[test]
    fn test_lexer_iterator_integer_literal() {
        // TODO: Implement integer literal tests
    }

    #[test]
    fn test_lexer_iterator_float_literal() {
        // TODO: Implement float literal tests
    }

    #[test]
    fn test_lexer_iterator_string_literal() {
        // TODO: Implement string literal tests
    }

    #[test]
    fn test_lexer_iterator_byte_string_literal() {
        // TODO: Implement byte string literal tests
    }

    #[test]
    fn test_lexer_iterator_single_quote() {
        lexical_equate("'", Token::SingleQuote);
    }

    #[test]
    fn test_lexer_iterator_semicolon() {
        lexical_equate(";", Token::Semi);
    }

    #[test]
    fn test_lexer_iterator_comma() {
        lexical_equate(",", Token::Comma);
    }

    #[test]
    fn test_lexer_iterator_dot() {
        lexical_equate(".", Token::Dot);
    }

    #[test]
    fn test_lexer_iterator_open_paren() {
        lexical_equate("(", Token::OpenParen);
    }

    #[test]
    fn test_lexer_iterator_close_paren() {
        lexical_equate(")", Token::CloseParen);
    }

    #[test]
    fn test_lexer_iterator_open_brace() {
        lexical_equate("{", Token::OpenBrace);
    }

    #[test]
    fn test_lexer_iterator_close_brace() {
        lexical_equate("}", Token::CloseBrace);
    }

    #[test]
    fn test_lexer_iterator_open_bracket() {
        lexical_equate("[", Token::OpenBracket);
    }

    #[test]
    fn test_lexer_iterator_close_bracket() {
        lexical_equate("]", Token::CloseBracket);
    }

    #[test]
    fn test_lexer_iterator_at() {
        lexical_equate("@", Token::At);
    }

    #[test]
    fn test_lexer_iterator_tilde() {
        lexical_equate("~", Token::Tilde);
    }

    #[test]
    fn test_lexer_iterator_question() {
        lexical_equate("?", Token::Question);
    }

    #[test]
    fn test_lexer_iterator_colon() {
        lexical_equate(":", Token::Colon);
    }

    #[test]
    fn test_lexer_iterator_dollar() {
        lexical_equate("$", Token::Dollar);
    }

    #[test]
    fn test_lexer_iterator_eq() {
        lexical_equate("=", Token::Eq);
    }

    #[test]
    fn test_lexer_iterator_bang() {
        lexical_equate("!", Token::Bang);
    }

    #[test]
    fn test_lexer_iterator_lt() {
        lexical_equate("<", Token::Lt);
    }

    #[test]
    fn test_lexer_iterator_gt() {
        lexical_equate(">", Token::Gt);
    }

    #[test]
    fn test_lexer_iterator_minus() {
        lexical_equate("-", Token::Minus);
    }

    #[test]
    fn test_lexer_iterator_and() {
        lexical_equate("&", Token::And);
    }

    #[test]
    fn test_lexer_iterator_or() {
        lexical_equate("|", Token::Or);
    }

    #[test]
    fn test_lexer_iterator_plus() {
        lexical_equate("+", Token::Plus);
    }

    #[test]
    fn test_lexer_iterator_star() {
        lexical_equate("*", Token::Star);
    }

    #[test]
    fn test_lexer_iterator_slash() {
        lexical_equate("/", Token::Slash);
    }

    #[test]
    fn test_lexer_iterator_caret() {
        lexical_equate("^", Token::Caret);
    }

    #[test]
    fn test_lexer_iterator_percent() {
        lexical_equate("%", Token::Percent);
    }

    #[test]
    fn test_lexer_iterator_single_line_comment() {
        let source = "# This is a comment";
        let lexer = Lexer::new(source.as_bytes(), None).expect("source is too big");
        let mut iter = LexerIterator::new(lexer);
        let first_token = iter.next().expect("should have at least one token");

        assert_eq!(
            first_token,
            AnnotatedToken {
                token: Token::Comment(Comment::new(
                    "# This is a comment".into(),
                    CommentKind::SingleLine
                )),
                start_line: 0,
                start_column: 0,
                start_offset: 0,
                end_line: 0,
                end_column: source.len() as u32,
                end_offset: source.len() as u32,
                fileid: None
            }
        );
        assert!(iter.next().is_none(), "should be no more tokens");
    }
}
