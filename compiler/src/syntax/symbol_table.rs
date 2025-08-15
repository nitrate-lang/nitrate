#[allow(unused_imports)]
use crate::parsetree::{Builder, Expr, nodes};
use smallvec::SmallVec;
use std::collections::HashMap;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct QualifiedScope<'a> {
    parts: SmallVec<[&'a str; 8]>,
}

impl<'a> QualifiedScope<'a> {
    #[must_use]
    pub fn new(parts: SmallVec<[&'a str; 8]>) -> Self {
        Self { parts }
    }

    #[must_use]
    pub fn parse(scope: &'a str) -> Self {
        let parts = scope
            .split("::")
            .filter(|s| !s.is_empty())
            .collect::<SmallVec<[&'a str; 8]>>();
        Self { parts }
    }

    #[must_use]
    pub fn is_root(&self) -> bool {
        self.parts.is_empty()
    }

    pub fn pop(&mut self) {
        if !self.parts.is_empty() {
            self.parts.pop();
        }
    }

    pub fn push(&mut self, part: &'a str) {
        self.parts.push(part);
    }

    #[must_use]
    pub fn names(&self) -> &[&'a str] {
        &self.parts
    }
}

impl std::fmt::Display for QualifiedScope<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}",
            self.parts.iter().copied().collect::<Vec<_>>().join("::")
        )
    }
}

#[derive(Debug, Clone, Default)]
pub struct SymbolTable<'a> {
    scopes: HashMap<QualifiedScope<'a>, HashMap<&'a str, Arc<Expr<'a>>>>,
}

impl<'a> SymbolTable<'a> {
    pub fn insert(
        &mut self,
        symbol_scope: QualifiedScope<'a>,
        symbol_name: &'a str,
        symbol: Arc<Expr<'a>>,
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
    ) -> Option<Arc<Expr<'a>>> {
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

    let func_1 = Builder::new().create_function().with_name("foo").build();
    let func_2 = Builder::new().create_function().with_name("foo").build();
    let func_3 = Builder::new().create_function().with_name("foo").build();
    let func_4 = Builder::new().create_function().with_name("foo").build();

    let symbols = [
        (QualifiedScope::parse(""), "foo", func_1.clone()),
        (QualifiedScope::parse("app"), "foo", func_2.clone()),
        (QualifiedScope::parse("app::sub"), "foo", func_3.clone()),
        (
            QualifiedScope::parse("app::sub::subsub"),
            "foo",
            func_4.clone(),
        ),
    ];

    for (scope, name, symbol) in symbols.clone() {
        assert!(symbol_table.insert(scope, name, symbol));
    }

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse(""), "foo"),
        Some(func_1.clone())
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("other"), "foo"),
        Some(func_1.clone())
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app"), "foo"),
        Some(func_2.clone())
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app::sub"), "foo"),
        Some(func_3.clone())
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app::sub::subsub"), "foo"),
        Some(func_4.clone())
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app::sub::subsub::other"), "foo"),
        Some(func_4.clone())
    );

    assert_eq!(
        symbol_table.resolve(
            QualifiedScope::parse("app::sub::subsub::other::deep::in::code"),
            "foo"
        ),
        Some(func_4.clone())
    );

    assert_eq!(
        symbol_table.resolve(
            QualifiedScope::parse("app::sub::subsub::other::deep::in::code"),
            "bar"
        ),
        None
    );
}
