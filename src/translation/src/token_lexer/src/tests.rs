#![cfg(test)]
mod tests {
    use nitrate_token::{AnnotatedToken, Token};

    use crate::{Lexer, LexerIterator};

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
}
