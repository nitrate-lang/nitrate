use crate::prelude::*;

pub struct BlockIterMut<'a> {
    pub(crate) node: &'a mut Block,
}

impl Block {
    pub fn iter_mut(&mut self) -> BlockIterMut<'_> {
        BlockIterMut { node: self }
    }
}

pub struct ValueIterMut<'a> {
    pub(crate) node: &'a mut Value,
}

impl Value {
    pub fn iter_mut(&mut self) -> ValueIterMut<'_> {
        ValueIterMut { node: self }
    }
}

pub struct GlobalVariableIterMut<'a> {
    pub(crate) node: &'a mut GlobalVariable,
}

impl GlobalVariable {
    pub fn iter_mut(&mut self) -> GlobalVariableIterMut<'_> {
        GlobalVariableIterMut { node: self }
    }
}

pub struct ModuleIterMut<'a> {
    pub(crate) node: &'a mut Module,
}

impl Module {
    pub fn iter_mut(&mut self) -> ModuleIterMut<'_> {
        ModuleIterMut { node: self }
    }
}

pub struct StructDefIterMut<'a> {
    pub(crate) node: &'a mut StructDef,
}

impl StructDef {
    pub fn iter_mut(&mut self) -> StructDefIterMut<'_> {
        StructDefIterMut { node: self }
    }
}

pub struct EnumDefIterMut<'a> {
    pub(crate) node: &'a mut EnumDef,
}

impl EnumDef {
    pub fn iter_mut(&mut self) -> EnumDefIterMut<'_> {
        EnumDefIterMut { node: self }
    }
}

pub struct FunctionIterMut<'a> {
    pub(crate) node: &'a mut Function,
}

impl Function {
    pub fn iter_mut(&mut self) -> FunctionIterMut<'_> {
        FunctionIterMut { node: self }
    }
}
