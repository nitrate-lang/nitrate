#[allow(unused_imports)]
use nitrate_parsetree::{
    Builder,
    kind::{Expr, QualifiedScope},
};
use std::collections::HashMap;

#[derive(Debug, Clone, Default)]
pub struct SymbolTable<'a> {
    scopes: HashMap<QualifiedScope<'a>, HashMap<&'a str, Expr<'a>>>,
}

impl<'a> SymbolTable<'a> {
    pub fn insert(
        &mut self,
        symbol_scope: QualifiedScope<'a>,
        symbol_name: &'a str,
        symbol: Expr<'a>,
    ) -> bool {
        self.scopes
            .entry(symbol_scope.clone())
            .or_default()
            .insert(symbol_name, symbol)
            .is_none()
    }

    #[must_use]
    pub fn resolve(
        &self,
        current_scope: QualifiedScope<'a>,
        symbol_name: &str,
    ) -> Option<Expr<'a>> {
        let mut search_scope = current_scope;

        loop {
            if let Some(available_symbols) = self.scopes.get(&search_scope) {
                if let Some(symbol) = available_symbols.get(symbol_name) {
                    return Some(symbol.clone());
                }
            }

            if search_scope.is_root() {
                return None;
            }

            search_scope.pop();
        }
    }
}

impl std::fmt::Display for SymbolTable<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        for (scope, symbols) in &self.scopes {
            for (name, symbol) in symbols {
                writeln!(f, "{scope}::{name}: {symbol:?}")?;
            }
        }
        Ok(())
    }
}

#[test]
fn test_symbol_table() {
    let mut symbol_table = SymbolTable::default();

    let var_1 = Builder::create_let()
        .with_name("foo")
        .with_type(Builder::get_u8())
        .build();

    let var_2 = Builder::create_let()
        .with_name("foo")
        .with_type(Builder::get_u16())
        .build();

    let var_3 = Builder::create_let()
        .with_name("foo")
        .with_type(Builder::get_u32())
        .build();

    let var_4 = Builder::create_let()
        .with_name("bar")
        .with_type(Builder::get_u64())
        .build();

    let symbols = [
        (QualifiedScope::parse(""), var_1.clone()),
        (QualifiedScope::parse("app"), var_2.clone()),
        (QualifiedScope::parse("app::sub"), var_3.clone()),
        (QualifiedScope::parse("app::sub"), var_4.clone()),
    ];

    for (scope, symbol) in symbols.clone() {
        let name = match &symbol {
            Expr::Variable(var) => var.name(),
            _ => panic!("Expected a variable expression"),
        };

        assert!(symbol_table.insert(scope, name, symbol));
    }

    assert_eq!(
        // Resolve func_1 in the root scope
        symbol_table.resolve(QualifiedScope::parse(""), "foo"),
        Some(var_1.clone())
    );

    assert_eq!(
        // Resolve func_1 in the root (because it the closest parent scope with a match)
        symbol_table.resolve(QualifiedScope::parse("this::is::cool"), "foo"),
        Some(var_1.clone())
    );

    assert_eq!(
        // Resolve func_2 in the ::app scope
        symbol_table.resolve(QualifiedScope::parse("app"), "foo"),
        Some(var_2.clone())
    );

    assert_eq!(
        // Resolve func_2 in the ::app (because it is the closest parent scope with a match)
        symbol_table.resolve(QualifiedScope::parse("app::deeper::namespace"), "foo"),
        Some(var_2.clone())
    );

    assert_eq!(
        // Resolve func_3 in the app::sub scope
        symbol_table.resolve(QualifiedScope::parse("app::sub"), "foo"),
        Some(var_3.clone())
    );

    assert_eq!(
        // Resolve func_4 in the app::sub scope
        symbol_table.resolve(QualifiedScope::parse("app::sub"), "bar"),
        Some(var_4.clone())
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app::sub::deeper"), "foo"),
        Some(var_3.clone())
    );
}
