use std::collections::BTreeSet;

use crate::{dump::write_indent, prelude::*};

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

pub(crate) fn dump_attributes<T: Dump>(
    attributes: &BTreeSet<T>,
    ctx: &mut DumpContext,
    o: &mut dyn std::fmt::Write,
) -> Result<(), std::fmt::Error> {
    if attributes.is_empty() {
        return Ok(());
    }

    write!(o, "[\n")?;

    for attr in attributes {
        ctx.indent += 1;

        write_indent(ctx, o)?;
        attr.dump(ctx, o)?;
        write!(o, ",\n")?;

        ctx.indent -= 1;
    }

    write_indent(ctx, o)?;
    write!(o, "] ")
}

impl Dump for GlobalVariableAttribute {
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

impl Dump for GlobalVariableId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "static::{}::`{}`", self.as_usize(), this.name)?;

        if this.is_mutable {
            write!(o, " mut")?;
        }

        dump_attributes(&this.attributes, ctx, o)?;

        write!(o, ": ")?;
        ctx.store[&this.ty].dump(ctx, o)?;

        write!(o, " = ")?;
        ctx.store[&this.initializer].borrow().dump(ctx, o)?;

        write!(o, ";")
    }
}

impl Dump for LocalVariableAttribute {
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

impl Dump for LocalVariableId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        match this.kind {
            LocalVariableKind::Stack => write!(o, "let::{}::`{}`", self.as_usize(), this.name)?,
            LocalVariableKind::Dynamic => write!(o, "var::{}::`{}`", self.as_usize(), this.name)?,
            LocalVariableKind::Static => write!(o, "static::{}::`{}`", self.as_usize(), this.name)?,
        }

        if this.is_mutable {
            write!(o, " mut")?;
        }

        dump_attributes(&this.attributes, ctx, o)?;

        write!(o, ": ")?;
        ctx.store[&this.ty].dump(ctx, o)?;

        if let Some(initializer) = &this.initializer {
            write!(o, " = ")?;
            ctx.store[initializer].borrow().dump(ctx, o)?;
        }

        write!(o, ";")
    }
}

impl Dump for ParameterAttribute {
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

impl Dump for ParameterId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        write!(o, "param::{}::`{}`", self.as_usize(), this.name)?;

        if this.is_mutable {
            write!(o, " mut")?;
        }

        dump_attributes(&this.attributes, ctx, o)?;

        write!(o, ": ")?;
        ctx.store[&this.ty].dump(ctx, o)?;

        if let Some(default_value) = &this.default_value {
            write!(o, " = ")?;
            ctx.store[default_value].borrow().dump(ctx, o)?;
        }

        Ok(())
    }
}

impl Dump for FunctionId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "fn::{}::`{}`", self.as_usize(), this.name)?;

        dump_attributes(&this.attributes, ctx, o)?;

        if this.params.is_empty() {
            write!(o, "()")?;
        } else {
            write!(o, "(\n")?;

            for param in this.params.iter() {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                param.dump(ctx, o)?;
                write!(o, ",\n")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, ")")?;
        }

        write!(o, " -> ")?;
        ctx.store[&this.return_type].dump(ctx, o)?;

        if let Some(body) = &this.body {
            write!(o, " ")?;
            ctx.store[body].borrow().dump(ctx, o)
        } else {
            write!(o, ";")
        }
    }
}

impl Dump for TraitId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "trait::{}::`{}`", self.as_usize(), this.name)?;

        if this.methods.is_empty() {
            write!(o, " {{}}")
        } else {
            write!(o, " {{\n")?;

            for method in &this.methods {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                method.dump(ctx, o)?;
                write!(o, "\n")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for ModuleAttribute {
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

impl Dump for ModuleId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        if let Some(name) = &this.name {
            write!(o, "mod::{}::`{}` ", self.as_usize(), name)?;
        } else {
            write!(o, "mod::{} ", self.as_usize())?;
        }

        dump_attributes(&this.attributes, ctx, o)?;

        if this.items.is_empty() {
            write!(o, "{{}}")
        } else {
            write!(o, "{{\n")?;

            for (i, item) in this.items.iter().enumerate() {
                if i != 0 {
                    write!(o, "\n")?;
                }

                ctx.indent += 1;

                write_indent(ctx, o)?;
                item.dump(ctx, o)?;
                write!(o, "\n")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for Module {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        if let Some(name) = &self.name {
            write!(o, "mod::`{}` ", name)?;
        } else {
            write!(o, "mod ")?;
        }

        dump_attributes(&self.attributes, ctx, o)?;

        if self.items.is_empty() {
            write!(o, "{{}}")
        } else {
            write!(o, "{{\n")?;

            for (i, item) in self.items.iter().enumerate() {
                if i != 0 {
                    write!(o, "\n")?;
                }

                ctx.indent += 1;

                write_indent(ctx, o)?;
                item.dump(ctx, o)?;
                write!(o, "\n")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for TypeAliasDefId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "typealias::{}::`{}`", self.as_usize(), this.name)?;

        write!(o, "= ")?;
        ctx.store[&this.type_id].dump(ctx, o)?;

        write!(o, ";")
    }
}

impl Dump for StructDefId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "struct::{}::`{}`", self.as_usize(), this.name)?;

        ctx.store[&this.struct_id].dump(ctx, o)?;

        write!(o, ";")
    }
}

impl Dump for EnumDefId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        let this = ctx.store[self].borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "enum::{}::`{}`", self.as_usize(), this.name)?;

        ctx.store[&this.enum_id].dump(ctx, o)?;

        write!(o, ";")
    }
}

impl SymbolId {
    pub fn dump_nocycle(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            SymbolId::Function(func_id) => {
                let symbol = ctx.store[func_id].borrow();
                write!(o, "fn::{}::`{}`", func_id.as_usize(), symbol.name)
            }

            SymbolId::GlobalVariable(global_id) => {
                let symbol = ctx.store[global_id].borrow();
                write!(o, "global::{}::`{}`", global_id.as_usize(), symbol.name)
            }

            SymbolId::LocalVariable(local_id) => {
                let symbol = ctx.store[local_id].borrow();
                write!(o, "local::{}::`{}`", local_id.as_usize(), symbol.name)
            }

            SymbolId::Parameter(param_id) => {
                let symbol = ctx.store[param_id].borrow();
                write!(o, "param::{}::`{}`", param_id.as_usize(), symbol.name)
            }
        }
    }
}

impl Dump for SymbolId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::fmt::Write,
    ) -> Result<(), std::fmt::Error> {
        match self {
            SymbolId::Parameter(fp) => fp.dump(ctx, o),
            SymbolId::Function(f) => f.dump(ctx, o),
            SymbolId::GlobalVariable(gv) => gv.dump(ctx, o),
            SymbolId::LocalVariable(lv) => lv.dump(ctx, o),
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
            Item::Module(m) => m.dump(ctx, o),
            Item::TypeAliasDef(ta) => ta.dump(ctx, o),
            Item::StructDef(sd) => sd.dump(ctx, o),
            Item::EnumDef(ed) => ed.dump(ctx, o),
        }
    }
}
