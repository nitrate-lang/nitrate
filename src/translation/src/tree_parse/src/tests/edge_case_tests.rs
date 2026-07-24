use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_empty_source() {
    assert!(parse_source("").items.is_empty());
}

#[test]
fn test_whitespace() {
    assert!(parse_source("   \n  \t  ").items.is_empty());
}

#[test]
fn test_unicode_name() {
    assert_eq!(&*single_function(parse_source("fn 日本語() {}")).name, "日本語");
}

#[test]
fn test_underscore_name() {
    assert_eq!(&*single_function(parse_source("fn _() {}")).name, "_");
}

#[test]
fn test_multi_items_same_line() {
    assert_eq!(parse_source("struct Foo {} fn bar() {}").items.len(), 2);
}

#[test]
fn test_multi_items_lines() {
    assert_eq!(parse_source("struct Foo {}\n\nfn bar() {}").items.len(), 2);
}
