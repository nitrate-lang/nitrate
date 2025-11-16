use nitrate_hir::prelude::*;
use std::collections::HashSet;

pub struct DumpContext<'a> {
    pub store: &'a Store,
    pub indent_str: &'a str,
    pub(crate) indent: usize,
    pub(crate) visited: HashSet<TypeId>,
}

impl<'a> DumpContext<'a> {
    pub fn new(store: &'a Store) -> DumpContext<'a> {
        DumpContext {
            store,
            indent: 0,
            indent_str: "  ",
            visited: HashSet::new(),
        }
    }
}

pub trait Dump {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::io::Write)
    -> Result<(), std::io::Error>;
}

pub(crate) fn write_indent(
    ctx: &DumpContext,
    o: &mut dyn std::io::Write,
) -> Result<(), std::io::Error> {
    for _ in 0..ctx.indent {
        write!(o, "{}", ctx.indent_str)?;
    }
    Ok(())
}
