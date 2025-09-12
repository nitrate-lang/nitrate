use crate::expression::Expr;
use serde::{Deserialize, Serialize};

#[derive(Clone, Serialize, Deserialize)]
pub enum Type {}

impl Type {}

impl std::fmt::Debug for Type {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let expr: Expr = self.to_owned().into();
        expr.fmt(f)
    }
}

impl TryInto<Type> for Expr {
    type Error = Self;

    fn try_into(self) -> Result<Type, Self::Error> {
        match self {}
    }
}
