use crate::{
    hir::{Block, Function, Value},
    ty::Type,
};

pub struct TypeIter<'a> {
    pub(crate) node: &'a Type,
}

pub struct BlockIter<'a> {
    pub(crate) node: &'a Block,
}

pub struct ValueIter<'a> {
    pub(crate) node: &'a Value,
}
pub struct FunctionIter<'a> {
    pub(crate) node: &'a Function,
}

impl Type {
    pub fn iter(&self) -> TypeIter<'_> {
        TypeIter { node: self }
    }
}

impl Block {
    pub fn iter(&self) -> BlockIter<'_> {
        BlockIter { node: self }
    }
}

impl Value {
    pub fn iter(&self) -> ValueIter<'_> {
        ValueIter { node: self }
    }
}

impl Function {
    pub fn iter(&self) -> FunctionIter<'_> {
        FunctionIter { node: self }
    }
}
