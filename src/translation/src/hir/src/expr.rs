use crate::ExprStore;

use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize)]
pub enum Expr {}

impl Expr {}

impl Expr {
    pub fn dump(
        &self,
        _store: &ExprStore,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            _ => write!(o, "<unimplemented>"),
        }
    }

    pub fn dump_string(&self, store: &ExprStore) -> String {
        let mut buf = String::new();
        self.dump(store, &mut buf).ok();
        buf
    }
}
