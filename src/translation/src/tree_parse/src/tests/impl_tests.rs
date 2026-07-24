use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_impl_trait_for() {
    assert!(
        single_impl(parse_source("impl trait Foo for Bar { fn m() {} }"))
            .trait_path
            .is_some()
    );
}
