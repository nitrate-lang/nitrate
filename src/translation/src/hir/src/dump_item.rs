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

impl Dump for ExternalFunction {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        self.visibility.dump(ctx, o)?;
        write!(o, " sym extern fn ")?;

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

        for (i, (param_name, param_type)) in self.parameters.iter().enumerate() {
            if i != 0 {
                write!(o, ", ")?;
            }

            write!(o, "{}: ", param_name)?;
            ctx.store[param_type].dump(ctx, o)?;
        }

        write!(o, ") -> ")?;
        ctx.store[&self.return_type].dump(ctx, o)?;
        write!(o, ";")
    }
}

impl Dump for StaticFunction {
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

        for (i, (param_name, param_type)) in self.parameters.iter().enumerate() {
            if i != 0 {
                write!(o, ", ")?;
            }

            write!(o, "{}: ", param_name)?;
            ctx.store[param_type].dump(ctx, o)?;
        }

        write!(o, ") -> ")?;
        ctx.store[&self.return_type].dump(ctx, o)?;

        write!(o, " ")?;
        ctx.store[&self.body].borrow().dump(ctx, o)
    }
}

impl Dump for ClosureFunction {
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

            ctx.store[capture].borrow().dump_nocycle(o)?;
        }
        write!(o, "] ")?;

        self.callee.dump(ctx, o)
    }
}

impl Dump for Function {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            Function::External(efn) => efn.dump(ctx, o),
            Function::Static(sfn) => sfn.dump(ctx, o),
            Function::Closure(cfn) => cfn.dump(ctx, o),
        }
    }
}

impl Symbol {
    pub fn dump_nocycle(&self, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Symbol::Unresolved { name } => write!(o, "sym nolink `{}`", name),
            Symbol::GlobalVariable(gv) => write!(o, "sym global `{}`", gv.name.0),
            Symbol::LocalVariable(lv) => write!(o, "sym local `{}`", lv.name.0),
            Symbol::Parameter(fp) => write!(o, "sym param `{}`", fp.name.0),
            Symbol::Function(f) => match f {
                Function::External(efn) => write!(o, "sym fn `{}`", efn.name.0),
                Function::Static(sfn) => write!(o, "sym fn `{}`", sfn.name.0),
                Function::Closure(cfn) => {
                    write!(o, "sym fn #{}", cfn.closure_unique_id)
                }
            },
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
            Symbol::GlobalVariable(gv) => gv.dump(ctx, o),
            Symbol::LocalVariable(lv) => lv.dump(ctx, o),
            Symbol::Parameter(fp) => fp.dump(ctx, o),
            Symbol::Function(f) => f.dump(ctx, o),
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
                ctx.store[item].borrow().dump(ctx, o)?;
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
            Item::Module(module) => module.dump(ctx, o),
            Item::GlobalVariable(gv) => gv.dump(ctx, o),
            Item::ExternalFunction(efn) => efn.dump(ctx, o),
            Item::StaticFunction(sfn) => sfn.dump(ctx, o),
        }
    }
}
