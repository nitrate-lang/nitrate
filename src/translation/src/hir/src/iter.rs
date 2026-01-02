use crate::prelude::*;

pub struct FunctionTypeIter<'a> {
    pub(crate) node: &'a FunctionType,
}

impl FunctionType {
    #[must_use]
    pub fn iter(&self) -> FunctionTypeIter<'_> {
        FunctionTypeIter { node: self }
    }
}

pub struct TypeIter<'a> {
    pub(crate) node: &'a Type,
}

impl Type {
    #[must_use]
    pub fn iter(&self) -> TypeIter<'_> {
        TypeIter { node: self }
    }
}

pub struct BlockIter<'a> {
    pub(crate) node: &'a Block,
}

impl Block {
    #[must_use]
    pub fn iter(&self) -> BlockIter<'_> {
        BlockIter { node: self }
    }
}

pub struct ValueIter<'a> {
    pub(crate) node: &'a Value,
}

impl Value {
    #[must_use]
    pub fn iter(&self) -> ValueIter<'_> {
        ValueIter { node: self }
    }
}

pub struct GlobalVariableIter<'a> {
    pub(crate) node: &'a GlobalVariable,
}

impl GlobalVariable {
    #[must_use]
    pub fn iter(&self) -> GlobalVariableIter<'_> {
        GlobalVariableIter { node: self }
    }
}

pub struct ModuleIter<'a> {
    pub(crate) node: &'a Module,
}

impl Module {
    #[must_use]
    pub fn iter(&self) -> ModuleIter<'_> {
        ModuleIter { node: self }
    }
}

pub struct TypeAliasDefIter<'a> {
    pub(crate) node: &'a TypeAliasDef,
}

impl TypeAliasDef {
    #[must_use]
    pub fn iter(&self) -> TypeAliasDefIter<'_> {
        TypeAliasDefIter { node: self }
    }
}

pub struct StructDefIter<'a> {
    pub(crate) node: &'a StructDef,
}

impl StructDef {
    #[must_use]
    pub fn iter(&self) -> StructDefIter<'_> {
        StructDefIter { node: self }
    }
}

pub struct EnumDefIter<'a> {
    pub(crate) node: &'a EnumDef,
}

impl EnumDef {
    #[must_use]
    pub fn iter(&self) -> EnumDefIter<'_> {
        EnumDefIter { node: self }
    }
}

pub struct FunctionIter<'a> {
    pub(crate) node: &'a Function,
}

impl Function {
    #[must_use]
    pub fn iter(&self) -> FunctionIter<'_> {
        FunctionIter { node: self }
    }
}

pub struct TraitIter<'a> {
    pub(crate) node: &'a Trait,
}

impl Trait {
    #[must_use]
    pub fn iter(&self) -> TraitIter<'_> {
        TraitIter { node: self }
    }
}
