use crate::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Ty {}

impl From<Ty> for TypeId {
    fn from(ty: Ty) -> Self {
        get_storage(|store| store.store_type(ty))
    }
}
