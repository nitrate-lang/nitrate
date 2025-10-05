use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
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
