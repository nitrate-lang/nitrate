use crate::{Dump, DumpContext, write_indent};
use nitrate_hir::prelude::*;
use nitrate_token::escape_string;
use std::{collections::BTreeSet, write};

impl Dump for Visibility {
    fn dump(&self, _ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
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
    fn dump(&self, _ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            GlobalVariableAttribute::NoMangle => write!(o, "no_mangle"),
        }
    }
}

impl Dump for GlobalVariable {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "static ")?;
        dump_attributes(&self.attributes, ctx, o)?;

        if self.is_mutable {
            write!(o, "mut ")?;
        }
        write!(o, "{} ", escape_string(&self.name, true))?;

        write!(o, ": ")?;
        self.ty.dump(ctx, o)?;

        write!(o, " = ")?;
        self.initializer.borrow().dump(ctx, o)?;

        write!(o, ";")
    }
}

impl Dump for LocalVariableAttribute {
    fn dump(&self, _ctx: &mut DumpContext, _o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        Ok(())
    }
}

impl Dump for LocalVariable {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self.kind {
            LocalKind::Let => write!(o, "let ")?,
            LocalKind::Var => write!(o, "var ")?,
            LocalKind::Static => write!(o, "static ")?,
        }

        dump_attributes(&self.attributes, ctx, o)?;

        if self.is_mutable {
            write!(o, "mut ")?;
        }
        write!(o, "{} ", escape_string(&self.name, true))?;

        write!(o, ": ")?;
        self.ty.dump(ctx, o)?;

        if let Some(init) = &self.initializer {
            write!(o, " = ")?;
            init.borrow().dump(ctx, o)?;
        }

        write!(o, ";")
    }
}

impl Dump for ParameterAttribute {
    fn dump(&self, _ctx: &mut DumpContext, _o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        Ok(())
    }
}

impl Dump for Parameter {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        dump_attributes(&self.attributes, ctx, o)?;

        if self.is_mutable {
            write!(o, "mut ")?;
        }
        write!(o, "{} ", escape_string(&self.name, true))?;

        write!(o, ": ")?;
        self.ty.dump(ctx, o)?;

        if let Some(default_value) = &self.default_value {
            write!(o, " = ")?;
            default_value.borrow().dump(ctx, o)?;
        }

        Ok(())
    }
}

impl Dump for Function {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "fn ")?;
        dump_attributes(&self.attributes, ctx, o)?;

        write!(o, "{} ", escape_string(&self.name, true))?;

        if self.params.is_empty() {
            write!(o, "()")?;
        } else {
            writeln!(o, "(")?;

            for param in &self.params {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                param.borrow().dump(ctx, o)?;
                writeln!(o, ",")?;

                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, ")")?;
        }

        write!(o, " -> ")?;
        self.return_type.dump(ctx, o)?;

        if let Some(body) = &self.body {
            write!(o, " ")?;
            writeln!(o, "{{")?;
            for element in body {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                element.dump(ctx, o)?;
                writeln!(o)?;

                ctx.indent -= 1;
            }
            write_indent(ctx, o)?;
            write!(o, "}}")
        } else {
            write!(o, ";")
        }
    }
}

impl Dump for Trait {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "trait ")?;
        write!(o, "{} ", escape_string(&self.name, true))?;

        if self.methods.is_empty() && self.associated_types.is_empty() && self.associated_constants.is_empty() {
            write!(o, " {{}}")
        } else {
            writeln!(o, " {{")?;

            for method in &self.methods {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                method.borrow().dump(ctx, o)?;
                writeln!(o)?;

                ctx.indent -= 1;
            }

            for assoc_type in &self.associated_types {
                ctx.indent += 1;
                write_indent(ctx, o)?;
                write!(o, "type {};", escape_string(assoc_type, true))?;
                writeln!(o)?;
                ctx.indent -= 1;
            }

            for assoc_const in &self.associated_constants {
                ctx.indent += 1;
                write_indent(ctx, o)?;
                write!(o, "const {};", escape_string(assoc_const, true))?;
                writeln!(o)?;
                ctx.indent -= 1;
            }

            write_indent(ctx, o)?;
            write!(o, "}}")
        }
    }
}

impl Dump for ModuleAttribute {
    fn dump(&self, _ctx: &mut DumpContext, _o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        Ok(())
    }
}

impl Dump for Module {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "mod ")?;
        dump_attributes(&self.attributes, ctx, o)?;

        write!(o, "{} ", escape_string(&self.name, true))?;

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

impl Dump for TypeAliasDef {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "type {} = ", escape_string(&self.name, true))?;
        self.type_id.dump(ctx, o)?;
        write!(o, ";")
    }
}

impl Dump for StructDef {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "struct ")?;
        dump_attributes(&self.attributes, ctx, o)?;
        write!(o, "{} ", escape_string(&self.name, true))?;

        if self.fields.is_empty() {
            write!(o, "{{}}")?;
        } else {
            writeln!(o, "{{")?;

            for field in self.fields.values() {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                write!(o, "{}", escape_string(&field.name, true))?;

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

impl Dump for EnumDef {
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        if self.visibility != Visibility::Sec {
            self.visibility.dump(ctx, o)?;
            write!(o, " ")?;
        }

        write!(o, "enum ")?;
        dump_attributes(&self.attributes, ctx, o)?;
        write!(o, "{} ", escape_string(&self.name, true))?;

        if self.variants.is_empty() {
            write!(o, "{{}}")?;
        } else {
            writeln!(o, "{{")?;

            for variant in &self.variants {
                ctx.indent += 1;

                write_indent(ctx, o)?;
                write!(o, "{}", escape_string(&variant.name, true))?;

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
    fn dump(&self, ctx: &mut DumpContext, o: &mut dyn std::fmt::Write) -> Result<(), std::fmt::Error> {
        match self {
            Item::Function(f) => f.borrow().dump(ctx, o),
            Item::GlobalVariable(gv) => gv.borrow().dump(ctx, o),
            Item::Module(m) => m.borrow().dump(ctx, o),
            Item::TypeAliasDef(ta) => ta.borrow().dump(ctx, o),
            Item::StructDef(sd) => sd.borrow().dump(ctx, o),
            Item::EnumDef(ed) => ed.borrow().dump(ctx, o),
            Item::Trait(t) => t.borrow().dump(ctx, o),
        }
    }
}
