use crate::prelude::*;

impl Dump for Visibility {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Visibility::Sec => write!(o, "sec"),
            Visibility::Pro => write!(o, "pro"),
            Visibility::Pub => write!(o, "pub"),
        }
    }
}

impl Dump for Function {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        self.visibility.dump(ctx, o)?;
        write!(o, " sym static fn ")?;

        if !self.attributes.is_empty() {
            write!(o, "[")?;
            for (i, attr) in self.attributes.iter().enumerate() {
                if i != 0 {
                    write!(o, ", ")?;
                }

                attr.dump(ctx, o)?;
            }
            write!(o, "] ")?;
        }

        write!(o, "{}(", self.name.0)?;

        for (i, param) in self.parameters.iter().enumerate() {
            let param = &ctx.store[param].borrow();

            if i != 0 {
                write!(o, ", ")?;
            }

            write!(o, "{}: ", param.name.0)?;
            ctx.store[&param.ty].dump(ctx, o)?;
            if let Some(default_value) = &param.default_value {
                write!(o, " = ")?;
                ctx.store[default_value].borrow().dump(ctx, o)?;
            }
        }

        write!(o, ") -> ")?;
        ctx.store[&self.return_type].dump(ctx, o)?;

        write!(o, " ")?;
        ctx.store[&self.body].borrow().dump(ctx, o)
    }
}

impl Dump for Closure {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        write!(o, "sym fn #{}", self.closure_unique_id)?;

        write!(o, " [")?;
        for (i, capture) in self.captures.iter().enumerate() {
            if i != 0 {
                write!(o, ", ")?;
            }

            ctx.store[capture].borrow().dump_nocycle(ctx, o)?;
        }
        write!(o, "] ")?;

        ctx.store[&self.callee].borrow().dump(ctx, o)
    }
}

impl Symbol {
    pub fn dump_nocycle(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Symbol::Unresolved { name } => write!(o, "sym nolink `{}`", name),
            Symbol::GlobalVariable(gv) => {
                write!(o, "sym global `{}`", ctx.store[gv].borrow().name.0)
            }
            Symbol::LocalVariable(lv) => write!(o, "sym local `{}`", ctx.store[lv].borrow().name.0),
            Symbol::Trait(tr) => write!(o, "sym trait `{}`", ctx.store[tr].borrow().name.0),
            Symbol::Parameter(fp) => write!(o, "sym param `{}`", ctx.store[fp].borrow().name.0),
            Symbol::Function(f) => write!(o, "sym fn `{}`", ctx.store[f].borrow().name.0),
        }
    }
}

impl Dump for GlobalVariable {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        self.visibility.dump(ctx, o)?;
        write!(o, " sym global ",)?;
        if self.is_mutable {
            write!(o, "mut ")?;
        }
        write!(o, "`{}`: ", self.name.0)?;
        ctx.store[&self.ty].dump(ctx, o)?;
        write!(o, " = ")?;
        ctx.store[&self.initializer].borrow().dump(ctx, o)?;
        write!(o, ";")
    }
}

impl Dump for LocalVariable {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        write!(o, "sym local `{}`: ", self.name.0)?;
        ctx.store[&self.ty].dump(ctx, o)?;
        write!(o, " = ")?;
        ctx.store[&self.initializer].borrow().dump(ctx, o)?;
        write!(o, ";")
    }
}

impl Dump for Trait {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        self.visibility.dump(ctx, o)?;
        write!(o, " sym trait `{}` {{\n", self.name.0)?;

        ctx.indent += 1;

        for method in &self.methods {
            ctx.indent += 1;
            self.write_indent(ctx, o)?;
            ctx.store[method].borrow().dump(ctx, o)?;
            write!(o, "\n")?;
            ctx.indent -= 1;
        }

        ctx.indent -= 1;
        self.write_indent(ctx, o)?;
        write!(o, "}}")
    }
}

impl Dump for Parameter {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        write!(o, "sym param `{}`: ", self.name.0)?;
        ctx.store[&self.ty].dump(ctx, o)?;
        if let Some(default_value) = &self.default_value {
            write!(o, " = ")?;
            ctx.store[default_value].borrow().dump(ctx, o)?;
        }
        Ok(())
    }
}

impl Dump for Symbol {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Symbol::Unresolved { name } => write!(o, "sym nolink `{}`", name),
            Symbol::Parameter(fp) => ctx.store[fp].borrow().dump(ctx, o),
            Symbol::Function(f) => ctx.store[f].borrow().dump(ctx, o),
            Symbol::GlobalVariable(gv) => ctx.store[gv].borrow().dump(ctx, o),
            Symbol::LocalVariable(lv) => ctx.store[lv].borrow().dump(ctx, o),
            Symbol::Trait(tr) => ctx.store[tr].borrow().dump(ctx, o),
        }
    }
}

impl Dump for ModuleAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        Ok(())
    }
}

impl Dump for Module {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        self.visibility.dump(ctx, o)?;
        write!(o, " mod ")?;

        if !self.attributes.is_empty() {
            write!(o, "[")?;
            for (i, attr) in self.attributes.iter().enumerate() {
                if i != 0 {
                    write!(o, ", ")?;
                }

                attr.dump(ctx, o)?;
            }
            write!(o, "] ")?;
        }

        write!(o, "`{}` ", self.name.0)?;

        if self.items.is_empty() {
            write!(o, "{{}}")
        } else {
            write!(o, "{{\n")?;

            for item in &self.items {
                ctx.indent += 1;

                self.write_indent(ctx, o)?;
                item.dump(ctx, o)?;
                write!(o, "\n")?;

                ctx.indent -= 1;
            }

            self.write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for Item {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Item::Module(module) => ctx.store[module].borrow().dump(ctx, o),
            Item::GlobalVariable(gv) => ctx.store[gv].borrow().dump(ctx, o),
            Item::Function(f) => ctx.store[f].borrow().dump(ctx, o),
            Item::Trait(tr) => ctx.store[tr].borrow().dump(ctx, o),
        }
    }
}
