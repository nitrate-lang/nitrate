use crate::expression::Expr;
use crate::expression::VariableKind;
use crate::kind::Path;
use crate::types::Type;
use interned_string::IString;
use nitrate_tokenize::{IntegerKind, NotNan};

use crate::builder_helper::{
    AwaitBuilder, BinExprBuilder, BlockBuilder, BreakBuilder, CallBuilder, ContinueBuilder,
    DoWhileLoopBuilder, ForEachBuilder, FunctionBuilder, IfBuilder, IndexAccessBuilder,
    IntegerBuilder, ListBuilder, ObjectBuilder, ReturnBuilder, SwitchBuilder, UnaryExprBuilder,
    VariableBuilder, WhileLoopBuilder,
};

#[derive(Debug, Default)]
pub struct Builder {}

impl Builder {
    #[must_use]
    pub fn new() -> Self {
        Builder {}
    }

    #[must_use]
    pub fn get_discard() -> Expr {
        Expr::Discard
    }

    #[must_use]
    pub fn create_boolean(value: bool) -> Expr {
        Expr::Boolean(value)
    }

    #[must_use]
    pub fn create_integer(x: u128, kind: IntegerKind) -> Expr {
        IntegerBuilder::new().with_u128(x).with_kind(kind).build()
    }

    #[must_use]
    pub fn create_float(x: NotNan<f64>) -> Expr {
        Expr::Float(x)
    }

    #[must_use]
    pub fn create_string(str: IString) -> Expr {
        Expr::String(str)
    }

    #[must_use]
    pub fn create_bstring(bytes: Vec<u8>) -> Expr {
        Expr::BString(Box::new(bytes))
    }

    #[must_use]
    pub fn create_type_info(inner: Type) -> Expr {
        Expr::TypeInfo(inner)
    }

    #[must_use]
    pub fn create_list() -> ListBuilder {
        ListBuilder::new()
    }

    #[must_use]
    pub fn create_unit() -> Expr {
        Expr::Unit
    }

    #[must_use]
    pub fn create_object() -> ObjectBuilder {
        ObjectBuilder::new()
    }

    #[must_use]
    pub fn create_unary_expr() -> UnaryExprBuilder {
        UnaryExprBuilder::new()
    }

    #[must_use]
    pub fn create_binexpr() -> BinExprBuilder {
        BinExprBuilder::new()
    }

    #[must_use]
    pub fn create_block() -> BlockBuilder {
        BlockBuilder::new()
    }

    #[must_use]
    pub fn create_function() -> FunctionBuilder {
        FunctionBuilder::new()
    }

    #[must_use]
    pub fn create_variable() -> VariableBuilder {
        VariableBuilder::new()
    }

    #[must_use]
    pub fn create_identifier(path: Vec<IString>) -> Expr {
        Expr::Path(Path { path: path.into() })
    }

    #[must_use]
    pub fn create_index_access() -> IndexAccessBuilder {
        IndexAccessBuilder::new()
    }

    #[must_use]
    pub fn create_let() -> VariableBuilder {
        Self::create_variable().with_kind(VariableKind::Let)
    }

    #[must_use]
    pub fn create_var() -> VariableBuilder {
        Self::create_variable().with_kind(VariableKind::Var)
    }

    #[must_use]
    pub fn create_if() -> IfBuilder {
        IfBuilder::new()
    }

    #[must_use]
    pub fn create_while_loop() -> WhileLoopBuilder {
        WhileLoopBuilder::new()
    }

    #[must_use]
    pub fn create_do_while_loop() -> DoWhileLoopBuilder {
        DoWhileLoopBuilder::new()
    }

    #[must_use]
    pub fn create_switch() -> SwitchBuilder {
        SwitchBuilder::new()
    }

    #[must_use]
    pub fn create_break() -> BreakBuilder {
        BreakBuilder::new()
    }

    #[must_use]
    pub fn create_continue() -> ContinueBuilder {
        ContinueBuilder::new()
    }

    #[must_use]
    pub fn create_return() -> ReturnBuilder {
        ReturnBuilder::new()
    }

    #[must_use]
    pub fn create_for_each() -> ForEachBuilder {
        ForEachBuilder::new()
    }

    #[must_use]
    pub fn create_await() -> AwaitBuilder {
        AwaitBuilder::new()
    }

    #[must_use]
    pub fn create_call() -> CallBuilder {
        CallBuilder::new()
    }
}
