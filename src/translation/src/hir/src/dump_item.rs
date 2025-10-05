use crate::{Dump, DumpContext, hir::Item};

impl Dump for Item {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            _ => Ok(()),
        }
    }
}
