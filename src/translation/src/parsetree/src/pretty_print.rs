use std::num::NonZeroUsize;

use nitrate_tokenize::IntegerKind;

use crate::{
    expr::{
        AttributeList, Await, BStringLit, BinExpr, BinExprOp, Block, BlockItem, BooleanLit, Break,
        Call, CallArgument, Cast, Closure, Continue, Expr, ExprParentheses, ExprPath,
        ExprSyntaxError, FloatLit, ForEach, If, IndexAccess, IntegerLit, List, Return, Safety,
        StringLit, StructInit, Switch, TypeInfo, UnaryExpr, UnaryExprOp, WhileLoop,
    },
    item::{
        Enum, FuncParam, FuncParams, Function, Impl, Import, Item, ItemSyntaxError, Module, Struct,
        Trait, TypeAlias, Variable, Visibility,
    },
    ty::{
        ArrayType, Bool, Float8, Float16, Float32, Float64, Float128, FunctionType, InferType,
        Int8, Int16, Int32, Int64, Int128, LatentType, Lifetime, OpaqueType, ReferenceType,
        RefinementType, SliceType, TupleType, Type, TypeParentheses, TypePath, TypeSyntaxError,
        UInt8, UInt16, UInt32, UInt64, UInt128, UnitType,
    },
};

pub struct PrintContext {
    pub indent: String,
    pub max_line_length: NonZeroUsize,
}

impl Default for PrintContext {
    fn default() -> Self {
        PrintContext {
            indent: "  ".to_string(),
            max_line_length: NonZeroUsize::new(80).unwrap(),
        }
    }
}

pub trait PrettyPrint {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result;

    fn pretty_print(&self, ctx: &PrintContext) -> Result<String, std::fmt::Error> {
        let mut output = String::new();
        self.pretty_print_fmt(ctx, &mut output)?;
        Ok(output)
    }
}

impl PrettyPrint for ExprSyntaxError {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // Expression syntax errors are unrepresentable
        Ok(())
    }
}

impl PrettyPrint for ExprParentheses {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
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
        _ctx: &PrintContext,
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
        _ctx: &PrintContext,
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
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        write!(writer, "{}", self.value)
    }
}

impl PrettyPrint for StringLit {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Test string escape logic

        writer.write_char('"')?;

        for c in self.value.chars() {
            if c.is_ascii() {
                match c {
                    '\0' => writer.write_str("\\0")?,
                    '\t' => writer.write_str("\\t")?,
                    '\n' => writer.write_str("\\n")?,
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
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('"')?;

        for byte in &self.value {
            match byte {
                b'\0' => writer.write_str("\\0")?,
                b'\t' => writer.write_str("\\t")?,
                b'\n' => writer.write_str("\\n")?,
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
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("type")?;
        self.the.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for List {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
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

impl PrettyPrint for StructInit {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Object pretty print
        Ok(())
    }
}

impl PrettyPrint for UnaryExprOp {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            UnaryExprOp::Add => writer.write_str("+"),
            UnaryExprOp::Sub => writer.write_str("-"),
            UnaryExprOp::Deref => writer.write_str("*"),
            UnaryExprOp::AddressOf => writer.write_str("&"),
            UnaryExprOp::BitNot => writer.write_str("~"),
            UnaryExprOp::LogicNot => writer.write_str("!"),
            UnaryExprOp::Typeof => writer.write_str("typeof"),
        }
    }
}

impl PrettyPrint for UnaryExpr {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.operator.pretty_print_fmt(_ctx, writer)?;
        self.operand.pretty_print_fmt(_ctx, writer)
    }
}

impl PrettyPrint for BinExprOp {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
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
            BinExprOp::LogicXor => writer.write_str("^^"),
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
            BinExprOp::SetLogicXor => writer.write_str("^^="),
            BinExprOp::Dot => writer.write_str("."),
            BinExprOp::Arrow => writer.write_str("->"),
            BinExprOp::Range => writer.write_str(".."),
        }
    }
}

impl PrettyPrint for BinExpr {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.left.pretty_print_fmt(_ctx, writer)?;

        writer.write_char(' ')?;
        self.operator.pretty_print_fmt(_ctx, writer)?;
        writer.write_char(' ')?;

        self.right.pretty_print_fmt(_ctx, writer)
    }
}

impl PrettyPrint for Cast {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.value.pretty_print_fmt(ctx, writer)?;
        writer.write_str(" as ")?;
        self.to.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for BlockItem {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            BlockItem::Variable(m) => m.read().unwrap().pretty_print_fmt(ctx, writer),

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
        ctx: &PrintContext,
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

        writer.write_char('{')?;

        for item in &self.elements {
            item.pretty_print_fmt(ctx, writer)?;
        }

        writer.write_char('}')
    }
}

impl PrettyPrint for AttributeList {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_char('[')?;
        for (i, attr) in self.elements.iter().enumerate() {
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
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("fn")?;

        if let Some(attributes) = &self.attributes {
            attributes.pretty_print_fmt(ctx, writer)?;
        }

        writer.write_char('(')?;
        for (i, param) in self.parameters.iter().enumerate() {
            if i > 0 {
                writer.write_str(", ")?;
            }
            param.pretty_print_fmt(ctx, writer)?;
        }
        writer.write_char(')')?;

        if let Some(return_type) = &self.return_type {
            writer.write_str(" -> ")?;
            return_type.pretty_print_fmt(ctx, writer)?;
        }

        self.definition.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for ExprPath {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Path pretty print
        Ok(())
    }
}

impl PrettyPrint for IndexAccess {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
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
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("if ")?;
        self.condition.pretty_print_fmt(ctx, writer)?;
        writer.write_str(" ")?;

        self.then_branch.pretty_print_fmt(ctx, writer)?;

        if let Some(else_branch) = &self.else_branch {
            writer.write_str(" else ")?;
            else_branch.pretty_print_fmt(ctx, writer)?;
        }

        Ok(())
    }
}

impl PrettyPrint for WhileLoop {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("while ")?;
        self.condition.pretty_print_fmt(ctx, writer)?;
        writer.write_str(" ")?;

        self.body.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for Switch {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Switch pretty print
        Ok(())
    }
}

impl PrettyPrint for Break {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
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
        _ctx: &PrintContext,
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
        ctx: &PrintContext,
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
        _ctx: &PrintContext,
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: For pretty print
        Ok(())
    }
}

impl PrettyPrint for Await {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("await ")?;
        self.future.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for CallArgument {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        if let Some(name) = &self.name {
            writer.write_str(name)?;
            writer.write_str(": ")?;
        }

        self.value.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for Call {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
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

impl PrettyPrint for Expr {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
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
            Expr::StructInit(m) => m.pretty_print_fmt(ctx, writer),
            Expr::UnaryExpr(m) => m.pretty_print_fmt(ctx, writer),
            Expr::BinExpr(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Cast(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Block(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Closure(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Variable(m) => m.read().unwrap().pretty_print_fmt(ctx, writer),
            Expr::Path(m) => m.pretty_print_fmt(ctx, writer),
            Expr::IndexAccess(m) => m.pretty_print_fmt(ctx, writer),
            Expr::If(m) => m.pretty_print_fmt(ctx, writer),
            Expr::While(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Switch(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Break(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Continue(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Return(m) => m.pretty_print_fmt(ctx, writer),
            Expr::For(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Await(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Call(m) => m.pretty_print_fmt(ctx, writer),
        }
    }
}

impl PrettyPrint for TypeSyntaxError {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // Type syntax errors are unrepresentable
        Ok(())
    }
}

impl PrettyPrint for Bool {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("bool")
    }
}

impl PrettyPrint for UInt8 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u8")
    }
}

impl PrettyPrint for UInt16 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u16")
    }
}

impl PrettyPrint for UInt32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u32")
    }
}

impl PrettyPrint for UInt64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u64")
    }
}

impl PrettyPrint for UInt128 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("u128")
    }
}

impl PrettyPrint for Int8 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i8")
    }
}

impl PrettyPrint for Int16 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i16")
    }
}

impl PrettyPrint for Int32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i32")
    }
}

impl PrettyPrint for Int64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i64")
    }
}

impl PrettyPrint for Int128 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("i128")
    }
}

impl PrettyPrint for Float8 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("f8")
    }
}

impl PrettyPrint for Float16 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("f16")
    }
}

impl PrettyPrint for Float32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("f32")
    }
}

impl PrettyPrint for Float64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("f64")
    }
}

impl PrettyPrint for Float128 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("f128")
    }
}

impl PrettyPrint for UnitType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("()")
    }
}

impl PrettyPrint for InferType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("_")
    }
}

impl PrettyPrint for TypePath {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: TypePath pretty print
        Ok(())
    }
}

impl PrettyPrint for RefinementType {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
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
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("(")?;

        for element in &self.element_types {
            element.pretty_print_fmt(ctx, writer)?;
            writer.write_str(", ")?;
        }

        writer.write_char(')')
    }
}

impl PrettyPrint for ArrayType {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
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
        ctx: &PrintContext,
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
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: FunctionType pretty print
        Ok(())
    }
}

impl PrettyPrint for ReferenceType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: ReferenceType pretty print
        Ok(())
    }
}

impl PrettyPrint for OpaqueType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        write!(writer, "opaque(\"{}\")", self.name)
    }
}

impl PrettyPrint for LatentType {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        self.body.pretty_print_fmt(ctx, writer)
    }
}

impl PrettyPrint for Lifetime {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Lifetime pretty print
        Ok(())
    }
}

impl PrettyPrint for TypeParentheses {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
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
        ctx: &PrintContext,
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
            Type::Int8(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int16(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int32(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int64(m) => m.pretty_print_fmt(ctx, writer),
            Type::Int128(m) => m.pretty_print_fmt(ctx, writer),
            Type::Float8(m) => m.pretty_print_fmt(ctx, writer),
            Type::Float16(m) => m.pretty_print_fmt(ctx, writer),
            Type::Float32(m) => m.pretty_print_fmt(ctx, writer),
            Type::Float64(m) => m.pretty_print_fmt(ctx, writer),
            Type::Float128(m) => m.pretty_print_fmt(ctx, writer),
            Type::UnitType(m) => m.pretty_print_fmt(ctx, writer),
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
        _ctx: &PrintContext,
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
        _ctx: &PrintContext,
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // Item syntax errors are unrepresentable
        Ok(())
    }
}

impl PrettyPrint for Module {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Module pretty print
        Ok(())
    }
}

impl PrettyPrint for Import {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Import pretty print
        Ok(())
    }
}

impl PrettyPrint for TypeAlias {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: TypeAlias pretty print
        Ok(())
    }
}

impl PrettyPrint for Struct {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Struct pretty print
        Ok(())
    }
}

impl PrettyPrint for Enum {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: impl pretty print
        Ok(())
    }
}

impl PrettyPrint for Trait {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Trait pretty print
        Ok(())
    }
}

impl PrettyPrint for Impl {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: impl pretty print
        Ok(())
    }
}

impl PrettyPrint for FuncParam {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Function pretty print
        Ok(())
    }
}

impl PrettyPrint for FuncParams {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Function pretty print
        Ok(())
    }
}

impl PrettyPrint for Function {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Function pretty print
        Ok(())
    }
}

impl PrettyPrint for Variable {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Variable pretty print
        Ok(())
    }
}

impl PrettyPrint for Item {
    fn pretty_print_fmt(
        &self,
        ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        match self {
            Item::SyntaxError(m) => m.pretty_print_fmt(ctx, writer),
            Item::Module(m) => m.pretty_print_fmt(ctx, writer),
            Item::Import(m) => m.pretty_print_fmt(ctx, writer),
            Item::TypeAlias(m) => m.read().unwrap().pretty_print_fmt(ctx, writer),
            Item::Struct(m) => m.read().unwrap().pretty_print_fmt(ctx, writer),
            Item::Enum(m) => m.read().unwrap().pretty_print_fmt(ctx, writer),
            Item::Trait(m) => m.read().unwrap().pretty_print_fmt(ctx, writer),
            Item::Impl(m) => m.pretty_print_fmt(ctx, writer),
            Item::Function(m) => m.read().unwrap().pretty_print_fmt(ctx, writer),
            Item::Variable(m) => m.read().unwrap().pretty_print_fmt(ctx, writer),
        }
    }
}
