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
