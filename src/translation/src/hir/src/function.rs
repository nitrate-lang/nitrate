use crate::{prelude::*, store::FunctionId, ty::FunctionAttribute};
use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub enum Function {
    External {
        attributes: Vec<FunctionAttribute>,
        name: IString,
        parameters: Vec<TypeId>,
        return_type: TypeId,
    },

    Internal {
        attributes: Vec<FunctionAttribute>,
        name: IString,
        parameters: Vec<TypeId>,
        return_type: TypeId,
        body: BlockId,
    },

    Closure {
        callee: FunctionId,
        captures: Vec<ValueId>,
    },

    Indirect {
        callee: ValueId,
    },
}
