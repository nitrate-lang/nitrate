use crate::prelude::*;
use std::ops::ControlFlow;

impl BlockIterMut<'_> {
    pub fn for_each_value_mut(&mut self, store: &Store, f: &mut dyn FnMut(&mut Value)) {
        let _ = self.try_for_each_mut(store, &mut |value: &mut Value| -> ControlFlow<()> {
            f(value);
            ControlFlow::Continue(())
        });
    }
}

impl ValueIterMut<'_> {
    pub fn for_each_value_mut(&mut self, store: &Store, f: &mut dyn FnMut(&mut Value)) {
        let _ = self.try_for_each_mut(store, &mut |value: &mut Value| -> ControlFlow<()> {
            f(value);
            ControlFlow::Continue(())
        });
    }
}

impl GlobalVariableIterMut<'_> {
    pub fn for_each_value_mut(&mut self, store: &Store, f: &mut dyn FnMut(&mut Value)) {
        let _ = self.try_for_each_mut(store, &mut |value: &mut Value| -> ControlFlow<()> {
            f(value);
            ControlFlow::Continue(())
        });
    }
}

impl ModuleIterMut<'_> {
    pub fn for_each_value_mut(&mut self, store: &Store, f: &mut dyn FnMut(&mut Value)) {
        let _ = self.try_for_each_mut(store, &mut |value: &mut Value| -> ControlFlow<()> {
            f(value);
            ControlFlow::Continue(())
        });
    }
}

impl StructDefIterMut<'_> {
    pub fn for_each_value_mut(&mut self, store: &Store, f: &mut dyn FnMut(&mut Value)) {
        let _ = self.try_for_each_mut(store, &mut |value: &mut Value| -> ControlFlow<()> {
            f(value);
            ControlFlow::Continue(())
        });
    }
}

impl EnumDefIterMut<'_> {
    pub fn for_each_value_mut(&mut self, store: &Store, f: &mut dyn FnMut(&mut Value)) {
        let _ = self.try_for_each_mut(store, &mut |value: &mut Value| -> ControlFlow<()> {
            f(value);
            ControlFlow::Continue(())
        });
    }
}

impl FunctionIterMut<'_> {
    pub fn for_each_value_mut(&mut self, store: &Store, f: &mut dyn FnMut(&mut Value)) {
        let _ = self.try_for_each_mut(store, &mut |value: &mut Value| -> ControlFlow<()> {
            f(value);
            ControlFlow::Continue(())
        });
    }
}
