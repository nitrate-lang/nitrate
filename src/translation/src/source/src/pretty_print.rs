use std::num::NonZeroUsize;

use nitrate_token::IntegerKind;

use crate::{
    ast::{LocalVariable, LocalVariableKind},
    expr::{
        AttributeList, Await, BStringLit, BinExpr, BinExprOp, Block, BlockItem, BooleanLit, Break,
        CallArgument, Cast, Closure, Continue, ElseIf, Expr, ExprParentheses, ExprPath,
        ExprPathSegment, ExprSyntaxError, FloatLit, ForEach, FunctionCall, If, IndexAccess,
        IntegerLit, List, Match, MatchCase, MethodCall, Return, Safety, StringLit, StructInit,
        Tuple, TypeArgument, TypeInfo, UnaryExpr, UnaryExprOp, WhileLoop,
    },
    item::{
        AssociatedItem, Enum, EnumVariant, FuncParam, FuncParams, Function, Generics,
        GlobalVariable, GlobalVariableKind, Impl, Import, Item, ItemSyntaxError, Module,
        Mutability, Struct, StructField, Trait, TypeAlias, TypeParam, Visibility,
    },
    ty::{
        ArrayType, Bool, Exclusivity, Float32, Float64, FunctionType, InferType, Int8, Int16,
        Int32, Int64, Int128, LatentType, Lifetime, OpaqueType, ReferenceType, RefinementType,
        SliceType, TupleType, Type, TypeParentheses, TypePath, TypePathSegment, TypeSyntaxError,
        UInt8, UInt16, UInt32, UInt64, UInt128, USize,
    },
};

pub struct PrintContext {
    pub indent: String,
    pub max_line_length: NonZeroUsize,
    pub show_resolution_links: bool,

    pub tab_depth: usize,
}

impl Default for PrintContext {
    fn default() -> Self {
        PrintContext {
            indent: "  ".to_string(),
            max_line_length: NonZeroUsize::new(80).unwrap(),
            show_resolution_links: false,
            tab_depth: 0,
        }
    }
}

impl PrintContext {
    fn write_indent(&self, writer: &mut dyn std::fmt::Write) -> std::fmt::Result {
        for _ in 0..self.tab_depth {
            writer.write_str(&self.indent)?;
        }
        Ok(())
    }
}

fn write_unresolved_link(writer: &mut dyn std::fmt::Write) -> std::fmt::Result {
    writer.write_str("$<unresolved>$ ")
}
fn write_resolve_link<T>(writer: &mut dyn std::fmt::Write, resolve_target: &T) -> std::fmt::Result {
    write!(writer, "$<0x{:x}>$ ", resolve_target as *const T as usize)
}

pub trait PrettyPrint {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result;

    fn pretty_print(&self, ctx: &mut PrintContext) -> Result<String, std::fmt::Error> {
        let mut output = String::new();
        self.pretty_print_fmt(ctx, &mut output)?;
        Ok(output)
    }
}

impl PrettyPrint for ExprSyntaxError {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // Expression syntax errors are unrepresentable
        Ok(())
    }
}

impl PrettyPrint for ExprParentheses {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('(')?;
        self.inner.pretty_print_fmt(ctx, writer)?;
        writer.write_char(')')
    }
}

impl PrettyPrint for BooleanLit {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self.value {
            true => writer.write_str("true"),
            false => writer.write_str("false"),
        }
    }
}

impl PrettyPrint for IntegerLit {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self.kind {
            IntegerKind::Bin => {
                write!(writer, "0b{:b}", self.value)
            }

            IntegerKind::Oct => {
                write!(writer, "0o{:o}", self.value)
            }

            IntegerKind::Dec => {
                write!(writer, "{}", self.value)
            }

            IntegerKind::Hex => {
                write!(writer, "0x{:x}", self.value)
            }
        }
    }
}

impl PrettyPrint for FloatLit {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        write!(writer, "{}", self.value)
    }
}

impl PrettyPrint for StringLit {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('"')?;

        for c in self.value.chars() {
            if c.is_ascii() {
                match c {
                    '\0' => writer.write_str("\\0")?,
                    '\x07' => writer.write_str("\\a")?,
                    '\x08' => writer.write_str("\\b")?,
                    '\t' => writer.write_str("\\t")?,
                    '\n' => writer.write_str("\\n")?,
                    '\x0b' => writer.write_str("\\v")?,
                    '\x0c' => writer.write_str("\\f")?,
                    '\r' => writer.write_str("\\r")?,
                    '\"' => writer.write_str("\\\"")?,
                    '\\' => writer.write_str("\\\\")?,

                    ' '..='~' => writer.write_char(c)?,
                    _ => write!(writer, "\\x{:02x}", c as u32)?,
                }
            } else {
                write!(writer, "\\u{{{:x}}}", c as u32)?;
            }
        }

        writer.write_char('"')
    }
}

impl PrettyPrint for BStringLit {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('"')?;

        for byte in &self.value {
            match byte {
                b'\0' => writer.write_str("\\0")?,
                b'\x07' => writer.write_str("\\a")?,
                b'\x08' => writer.write_str("\\b")?,
                b'\t' => writer.write_str("\\t")?,
                b'\n' => writer.write_str("\\n")?,
                b'\x0b' => writer.write_str("\\v")?,
                b'\x0c' => writer.write_str("\\f")?,
                b'\r' => writer.write_str("\\r")?,
                b'\"' => writer.write_str("\\\"")?,
                b'\\' => writer.write_str("\\\\")?,
                b' '..=b'~' => writer.write_char(*byte as char)?,
                _ => write!(writer, "\\x{:02x}", *byte as u32)?,
            }
        }

        writer.write_char('"')
    }
}

impl PrettyPrint for TypeInfo {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("type ")?;
        self.the.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for List {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('[')?;

        for (i, item) in self.elements.iter().enumerate() {
            if i > 0 {
                writer.write_str(", ")?;
            }

            item.pretty_print_fmt(ctx, writer)?;
        }

        writer.write_char(']')
    }
}

impl PrettyPrint for Tuple {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('(')?;

        for item in &self.elements {
            item.pretty_print_fmt(ctx, writer)?;
            writer.write_str(", ")?;
        }

        writer.write_char(')')
    }
}

impl PrettyPrint for StructInit {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.type_name.pretty_print_fmt(ctx, writer)?;

        if self.fields.is_empty() {
            writer.write_str(" {}")
        } else {
            writer.write_str("{\n")?;

            for (name, value) in &self.fields {
                ctx.tab_depth += 1;

                ctx.write_indent(writer)?;
                writer.write_str(name)?;
                writer.write_str(": ")?;
                value.pretty_print_fmt(ctx, writer)?;
                writer.write_str(",\n")?;

                ctx.tab_depth -= 1;
            }

            writer.write_char('}')
        }
    }
}

impl PrettyPrint for UnaryExprOp {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            UnaryExprOp::Add => writer.write_str("+"),
            UnaryExprOp::Sub => writer.write_str("-"),
            UnaryExprOp::Deref => writer.write_str("*"),
            UnaryExprOp::Borrow => writer.write_str("&"),
            UnaryExprOp::BitNot => writer.write_str("~"),
            UnaryExprOp::LogicNot => writer.write_str("!"),
            UnaryExprOp::Typeof => writer.write_str("typeof"),
        }
    }
}

impl PrettyPrint for UnaryExpr {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.operator.pretty_print_fmt(ctx, writer)?;

        if self.operator == UnaryExprOp::Typeof {
            writer.write_char(' ')?;
        }

        self.operand.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for BinExprOp {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            BinExprOp::Add => writer.write_str("+"),
            BinExprOp::Sub => writer.write_str("-"),
            BinExprOp::Mul => writer.write_str("*"),
            BinExprOp::Div => writer.write_str("/"),
            BinExprOp::Mod => writer.write_str("%"),
            BinExprOp::BitAnd => writer.write_str("&"),
            BinExprOp::BitOr => writer.write_str("|"),
            BinExprOp::BitXor => writer.write_str("^"),
            BinExprOp::BitShl => writer.write_str("<<"),
            BinExprOp::BitShr => writer.write_str(">>"),
            BinExprOp::BitRol => writer.write_str("<<<"),
            BinExprOp::BitRor => writer.write_str(">>>"),
            BinExprOp::LogicAnd => writer.write_str("&&"),
            BinExprOp::LogicOr => writer.write_str("||"),
            BinExprOp::LogicLt => writer.write_str("<"),
            BinExprOp::LogicGt => writer.write_str(">"),
            BinExprOp::LogicLe => writer.write_str("<="),
            BinExprOp::LogicGe => writer.write_str(">="),
            BinExprOp::LogicEq => writer.write_str("=="),
            BinExprOp::LogicNe => writer.write_str("!="),
            BinExprOp::Set => writer.write_str("="),
            BinExprOp::SetPlus => writer.write_str("+="),
            BinExprOp::SetMinus => writer.write_str("-="),
            BinExprOp::SetTimes => writer.write_str("*="),
            BinExprOp::SetSlash => writer.write_str("/="),
            BinExprOp::SetPercent => writer.write_str("%="),
            BinExprOp::SetBitAnd => writer.write_str("&="),
            BinExprOp::SetBitOr => writer.write_str("|="),
            BinExprOp::SetBitXor => writer.write_str("^="),
            BinExprOp::SetBitShl => writer.write_str("<<="),
            BinExprOp::SetBitShr => writer.write_str(">>="),
            BinExprOp::SetBitRotl => writer.write_str("<<<="),
            BinExprOp::SetBitRotr => writer.write_str(">>>="),
            BinExprOp::SetLogicAnd => writer.write_str("&&="),
            BinExprOp::SetLogicOr => writer.write_str("||="),
            BinExprOp::Dot => writer.write_str("."),
            BinExprOp::Arrow => writer.write_str("->"),
            BinExprOp::Range => writer.write_str(".."),
        }
    }
}

impl PrettyPrint for BinExpr {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.left.pretty_print_fmt(ctx, writer)?;

        writer.write_char(' ')?;
        self.operator.pretty_print_fmt(ctx, writer)?;
        writer.write_char(' ')?;

        self.right.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for Cast {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.value.pretty_print_fmt(ctx, writer)?;
        writer.write_str(" as ")?;
        self.to.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for LocalVariable {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if ctx.show_resolution_links {
            write_resolve_link(writer, self)?;
        }

        match self.kind {
            LocalVariableKind::Let => writer.write_str("let ")?,
            LocalVariableKind::Var => writer.write_str("var ")?,
        }

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if let Some(mutability) = &self.mutability {
            mutability.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        if let Some(var_type) = &self.ty {
            writer.write_str(": ")?;
            var_type.pretty_print_fmt(ctx, writer)?;
        }

        if let Some(initializer) = &self.initializer {
            writer.write_str(" = ")?;
            initializer.pretty_print_fmt(ctx, writer)?;
        }

        writer.write_char(';')
    }
}

impl PrettyPrint for BlockItem {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            BlockItem::Variable(m) => m.pretty_print_fmt(ctx, writer),

            BlockItem::Expr(m) => m.pretty_print_fmt(ctx, writer),

            BlockItem::Stmt(m) => {
                m.pretty_print_fmt(ctx, writer)?;
                writer.write_char(';')
            }
        }
    }
}

impl PrettyPrint for Block {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(safety) = &self.safety {
            match safety {
                Safety::Safe => writer.write_str("safe ")?,

                Safety::Unsafe(None) => writer.write_str("unsafe ")?,

                Safety::Unsafe(Some(reason)) => {
                    writer.write_str("unsafe(")?;
                    reason.pretty_print_fmt(ctx, writer)?;
                    writer.write_str(") ")?;
                }
            }
        }

        if self.elements.is_empty() {
            writer.write_str("{}")
        } else {
            writer.write_str("{\n")?;

            for item in &self.elements {
                ctx.tab_depth += 1;

                ctx.write_indent(writer)?;
                item.pretty_print_fmt(ctx, writer)?;
                writer.write_char('\n')?;

                ctx.tab_depth -= 1;
            }

            ctx.write_indent(writer)?;
            writer.write_char('}')
        }
    }
}

impl PrettyPrint for AttributeList {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('[')?;
        for (i, attr) in self.iter().enumerate() {
            if i > 0 {
                writer.write_str(", ")?;
            }

            attr.pretty_print_fmt(ctx, writer)?;
        }
        writer.write_char(']')
    }
}

impl PrettyPrint for Closure {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if self.attributes.is_some() || self.parameters.is_some() || self.return_type.is_some() {
            writer.write_str("fn")?;
        }

        if let Some(attributes) = &self.attributes {
            writer.write_char(' ')?;
            attributes.pretty_print_fmt(ctx, writer)?;
        }

        if let Some(parameters) = &self.parameters {
            if self.attributes.is_some() {
                writer.write_char(' ')?;
            }

            writer.write_char('(')?;
            for (i, param) in parameters.iter().enumerate() {
                if i > 0 {
                    writer.write_str(", ")?;
                }

                param.pretty_print_fmt(ctx, writer)?;
            }
            writer.write_str(")")?;
        }

        if let Some(return_type) = &self.return_type {
            writer.write_str(" -> ")?;
            return_type.pretty_print_fmt(ctx, writer)?;
        }

        if self.attributes.is_some() || self.parameters.is_some() || self.return_type.is_some() {
            writer.write_char(' ')?;
        }

        self.definition.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for TypeArgument {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(name) = &self.name {
            writer.write_str(name)?;
            writer.write_str(": ")?;
        }

        self.value.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for ExprPathSegment {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str(&self.name)?;

        if let Some(type_args) = &self.type_arguments {
            writer.write_str("::<")?;
            for (i, arg) in type_args.iter().enumerate() {
                if i > 0 {
                    writer.write_str(", ")?;
                }

                arg.pretty_print_fmt(ctx, writer)?;
            }
            writer.write_char('>')?;
        }

        Ok(())
    }
}

impl PrettyPrint for ExprPath {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        for (i, segment) in self.segments.iter().enumerate() {
            if i > 0 {
                writer.write_str("::")?;
            }

            segment.pretty_print_fmt(ctx, writer)?;
        }

        if ctx.show_resolution_links {
            writer.write_char(' ')?;

            if let Some(resolved_path) = &self.resolved_path {
                writer.write_str("$<")?;
                writer.write_str(resolved_path)?;
                writer.write_str(">$ ")?;
            } else {
                write_unresolved_link(writer)?;
            }
        }

        Ok(())
    }
}

impl PrettyPrint for IndexAccess {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.collection.pretty_print_fmt(ctx, writer)?;
        writer.write_char('[')?;
        self.index.pretty_print_fmt(ctx, writer)?;
        writer.write_char(']')
    }
}

impl PrettyPrint for If {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("if ")?;

        self.condition.pretty_print_fmt(ctx, writer)?;
        writer.write_char(' ')?;

        self.true_branch.pretty_print_fmt(ctx, writer)?;

        match &self.false_branch {
            None => {}

            Some(ElseIf::If(else_if)) => {
                writer.write_str(" else ")?;
                else_if.pretty_print_fmt(ctx, writer)?;
            }

            Some(ElseIf::Block(else_block)) => {
                writer.write_str(" else ")?;
                else_block.pretty_print_fmt(ctx, writer)?;
            }
        }

        Ok(())
    }
}

impl PrettyPrint for WhileLoop {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("while ")?;

        if let Some(condition) = &self.condition {
            condition.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        self.body.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for MatchCase {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.condition.pretty_print_fmt(ctx, writer)?;
        writer.write_str(" => ")?;
        self.body.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for Match {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("match ")?;

        self.condition.pretty_print_fmt(ctx, writer)?;

        if self.cases.is_empty() && self.default_case.is_none() {
            return writer.write_str(" {}");
        }

        writer.write_str(" {\n")?;

        for case in &self.cases {
            ctx.tab_depth += 1;

            ctx.write_indent(writer)?;
            case.pretty_print_fmt(ctx, writer)?;
            writer.write_str(",\n")?;

            ctx.tab_depth -= 1;
        }

        if let Some(default) = &self.default_case {
            ctx.tab_depth += 1;

            ctx.write_indent(writer)?;
            writer.write_str("_ => ")?;
            default.pretty_print_fmt(ctx, writer)?;
            writer.write_str(",\n")?;

            ctx.tab_depth -= 1;
        }

        ctx.write_indent(writer)?;
        writer.write_str("}")
    }
}

impl PrettyPrint for Break {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("break")?;

        if let Some(label) = &self.label {
            writer.write_str(" '")?;
            writer.write_str(label)?;
        }

        Ok(())
    }
}

impl PrettyPrint for Continue {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("continue")?;

        if let Some(label) = &self.label {
            writer.write_str(" '")?;
            writer.write_str(label)?;
        }

        Ok(())
    }
}

impl PrettyPrint for Return {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("return")?;

        if let Some(value) = &self.value {
            writer.write_char(' ')?;
            value.pretty_print_fmt(ctx, writer)?;
        }

        Ok(())
    }
}

impl PrettyPrint for ForEach {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("for ")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if self.bindings.len() == 1 {
            writer.write_str(&self.bindings[0])?;
        } else {
            writer.write_char('(')?;
            for (i, binding) in self.bindings.iter().enumerate() {
                if i > 0 {
                    writer.write_str(", ")?;
                }

                writer.write_str(binding)?;
            }
            writer.write_char(')')?;
        }

        writer.write_str(" in ")?;
        self.iterable.pretty_print_fmt(ctx, writer)?;
        writer.write_char(' ')?;

        self.body.pretty_print_fmt(ctx, writer)?;

        Ok(())
    }
}

impl PrettyPrint for Await {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("await ")?;
        self.future.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for CallArgument {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(name) = &self.name {
            writer.write_str(name)?;
            writer.write_str(": ")?;
        }

        self.value.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for FunctionCall {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.callee.pretty_print_fmt(ctx, writer)?;

        writer.write_char('(')?;
        for (i, arg) in self.arguments.iter().enumerate() {
            if i > 0 {
                writer.write_str(", ")?;
            }

            arg.pretty_print_fmt(ctx, writer)?;
        }
        writer.write_char(')')
    }
}

impl PrettyPrint for MethodCall {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.object.pretty_print_fmt(ctx, writer)?;
        writer.write_str(".")?;
        writer.write_str(&self.method_name)?;

        writer.write_char('(')?;
        for (i, arg) in self.arguments.iter().enumerate() {
            if i > 0 {
                writer.write_str(", ")?;
            }

            arg.pretty_print_fmt(ctx, writer)?;
        }
        writer.write_char(')')
    }
}

impl PrettyPrint for Expr {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            Expr::SyntaxError(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Parentheses(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Boolean(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Integer(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Float(m) => m.pretty_print_fmt(ctx, writer),
            Expr::String(m) => m.pretty_print_fmt(ctx, writer),
            Expr::BString(m) => m.pretty_print_fmt(ctx, writer),
            Expr::TypeInfo(m) => m.pretty_print_fmt(ctx, writer),
            Expr::List(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Tuple(m) => m.pretty_print_fmt(ctx, writer),
            Expr::StructInit(m) => m.pretty_print_fmt(ctx, writer),
            Expr::UnaryExpr(m) => m.pretty_print_fmt(ctx, writer),
            Expr::BinExpr(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Cast(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Block(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Closure(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Path(m) => m.pretty_print_fmt(ctx, writer),
            Expr::IndexAccess(m) => m.pretty_print_fmt(ctx, writer),
            Expr::If(m) => m.pretty_print_fmt(ctx, writer),
            Expr::While(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Match(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Break(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Continue(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Return(m) => m.pretty_print_fmt(ctx, writer),
            Expr::For(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Await(m) => m.pretty_print_fmt(ctx, writer),
            Expr::FunctionCall(m) => m.pretty_print_fmt(ctx, writer),
            Expr::MethodCall(m) => m.pretty_print_fmt(ctx, writer),
        }
    }
}

impl PrettyPrint for TypeSyntaxError {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // Type syntax errors are unrepresentable
        Ok(())
    }
}

impl PrettyPrint for Bool {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("bool")
    }
}

impl PrettyPrint for UInt8 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u8")
    }
}

impl PrettyPrint for UInt16 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u16")
    }
}

impl PrettyPrint for UInt32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u32")
    }
}

impl PrettyPrint for UInt64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u64")
    }
}

impl PrettyPrint for UInt128 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u128")
    }
}

impl PrettyPrint for USize {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("usize")
    }
}

impl PrettyPrint for Int8 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i8")
    }
}

impl PrettyPrint for Int16 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i16")
    }
}

impl PrettyPrint for Int32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i32")
    }
}

impl PrettyPrint for Int64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i64")
    }
}

impl PrettyPrint for Int128 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i128")
    }
}

impl PrettyPrint for Float32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("f32")
    }
}

impl PrettyPrint for Float64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("f64")
    }
}

impl PrettyPrint for InferType {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("_")
    }
}

impl PrettyPrint for TypePathSegment {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str(&self.name)?;

        if let Some(type_args) = &self.type_arguments {
            writer.write_str("<")?;
            for (i, arg) in type_args.iter().enumerate() {
                if i > 0 {
                    writer.write_str(", ")?;
                }

                arg.pretty_print_fmt(ctx, writer)?;
            }
            writer.write_char('>')?;
        }

        Ok(())
    }
}

impl PrettyPrint for TypePath {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        for (i, segment) in self.segments.iter().enumerate() {
            if i > 0 {
                writer.write_str("::")?;
            }

            segment.pretty_print_fmt(ctx, writer)?;
        }

        if ctx.show_resolution_links {
            writer.write_char(' ')?;

            if let Some(resolved_path) = &self.resolved_path {
                writer.write_str("$<")?;
                writer.write_str(resolved_path)?;
                writer.write_str(">$ ")?;
            } else {
                write_unresolved_link(writer)?;
            }
        }

        Ok(())
    }
}

impl PrettyPrint for RefinementType {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.basis_type.pretty_print_fmt(ctx, writer)?;

        if let Some(width) = &self.width {
            writer.write_str(": ")?;
            width.pretty_print_fmt(ctx, writer)?;
        }

        if self.minimum.is_some() || self.maximum.is_some() {
            writer.write_str(": [")?;

            if let Some(minimum) = &self.minimum {
                minimum.pretty_print_fmt(ctx, writer)?;
            }

            writer.write_str(":")?;

            if let Some(maximum) = &self.maximum {
                maximum.pretty_print_fmt(ctx, writer)?;
            }

            writer.write_char(']')?;
        }

        Ok(())
    }
}

impl PrettyPrint for TupleType {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("(")?;

        for (i, element) in self.element_types.iter().enumerate() {
            if i > 0 {
                writer.write_str(", ")?;
            }

            element.pretty_print_fmt(ctx, writer)?;
        }

        if self.element_types.len() == 1 {
            writer.write_str(",")?;
        }

        writer.write_char(')')
    }
}

impl PrettyPrint for ArrayType {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('[')?;
        self.element_type.pretty_print_fmt(ctx, writer)?;
        writer.write_str("; ")?;
        self.len.pretty_print_fmt(ctx, writer)?;
        writer.write_char(']')
    }
}

impl PrettyPrint for SliceType {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('[')?;
        self.element_type.pretty_print_fmt(ctx, writer)?;
        writer.write_char(']')
    }
}

impl PrettyPrint for FunctionType {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        write!(writer, "fn")?;

        if let Some(attributes) = &self.attributes {
            writer.write_char(' ')?;
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        self.parameters.pretty_print_fmt(ctx, writer)?;

        if let Some(return_type) = &self.return_type {
            writer.write_str(" -> ")?;
            return_type.pretty_print_fmt(ctx, writer)?;
        }

        Ok(())
    }
}

impl PrettyPrint for Exclusivity {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            Exclusivity::Iso => writer.write_str("iso"),
            Exclusivity::Poly => writer.write_str("poly"),
        }
    }
}

impl PrettyPrint for ReferenceType {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("&")?;

        if let Some(lifetime) = &self.lifetime {
            lifetime.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if let Some(exclusivity) = &self.exclusivity {
            exclusivity.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if let Some(mutability) = &self.mutability {
            mutability.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        self.to.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for OpaqueType {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        write!(writer, "opaque(\"{}\")", self.name)
    }
}

impl PrettyPrint for LatentType {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.body.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for Lifetime {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('\'')?;
        writer.write_str(&self.name)
    }
}

impl PrettyPrint for TypeParentheses {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('(')?;
        self.inner.pretty_print_fmt(ctx, writer)?;
        writer.write_char(')')
    }
}

impl PrettyPrint for Type {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            Type::SyntaxError(m) => m.pretty_print_fmt(ctx, writer),
            Type::Bool(m) => m.pretty_print_fmt(ctx, writer),
            Type::UInt8(m) => m.pretty_print_fmt(ctx, writer),
            Type::UInt16(m) => m.pretty_print_fmt(ctx, writer),
            Type::UInt32(m) => m.pretty_print_fmt(ctx, writer),
            Type::UInt64(m) => m.pretty_print_fmt(ctx, writer),
            Type::UInt128(m) => m.pretty_print_fmt(ctx, writer),
            Type::USize(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int8(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int16(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int32(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int64(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int128(m) => m.pretty_print_fmt(ctx, writer),
            Type::Float32(m) => m.pretty_print_fmt(ctx, writer),
            Type::Float64(m) => m.pretty_print_fmt(ctx, writer),
            Type::InferType(m) => m.pretty_print_fmt(ctx, writer),
            Type::TypePath(m) => m.pretty_print_fmt(ctx, writer),
            Type::RefinementType(m) => m.pretty_print_fmt(ctx, writer),
            Type::TupleType(m) => m.pretty_print_fmt(ctx, writer),
            Type::ArrayType(m) => m.pretty_print_fmt(ctx, writer),
            Type::SliceType(m) => m.pretty_print_fmt(ctx, writer),
            Type::FunctionType(m) => m.pretty_print_fmt(ctx, writer),
            Type::ReferenceType(m) => m.pretty_print_fmt(ctx, writer),
            Type::OpaqueType(m) => m.pretty_print_fmt(ctx, writer),
            Type::LatentType(m) => m.pretty_print_fmt(ctx, writer),
            Type::Lifetime(m) => m.pretty_print_fmt(ctx, writer),
            Type::Parentheses(m) => m.pretty_print_fmt(ctx, writer),
        }
    }
}

impl PrettyPrint for Visibility {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            Visibility::Public => writer.write_str("pub"),
            Visibility::Private => writer.write_str("sec"),
            Visibility::Protected => writer.write_str("pro"),
        }
    }
}

impl PrettyPrint for ItemSyntaxError {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // Item syntax errors are unrepresentable
        Ok(())
    }
}

impl PrettyPrint for Module {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str("mod ")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if let Some(name) = &self.name {
            writer.write_str(name)?;
            writer.write_char(' ')?;
        }

        if self.items.is_empty() {
            writer.write_str("{}")
        } else {
            writer.write_str("{\n")?;

            for item in &self.items {
                ctx.tab_depth += 1;

                ctx.write_indent(writer)?;
                item.pretty_print_fmt(ctx, writer)?;
                writer.write_str("\n")?;

                ctx.tab_depth -= 1;
            }

            ctx.write_indent(writer)?;
            writer.write_str("}")
        }
    }
}

impl PrettyPrint for Import {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str("use ")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.import_name)?;

        if let Some(resolved) = &self.resolved {
            writer.write_str(" --> ")?;
            resolved.pretty_print_fmt(ctx, writer)?;
        }

        writer.write_char(';')
    }
}

impl PrettyPrint for TypeParam {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str(&self.name)?;

        if let Some(default_value) = &self.default_value {
            writer.write_str(" = ")?;
            default_value.pretty_print_fmt(ctx, writer)?;
        }

        Ok(())
    }
}

impl PrettyPrint for Generics {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("<")?;
        for (i, param) in self.params.iter().enumerate() {
            if i > 0 {
                writer.write_str(", ")?;
            }

            param.pretty_print_fmt(ctx, writer)?;
        }
        writer.write_str(">")
    }
}

impl PrettyPrint for TypeAlias {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if ctx.show_resolution_links {
            write_resolve_link(writer, self)?;
        }

        writer.write_str("type ")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        if let Some(generics) = &self.generics {
            generics.pretty_print_fmt(ctx, writer)?;
        }

        if let Some(alias) = &self.alias_type {
            writer.write_str(" = ")?;
            alias.pretty_print_fmt(ctx, writer)?;
        }

        writer.write_char(';')
    }
}

impl PrettyPrint for StructField {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        writer.write_str(": ")?;
        self.ty.pretty_print_fmt(ctx, writer)?;

        if let Some(default_value) = &self.default_value {
            writer.write_str(" = ")?;
            default_value.pretty_print_fmt(ctx, writer)?;
        }

        Ok(())
    }
}

impl PrettyPrint for Struct {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if ctx.show_resolution_links {
            write_resolve_link(writer, self)?;
        }

        writer.write_str("struct ")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        if let Some(generics) = &self.generics {
            generics.pretty_print_fmt(ctx, writer)?;
        }

        if self.fields.is_empty() {
            writer.write_str(" {}")
        } else {
            writer.write_str(" {\n")?;

            for field in &self.fields {
                ctx.tab_depth += 1;

                ctx.write_indent(writer)?;
                field.pretty_print_fmt(ctx, writer)?;
                writer.write_str(",\n")?;

                ctx.tab_depth -= 1;
            }

            ctx.write_indent(writer)?;
            writer.write_str("}")
        }
    }
}

impl PrettyPrint for EnumVariant {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        if let Some(variant_type) = &self.ty {
            writer.write_char('(')?;
            variant_type.pretty_print_fmt(ctx, writer)?;
            writer.write_char(')')?;
        }

        if let Some(default_value) = &self.default_value {
            writer.write_str(" = ")?;
            default_value.pretty_print_fmt(ctx, writer)?;
        }

        Ok(())
    }
}

impl PrettyPrint for Enum {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if ctx.show_resolution_links {
            write_resolve_link(writer, self)?;
        }

        writer.write_str("enum ")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        if let Some(generics) = &self.generics {
            generics.pretty_print_fmt(ctx, writer)?;
        }

        if self.variants.is_empty() {
            writer.write_str(" {}")
        } else {
            writer.write_str(" {\n")?;

            for variant in &self.variants {
                ctx.tab_depth += 1;

                ctx.write_indent(writer)?;
                variant.pretty_print_fmt(ctx, writer)?;
                writer.write_str(",\n")?;

                ctx.tab_depth -= 1;
            }

            ctx.write_indent(writer)?;
            writer.write_str("}")
        }
    }
}

impl PrettyPrint for AssociatedItem {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            AssociatedItem::SyntaxError(m) => m.pretty_print_fmt(ctx, writer),
            AssociatedItem::TypeAlias(m) => m.pretty_print_fmt(ctx, writer),
            AssociatedItem::Method(m) => m.pretty_print_fmt(ctx, writer),
            AssociatedItem::ConstantItem(m) => m.pretty_print_fmt(ctx, writer),
        }
    }
}

impl PrettyPrint for Trait {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if ctx.show_resolution_links {
            write_resolve_link(writer, self)?;
        }

        writer.write_str("trait ")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        if let Some(generics) = &self.generics {
            generics.pretty_print_fmt(ctx, writer)?;
        }

        if self.items.is_empty() {
            writer.write_str(" {}")
        } else {
            writer.write_str(" {\n")?;

            for items in &self.items {
                ctx.tab_depth += 1;

                ctx.write_indent(writer)?;
                items.pretty_print_fmt(ctx, writer)?;
                writer.write_str("\n")?;

                ctx.tab_depth -= 1;
            }

            ctx.write_indent(writer)?;
            writer.write_str("}")
        }
    }
}

impl PrettyPrint for Impl {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("impl")?;
        if let Some(generics) = &self.generics {
            generics.pretty_print_fmt(ctx, writer)?;
        }

        writer.write_char(' ')?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if let Some(trait_path) = &self.trait_path {
            writer.write_str("trait ")?;
            trait_path.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
            writer.write_str("for ")?;
        }

        self.for_type.pretty_print_fmt(ctx, writer)?;

        if self.items.is_empty() {
            writer.write_str(" {}")
        } else {
            writer.write_str(" {\n")?;

            for item in &self.items {
                ctx.tab_depth += 1;

                ctx.write_indent(writer)?;
                item.pretty_print_fmt(ctx, writer)?;
                writer.write_str("\n")?;

                ctx.tab_depth -= 1;
            }

            ctx.write_indent(writer)?;
            writer.write_str("}")
        }
    }
}

impl PrettyPrint for Mutability {
    fn pretty_print_fmt(
        &self,
        _ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            Mutability::Mut => writer.write_str("mut"),
            Mutability::Const => writer.write_str("const"),
        }
    }
}

impl PrettyPrint for FuncParam {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if let Some(mutability) = &self.mutability {
            mutability.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        writer.write_str(": ")?;
        self.ty.pretty_print_fmt(ctx, writer)?;

        if let Some(default_value) = &self.default_value {
            writer.write_str(" = ")?;
            default_value.pretty_print_fmt(ctx, writer)?;
        }

        Ok(())
    }
}

impl PrettyPrint for FuncParams {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('(')?;
        for (i, param) in self.iter().enumerate() {
            if i > 0 {
                writer.write_str(", ")?;
            }

            param.pretty_print_fmt(ctx, writer)?;
        }
        writer.write_char(')')
    }
}

impl PrettyPrint for Function {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if ctx.show_resolution_links {
            write_resolve_link(writer, self)?;
        }

        writer.write_str("fn ")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        if let Some(generics) = &self.generics {
            generics.pretty_print_fmt(ctx, writer)?;
        }

        self.parameters.pretty_print_fmt(ctx, writer)?;

        if let Some(return_type) = &self.return_type {
            writer.write_str(" -> ")?;
            return_type.pretty_print_fmt(ctx, writer)?;
        }

        if let Some(definition) = &self.definition {
            writer.write_char(' ')?;
            definition.pretty_print_fmt(ctx, writer)?;
        } else {
            writer.write_str(";")?;
        }

        Ok(())
    }
}

impl PrettyPrint for GlobalVariable {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(visibility) = &self.visibility {
            visibility.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if ctx.show_resolution_links {
            write_resolve_link(writer, self)?;
        }

        match self.kind {
            GlobalVariableKind::Const => writer.write_str("const ")?,
            GlobalVariableKind::Static => writer.write_str("static ")?,
        }

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        if let Some(mutability) = &self.mutability {
            mutability.pretty_print_fmt(ctx, writer)?;
            writer.write_char(' ')?;
        }

        writer.write_str(&self.name)?;

        if let Some(var_type) = &self.ty {
            writer.write_str(": ")?;
            var_type.pretty_print_fmt(ctx, writer)?;
        }

        if let Some(initializer) = &self.initializer {
            writer.write_str(" = ")?;
            initializer.pretty_print_fmt(ctx, writer)?;
        }

        writer.write_char(';')
    }
}

impl PrettyPrint for Item {
    fn pretty_print_fmt(
        &self,
        ctx: &mut PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            Item::SyntaxError(m) => m.pretty_print_fmt(ctx, writer),
            Item::Module(m) => m.pretty_print_fmt(ctx, writer),
            Item::Import(m) => m.pretty_print_fmt(ctx, writer),
            Item::TypeAlias(m) => m.pretty_print_fmt(ctx, writer),
            Item::Struct(m) => m.pretty_print_fmt(ctx, writer),
            Item::Enum(m) => m.pretty_print_fmt(ctx, writer),
            Item::Trait(m) => m.pretty_print_fmt(ctx, writer),
            Item::Impl(m) => m.pretty_print_fmt(ctx, writer),
            Item::Function(m) => m.pretty_print_fmt(ctx, writer),
            Item::Variable(m) => m.pretty_print_fmt(ctx, writer),
        }
    }
}
