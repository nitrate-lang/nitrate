#![cfg(test)]
mod tests {
    use crate::{Lexer, LexerIterator};

    #[test]
    fn test_lexer_iterator() {
        let source = "fn main() { }";
        let lexer = Lexer::new(source.as_bytes(), None).expect("source is too big");
        let iter = LexerIterator::new(lexer);
        let tokens: Vec<_> = iter.collect();
        assert!(!tokens.is_empty(), "LexerIterator should produce tokens");
    }
}
