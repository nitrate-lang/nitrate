use crate::Module;

pub trait HirPass {
    fn run_on(&mut self, module: &Module);
}

pub trait HirPassMut {
    fn run_on_mut(&mut self, module: &mut Module);
}
