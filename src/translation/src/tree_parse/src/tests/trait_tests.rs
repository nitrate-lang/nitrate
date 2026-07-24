use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_trait_empty() {
    assert!(single_trait(parse_source("trait F {}")).items.is_empty());
}

#[test]
fn test_trait_method() {
    assert_eq!(single_trait(parse_source("trait F { fn foo(); }")).items.len(), 1);
}

#[test]
fn test_trait_const() {
    assert!(matches!(
        &single_trait(parse_source("trait F { const X: i32; }")).items[0],
        AssociatedItem::ConstantItem(_)
    ));
}

#[test]
fn test_trait_type() {
    assert!(matches!(
        &single_trait(parse_source("trait F { type X; }")).items[0],
        AssociatedItem::TypeAlias(_)
    ));
}

#[test]
fn test_trait_pub() {
    assert!(matches!(
        single_trait(parse_source("pub trait F {}")).visibility,
        Some(Visibility::Public)
    ));
}

#[test]
fn test_trait_attr() {
    assert!(
        single_trait(parse_source("trait [must_use] F { fn foo(); }"))
            .attributes
            .is_some()
    );
}
