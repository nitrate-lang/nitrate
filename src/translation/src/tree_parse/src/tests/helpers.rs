use nitrate_diagnosis::CompilerLog;
use nitrate_token_lexer::Lexer;
use nitrate_tree::ast::*;

use crate::Parser;

pub fn parse_source(source: &str) -> Module {
    let log = CompilerLog::default();
    let lexer = Lexer::new(source.as_bytes(), None).expect("Failed to create lexer");
    let mut parser = Parser::new(lexer, &log);
    let module = parser.parse_source("test".into());
    assert!(!log.error_bit(), "Parsing failed for:\n```\n{source}\n```");
    module
}

pub fn parse_type(source: &str) -> Type {
    let log = CompilerLog::default();
    let lexer = Lexer::new(source.as_bytes(), None).expect("Failed to create lexer");
    let mut parser = Parser::new(lexer, &log);
    parser.lexer.disable_trivia();
    let ty = parser.parse_type();
    assert!(!log.error_bit(), "Type parsing failed: `{source}`");
    ty
}

pub fn parse_expr(source: &str) -> Expr {
    let log = CompilerLog::default();
    let lexer = Lexer::new(source.as_bytes(), None).expect("Failed to create lexer");
    let mut parser = Parser::new(lexer, &log);
    parser.lexer.disable_trivia();
    let expr = parser.parse_expression();
    assert!(!log.error_bit(), "Expr parsing failed: `{source}`");
    expr
}

pub fn single_item(module: Module) -> Item {
    assert_eq!(module.items.len(), 1, "Expected 1 item, got {}", module.items.len());
    module.items.into_iter().next().unwrap()
}

pub fn single_function(module: Module) -> Function {
    match single_item(module) {
        Item::Function(f) => f,
        other => panic!("Expected Function, got {other:?}"),
    }
}

pub fn single_struct(module: Module) -> Struct {
    match single_item(module) {
        Item::Struct(s) => s,
        other => panic!("Expected Struct, got {other:?}"),
    }
}

pub fn single_enum(module: Module) -> Enum {
    match single_item(module) {
        Item::Enum(e) => e,
        other => panic!("Expected Enum, got {other:?}"),
    }
}

pub fn single_trait(module: Module) -> Trait {
    match single_item(module) {
        Item::Trait(t) => t,
        other => panic!("Expected Trait, got {other:?}"),
    }
}

pub fn single_impl(module: Module) -> Impl {
    match single_item(module) {
        Item::Impl(i) => *i,
        other => panic!("Expected Impl, got {other:?}"),
    }
}

pub fn single_type_alias(module: Module) -> TypeAlias {
    match single_item(module) {
        Item::TypeAlias(t) => t,
        other => panic!("Expected TypeAlias, got {other:?}"),
    }
}

pub fn single_variable(module: Module) -> GlobalVariable {
    match single_item(module) {
        Item::Variable(v) => v,
        other => panic!("Expected Variable, got {other:?}"),
    }
}

pub fn single_import(module: Module) -> Import {
    match single_item(module) {
        Item::Import(i) => *i,
        other => panic!("Expected Import, got {other:?}"),
    }
}
