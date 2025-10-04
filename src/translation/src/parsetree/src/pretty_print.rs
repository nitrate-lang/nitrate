use crate::{
    expr::{
        Await, BStringLit, BinExpr, Block, BooleanLit, Break, Call, Cast, Closure, Continue,
        DoWhileLoop, Expr, ExprParentheses, ExprPath, ExprSyntaxError, FloatLit, ForEach, If,
        IndexAccess, IntegerLit, List, Object, Return, StringLit, Switch, TypeInfo, UnaryExpr,
        UnitLit, WhileLoop,
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

pub struct StyleOptions {
    pub indent: String,
    pub max_line_length: usize,
}

pub trait PrettyPrint {
    fn pretty_print(&self, _options: &StyleOptions) -> String;
}

impl PrettyPrint for ExprSyntaxError {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: SyntaxError pretty print
        String::new()
    }
}

impl PrettyPrint for ExprParentheses {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Parentheses pretty print
        String::new()
    }
}

impl PrettyPrint for BooleanLit {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Boolean pretty print
        String::new()
    }
}

impl PrettyPrint for IntegerLit {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Integer pretty print
        String::new()
    }
}

impl PrettyPrint for FloatLit {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Float pretty print
        String::new()
    }
}

impl PrettyPrint for StringLit {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: String pretty print
        String::new()
    }
}

impl PrettyPrint for BStringLit {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: BString pretty print
        String::new()
    }
}

impl PrettyPrint for UnitLit {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Unit pretty print
        String::new()
    }
}

impl PrettyPrint for TypeInfo {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: TypeInfo pretty print
        String::new()
    }
}

impl PrettyPrint for List {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: List pretty print
        String::new()
    }
}

impl PrettyPrint for Object {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Object pretty print
        String::new()
    }
}

impl PrettyPrint for UnaryExpr {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: UnaryExpr pretty print
        String::new()
    }
}

impl PrettyPrint for BinExpr {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: BinExpr pretty print
        String::new()
    }
}

impl PrettyPrint for Cast {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Cast pretty print
        String::new()
    }
}

impl PrettyPrint for Block {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Block pretty print
        String::new()
    }
}

impl PrettyPrint for Closure {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Closure pretty print
        String::new()
    }
}

impl PrettyPrint for ExprPath {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Path pretty print
        String::new()
    }
}

impl PrettyPrint for IndexAccess {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: IndexAccess pretty print
        String::new()
    }
}

impl PrettyPrint for If {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: If pretty print
        String::new()
    }
}

impl PrettyPrint for WhileLoop {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: While pretty print
        String::new()
    }
}

impl PrettyPrint for DoWhileLoop {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: DoWhileLoop pretty print
        String::new()
    }
}

impl PrettyPrint for Switch {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Switch pretty print
        String::new()
    }
}

impl PrettyPrint for Break {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Break pretty print
        String::new()
    }
}

impl PrettyPrint for Continue {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Continue pretty print
        String::new()
    }
}

impl PrettyPrint for Return {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Return pretty print
        String::new()
    }
}

impl PrettyPrint for ForEach {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: For pretty print
        String::new()
    }
}

impl PrettyPrint for Await {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Await pretty print
        String::new()
    }
}

impl PrettyPrint for Call {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Call pretty print
        String::new()
    }
}

impl PrettyPrint for Expr {
    fn pretty_print(&self, options: &StyleOptions) -> String {
        match self {
            Expr::SyntaxError(m) => m.pretty_print(options),
            Expr::Parentheses(m) => m.pretty_print(options),
            Expr::Boolean(m) => m.pretty_print(options),
            Expr::Integer(m) => m.pretty_print(options),
            Expr::Float(m) => m.pretty_print(options),
            Expr::String(m) => m.pretty_print(options),
            Expr::BString(m) => m.pretty_print(options),
            Expr::Unit(m) => m.pretty_print(options),
            Expr::TypeInfo(m) => m.pretty_print(options),
            Expr::List(m) => m.pretty_print(options),
            Expr::Object(m) => m.pretty_print(options),
            Expr::UnaryExpr(m) => m.pretty_print(options),
            Expr::BinExpr(m) => m.pretty_print(options),
            Expr::Cast(m) => m.pretty_print(options),
            Expr::Block(m) => m.pretty_print(options),
            Expr::Closure(m) => m.pretty_print(options),
            Expr::Variable(m) => m.read().unwrap().pretty_print(options),
            Expr::Path(m) => m.pretty_print(options),
            Expr::IndexAccess(m) => m.pretty_print(options),
            Expr::If(m) => m.pretty_print(options),
            Expr::While(m) => m.pretty_print(options),
            Expr::DoWhileLoop(m) => m.pretty_print(options),
            Expr::Switch(m) => m.pretty_print(options),
            Expr::Break(m) => m.pretty_print(options),
            Expr::Continue(m) => m.pretty_print(options),
            Expr::Return(m) => m.pretty_print(options),
            Expr::For(m) => m.pretty_print(options),
            Expr::Await(m) => m.pretty_print(options),
            Expr::Call(m) => m.pretty_print(options),
        }
    }
}

impl PrettyPrint for TypeSyntaxError {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: SyntaxError pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Bool {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Bool pretty print
        unimplemented!()
    }
}

impl PrettyPrint for UInt8 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: UInt8 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for UInt16 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: UInt16 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for UInt32 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: UInt32 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for UInt64 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: UInt64 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for UInt128 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: UInt128 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Int8 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Int8 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Int16 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Int16 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Int32 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Int32 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Int64 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Int64 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Int128 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Int128 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Float8 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Float8 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Float16 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Float16 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Float32 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Float32 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Float64 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Float64 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Float128 {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Float128 pretty print
        unimplemented!()
    }
}

impl PrettyPrint for UnitType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: UnitType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for InferType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: InferType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for TypePath {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: TypePath pretty print
        unimplemented!()
    }
}

impl PrettyPrint for RefinementType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: RefinementType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for TupleType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: TupleType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for ArrayType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: ArrayType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for SliceType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: SliceType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for FunctionType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: FunctionType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for ReferenceType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: ReferenceType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for OpaqueType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: OpaqueType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for LatentType {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: LatentType pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Lifetime {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Lifetime pretty print
        unimplemented!()
    }
}

impl PrettyPrint for TypeParentheses {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Parentheses pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Type {
    fn pretty_print(&self, options: &StyleOptions) -> String {
        match self {
            Type::SyntaxError(m) => m.pretty_print(options),
            Type::Bool(m) => m.pretty_print(options),
            Type::UInt8(m) => m.pretty_print(options),
            Type::UInt16(m) => m.pretty_print(options),
            Type::UInt32(m) => m.pretty_print(options),
            Type::UInt64(m) => m.pretty_print(options),
            Type::UInt128(m) => m.pretty_print(options),
            Type::Int8(m) => m.pretty_print(options),
            Type::Int16(m) => m.pretty_print(options),
            Type::Int32(m) => m.pretty_print(options),
            Type::Int64(m) => m.pretty_print(options),
            Type::Int128(m) => m.pretty_print(options),
            Type::Float8(m) => m.pretty_print(options),
            Type::Float16(m) => m.pretty_print(options),
            Type::Float32(m) => m.pretty_print(options),
            Type::Float64(m) => m.pretty_print(options),
            Type::Float128(m) => m.pretty_print(options),
            Type::UnitType(m) => m.pretty_print(options),
            Type::InferType(m) => m.pretty_print(options),
            Type::TypePath(m) => m.pretty_print(options),
            Type::RefinementType(m) => m.pretty_print(options),
            Type::TupleType(m) => m.pretty_print(options),
            Type::ArrayType(m) => m.pretty_print(options),
            Type::SliceType(m) => m.pretty_print(options),
            Type::FunctionType(m) => m.pretty_print(options),
            Type::ReferenceType(m) => m.pretty_print(options),
            Type::OpaqueType(m) => m.pretty_print(options),
            Type::LatentType(m) => m.pretty_print(options),
            Type::Lifetime(m) => m.pretty_print(options),
            Type::Parentheses(m) => m.pretty_print(options),
        }
    }
}

impl PrettyPrint for ItemSyntaxError {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: SyntaxError pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Module {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Module pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Import {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Import pretty print
        unimplemented!()
    }
}

impl PrettyPrint for TypeAlias {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: TypeAlias pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Struct {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Struct pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Enum {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: impl pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Trait {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Trait pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Impl {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: impl pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Function {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Function pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Variable {
    fn pretty_print(&self, _options: &StyleOptions) -> String {
        // TODO: Variable pretty print
        unimplemented!()
    }
}

impl PrettyPrint for Item {
    fn pretty_print(&self, options: &StyleOptions) -> String {
        match self {
            Item::SyntaxError(m) => m.pretty_print(options),
            Item::Module(m) => m.pretty_print(options),
            Item::Import(m) => m.pretty_print(options),
            Item::TypeAlias(m) => m.read().unwrap().pretty_print(options),
            Item::Struct(m) => m.read().unwrap().pretty_print(options),
            Item::Enum(m) => m.read().unwrap().pretty_print(options),
            Item::Trait(m) => m.read().unwrap().pretty_print(options),
            Item::Impl(m) => m.pretty_print(options),
            Item::Function(m) => m.read().unwrap().pretty_print(options),
            Item::Variable(m) => m.read().unwrap().pretty_print(options),
        }
    }
}
