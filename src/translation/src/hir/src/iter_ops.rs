use crate::prelude::*;
use std::ops::ControlFlow;

impl StructTypeIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
        );
    }

    pub fn for_each_value(&self, store: &Store, f: &mut dyn FnMut(&Value)) {
        let _ = self.try_for_each(
            store,
            &mut |value: &Value| -> ControlFlow<()> {
                f(value);
                ControlFlow::Continue(())
            },
            &mut |_| ControlFlow::Continue(()),
        );
    }
}

impl EnumTypeIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
        );
    }

    pub fn for_each_value(&self, store: &Store, f: &mut dyn FnMut(&Value)) {
        let _ = self.try_for_each(
            store,
            &mut |value: &Value| -> ControlFlow<()> {
                f(value);
                ControlFlow::Continue(())
            },
            &mut |_| ControlFlow::Continue(()),
        );
    }
}

impl FunctionTypeIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
        );
    }

    pub fn for_each_value(&self, store: &Store, f: &mut dyn FnMut(&Value)) {
        let _ = self.try_for_each(
            store,
            &mut |value: &Value| -> ControlFlow<()> {
                f(value);
                ControlFlow::Continue(())
            },
            &mut |_| ControlFlow::Continue(()),
        );
    }
}

impl TypeIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
        );
    }

    pub fn for_each_value(&self, store: &Store, f: &mut dyn FnMut(&Value)) {
        let _ = self.try_for_each(
            store,
            &mut |value: &Value| -> ControlFlow<()> {
                f(value);
                ControlFlow::Continue(())
            },
            &mut |_| ControlFlow::Continue(()),
        );
    }
}

impl BlockIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
        );
    }

    pub fn for_each_value(&self, store: &Store, f: &mut dyn FnMut(&Value)) {
        let _ = self.try_for_each(
            store,
            &mut |value: &Value| -> ControlFlow<()> {
                f(value);
                ControlFlow::Continue(())
            },
            &mut |_| ControlFlow::Continue(()),
        );
    }
}

impl ValueIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
        );
    }

    pub fn for_each_value(&self, store: &Store, f: &mut dyn FnMut(&Value)) {
        let _ = self.try_for_each(
            store,
            &mut |value: &Value| -> ControlFlow<()> {
                f(value);
                ControlFlow::Continue(())
            },
            &mut |_| ControlFlow::Continue(()),
        );
    }
}

impl FunctionIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
        );
    }

    pub fn for_each_value(&self, store: &Store, f: &mut dyn FnMut(&Value)) {
        let _ = self.try_for_each(
            store,
            &mut |value: &Value| -> ControlFlow<()> {
                f(value);
                ControlFlow::Continue(())
            },
            &mut |_| ControlFlow::Continue(()),
        );
    }
}
