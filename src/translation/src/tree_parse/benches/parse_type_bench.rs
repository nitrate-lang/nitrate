use criterion::{Criterion, criterion_group, criterion_main};
use nitrate_diagnosis::DiagnosticCollector;
use nitrate_tree_parse::*;
use nitrate_token::*;

struct ParserUtil {
    collector: DiagnosticCollector,
}

fn parse_type(source_code: &'static str, util: &ParserUtil) {
    let lexer = Lexer::new(source_code.as_bytes(), None).expect("Failed to create lexer");
    let mut parser = Parser::new(lexer, &util.collector);
    parser.parse_type();

    if util.collector.error_bit() {
        panic!("Parsing failed");
    }
}

#[inline(never)]
fn primitive(util: &mut ParserUtil) {
    parse_type("i32", util);
}

#[inline(never)]
fn infer(util: &mut ParserUtil) {
    parse_type("_", util);
}

#[inline(never)]
fn name(util: &mut ParserUtil) {
    parse_type("std::Covariant", util);
}

#[inline(never)]
fn refinement(util: &mut ParserUtil) {
    parse_type("u8: 6: [30:40]", util);
}

#[inline(never)]
fn tuple(util: &mut ParserUtil) {
    parse_type("(String, u8, bool)", util);
}

#[inline(never)]
fn array(util: &mut ParserUtil) {
    parse_type("[u8; 10]", util);
}

#[inline(never)]
fn slice(util: &mut ParserUtil) {
    parse_type("[u8]", util);
}

#[inline(never)]
fn function(util: &mut ParserUtil) {
    parse_type("fn[10, 20](a, b: i32 = 40) -> bool", util);
}

#[inline(never)]
fn reference(util: &mut ParserUtil) {
    parse_type("&i32", util);
}

#[inline(never)]
fn pointer(util: &mut ParserUtil) {
    parse_type("*i32", util);
}

#[inline(never)]
fn generic(util: &mut ParserUtil) {
    parse_type("Vec<element: Point<f32>, second: u8>", util);
}

#[inline(never)]
fn opaque(util: &mut ParserUtil) {
    parse_type("opaque(\"sqlite3\")", util);
}

#[inline(never)]
fn monster(util: &mut ParserUtil) {
    parse_type(
        "Option<HashMap<str, Vec<u8, s: i32, Set<Address<str>>: 2: [1:]>>>: 1",
        util,
    );
}

fn parse_type_benchmark(c: &mut Criterion) {
    let mut util = ParserUtil {
        collector: DiagnosticCollector::default(),
    };

    let mut g = c.benchmark_group("parse_type");

    g.bench_function("primitive", |b| b.iter(|| primitive(&mut util)));
    g.bench_function("infer", |b| b.iter(|| infer(&mut util)));
    g.bench_function("name", |b| b.iter(|| name(&mut util)));
    g.bench_function("refinement", |b| b.iter(|| refinement(&mut util)));
    g.bench_function("tuple", |b| b.iter(|| tuple(&mut util)));
    g.bench_function("array", |b| b.iter(|| array(&mut util)));
    g.bench_function("slice", |b| b.iter(|| slice(&mut util)));
    g.bench_function("function", |b| b.iter(|| function(&mut util)));
    g.bench_function("reference", |b| b.iter(|| reference(&mut util)));
    g.bench_function("pointer", |b| b.iter(|| pointer(&mut util)));
    g.bench_function("generic", |b| b.iter(|| generic(&mut util)));
    g.bench_function("opaque", |b| b.iter(|| opaque(&mut util)));
    g.bench_function("monster", |b| b.iter(|| monster(&mut util)));

    g.finish();
}

criterion_group!(benches, parse_type_benchmark);
criterion_main!(benches);
