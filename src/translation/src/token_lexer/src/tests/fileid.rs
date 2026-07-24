//! Tests for FileId propagation through the lexer.

use crate::Lexer;

#[test]
fn test_fileid_is_propagated_to_tokens() {
    let fid = nitrate_diagnosis::intern_file_id("test_file.nit");
    let old_fid = fid.clone();
    let mut lexer = Lexer::new(b"fn main() {}", fid).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.fileid, old_fid);
    let tok2 = lexer.next_tok();
    assert_eq!(tok2.fileid, old_fid);
}

#[test]
fn test_fileid_none_when_omitted() {
    let mut lexer = Lexer::new(b"x", None).expect("source too big");
    lexer.disable_trivia();
    let tok = lexer.next_tok();
    assert_eq!(tok.fileid, None);
}

#[test]
fn test_fileid_through_peek() {
    let fid = nitrate_diagnosis::intern_file_id("peek_test.nit");
    let mut lexer = Lexer::new(b"x", fid.clone()).expect("source too big");
    lexer.disable_trivia();
    let p = lexer.peek_tok();
    assert_eq!(p.fileid, fid);
}
