use crate::{Dump, DumpContext, write_indent};
use nitrate_hir::prelude::*;
use std::collections::BTreeSet;

impl Dump for Visibility {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
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
    o: &mut dyn std::io::Write,
) -> Result<(), std::io::Error> {
    if attributes.is_empty() {
        return Ok(());
    }

    writeln!(o, "[")?;

    for attr in attributes {
        ctx.indent += 1;

        write_indent(ctx, o)?;
        attr.dump(ctx, o)?;
        writeln!(o, ",")?;

        ctx.indent -= 1;
    }

    write_indent(ctx, o)?;
    write!(o, "] ")
}

impl Dump for GlobalVariableAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        Ok(())
    }
}

impl Dump for GlobalVariableId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "static::{}::`{}` ", self.as_usize(), this.name)?;

        if this.is_mutable {
            write!(o, "mut ")?;
        }

        dump_attributes(&this.attributes, ctx, o)?;

        write!(o, ": ")?;
        this.ty.dump(ctx, o)?;

        write!(o, " = ")?;
        this.init.borrow().dump(ctx, o)?;

        write!(o, ";")
    }
}

impl Dump for LocalVariableAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        Ok(())
    }
}

impl Dump for LocalVariableId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        match this.kind {
            LocalVariableKind::Stack => write!(o, "let::{}::`{}` ", self.as_usize(), this.name)?,
            LocalVariableKind::Dynamic => write!(o, "var::{}::`{}` ", self.as_usize(), this.name)?,
            LocalVariableKind::Static => {
                write!(o, "static::{}::`{}` ", self.as_usize(), this.name)?;
            }
        }

        if this.is_mutable {
            write!(o, "mut ")?;
        }

        dump_attributes(&this.attributes, ctx, o)?;

        write!(o, ": ")?;
        this.ty.dump(ctx, o)?;

        if let Some(initializer) = &this.init {
            write!(o, " = ")?;
            initializer.borrow().dump(ctx, o)?;
        }

        write!(o, ";")
    }
}

impl Dump for ParameterAttribute {
    fn dump(
        &self,
        _ctx: &mut DumpContext,
        _o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        Ok(())
    }
}

impl Dump for ParameterId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        write!(o, "param::{}::`{}` ", self.as_usize(), this.name)?;

        if this.is_mutable {
            write!(o, "mut ")?;
        }

        dump_attributes(&this.attributes, ctx, o)?;

        write!(o, ": ")?;
        this.ty.dump(ctx, o)?;

        if let Some(default_value) = &this.default_value {
            write!(o, " = ")?;
            default_value.borrow().dump(ctx, o)?;
        }

        Ok(())
    }
}

impl Dump for FunctionId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "fn::{}::`{}` ", self.as_usize(), this.name)?;

        dump_attributes(&this.attributes, ctx, o)?;

        if this.params.is_empty() {
            write!(o, "()")?;
        } else {
            writeln!(o, "(")?;

            for param in &this.params {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                param.dump(ctx, o)?;
                writeln!(o, ",")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, ")")?;
        }

        write!(o, " -> ")?;
        this.return_type.dump(ctx, o)?;

        if let Some(body) = &this.body {
            write!(o, " ")?;
            body.borrow().dump(ctx, o)
        } else {
            write!(o, ";")
        }
    }
}

impl Dump for TraitId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "trait::{}::`{}` ", self.as_usize(), this.name)?;

        if this.methods.is_empty() {
            write!(o, " {{}}")
        } else {
            writeln!(o, " {{")?;

            for method in &this.methods {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                method.dump(ctx, o)?;
                writeln!(o)?;

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
        _o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        Ok(())
    }
}

impl Dump for ModuleId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "mod::{}::`{}` ", self.as_usize(), this.name)?;

        dump_attributes(&this.attributes, ctx, o)?;

        if this.items.is_empty() {
            write!(o, "{{}}")
        } else {
            writeln!(o, "{{")?;

            for (i, item) in this.items.iter().enumerate() {
                if i != 0 {
                    writeln!(o)?;
                }

                ctx.indent += 1;

                write_indent(ctx, o)?;
                item.dump(ctx, o)?;
                writeln!(o)?;

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
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "mod::`{}` ", self.name)?;

        dump_attributes(&self.attributes, ctx, o)?;

        if self.items.is_empty() {
            write!(o, "{{}}")
        } else {
            writeln!(o, "{{")?;

            for (i, item) in self.items.iter().enumerate() {
                if i != 0 {
                    writeln!(o)?;
                }

                ctx.indent += 1;

                write_indent(ctx, o)?;
                item.dump(ctx, o)?;
                writeln!(o)?;

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
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "typealias::{}::`{}` ", self.as_usize(), this.name)?;

        write!(o, "= ")?;
        this.type_id.dump(ctx, o)?;

        write!(o, ";")
    }
}

impl Dump for StructDefId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "struct::{}::`{}` ", self.as_usize(), this.name)?;

        write!(o, "= ")?;

        write!(o, "struct ")?;

        dump_attributes(&this.attributes, ctx, o)?;

        if this.fields.is_empty() {
            write!(o, "{{}}")?;
        } else {
            writeln!(o, "{{")?;

            for field in this.fields.values() {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                write!(o, "{}", field.name)?;

                write!(o, ": ")?;
                field.ty.dump(ctx, o)?;

                writeln!(o, ",")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")?;
        }

        write!(o, ";")
    }
}

impl Dump for EnumDefId {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
        let this = self.borrow();

        if this.visibility != Visibility::Sec {
            this.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "enum::{}::`{}` ", self.as_usize(), this.name)?;

        write!(o, "= ")?;
        write!(o, "enum ")?;

        dump_attributes(&this.attributes, ctx, o)?;

        if this.variants.is_empty() {
            write!(o, "{{}}")?;
        } else {
            writeln!(o, "{{")?;

            for variant in &this.variants {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                write!(o, "{}", variant.name)?;

                write!(o, ": ")?;
                variant.ty.dump(ctx, o)?;

                writeln!(o, ",")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")?;
        }
        write!(o, ";")
    }
}

impl Dump for Item {
    fn dump(
        &self,
        ctx: &mut DumpContext,
        o: &mut dyn std::io::Write,
    ) -> Result<(), std::io::Error> {
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
