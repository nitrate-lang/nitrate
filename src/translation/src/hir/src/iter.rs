use crate::prelude::*;

pub struct StructTypeIter<'a> {
    pub(crate) node: &'a StructType,
}

impl StructType {
    pub fn iter(&self) -> StructTypeIter<'_> {
        StructTypeIter { node: self }
    }
}

pub struct EnumTypeIter<'a> {
    pub(crate) node: &'a EnumType,
}

impl EnumType {
    pub fn iter(&self) -> EnumTypeIter<'_> {
        EnumTypeIter { node: self }
    }
}

pub struct FunctionTypeIter<'a> {
    pub(crate) node: &'a FunctionType,
}

impl FunctionType {
    pub fn iter(&self) -> FunctionTypeIter<'_> {
        FunctionTypeIter { node: self }
    }
}

pub struct TypeIter<'a> {
    pub(crate) node: &'a Type,
}

impl Type {
    pub fn iter(&self) -> TypeIter<'_> {
        TypeIter { node: self }
    }
}

pub struct BlockIter<'a> {
    pub(crate) node: &'a Block,
}

impl Block {
    pub fn iter(&self) -> BlockIter<'_> {
        BlockIter { node: self }
    }
}

pub struct ValueIter<'a> {
    pub(crate) node: &'a Value,
}

impl Value {
    pub fn iter(&self) -> ValueIter<'_> {
        ValueIter { node: self }
    }
}

pub struct FunctionIter<'a> {
    pub(crate) node: &'a Function,
}

impl Function {
    pub fn iter(&self) -> FunctionIter<'_> {
        FunctionIter { node: self }
    }
}
