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

// ========== SELF IMPORT ==========

#[test]
fn test_import_self_bare() {
    let imp = single_import(parse_source("use self;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 1);
    assert_eq!(path.segments[0].segment, "self");
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::SelfPath)));
}

#[test]
fn test_import_self_module() {
    let imp = single_import(parse_source("use self::bar;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 2);
    assert_eq!(path.segments[0].segment, "self");
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::SelfPath)));
    assert_eq!(path.segments[1].segment, "bar");
    assert!(path.segments[1].prefix.is_none());
}

#[test]
fn test_import_self_module_alias() {
    assert!(matches!(
        &single_import(parse_source("use self::bar as b;")).use_tree,
        UseTree::Alias { .. }
    ));
}

// ========== CRATE IMPORT ==========

#[test]
fn test_import_crate_bare() {
    let imp = single_import(parse_source("use crate;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 1);
    assert_eq!(path.segments[0].segment, "crate");
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Crate)));
}

#[test]
fn test_import_crate_module() {
    let imp = single_import(parse_source("use crate::bar;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 2);
    assert_eq!(path.segments[0].segment, "crate");
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Crate)));
    assert_eq!(path.segments[1].segment, "bar");
    assert!(path.segments[1].prefix.is_none());
}

#[test]
fn test_import_crate_deep_path() {
    let imp = single_import(parse_source("use crate::foo::bar;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 3);
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Crate)));
    assert_eq!(path.segments[1].segment, "foo");
    assert_eq!(path.segments[2].segment, "bar");
}

#[test]
fn test_import_crate_glob() {
    assert!(matches!(
        &single_import(parse_source("use crate::foo::*;")).use_tree,
        UseTree::UseAll { .. }
    ));
}

#[test]
fn test_import_crate_group() {
    assert!(matches!(
        &single_import(parse_source("use crate::foo::{a, b};")).use_tree,
        UseTree::Group { .. }
    ));
}

#[test]
fn test_import_crate_alias() {
    assert!(matches!(
        &single_import(parse_source("use crate::foo as f;")).use_tree,
        UseTree::Alias { .. }
    ));
}

// ========== SUPER IMPORT ==========

#[test]
fn test_import_super_bare() {
    let imp = single_import(parse_source("use super;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 1);
    assert_eq!(path.segments[0].segment, "super");
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Super)));
}

#[test]
fn test_import_super_module() {
    let imp = single_import(parse_source("use super::bar;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 2);
    assert_eq!(path.segments[0].segment, "super");
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Super)));
    assert_eq!(path.segments[1].segment, "bar");
    assert!(path.segments[1].prefix.is_none());
}

#[test]
fn test_import_super_super() {
    let imp = single_import(parse_source("use super::super::bar;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 3);
    assert_eq!(path.segments[0].segment, "super");
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Super)));
    assert_eq!(path.segments[1].segment, "super");
    assert_eq!(path.segments[2].segment, "bar");
}

#[test]
fn test_import_super_super_bare() {
    let imp = single_import(parse_source("use super::super;"));
    assert!(matches!(&imp.use_tree, UseTree::Single { .. }));
    let path = imp.use_tree.path();
    assert_eq!(path.segments.len(), 2);
    assert_eq!(path.segments[0].segment, "super");
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Super)));
    assert_eq!(path.segments[1].segment, "super");
}

#[test]
fn test_import_super_glob() {
    assert!(matches!(
        &single_import(parse_source("use super::*;")).use_tree,
        UseTree::UseAll { .. }
    ));
}

#[test]
fn test_import_super_group() {
    assert!(matches!(
        &single_import(parse_source("use super::{a, b};")).use_tree,
        UseTree::Group { .. }
    ));
}

#[test]
fn test_import_super_alias() {
    assert!(matches!(
        &single_import(parse_source("use super::foo as f;")).use_tree,
        UseTree::Alias { .. }
    ));
}

// ========== PUB IMPORT WITH PREFIX ==========

#[test]
fn test_import_pub_crate() {
    let imp = single_import(parse_source("pub use crate::foo;"));
    assert!(matches!(imp.visibility, Some(Visibility::Public)));
    let path = imp.use_tree.path();
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Crate)));
}

#[test]
fn test_import_pub_super() {
    let imp = single_import(parse_source("pub use super::foo;"));
    assert!(matches!(imp.visibility, Some(Visibility::Public)));
    let path = imp.use_tree.path();
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::Super)));
}

#[test]
fn test_import_pub_self() {
    let imp = single_import(parse_source("pub use self::foo;"));
    assert!(matches!(imp.visibility, Some(Visibility::Public)));
    let path = imp.use_tree.path();
    assert!(matches!(path.segments[0].prefix, Some(PathPrefix::SelfPath)));
}
