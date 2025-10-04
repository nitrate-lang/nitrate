use std::num::NonZeroUsize;

use nitrate_tokenize::IntegerKind;
use serde::de::value;

use crate::{
    expr::{
        Await, BStringLit, BinExpr, BinExprOp, Block, BooleanLit, Break, Call, Cast, Closure,
        Continue, DoWhileLoop, Expr, ExprParentheses, ExprPath, ExprSyntaxError, FloatLit, ForEach,
        If, IndexAccess, IntegerLit, List, Object, Return, StringLit, Switch, TypeInfo, UnaryExpr,
        UnaryExprOp, UnitLit, WhileLoop,
    },
    item::{
        Enum, Function, Impl, Import, Item, ItemSyntaxError, Module, Struct, Trait, TypeAlias,
        Variable,
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

    current_indent: String,
    current_line_length: usize,
}

impl Default for PrintContext {
    fn default() -> Self {
        PrintContext {
            indent: "  ".to_string(),
            max_line_length: NonZeroUsize::new(80).unwrap(),
            current_indent: "".to_string(),
            current_line_length: 0,
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
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: String pretty print
        Ok(())
    }
}

impl PrettyPrint for BStringLit {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        _writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: BString pretty print
        Ok(())
    }
}

impl PrettyPrint for UnitLit {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        writer.write_str("()")
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

impl PrettyPrint for Object {
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
        // TODO: UnaryExprOp pretty print
        Ok(())
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
        // TODO: BinExprOp pretty print
        Ok(())
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

impl PrettyPrint for Block {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Block pretty print
        Ok(())
    }
}

impl PrettyPrint for Closure {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Closure pretty print
        Ok(())
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
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: IndexAccess pretty print
        Ok(())
    }
}

impl PrettyPrint for If {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: If pretty print
        Ok(())
    }
}

impl PrettyPrint for WhileLoop {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: While pretty print
        Ok(())
    }
}

impl PrettyPrint for DoWhileLoop {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: DoWhileLoop pretty print
        Ok(())
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
        // TODO: Break pretty print
        Ok(())
    }
}

impl PrettyPrint for Continue {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Continue pretty print
        Ok(())
    }
}

impl PrettyPrint for Return {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Return pretty print
        Ok(())
    }
}

impl PrettyPrint for ForEach {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: For pretty print
        Ok(())
    }
}

impl PrettyPrint for Await {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Await pretty print
        Ok(())
    }
}

impl PrettyPrint for Call {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Call pretty print
        Ok(())
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
            Expr::Unit(m) => m.pretty_print_fmt(ctx, writer),
            Expr::TypeInfo(m) => m.pretty_print_fmt(ctx, writer),
            Expr::List(m) => m.pretty_print_fmt(ctx, writer),
            Expr::Object(m) => m.pretty_print_fmt(ctx, writer),
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
            Expr::DoWhileLoop(m) => m.pretty_print_fmt(ctx, writer),
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
        // TODO: SyntaxError pretty print
        Ok(())
    }
}

impl PrettyPrint for Bool {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Bool pretty print
        Ok(())
    }
}

impl PrettyPrint for UInt8 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: UInt8 pretty print
        Ok(())
    }
}

impl PrettyPrint for UInt16 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: UInt16 pretty print
        Ok(())
    }
}

impl PrettyPrint for UInt32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: UInt32 pretty print
        Ok(())
    }
}

impl PrettyPrint for UInt64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: UInt64 pretty print
        Ok(())
    }
}

impl PrettyPrint for UInt128 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: UInt128 pretty print
        Ok(())
    }
}

impl PrettyPrint for Int8 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Int8 pretty print
        Ok(())
    }
}

impl PrettyPrint for Int16 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Int16 pretty print
        Ok(())
    }
}

impl PrettyPrint for Int32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Int32 pretty print
        Ok(())
    }
}

impl PrettyPrint for Int64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Int64 pretty print
        Ok(())
    }
}

impl PrettyPrint for Int128 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Int128 pretty print
        Ok(())
    }
}

impl PrettyPrint for Float8 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Float8 pretty print
        Ok(())
    }
}

impl PrettyPrint for Float16 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Float16 pretty print
        Ok(())
    }
}

impl PrettyPrint for Float32 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Float32 pretty print
        Ok(())
    }
}

impl PrettyPrint for Float64 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Float64 pretty print
        Ok(())
    }
}

impl PrettyPrint for Float128 {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Float128 pretty print
        Ok(())
    }
}

impl PrettyPrint for UnitType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: UnitType pretty print
        Ok(())
    }
}

impl PrettyPrint for InferType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: InferType pretty print
        Ok(())
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
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: RefinementType pretty print
        Ok(())
    }
}

impl PrettyPrint for TupleType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: TupleType pretty print
        Ok(())
    }
}

impl PrettyPrint for ArrayType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: ArrayType pretty print
        Ok(())
    }
}

impl PrettyPrint for SliceType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: SliceType pretty print
        Ok(())
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
        // TODO: OpaqueType pretty print
        Ok(())
    }
}

impl PrettyPrint for LatentType {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: LatentType pretty print
        Ok(())
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
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: Parentheses pretty print
        Ok(())
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

impl PrettyPrint for ItemSyntaxError {
    fn pretty_print_fmt(
        &self,
        _ctx: &PrintContext,
        writer: &mut dyn std::fmt::Write,
    ) -> std::fmt::Result {
        // TODO: SyntaxError pretty print
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
