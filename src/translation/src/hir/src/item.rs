use crate::{DumpContext, dump::Dump};

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

impl Dump for Item {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            _ => Ok(()),
        }
    }
}
