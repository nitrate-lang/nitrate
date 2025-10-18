use crate::Store;

pub struct DumpContext<'a> {
    pub store: &'a Store,
    pub indent_str: &'a str,

    pub(crate) indent: usize,
}

impl<'a> DumpContext<'a> {
    pub fn new(store: &'a Store) -> DumpContext<'a> {
        DumpContext {
            store,
            indent: 0,
            indent_str: "  ",
        }
    }
}

pub trait Dump {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error>;

    fn dump_to_string(&self, ctx: &mut DumpContext) -> String {
        let mut s = String::new();
        self.dump(ctx, &mut s).expect("Failed to dump");
        s
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
