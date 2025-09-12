use crate::ItemStore;

use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize)]
pub enum Item {}

impl Item {}

impl Item {
    pub fn dump(
        &self,
        _storage: &ItemStore,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            _ => write!(o, "<unimplemented>"),
        }
    }

    pub fn dump_string(&self, storage: &ItemStore) -> String {
        let mut buf = String::new();
        self.dump(storage, &mut buf).ok();
        buf
    }
}
