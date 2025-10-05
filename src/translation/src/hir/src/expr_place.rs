use crate::prelude::{hir::*, *};
use interned_string::IString;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Function {
    External {
        attributes: Vec<FunctionAttribute>,
        name: IString,
        parameters: Vec<TypeId>,
        return_type: TypeId,
    },

    Static {
        attributes: Vec<FunctionAttribute>,
        name: IString,
        parameters: Vec<TypeId>,
        return_type: TypeId,
        body: BlockId,
    },

    Closure {
        closure_unique_id: u64,
        callee: Box<Function>,
        captures: Vec<ValueId>,
    },
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GlobalVariable {
    pub name: IString,
    pub ty: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LocalVariable {
    pub name: IString,
    pub ty: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Parameter {
    pub name: IString,
    pub ty: TypeId,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Symbol {
    GlobalVariable(GlobalVariable),
    LocalVariable(LocalVariable),
    Parameter(Parameter),
    Function(Function),

    Unresolved { name: IString },
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Place {
    Symbol { symbol: SymbolId },
    FieldAccess { place: PlaceId, field: IString },
    ArrayIndex { place: PlaceId, index: ValueId },
    Assign { place: PlaceId, value: ValueId },
    Deref { place: PlaceId },
}
