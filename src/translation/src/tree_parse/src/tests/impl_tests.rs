use super::helpers::*;

#[test]
fn test_impl_trait_for() {
    assert!(
        single_impl(parse_source("impl trait Foo for Bar { fn m() {} }"))
            .trait_path
            .is_some()
    );
}

#[test]
fn test_impl_direct() {
    assert!(
        single_impl(parse_source("impl Foo { fn bar() {} }"))
            .trait_path
            .is_none()
    );
}

// ========== IMPL ERROR PATHS ==========

#[test]
fn test_impl_missing_trait_for() {
    let (_, log) = parse_source_no_assert("impl trait Foo Bar {}");
    assert!(log.error_bit());
}

#[test]
fn test_impl_cannot_be_visible() {
    let (_, log) = parse_source_no_assert("pub impl Foo {}");
    assert!(log.error_bit());
}

#[test]
fn test_impl_missing_brace() {
    let (_, log) = parse_source_no_assert("impl Foo");
    assert!(log.error_bit());
}



// ---------- IMPL ERRORS ----------

// SyntaxErr::ImplMissingFor (variant 200)
#[test]
fn test_impl_missing_for() {
    let (_, log) = parse_source_no_assert("impl Trait Foo {}");
    assert!(log.error_bit());
}


// SyntaxErr::ImplExpectedEnd (variant 201)
#[test]
fn test_impl_expected_end() {
    let (_, log) = parse_source_no_assert("impl Foo { fn f() {}");
    assert!(log.error_bit());
}


// SyntaxErr::ImplItemLimit (variant 202) - needs >65536 items
#[test]
fn test_impl_item_limit() {
    let mut items = String::new();
    for i in 0..65538 {
        items.push_str(&format!("fn f{i}() {{}} "));
    }
    let src = format!("impl Foo {{ {items} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}


// SyntaxErr::ImplCannotBeVisible (variant 203)


// ========== IMPL EDGE CASES ==========

#[test]
fn test_impl_empty() {
    let i = single_impl(parse_source("impl Foo {}"));
    assert!(i.items.is_empty());
    assert!(i.trait_path.is_none());
}


#[test]
fn test_impl_with_generics() {
    let i = single_impl(parse_source("impl<T> Foo<T> {}"));
    assert!(i.generics.is_some());
}


#[test]
fn test_impl_unclosed() {
    let (_, log) = parse_source_no_assert("impl Foo { fn bar() {}");
    assert!(log.error_bit());
}

