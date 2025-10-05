use crate::{prelude::*, store::SymbolId, ty::FunctionAttribute};
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

#[derive(Debug, Serialize, Deserialize)]
pub struct Variable {
    pub name: IString,
    pub ty: TypeId,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Parameter {
    pub name: IString,
    pub ty: TypeId,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Symbol {
    GlobalVariable(Variable),
    LocalVariable(Variable),
    Parameter(Parameter),
    Function(Function),

    Unresolved { name: IString },
}

impl Symbol {
    pub fn dump_nocycle(&self, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Symbol::GlobalVariable(v) => write!(o, "sym global {}", v.name),
            Symbol::LocalVariable(v) => write!(o, "sym local {}", v.name),
            Symbol::Parameter(p) => write!(o, "sym param {}", p.name),
            Symbol::Function(f) => match f {
                Function::External { name, .. } => write!(o, "sym fn {}", name),
                Function::Static { name, .. } => write!(o, "sym fn {}", name),
                Function::Closure {
                    closure_unique_id, ..
                } => {
                    write!(o, "sym fn #{}", closure_unique_id)
                }
            },
            Symbol::Unresolved { name } => write!(o, "sym nores {}", name),
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub enum Place {
    Symbol { symbol: SymbolId },
    FieldAccess { place: PlaceId, field: IString },
    ArrayIndex { place: PlaceId, index: ValueId },
    Assign { place: PlaceId, value: ValueId },
    Deref { place: PlaceId },
}
