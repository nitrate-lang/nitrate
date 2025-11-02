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

pub struct GlobalVariableIter<'a> {
    pub(crate) node: &'a GlobalVariable,
}

impl GlobalVariable {
    pub fn iter(&self) -> GlobalVariableIter<'_> {
        GlobalVariableIter { node: self }
    }
}

pub struct ModuleIter<'a> {
    pub(crate) node: &'a Module,
}

impl Module {
    pub fn iter(&self) -> ModuleIter<'_> {
        ModuleIter { node: self }
    }
}

pub struct TypeAliasDefIter<'a> {
    pub(crate) node: &'a TypeAliasDef,
}

impl TypeAliasDef {
    pub fn iter(&self) -> TypeAliasDefIter<'_> {
        TypeAliasDefIter { node: self }
    }
}

pub struct StructDefIter<'a> {
    pub(crate) node: &'a StructDef,
}

impl StructDef {
    pub fn iter(&self) -> StructDefIter<'_> {
        StructDefIter { node: self }
    }
}

pub struct EnumDefIter<'a> {
    pub(crate) node: &'a EnumDef,
}

impl EnumDef {
    pub fn iter(&self) -> EnumDefIter<'_> {
        EnumDefIter { node: self }
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
