use criterion::{Criterion, criterion_group, criterion_main};
use nitrate_compiler::lexer::*;
use nitrate_compiler::parser::*;
use nitrate_compiler::parsetree::*;

static mut STORAGE: Option<Storage<'static>> = None;

fn parse_type(source_code: &'static str) {
    #[allow(static_mut_refs)]
    let storage = unsafe { STORAGE.as_mut().unwrap_unchecked() };

    let lexer = Lexer::new(source_code.as_bytes(), "").expect("Failed to create lexer");
    let mut parser = Parser::new(lexer, storage, None);
    parser.parse_type().expect("Failed to parse type");
}

fn primitive() {
    parse_type("i32");
}

fn infer() {
    parse_type("_");
}

fn name() {
    parse_type("std::Covariant");
}

fn refinement() {
    parse_type("u8: 6: [30:40]");
}

fn tuple() {
    parse_type("{String, u8, bool}");
}

fn array() {
    parse_type("[u8; 10]");
}

fn map() {
    parse_type("[char -> u8]");
}

fn slice() {
    parse_type("[u8]");
}

fn function() {
    parse_type("fn[10, 20](a, b: i32 = 40) -> bool");
}

fn reference() {
    parse_type("&i32");
}

fn pointer() {
    parse_type("*i32");
}

fn generic() {
    parse_type("Vec<Point<f32>: 20, growth: 3.2>");
}

fn opaque() {
    parse_type("opaque(\"sqlite3\")");
}

fn monster() {
    parse_type("Option<[str -> Vec<{u8, str: 48, Set<Address<str>>: 2: [1:]}>]>: 1");
}

fn parse_type_benchmark(c: &mut Criterion) {
    unsafe {
        STORAGE = Some(Storage::new());
    }

    let mut g = c.benchmark_group("parse_type");

    g.bench_function("primitive", |b| b.iter(|| primitive()));
    g.bench_function("infer", |b| b.iter(|| infer()));
    g.bench_function("name", |b| b.iter(|| name()));
    g.bench_function("refinement", |b| b.iter(|| refinement()));
    g.bench_function("tuple", |b| b.iter(|| tuple()));
    g.bench_function("array", |b| b.iter(|| array()));
    g.bench_function("map", |b| b.iter(|| map()));
    g.bench_function("slice", |b| b.iter(|| slice()));
    g.bench_function("function", |b| b.iter(|| function()));
    g.bench_function("reference", |b| b.iter(|| reference()));
    g.bench_function("pointer", |b| b.iter(|| pointer()));
    g.bench_function("generic", |b| b.iter(|| generic()));
    g.bench_function("opaque", |b| b.iter(|| opaque()));
    g.bench_function("monster", |b| b.iter(|| monster()));

    g.finish();
}

criterion_group!(benches, parse_type_benchmark);
criterion_main!(benches);
