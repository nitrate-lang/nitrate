use nitrate_hir::prelude::*;
use std::collections::HashSet;

pub struct DumpContext<'a> {
    pub indent_str: &'a str,
    pub(crate) indent: usize,
    pub(crate) visited: HashSet<TypeId>,
}

impl<'a> Default for DumpContext<'a> {
    fn default() -> Self {
        Self::new()
    }
}

impl<'a> DumpContext<'a> {
    #[must_use]
    pub fn new() -> DumpContext<'a> {
        DumpContext {
            indent: 0,
            indent_str: "  ",
            visited: HashSet::new(),
        }
    }
}

pub trait Dump {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error>;

    fn to_string(&self) -> String {
        let mut ctx = DumpContext::default();
        let mut output = String::new();
        self.dump(&mut ctx, &mut output).unwrap();
        output
    }
}

pub(crate) fn write_indent(
    ctx: &DumpContext,
    o: &mut dyn std::fmt::Write,
) -> Result<(), std::fmt::Error> {
    for _ in 0..ctx.indent {
        write!(o, "{}", ctx.indent_str)?;
    }
    Ok(())
}
