use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_generic_struct() {
    assert_eq!(
        &*single_struct(parse_source("struct F<T> { x: T }"))
            .generics
            .unwrap()
            .params[0]
            .name,
        "T"
    );
}

#[test]
fn test_generic_default() {
    assert!(
        single_struct(parse_source("struct F<T = i32> { x: T }"))
            .generics
            .unwrap()
            .params[0]
            .default_value
            .is_some()
    );
}
