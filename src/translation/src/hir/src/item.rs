use crate::Store;

use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize)]
pub enum Item {
    Module,
    Import,
    Function,
    TypeAlias,
    Struct,
    Enum,
    Constant,
    Static,
    Trait,
    Impl,
    ExternBlock,
}

impl Item {}

impl Item {
    pub fn dump(&self, _store: &Store, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            _ => write!(o, "<unimplemented>"),
        }
    }

    pub fn dump_string(&self, store: &Store) -> String {
        let mut buf = String::new();
        self.dump(store, &mut buf).ok();
        buf
    }
}
