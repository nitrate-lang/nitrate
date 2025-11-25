use crate::prelude::*;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub enum Pat {}

impl From<Pat> for PatId {
    fn from(pat: Pat) -> Self {
        get_storage(|store| store.store_pat(pat))
    }
}
