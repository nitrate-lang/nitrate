use super::node::{Expr, Type};
use crate::lexical::IntegerKind;
use apint::UInt;
use smallvec::SmallVec;
use std::collections::BTreeMap;
use std::sync::Arc;

#[derive(Debug, Clone, PartialEq)]
pub struct IntegerLit {
    value: UInt,
    kind: IntegerKind,
}

impl IntegerLit {
    #[must_use]
    pub(crate) fn new(value: UInt, kind: IntegerKind) -> Option<Self> {
        if value.try_to_u128().is_ok() {
            Some(IntegerLit { value, kind })
        } else {
            None
        }
    }

    #[must_use]
    pub fn into_inner(self) -> UInt {
        self.value
    }

    #[must_use]
    pub fn get(&self) -> &UInt {
        &self.value
    }

    #[must_use]
    pub fn get_u128(&self) -> u128 {
        self.value
            .try_to_u128()
            .expect("IntegerLit value should fit in u128")
    }

    #[must_use]
    pub fn kind(&self) -> IntegerKind {
        self.kind
    }
}

impl std::ops::Deref for IntegerLit {
    type Target = UInt;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct ListLit<'a> {
    elements: Vec<Arc<Expr<'a>>>,
}

impl<'a> ListLit<'a> {
    #[must_use]
    pub(crate) fn new(elements: Vec<Arc<Expr<'a>>>) -> Self {
        ListLit { elements }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<Arc<Expr<'a>>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[Arc<Expr<'a>>] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<Arc<Expr<'a>>> {
        &mut self.elements
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct ObjectLit<'a> {
    fields: BTreeMap<&'a str, Arc<Expr<'a>>>,
}

impl<'a> ObjectLit<'a> {
    #[must_use]
    pub(crate) fn new(fields: BTreeMap<&'a str, Arc<Expr<'a>>>) -> Self {
        ObjectLit { fields }
    }

    #[must_use]
    pub fn into_inner(self) -> BTreeMap<&'a str, Arc<Expr<'a>>> {
        self.fields
    }

    #[must_use]
    pub fn get(&self) -> &BTreeMap<&'a str, Arc<Expr<'a>>> {
        &self.fields
    }

    #[must_use]
    pub fn get_mut(&mut self) -> &mut BTreeMap<&'a str, Arc<Expr<'a>>> {
        &mut self.fields
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum UnaryExprOp {
    /*----------------------------------------------------------------*
     * Arithmetic Operators                                           *
     *----------------------------------------------------------------*/
    Add, /* '+': "Addition Operator" */
    Sub, /* '-': "Subtraction Operator" */
    Mul, /* '*': "Multiplication Operator" */

    /*----------------------------------------------------------------*
     * Bitwise Operators                                              *
     *----------------------------------------------------------------*/
    BitAnd, /* '&':   "Bitwise AND Operator" */
    BitNot, /* '~':   "Bitwise NOT Operator" */

    /*----------------------------------------------------------------*
     * Logical Operators                                              *
     *----------------------------------------------------------------*/
    LogicNot, /* '!':  "Logical NOT Operator" */

    /*----------------------------------------------------------------*
     * Assignment Operators                                           *
     *----------------------------------------------------------------*/
    Inc, /* '++':   "Increment Operator" */
    Dec, /* '--':   "Decrement Operator" */

    /*----------------------------------------------------------------*
     * Type System Operators                                          *
     *----------------------------------------------------------------*/
    Sizeof,  /* 'sizeof':     "Size Of Operator" */
    Alignof, /* 'alignof':    "Alignment Of Operator" */
    Typeof,  /* 'typeof':     "Type Of Operator" */

    /*----------------------------------------------------------------*
     * Special Operators                                              *
     *----------------------------------------------------------------*/
    Question, /* '?':          "Ternary Operator" */
}

#[derive(Debug, Clone, PartialEq)]
pub struct UnaryExpr<'a> {
    operand: Arc<Expr<'a>>,
    operator: UnaryExprOp,
    is_postfix: bool,
}

impl<'a> UnaryExpr<'a> {
    #[must_use]
    pub(crate) fn new(operand: Arc<Expr<'a>>, operator: UnaryExprOp, is_postfix: bool) -> Self {
        UnaryExpr {
            operand,
            operator,
            is_postfix,
        }
    }

    #[must_use]
    pub fn operand(&self) -> Arc<Expr<'a>> {
        self.operand.clone()
    }

    #[must_use]
    pub fn operator(&self) -> UnaryExprOp {
        self.operator
    }

    #[must_use]
    pub fn is_postfix(&self) -> bool {
        self.is_postfix
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum BinExprOp {
    /*----------------------------------------------------------------*
     * Arithmetic Operators                                           *
     *----------------------------------------------------------------*/
    Add, /* '+': "Addition Operator" */
    Sub, /* '-': "Subtraction Operator" */
    Mul, /* '*': "Multiplication Operator" */
    Div, /* '/': "Division Operator" */
    Mod, /* '%': "Modulus Operator" */

    /*----------------------------------------------------------------*
     * Bitwise Operators                                              *
     *----------------------------------------------------------------*/
    BitAnd,  /* '&':   "Bitwise AND Operator" */
    BitOr,   /* '|':   "Bitwise OR Operator" */
    BitXor,  /* '^':   "Bitwise XOR Operator" */
    BitShl,  /* '<<':  "Bitwise Left-Shift Operator" */
    BitShr,  /* '>>':  "Bitwise Right-Shift Operator" */
    BitRotl, /* '<<<': "Bitwise Left-Rotate Operator" */
    BitRotr, /* '>>>': "Bitwise Right-Rotate Operator" */

    /*----------------------------------------------------------------*
     * Logical Operators                                              *
     *----------------------------------------------------------------*/
    LogicAnd, /* '&&': "Logical AND Operator" */
    LogicOr,  /* '||': "Logical OR Operator" */
    LogicXor, /* '^^': "Logical XOR Operator" */
    LogicLt,  /* '<':  "Logical Less-Than Operator" */
    LogicGt,  /* '>':  "Logical Greater-Than Operator" */
    LogicLe,  /* '<=': "Logical Less-Than or Equal-To Operator" */
    LogicGe,  /* '>=': "Logical Greater-Than or Equal-To Operator" */
    LogicEq,  /* '==': "Logical Equal-To Operator" */
    LogicNe,  /* '!=': "Logical Not Equal-To Operator" */

    /*----------------------------------------------------------------*
     * Assignment Operators                                           *
     *----------------------------------------------------------------*/
    Set,         /* '=':    "Assignment Operator" */
    SetPlus,     /* '+=':   "Addition Assignment Operator" */
    SetMinus,    /* '-=':   "Subtraction Assignment Operator" */
    SetTimes,    /* '*=':   "Multiplication Assignment Operator" */
    SetSlash,    /* '/=':   "Division Assignment Operator" */
    SetPercent,  /* '%=':   "Modulus Assignment Operator" */
    SetBitAnd,   /* '&=':   "Bitwise AND Assignment Operator" */
    SetBitOr,    /* '|=':   "Bitwise OR Assignment Operator" */
    SetBitXor,   /* '^=':   "Bitwise XOR Assignment Operator" */
    SetBitShl,   /* '<<=':  "Bitwise Left-Shift Assignment Operator" */
    SetBitShr,   /* '>>=':  "Bitwise Right-Shift Assignment Operator" */
    SetBitRotl,  /* '<<<=': "Bitwise Rotate-Left Assignment Operator" */
    SetBitRotr,  /* '>>>=': "Bitwise Rotate-Right Assignment Operator" */
    SetLogicAnd, /* '&&=':  "Logical AND Assignment Operator" */
    SetLogicOr,  /* '||=':  "Logical OR Assignment Operator" */
    SetLogicXor, /* '^^=':  "Logical XOR Assignment Operator" */

    /*----------------------------------------------------------------*
     * Type System Operators                                          *
     *----------------------------------------------------------------*/
    As, /* 'as':         "Type Cast Operator" */

    /*----------------------------------------------------------------*
     * Syntactic Operators                                            *
     *----------------------------------------------------------------*/
    Dot,        /* '.':          "Dot Operator" */
    Ellipsis,   /* '...':        "Ellipsis Operator" */
    Scope,      /* '::':         "Scope Resolution Operator" */
    Arrow,      /* '->':         "Arrow Operator" */
    BlockArrow, /* '=>':         "Block Arrow Operator" */

    /*----------------------------------------------------------------*
     * Special Operators                                              *
     *----------------------------------------------------------------*/
    Range,     /* '..':         "Range Operator" */
    Question,  /* '?':          "Ternary Operator" */
    Spaceship, /* '<=>':        "Spaceship Operator" */
}

#[derive(Debug, Clone, PartialEq)]
pub struct BinExpr<'a> {
    left: Arc<Expr<'a>>,
    right: Arc<Expr<'a>>,
    operator: BinExprOp,
}

impl<'a> BinExpr<'a> {
    #[must_use]
    pub(crate) fn new(left: Arc<Expr<'a>>, operator: BinExprOp, right: Arc<Expr<'a>>) -> Self {
        BinExpr {
            left,
            right,
            operator,
        }
    }

    #[must_use]
    pub fn left(&self) -> Arc<Expr<'a>> {
        self.left.clone()
    }

    #[must_use]
    pub fn op(&self) -> BinExprOp {
        self.operator
    }

    #[must_use]
    pub fn right(&self) -> Arc<Expr<'a>> {
        self.right.clone()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Statement<'a> {
    expr: Arc<Expr<'a>>,
}

impl<'a> Statement<'a> {
    #[must_use]
    pub(crate) fn new(expr: Arc<Expr<'a>>) -> Self {
        Statement { expr }
    }

    #[must_use]
    pub fn into_inner(self) -> Arc<Expr<'a>> {
        self.expr
    }

    #[must_use]
    pub fn get(&self) -> Arc<Expr<'a>> {
        self.expr.clone()
    }

    pub fn set(&mut self, expr: Arc<Expr<'a>>) {
        self.expr = expr;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Block<'a> {
    elements: Vec<Arc<Expr<'a>>>,
}

impl<'a> Block<'a> {
    #[must_use]
    pub(crate) fn new(items: Vec<Arc<Expr<'a>>>) -> Self {
        Block { elements: items }
    }

    #[must_use]
    pub fn into_inner(self) -> Vec<Arc<Expr<'a>>> {
        self.elements
    }

    #[must_use]
    pub fn elements(&self) -> &[Arc<Expr<'a>>] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<Arc<Expr<'a>>> {
        &mut self.elements
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct FunctionParameter<'a> {
    name: &'a str,
    param_type: Type<'a>,
    default_value: Option<Arc<Expr<'a>>>,
}

impl<'a> FunctionParameter<'a> {
    #[must_use]
    pub fn new(name: &'a str, param_type: Type<'a>, default_value: Option<Arc<Expr<'a>>>) -> Self {
        FunctionParameter {
            name,
            param_type,
            default_value,
        }
    }

    #[must_use]
    pub fn name(&self) -> &'a str {
        self.name
    }

    #[must_use]
    pub fn type_(&self) -> Type<'a> {
        self.param_type.clone()
    }

    #[must_use]
    pub fn default(&self) -> Option<Arc<Expr<'a>>> {
        self.default_value.clone()
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Function<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Type<'a>,
    attributes: Vec<Arc<Expr<'a>>>,
    name: &'a str,
    definition: Option<Arc<Expr<'a>>>,
}

impl<'a> Function<'a> {
    #[must_use]
    pub(crate) fn new(
        name: &'a str,
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Type<'a>,
        attributes: Vec<Arc<Expr<'a>>>,
        definition: Option<Arc<Expr<'a>>>,
    ) -> Self {
        Function {
            parameters,
            return_type,
            attributes,
            name,
            definition,
        }
    }

    #[must_use]
    pub fn parameters(&self) -> &[FunctionParameter<'a>] {
        &self.parameters
    }

    #[must_use]
    pub fn parameters_mut(&mut self) -> &mut Vec<FunctionParameter<'a>> {
        &mut self.parameters
    }

    #[must_use]
    pub fn return_type(&self) -> Type<'a> {
        self.return_type.clone()
    }

    pub fn set_return_type(&mut self, ty: Type<'a>) {
        self.return_type = ty;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<Expr<'a>>] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<Arc<Expr<'a>>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    #[must_use]
    pub fn definition(&self) -> Option<&Arc<Expr<'a>>> {
        self.definition.as_ref()
    }

    pub fn set_definition(&mut self, definition: Option<Arc<Expr<'a>>>) {
        self.definition = definition;
    }
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Variable<'a> {
    kind: VariableKind,
    is_mutable: bool,
    attributes: Vec<Arc<Expr<'a>>>,
    name: &'a str,
    var_type: Type<'a>,
    value: Option<Arc<Expr<'a>>>,
}

impl<'a> Variable<'a> {
    #[must_use]
    pub(crate) fn new(
        kind: VariableKind,
        is_mutable: bool,
        attributes: Vec<Arc<Expr<'a>>>,
        name: &'a str,
        var_type: Type<'a>,
        value: Option<Arc<Expr<'a>>>,
    ) -> Self {
        Variable {
            kind,
            is_mutable,
            attributes,
            name,
            var_type,
            value,
        }
    }

    #[must_use]
    pub fn kind(&self) -> VariableKind {
        self.kind
    }

    #[must_use]
    pub fn is_mutable(&self) -> bool {
        self.is_mutable
    }

    pub fn set_mutable(&mut self, is_mutable: bool) {
        self.is_mutable = is_mutable;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<Expr<'a>>] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<Arc<Expr<'a>>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    #[must_use]
    pub fn get_type(&self) -> Type<'a> {
        self.var_type.clone()
    }

    pub fn set_type(&mut self, var_type: Type<'a>) {
        self.var_type = var_type;
    }

    #[must_use]
    pub fn value(&self) -> Option<Arc<Expr<'a>>> {
        self.value.clone()
    }

    pub fn set_value(&mut self, value: Option<Arc<Expr<'a>>>) {
        self.value = value;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct QualifiedScope<'a> {
    parts: SmallVec<[&'a str; 8]>,
}

impl<'a> QualifiedScope<'a> {
    #[must_use]
    pub fn new(parts: SmallVec<[&'a str; 8]>) -> Self {
        Self { parts }
    }

    #[must_use]
    pub fn parse(scope: &'a str) -> Self {
        let parts = scope
            .split("::")
            .filter(|s| !s.is_empty())
            .collect::<SmallVec<[&'a str; 8]>>();
        Self { parts }
    }

    #[must_use]
    pub fn is_root(&self) -> bool {
        self.parts.is_empty()
    }

    pub fn pop(&mut self) {
        if !self.parts.is_empty() {
            self.parts.pop();
        }
    }

    pub fn push(&mut self, part: &'a str) {
        self.parts.push(part);
    }

    #[must_use]
    pub fn names(&self) -> &[&'a str] {
        &self.parts
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Scope<'a> {
    scope: QualifiedScope<'a>,
    attributes: Vec<Arc<Expr<'a>>>,
    block: Arc<Expr<'a>>,
}

impl<'a> Scope<'a> {
    #[must_use]
    pub fn new(
        scope: QualifiedScope<'a>,
        attributes: Vec<Arc<Expr<'a>>>,
        block: Arc<Expr<'a>>,
    ) -> Self {
        Scope {
            scope,
            attributes,
            block,
        }
    }

    #[must_use]
    pub fn scope(&self) -> &QualifiedScope<'a> {
        &self.scope
    }

    pub fn set_scope(&mut self, scope: QualifiedScope<'a>) {
        self.scope = scope;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Arc<Expr<'a>>] {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Arc<Expr<'a>>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn block(&self) -> Arc<Expr<'a>> {
        self.block.clone()
    }

    pub fn set_block(&mut self, block: Arc<Expr<'a>>) {
        self.block = block;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct If<'a> {
    condition: Arc<Expr<'a>>,
    then_branch: Arc<Expr<'a>>,
    else_branch: Option<Arc<Expr<'a>>>,
}

impl<'a> If<'a> {
    #[must_use]
    pub(crate) fn new(
        condition: Arc<Expr<'a>>,
        then_branch: Arc<Expr<'a>>,
        else_branch: Option<Arc<Expr<'a>>>,
    ) -> Self {
        If {
            condition,
            then_branch,
            else_branch,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<Expr<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<Expr<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn then_branch(&self) -> &Arc<Expr<'a>> {
        &self.then_branch
    }

    pub fn set_then_branch(&mut self, then_branch: Arc<Expr<'a>>) {
        self.then_branch = then_branch;
    }

    #[must_use]
    pub fn else_branch(&self) -> Option<&Arc<Expr<'a>>> {
        self.else_branch.as_ref()
    }

    pub fn set_else_branch(&mut self, else_branch: Option<Arc<Expr<'a>>>) {
        self.else_branch = else_branch;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct WhileLoop<'a> {
    condition: Arc<Expr<'a>>,
    body: Arc<Expr<'a>>,
}

impl<'a> WhileLoop<'a> {
    #[must_use]
    pub(crate) fn new(condition: Arc<Expr<'a>>, body: Arc<Expr<'a>>) -> Self {
        WhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<Expr<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<Expr<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &Arc<Expr<'a>> {
        &self.body
    }

    pub fn set_body(&mut self, body: Arc<Expr<'a>>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct DoWhileLoop<'a> {
    condition: Arc<Expr<'a>>,
    body: Arc<Expr<'a>>,
}

impl<'a> DoWhileLoop<'a> {
    #[must_use]
    pub(crate) fn new(condition: Arc<Expr<'a>>, body: Arc<Expr<'a>>) -> Self {
        DoWhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<Expr<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<Expr<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &Arc<Expr<'a>> {
        &self.body
    }

    pub fn set_body(&mut self, body: Arc<Expr<'a>>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Switch<'a> {
    condition: Arc<Expr<'a>>,
    cases: Vec<(Arc<Expr<'a>>, Arc<Expr<'a>>)>,
    default_case: Option<Arc<Expr<'a>>>,
}

impl<'a> Switch<'a> {
    #[must_use]
    pub(crate) fn new(
        condition: Arc<Expr<'a>>,
        cases: Vec<(Arc<Expr<'a>>, Arc<Expr<'a>>)>,
        default_case: Option<Arc<Expr<'a>>>,
    ) -> Self {
        Switch {
            condition,
            cases,
            default_case,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<Expr<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<Expr<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn cases(&self) -> &[(Arc<Expr<'a>>, Arc<Expr<'a>>)] {
        &self.cases
    }

    #[must_use]
    pub fn cases_mut(&mut self) -> &mut Vec<(Arc<Expr<'a>>, Arc<Expr<'a>>)> {
        &mut self.cases
    }

    #[must_use]
    pub fn default_case(&self) -> Option<&Arc<Expr<'a>>> {
        self.default_case.as_ref()
    }

    pub fn set_default_case(&mut self, default_case: Option<Arc<Expr<'a>>>) {
        self.default_case = default_case;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Break<'a> {
    label: Option<&'a str>,
}

impl<'a> Break<'a> {
    #[must_use]
    pub(crate) fn new(label: Option<&'a str>) -> Self {
        Break { label }
    }

    #[must_use]
    pub fn label(&self) -> Option<&'a str> {
        self.label
    }

    pub fn set_label(&mut self, label: Option<&'a str>) {
        self.label = label;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Continue<'a> {
    label: Option<&'a str>,
}

impl<'a> Continue<'a> {
    #[must_use]
    pub(crate) fn new(label: Option<&'a str>) -> Self {
        Continue { label }
    }

    #[must_use]
    pub fn label(&self) -> Option<&'a str> {
        self.label
    }

    pub fn set_label(&mut self, label: Option<&'a str>) {
        self.label = label;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Return<'a> {
    value: Option<Arc<Expr<'a>>>,
}

impl<'a> Return<'a> {
    #[must_use]
    pub(crate) fn new(value: Option<Arc<Expr<'a>>>) -> Self {
        Return { value }
    }

    #[must_use]
    pub fn value(&self) -> Option<Arc<Expr<'a>>> {
        self.value.clone()
    }

    pub fn set_value(&mut self, value: Option<Arc<Expr<'a>>>) {
        self.value = value;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct ForEach<'a> {
    iterable: Arc<Expr<'a>>,
    bindings: Vec<(&'a str, Option<Type<'a>>)>,
    body: Arc<Expr<'a>>,
}

impl<'a> ForEach<'a> {
    #[must_use]
    pub(crate) fn new(
        bindings: Vec<(&'a str, Option<Type<'a>>)>,
        iterable: Arc<Expr<'a>>,
        body: Arc<Expr<'a>>,
    ) -> Self {
        ForEach {
            iterable,
            bindings,
            body,
        }
    }

    #[must_use]
    pub fn iterable(&self) -> &Arc<Expr<'a>> {
        &self.iterable
    }

    pub fn set_iterable(&mut self, iterable: Arc<Expr<'a>>) {
        self.iterable = iterable;
    }

    #[must_use]
    pub fn bindings(&self) -> &[(&'a str, Option<Type<'a>>)] {
        &self.bindings
    }

    #[must_use]
    pub fn bindings_mut(&mut self) -> &mut Vec<(&'a str, Option<Type<'a>>)> {
        &mut self.bindings
    }

    #[must_use]
    pub fn body(&self) -> &Arc<Expr<'a>> {
        &self.body
    }

    pub fn set_body(&mut self, body: Arc<Expr<'a>>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Await<'a> {
    expression: Arc<Expr<'a>>,
}

impl<'a> Await<'a> {
    #[must_use]
    pub(crate) fn new(expression: Arc<Expr<'a>>) -> Self {
        Await { expression }
    }

    #[must_use]
    pub fn expression(&self) -> &Arc<Expr<'a>> {
        &self.expression
    }

    pub fn set_expression(&mut self, expression: Arc<Expr<'a>>) {
        self.expression = expression;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Assert<'a> {
    condition: Arc<Expr<'a>>,
    message: Option<Arc<Expr<'a>>>,
}

impl<'a> Assert<'a> {
    #[must_use]
    pub(crate) fn new(condition: Arc<Expr<'a>>, message: Option<Arc<Expr<'a>>>) -> Self {
        Assert { condition, message }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<Expr<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<Expr<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn message(&self) -> Option<&Arc<Expr<'a>>> {
        self.message.as_ref()
    }

    pub fn set_message(&mut self, message: Option<Arc<Expr<'a>>>) {
        self.message = message;
    }
}
