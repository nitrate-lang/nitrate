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

// ========== MODULE ERROR PATHS ==========

#[test]
fn test_mod_missing_name() {
    let (_, log) = parse_source_no_assert("mod { fn f() {} }");
    assert!(log.error_bit());
}

#[test]
fn test_mod_missing_brace() {
    let (_, log) = parse_source_no_assert("mod m fn f() {} }");
    assert!(log.error_bit());
}



// Additional coverage tests targeting specific remaining uncovered format() arms

// ========== EXPECTED CLOSE BRACE ==========
#[test]
fn test_err_stmt_after_module() {
    // Trigger ExpectedCloseBrace by having statement directly at module level
    let (_, log) = parse_source_no_assert("mod foo { 42 }");
    // Should error because 42 appears in a module context as a statement
    assert!(log.error_bit());
}


// SyntaxErr::ModuleItemLimit (variant 21) - needs >65536 items in module
#[test]
fn test_mod_item_limit() {
    let mut items = String::new();
    for _ in 0..65538 {
        items.push_str("fn f() {} ");
    }
    let src = format!("mod m {{ {items} }}");
    let (_, log) = parse_source_no_assert(&src);
    assert!(log.error_bit());
}


// SyntaxErr::ModuleExpectedEnd (variant 22)
#[test]
fn test_mod_expected_end() {
    let (_, log) = parse_source_no_assert("mod foo { fn f() {}");
    assert!(log.error_bit());
}


// ========== MODULE EDGE CASES ==========

#[test]
fn test_module_with_items() {
    let mod_item = match single_item(parse_source("mod bar { fn f() {} fn g() {} }")) {
        Item::Module(m) => m,
        other => panic!("Expected Module, got {other:?}"),
    };
    assert_eq!(&*mod_item.name, "bar");
    assert_eq!(mod_item.items.len(), 2);
}


#[test]
fn test_module_missing_name() {
    let (_, log) = parse_source_no_assert("mod { fn f() {} }");
    assert!(log.error_bit());
}


#[test]
fn test_module_unclosed() {
    let (_, log) = parse_source_no_assert("mod foo { fn f() {}");
    assert!(log.error_bit());
}


#[test]
fn test_module_attributes() {
    let mod_item = match single_item(parse_source("mod [attr] foo { fn f() {} }")) {
        Item::Module(m) => m,
        other => panic!("Expected Module, got {other:?}"),
    };
    assert!(mod_item.attributes.is_some());
}


// ========== EMPTY MODULE ==========

#[test]
fn test_empty_module() {
    let mod_item = match single_item(parse_source("mod empty {}")) {
        Item::Module(m) => m,
        other => panic!("Expected Module, got {other:?}"),
    };
    assert!(mod_item.items.is_empty());
}


// ========== NESTED MODULE ERROR ==========

#[test]
fn test_module_missing_close_brace() {
    let (_, log) = parse_source_no_assert("mod foo { fn f() {} ");
    assert!(log.error_bit(), "Module should fail if close brace is missing");
}


// ========== MULTIPLE IMPORTS ==========

#[test]
fn test_program_with_only_imports() {
    let m = parse_source("use a; use b; use c;");
    assert_eq!(m.items.len(), 3);
}

