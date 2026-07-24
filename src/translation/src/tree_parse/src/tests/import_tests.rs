use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_import_simple() {
    assert!(matches!(
        &single_import(parse_source("use std::mem;")).use_tree,
        UseTree::Single { .. }
    ));
}

#[test]
fn test_import_glob() {
    assert!(matches!(
        &single_import(parse_source("use std::*;")).use_tree,
        UseTree::UseAll { .. }
    ));
}

#[test]
fn test_import_group() {
    assert!(matches!(
        &single_import(parse_source("use std::{a, b};")).use_tree,
        UseTree::Group { .. }
    ));
}

#[test]
fn test_import_alias() {
    assert!(matches!(
        &single_import(parse_source("use std::mem as m;")).use_tree,
        UseTree::Alias { .. }
    ));
}

#[test]
fn test_import_pub() {
    assert!(matches!(
        single_import(parse_source("pub use std::mem;")).visibility,
        Some(Visibility::Public)
    ));
}

#[test]
fn test_import_attr() {
    assert!(
        single_import(parse_source("use [allow(unused)] std::mem;"))
            .attributes
            .is_some()
    );
}

#[test]
fn test_import_nested_group() {
    let m = parse_source("use foo::{bar::{baz, qux}};");
    assert!(matches!(&single_import(m).use_tree, UseTree::Group { .. }));
}

// ========== IMPORT ERROR PATHS ==========

#[test]
fn test_import_alias_missing_name() {
    let (_, log) = parse_source_no_assert("use foo as ;");
    assert!(log.error_bit());
}

#[test]
fn test_import_expected_star_or_group() {
    let (_, log) = parse_source_no_assert("use foo::bar::;");
    assert!(log.error_bit());
}

#[test]
fn test_import_group_unclosed() {
    let (_, log) = parse_source_no_assert("use foo::{bar");
    assert!(log.error_bit());
}

#[test]
fn test_import_missing_semicolon() {
    let (_, log) = parse_source_no_assert("use foo");
    assert!(log.error_bit());
}



// ---------- IMPORT ERRORS ----------

// SyntaxErr::ImportAliasMissingName (variant 41)


// SyntaxErr::ImportExpectedStarOrGroup (variant 42)


// SyntaxErr::ImportGroupExpectedEnd (variant 43)
#[test]
fn test_import_group_expected_end() {
    let (_, log) = parse_source_no_assert("use foo::{bar, baz");
    assert!(log.error_bit());
}


// ========== IMPORT EDGE CASES ==========

#[test]
fn test_import_global_path() {
    let imp = single_import(parse_source("use ::std::mem;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
}


// ========== ITEM PATH EDGE CASES ==========

#[test]
fn test_use_parse_item_path_expected_name() {
    let (_, log) = parse_source_no_assert("use ::;");
    assert!(log.error_bit());
}

