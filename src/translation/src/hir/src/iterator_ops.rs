use crate::prelude::*;
use std::{collections::HashSet, ops::ControlFlow};

impl StructTypeIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
        );
    }

    pub fn all(&self, store: &Store, f: &mut dyn FnMut(&Type) -> bool) -> bool {
        let mut result = true;

        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |value: &Type| -> ControlFlow<()> {
                if f(value) {
                    ControlFlow::Continue(())
                } else {
                    result = false;
                    ControlFlow::Break(())
                }
            },
            &mut HashSet::new(),
        );

        result
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
        );
    }
}

impl GlobalVariableIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
            &mut HashSet::new(),
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
            &mut HashSet::new(),
        );
    }
}

impl ModuleIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
            &mut HashSet::new(),
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
            &mut HashSet::new(),
        );
    }
}

impl TypeAliasDefIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
            &mut HashSet::new(),
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
            &mut HashSet::new(),
        );
    }
}

impl StructDefIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
            &mut HashSet::new(),
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
            &mut HashSet::new(),
        );
    }
}

impl EnumDefIter<'_> {
    pub fn for_each_type(&self, store: &Store, f: &mut dyn FnMut(&Type)) {
        let _ = self.try_for_each(
            store,
            &mut |_| ControlFlow::Continue(()),
            &mut |ty: &Type| -> ControlFlow<()> {
                f(ty);
                ControlFlow::Continue(())
            },
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
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
            &mut HashSet::new(),
        );
    }
}
