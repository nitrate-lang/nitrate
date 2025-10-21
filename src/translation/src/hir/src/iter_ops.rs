use crate::prelude::*;
use std::ops::ControlFlow;

impl TypeIter<'_> {
    pub fn for_each(&self, store: &Store, vcb: &mut dyn FnMut(&Value), tcb: &mut dyn FnMut(&Type)) {
        let mut vcb_wrapper = move |value: &Value| -> ControlFlow<()> {
            vcb(value);
            ControlFlow::Continue(())
        };

        let mut tcb_wrapper = move |ty: &Type| -> ControlFlow<()> {
            tcb(ty);
            ControlFlow::Continue(())
        };

        self.try_for_each(store, &mut vcb_wrapper, &mut tcb_wrapper)
            .is_continue(); // ignore cf signal
    }
}
