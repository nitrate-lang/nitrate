use crate::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Stmt {}

impl From<Stmt> for StmtId {
    fn from(stmt: Stmt) -> Self {
        get_storage(|store| store.store_stmt(stmt))
    }
}
