use criterion::{Criterion, criterion_group, criterion_main};
use nitrate_tokenize::*;

struct LexerUtil {}

fn lex(source_code: &[u8], _util: &mut LexerUtil) {
    let mut lexer = Lexer::new(source_code, None).expect("Failed to create lexer");

    while lexer.next_t() != Token::Eof {}
}

#[inline(never)]
fn identifier(util: &mut LexerUtil) {
    const SOURCE: &[u8] = b"identifier_example";
    lex(SOURCE, util);
}

#[inline(never)]
fn integer(util: &mut LexerUtil) {
    const SOURCE: &[u8] = b"1234567890";
    lex(SOURCE, util);
}

#[inline(never)]
fn float(util: &mut LexerUtil) {
    const SOURCE: &[u8] = b"3.14159";
    lex(SOURCE, util);
}

#[inline(never)]
fn basic_string(util: &mut LexerUtil) {
    const SOURCE: &[u8] = b"\"basic_string_example\"";
    lex(SOURCE, util);
}

#[inline(never)]
fn emoji_string(util: &mut LexerUtil) {
    const SOURCE: &[u8] = "\"this is fun 😀 string\\n\"".as_bytes();
    lex(SOURCE, util);
}

#[inline(never)]
fn binary_string(util: &mut LexerUtil) {
    const SOURCE: &[u8] = b"\"binary_string_\x00\xff_example\"";
    lex(SOURCE, util);
}

#[inline(never)]
fn comment(util: &mut LexerUtil) {
    const SOURCE: &[u8] = b"# This is a comment\nidentifier_after_comment";
    lex(SOURCE, util);
}

fn lex_benchmark(c: &mut Criterion) {
    let mut util = LexerUtil {};

    let mut g = c.benchmark_group("lex");

    g.bench_function("identifier", |b| b.iter(|| identifier(&mut util)));
    g.bench_function("integer", |b| b.iter(|| integer(&mut util)));
    g.bench_function("float", |b| b.iter(|| float(&mut util)));
    g.bench_function("basic_string", |b| b.iter(|| basic_string(&mut util)));
    g.bench_function("emoji_string", |b| b.iter(|| emoji_string(&mut util)));
    g.bench_function("binary_string", |b| b.iter(|| binary_string(&mut util)));
    g.bench_function("comment", |b| b.iter(|| comment(&mut util)));

    g.finish();
}

criterion_group!(benches, lex_benchmark);
criterion_main!(benches);
