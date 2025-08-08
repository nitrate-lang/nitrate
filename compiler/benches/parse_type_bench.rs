use criterion::{Criterion, criterion_group, criterion_main};
use nitrate_compiler::lexer::*;
use nitrate_compiler::parser::*;
use nitrate_compiler::parsetree::*;

static mut STORAGE: Option<Storage<'static>> = None;

fn parse_type(source_code: &'static str) {
    #[allow(static_mut_refs)]
    let storage = unsafe { STORAGE.as_mut().unwrap() };

    let lexer = Lexer::new(source_code.as_bytes(), "<inline>").expect("Failed to create lexer");
    let mut parser = Parser::new(lexer, storage, None);
    parser.parse_type().expect("Failed to parse source code");
}

fn parse_type_primitive() {
    parse_type("i32");
}

fn parse_type_infer() {
    parse_type("_");
}

fn parse_type_name() {
    parse_type("std::Covariant");
}

fn parse_type_refine() {
    parse_type("u8: 6: [30:40]");
}

fn parse_type_tuple() {
    parse_type("{String, u8, bool}");
}

fn parse_type_array() {
    parse_type("[u8; 10]");
}

fn parse_type_map() {
    parse_type("[char -> u8]");
}

fn parse_type_slice() {
    parse_type("[u8]");
}

fn parse_type_function() {
    parse_type("fn[10, 20](a, b: i32 = 40) -> bool");
}

fn parse_type_ref() {
    parse_type("&i32");
}

fn parse_type_ptr() {
    parse_type("*i32");
}

fn parse_type_generic() {
    parse_type("Vec<Point<f32>: 20, growth: 3.2>");
}

fn parse_type_opaque() {
    parse_type("opaque(\"sqlite3\")");
}

fn parse_type_monster() {
    parse_type("Option<[str -> Vec<{u8, str: 48, Set<Address<str>>: 2: [1:]}>]>: 1");
}

fn criterion_benchmark(c: &mut Criterion) {
    unsafe {
        STORAGE = Some(Storage::new());
    }

    c.bench_function("parse_type_primitive", |b| {
        b.iter(|| parse_type_primitive())
    });

    c.bench_function("parse_type_infer", |b| b.iter(|| parse_type_infer()));
    c.bench_function("parse_type_name", |b| b.iter(|| parse_type_name()));
    c.bench_function("parse_type_refine", |b| b.iter(|| parse_type_refine()));
    c.bench_function("parse_type_tuple", |b| b.iter(|| parse_type_tuple()));
    c.bench_function("parse_type_array", |b| b.iter(|| parse_type_array()));
    c.bench_function("parse_type_map", |b| b.iter(|| parse_type_map()));
    c.bench_function("parse_type_slice", |b| b.iter(|| parse_type_slice()));
    c.bench_function("parse_type_function", |b| b.iter(|| parse_type_function()));
    c.bench_function("parse_type_ref", |b| b.iter(|| parse_type_ref()));
    c.bench_function("parse_type_ptr", |b| b.iter(|| parse_type_ptr()));
    c.bench_function("parse_type_generic", |b| b.iter(|| parse_type_generic()));
    c.bench_function("parse_type_opaque", |b| b.iter(|| parse_type_opaque()));
    c.bench_function("parse_type_monster", |b| b.iter(|| parse_type_monster()));
}

criterion_group!(benches, criterion_benchmark);
criterion_main!(benches);
