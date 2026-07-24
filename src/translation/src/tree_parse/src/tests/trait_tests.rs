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

// ========== TRAIT ERROR PATHS ==========

#[test]
fn test_trait_missing_name() {
    let (_, log) = parse_source_no_assert("trait {}");
    assert!(log.error_bit());
}

#[test]
fn test_trait_missing_brace() {
    let (_, log) = parse_source_no_assert("trait Foo");
    assert!(log.error_bit());
}

#[test]
fn test_trait_invalid_item() {
    let (_, log) = parse_source_no_assert("trait Foo { struct Bad {} }");
    assert!(log.error_bit());
}



// SyntaxErr::TraitItemLimit (variant 181) - needs >65536 items
#[test]
fn test_trait_item_limit() {
    let mut items = String::new();
    for i in 0..65538 {
        items.push_str(&format!("fn f{i}(); "));
    }
    let src = format!("trait Foo {{ {items} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}


// SyntaxErr::TraitExpectedEnd (variant 183)
#[test]
fn test_trait_expected_end() {
    let (_, log) = parse_source_no_assert("trait Foo { fn f();");
    assert!(log.error_bit());
}


#[test]
fn test_trait_with_fn() {
    let t = single_trait(parse_source("trait Foo { fn bar(); }"));
    assert_eq!(t.items.len(), 1);
    assert!(matches!(&t.items[0], AssociatedItem::Method(f) if f.definition.is_none()));
}


#[test]
fn test_trait_with_const() {
    let t = single_trait(parse_source("trait Foo { const X: i32; }"));
    assert_eq!(t.items.len(), 1);
    assert!(matches!(&t.items[0], AssociatedItem::ConstantItem(_)));
}


#[test]
fn test_trait_with_type() {
    let t = single_trait(parse_source("trait Foo { type Bar; }"));
    assert_eq!(t.items.len(), 1);
    assert!(matches!(&t.items[0], AssociatedItem::TypeAlias(_)));
}


#[test]
fn test_trait_with_visibility() {
    let t = single_trait(parse_source("pub trait Foo { fn bar(); }"));
    assert!(matches!(t.visibility, Some(Visibility::Public)));
}


#[test]
fn test_trait_with_generics() {
    let t = single_trait(parse_source("trait Foo<T> { fn bar(x: T); }"));
    assert!(t.generics.is_some());
}


#[test]
fn test_trait_with_attributes() {
    let t = single_trait(parse_source("trait [auto] Foo { fn bar(); }"));
    assert!(t.attributes.is_some());
}


#[test]
fn test_trait_unclosed() {
    let (_, log) = parse_source_no_assert("trait Foo { fn bar();");
    assert!(log.error_bit());
}


#[test]
fn test_trait_missing_open_brace() {
    let (_, log) = parse_source_no_assert("trait Foo fn bar(); }");
    assert!(log.error_bit());
}


// ========== TRAIT WITH INVALID ITEM ==========

#[test]
fn test_trait_syntax_error_item() {
    let (_, log) = parse_source_no_assert("trait Foo { struct Bar; }");
    assert!(log.error_bit());
}

