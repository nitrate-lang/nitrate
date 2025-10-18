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

impl Dump for FunctionId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        this.visibility.dump(ctx, o)?;
        write!(o, " sym[{}] static fn ", self.as_usize())?;

        if !this.attributes.is_empty() {
            write!(o, "[")?;
            for (i, attr) in this.attributes.iter().enumerate() {
                if i != 0 {
                    write!(o, ", ")?;
                }

                attr.dump(ctx, o)?;
            }
            write!(o, "] ")?;
        }

        write!(o, "{}(", this.name)?;

        for (i, param) in this.parameters.iter().enumerate() {
            let param = &ctx.store[param].borrow();

            if i != 0 {
                write!(o, ", ")?;
            }

            write!(o, "{}: ", param.name)?;
            ctx.store[&param.ty].dump(ctx, o)?;
            if let Some(default_value) = &param.default_value {
                write!(o, " = ")?;
                ctx.store[default_value].borrow().dump(ctx, o)?;
            }
        }

        write!(o, ") -> ")?;
        ctx.store[&this.return_type].dump(ctx, o)?;

        write!(o, " ")?;
        ctx.store[&this.body].borrow().dump(ctx, o)
    }
}

impl SymbolId {
    pub fn dump_nocycle(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match &*ctx.store[self].borrow() {
            Symbol::GlobalVariable(gv) => {
                write!(
                    o,
                    "sym[{}] global `{}`",
                    gv.as_usize(),
                    ctx.store[gv].borrow().name
                )
            }

            Symbol::LocalVariable(lv) => write!(
                o,
                "sym[{}] local `{}`",
                lv.as_usize(),
                ctx.store[lv].borrow().name
            ),

            Symbol::Trait(tr) => write!(
                o,
                "sym[{}] trait `{}`",
                tr.as_usize(),
                ctx.store[tr].borrow().name
            ),

            Symbol::Parameter(fp) => write!(
                o,
                "sym[{}] param `{}`",
                fp.as_usize(),
                ctx.store[fp].borrow().name
            ),

            Symbol::Function(f) => write!(
                o,
                "sym[{}] fn `{}`",
                f.as_usize(),
                ctx.store[f].borrow().name
            ),
        }
    }
}

impl Dump for GlobalVariableId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        this.visibility.dump(ctx, o)?;
        write!(o, " sym[{}] global ", self.as_usize())?;
        if this.is_mutable {
            write!(o, "mut ")?;
        }
        write!(o, "`{}`: ", this.name)?;
        ctx.store[&this.ty].dump(ctx, o)?;
        write!(o, " = ")?;
        ctx.store[&this.initializer].borrow().dump(ctx, o)?;
        write!(o, ";")
    }
}

impl Dump for LocalVariableId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        write!(o, "sym[{}] local `{}`: ", self.as_usize(), this.name)?;
        ctx.store[&this.ty].dump(ctx, o)?;
        write!(o, " = ")?;
        ctx.store[&this.initializer].borrow().dump(ctx, o)?;
        write!(o, ";")
    }
}

impl Dump for TraitId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        this.visibility.dump(ctx, o)?;
        write!(o, " sym[{}] trait `{}` {{\n", self.as_usize(), this.name)?;

        ctx.indent += 1;

        for method in &this.methods {
            ctx.indent += 1;
            self.write_indent(ctx, o)?;
            method.dump(ctx, o)?;
            write!(o, "\n")?;
            ctx.indent -= 1;
        }

        ctx.indent -= 1;
        self.write_indent(ctx, o)?;
        write!(o, "}}")
    }
}

impl Dump for ParameterId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        write!(o, "sym[{}] param `{}`: ", self.as_usize(), this.name)?;
        ctx.store[&this.ty].dump(ctx, o)?;
        if let Some(default_value) = &this.default_value {
            write!(o, " = ")?;
            ctx.store[default_value].borrow().dump(ctx, o)?;
        }
        Ok(())
    }
}

impl Dump for SymbolId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match &*ctx.store[self].borrow() {
            Symbol::Parameter(fp) => fp.dump(ctx, o),
            Symbol::Function(f) => f.dump(ctx, o),
            Symbol::GlobalVariable(gv) => gv.dump(ctx, o),
            Symbol::LocalVariable(lv) => lv.dump(ctx, o),
            Symbol::Trait(tr) => tr.dump(ctx, o),
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

        if let Some(name) = &self.name {
            write!(o, "`{}` ", name)?;
        }

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
            Item::Function(f) => f.dump(ctx, o),
            Item::GlobalVariable(gv) => gv.dump(ctx, o),
            Item::Module(m) => ctx.store[m].borrow().dump(ctx, o),
        }
    }
}
