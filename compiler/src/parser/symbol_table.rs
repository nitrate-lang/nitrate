#[allow(unused_imports)]
use crate::parsetree::{Builder, ExprKey, Storage, node};
use smallvec::SmallVec;
use std::collections::HashMap;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct QualifiedScope<'a> {
    parts: SmallVec<[&'a str; 8]>,
}

impl<'a> QualifiedScope<'a> {
    pub fn new(parts: SmallVec<[&'a str; 8]>) -> Self {
        Self { parts }
    }

    pub fn parse(scope: &'a str) -> Self {
        let parts = scope
            .split("::")
            .filter(|s| !s.is_empty())
            .collect::<SmallVec<[&'a str; 8]>>();
        Self { parts }
    }

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

    pub fn names(&self) -> &[&'a str] {
        &self.parts
    }
}

impl std::string::ToString for QualifiedScope<'_> {
    fn to_string(&self) -> String {
        self.parts.iter().cloned().collect::<Vec<_>>().join("::")
    }
}

#[derive(Debug, Clone, Default)]
pub struct SymbolTable<'a> {
    scopes: HashMap<QualifiedScope<'a>, HashMap<&'a str, ExprKey<'a>>>,
}

impl<'a> SymbolTable<'a> {
    pub fn insert(
        &mut self,
        symbol_scope: QualifiedScope<'a>,
        symbol_name: &'a str,
        symbol: ExprKey<'a>,
    ) -> bool {
        self.scopes
            .entry(symbol_scope.clone())
            .or_insert_with(HashMap::new)
            .insert(symbol_name, symbol)
            .is_none()
    }

    pub fn resolve(
        &self,
        current_scope: QualifiedScope<'a>,
        symbol_name: &str,
    ) -> Option<ExprKey<'a>> {
        let mut search_scope = current_scope;

        loop {
            if let Some(available_symbols) = self.scopes.get(&search_scope) {
                if let Some(symbol) = available_symbols.get(symbol_name) {
                    return Some(*symbol);
                }
            }

            if search_scope.is_root() {
                return None;
            }

            search_scope.pop();
        }
    }
}

#[test]
fn test_symbol_table() {
    let mut storage = Storage::default();
    let mut symbol_table = SymbolTable::default();

    let func_1 = Builder::new(&mut storage)
        .create_function()
        .with_name("foo")
        .build();
    let func_2 = Builder::new(&mut storage)
        .create_function()
        .with_name("foo")
        .build();
    let func_3 = Builder::new(&mut storage)
        .create_function()
        .with_name("foo")
        .build();
    let func_4 = Builder::new(&mut storage)
        .create_function()
        .with_name("foo")
        .build();

    let symbols = [
        (QualifiedScope::parse(""), "foo", func_1),
        (QualifiedScope::parse("app"), "foo", func_2),
        (QualifiedScope::parse("app::sub"), "foo", func_3),
        (QualifiedScope::parse("app::sub::subsub"), "foo", func_4),
    ];

    for (scope, name, symbol) in symbols.clone() {
        assert!(symbol_table.insert(scope, name, symbol));
    }

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse(""), "foo"),
        Some(func_1)
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("other"), "foo"),
        Some(func_1)
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app"), "foo"),
        Some(func_2)
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app::sub"), "foo"),
        Some(func_3)
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app::sub::subsub"), "foo"),
        Some(func_4)
    );

    assert_eq!(
        symbol_table.resolve(QualifiedScope::parse("app::sub::subsub::other"), "foo"),
        Some(func_4)
    );

    assert_eq!(
        symbol_table.resolve(
            QualifiedScope::parse("app::sub::subsub::other::deep::in::code"),
            "foo"
        ),
        Some(func_4)
    );

    assert_eq!(
        symbol_table.resolve(
            QualifiedScope::parse("app::sub::subsub::other::deep::in::code"),
            "bar"
        ),
        None
    );
}
