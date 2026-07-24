use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_program_simple() {
    let m = parse_source(
        "struct Point { x: i32, y: i32 }
        fn main() { let p = Point { x: 0, y: 0 }; }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_with_imports() {
    let m = parse_source(
        "use std::mem;
        use std::io::{self, Write};
        fn main() {}",
    );
    assert_eq!(m.items.len(), 3);
}

#[test]
fn test_program_visibility() {
    let m = parse_source(
        "pub fn f1() {} sec fn f2() {} pro fn f3() {}
        pub struct S { pub x: i32, sec y: i32, pro z: i32 }",
    );
    assert_eq!(m.items.len(), 4);
}

#[test]
fn test_program_consts() {
    let m = parse_source("const PI: f64 = 3.14; const E: f64 = 2.71; static APP: str = \"My\";");
    assert_eq!(m.items.len(), 3);
}

#[test]
fn test_program_enums() {
    let m = parse_source(
        "enum Option<T> { Some(T), None }
        enum Result<T, E> { Ok(T), Err(E) }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_aliases() {
    let m = parse_source("type Int = i32; type Float = f64; type Pair = (Int, Float);");
    assert_eq!(m.items.len(), 3);
}

#[test]
fn test_program_variadic() {
    let m = parse_source(
        "fn printf(fmt: str, ...) {}
        fn main() { printf(\"hello\"); }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_with_refinements() {
    let m = parse_source(
        "fn add(a: u8: 6, b: u8: 10) -> u8: 16 { a + b }
        fn main() {}",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_attributes() {
    let m = parse_source(
        "struct [derive(Debug)] [repr(C)] Vec2 { x: f64, y: f64 }
        fn [inline] dot(a: Vec2, b: Vec2) -> f64 { 42.0 }",
    );
    assert_eq!(m.items.len(), 2);
}

#[test]
fn test_program_modules() {
    let m = parse_source(
        "mod math { fn add(x: i32, y: i32) -> i32 { 42 } }
        mod io { fn print(s: String) {} }
        fn main() { math::add(1, 2); }",
    );
    assert_eq!(m.items.len(), 3);
}
