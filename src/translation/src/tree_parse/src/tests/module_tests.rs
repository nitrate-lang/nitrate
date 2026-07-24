use super::helpers::*;
use nitrate_tree::ast::*;

#[test]
fn test_mod() {
    assert!(matches!(&parse_source("mod m { fn f() {} }").items[0], Item::Module(m) if m.items.len() == 1));
}

#[test]
fn test_mod_pub() {
    assert!(
        matches!(&parse_source("pub mod m { fn f() {} }").items[0], Item::Module(m) if matches!(m.visibility, Some(Visibility::Public)))
    );
}

#[test]
fn test_mod_attr() {
    assert!(
        matches!(&parse_source("mod [cfg(test)] test { fn h() {} }").items[0], Item::Module(m) if m.attributes.is_some())
    );
}

#[test]
fn test_mod_nested() {
    assert!(
        matches!(&parse_source("mod a { mod b { fn f() {} } }").items[0], Item::Module(outer) if outer.items.len() == 1)
    );
}
