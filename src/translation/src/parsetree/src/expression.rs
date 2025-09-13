use crate::kind::{FunctionParameter, Type};

use interned_string::IString;
use nitrate_tokenize::{IntegerKind, Op};
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};
use smallvec::SmallVec;
use std::collections::BTreeMap;
use std::ops::Deref;

#[derive(Clone, Serialize, Deserialize)]
pub struct Integer {
    value: u128,
    kind: IntegerKind,
}

impl Integer {
    #[must_use]
    pub(crate) fn new(value: u128, kind: IntegerKind) -> Option<Self> {
        Some(Integer { value, kind })
    }

    #[must_use]
    pub fn get_u128(&self) -> u128 {
        self.value
    }

    #[must_use]
    pub fn kind(&self) -> IntegerKind {
        self.kind
    }
}

impl std::fmt::Debug for Integer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Integer({}, {:?})", self.get_u128(), self.kind())
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct List {
    elements: Vec<Expr>,
}

impl List {
    #[must_use]
    pub(crate) fn new(elements: Vec<Expr>) -> Self {
        List { elements }
    }

    #[must_use]
    pub fn elements(&self) -> &[Expr] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<Expr> {
        &mut self.elements
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Object {
    fields: BTreeMap<IString, Expr>,
}

impl Object {
    #[must_use]
    pub(crate) fn new(fields: BTreeMap<IString, Expr>) -> Self {
        Object { fields }
    }

    #[must_use]
    pub fn fields(&self) -> &BTreeMap<IString, Expr> {
        &self.fields
    }

    #[must_use]
    pub fn fields_mut(&mut self) -> &mut BTreeMap<IString, Expr> {
        &mut self.fields
    }

    #[must_use]
    pub fn access(&self, key: IString) -> Option<&Expr> {
        self.fields.get(&key)
    }

    #[must_use]
    pub fn access_mut(&mut self, key: IString) -> Option<&mut Expr> {
        self.fields_mut().get_mut(&key)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum UnaryExprOp {
    /*----------------------------------------------------------------*
     * Arithmetic Operators                                           *
     *----------------------------------------------------------------*/
    Add,   /* '+': "Addition Operator" */
    Sub,   /* '-': "Subtraction Operator" */
    Deref, /* '*': "Multiplication Operator" */

    /*----------------------------------------------------------------*
     * Bitwise Operators                                              *
     *----------------------------------------------------------------*/
    AddressOf, /* '&':   "Bitwise AND Operator" */
    BitNot,    /* '~':   "Bitwise NOT Operator" */

    /*----------------------------------------------------------------*
     * Logical Operators                                              *
     *----------------------------------------------------------------*/
    LogicNot, /* '!':  "Logical NOT Operator" */

    /*----------------------------------------------------------------*
     * Type System Operators                                          *
     *----------------------------------------------------------------*/
    Typeof, /* 'typeof':     "Type Of Operator" */
}

impl TryFrom<Op> for UnaryExprOp {
    type Error = ();

    fn try_from(op: Op) -> Result<Self, Self::Error> {
        match op {
            Op::Add => Ok(UnaryExprOp::Add),
            Op::Sub => Ok(UnaryExprOp::Sub),
            Op::Mul => Ok(UnaryExprOp::Deref),
            Op::BitAnd => Ok(UnaryExprOp::AddressOf),
            Op::BitNot => Ok(UnaryExprOp::BitNot),
            Op::LogicNot => Ok(UnaryExprOp::LogicNot),
            Op::Typeof => Ok(UnaryExprOp::Typeof),

            Op::Div
            | Op::Mod
            | Op::BitOr
            | Op::BitXor
            | Op::BitShl
            | Op::BitShr
            | Op::BitRol
            | Op::BitRor
            | Op::LogicAnd
            | Op::LogicOr
            | Op::LogicXor
            | Op::LogicLt
            | Op::LogicGt
            | Op::LogicLe
            | Op::LogicGe
            | Op::LogicEq
            | Op::LogicNe
            | Op::Set
            | Op::SetPlus
            | Op::SetMinus
            | Op::SetTimes
            | Op::SetSlash
            | Op::SetPercent
            | Op::SetBitAnd
            | Op::SetBitOr
            | Op::SetBitXor
            | Op::SetBitShl
            | Op::SetBitShr
            | Op::SetBitRotl
            | Op::SetBitRotr
            | Op::SetLogicAnd
            | Op::SetLogicOr
            | Op::SetLogicXor
            | Op::As
            | Op::BitcastAs
            | Op::Dot
            | Op::Ellipsis
            | Op::Scope
            | Op::Arrow
            | Op::BlockArrow
            | Op::Range => Err(()),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct UnaryExpr {
    operator: UnaryExprOp,
    is_postfix: bool,
    operand: Expr,
}

impl UnaryExpr {
    #[must_use]
    pub(crate) fn new(operand: Expr, operator: UnaryExprOp, is_postfix: bool) -> Self {
        UnaryExpr {
            operand,
            operator,
            is_postfix,
        }
    }

    #[must_use]
    pub fn operand(&self) -> &Expr {
        &self.operand
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
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
    BitAnd, /* '&':   "Bitwise AND Operator" */
    BitOr,  /* '|':   "Bitwise OR Operator" */
    BitXor, /* '^':   "Bitwise XOR Operator" */
    BitShl, /* '<<':  "Bitwise Left-Shift Operator" */
    BitShr, /* '>>':  "Bitwise Right-Shift Operator" */
    BitRol, /* '<<<': "Bitwise Left-Rotate Operator" */
    BitRor, /* '>>>': "Bitwise Right-Rotate Operator" */

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
    As,        /* 'as':         "Type Cast Operator" */
    BitcastAs, /* 'bitcast_as': "Bitwise Type Cast Operator" */

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
    Range, /* '..':         "Range Operator" */
}

impl TryFrom<Op> for BinExprOp {
    type Error = ();

    fn try_from(op: Op) -> Result<Self, Self::Error> {
        match op {
            Op::Add => Ok(BinExprOp::Add),
            Op::Sub => Ok(BinExprOp::Sub),
            Op::Mul => Ok(BinExprOp::Mul),
            Op::Div => Ok(BinExprOp::Div),
            Op::Mod => Ok(BinExprOp::Mod),
            Op::BitAnd => Ok(BinExprOp::BitAnd),
            Op::BitOr => Ok(BinExprOp::BitOr),
            Op::BitXor => Ok(BinExprOp::BitXor),
            Op::BitShl => Ok(BinExprOp::BitShl),
            Op::BitShr => Ok(BinExprOp::BitShr),
            Op::BitRol => Ok(BinExprOp::BitRol),
            Op::BitRor => Ok(BinExprOp::BitRor),
            Op::LogicAnd => Ok(BinExprOp::LogicAnd),
            Op::LogicOr => Ok(BinExprOp::LogicOr),
            Op::LogicXor => Ok(BinExprOp::LogicXor),
            Op::LogicLt => Ok(BinExprOp::LogicLt),
            Op::LogicGt => Ok(BinExprOp::LogicGt),
            Op::LogicLe => Ok(BinExprOp::LogicLe),
            Op::LogicGe => Ok(BinExprOp::LogicGe),
            Op::LogicEq => Ok(BinExprOp::LogicEq),
            Op::LogicNe => Ok(BinExprOp::LogicNe),
            Op::Set => Ok(BinExprOp::Set),
            Op::SetPlus => Ok(BinExprOp::SetPlus),
            Op::SetMinus => Ok(BinExprOp::SetMinus),
            Op::SetTimes => Ok(BinExprOp::SetTimes),
            Op::SetSlash => Ok(BinExprOp::SetSlash),
            Op::SetPercent => Ok(BinExprOp::SetPercent),
            Op::SetBitAnd => Ok(BinExprOp::SetBitAnd),
            Op::SetBitOr => Ok(BinExprOp::SetBitOr),
            Op::SetBitXor => Ok(BinExprOp::SetBitXor),
            Op::SetBitShl => Ok(BinExprOp::SetBitShl),
            Op::SetBitShr => Ok(BinExprOp::SetBitShr),
            Op::SetBitRotl => Ok(BinExprOp::SetBitRotl),
            Op::SetBitRotr => Ok(BinExprOp::SetBitRotr),
            Op::SetLogicAnd => Ok(BinExprOp::SetLogicAnd),
            Op::SetLogicOr => Ok(BinExprOp::SetLogicOr),
            Op::SetLogicXor => Ok(BinExprOp::SetLogicXor),
            Op::As => Ok(BinExprOp::As),
            Op::BitcastAs => Ok(BinExprOp::BitcastAs),
            Op::Dot => Ok(BinExprOp::Dot),
            Op::Ellipsis => Ok(BinExprOp::Ellipsis),
            Op::Scope => Ok(BinExprOp::Scope),
            Op::Arrow => Ok(BinExprOp::Arrow),
            Op::BlockArrow => Ok(BinExprOp::BlockArrow),
            Op::Range => Ok(BinExprOp::Range),

            Op::BitNot | Op::LogicNot | Op::Typeof => Err(()),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BinExpr {
    operator: BinExprOp,
    left: Expr,
    right: Expr,
}

impl BinExpr {
    #[must_use]
    pub(crate) fn new(left: Expr, operator: BinExprOp, right: Expr) -> Self {
        BinExpr {
            left,
            right,
            operator,
        }
    }

    #[must_use]
    pub fn left(&self) -> &Expr {
        &self.left
    }

    #[must_use]
    pub fn op(&self) -> BinExprOp {
        self.operator
    }

    #[must_use]
    pub fn right(&self) -> &Expr {
        &self.right
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Block {
    elements: Vec<Expr>,
    ends_with_semi: bool,
}

impl Block {
    #[must_use]
    pub(crate) fn new(elements: Vec<Expr>, ends_with_semi: bool) -> Self {
        Block {
            elements,
            ends_with_semi,
        }
    }

    #[must_use]
    pub fn elements(&self) -> &[Expr] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<Expr> {
        &mut self.elements
    }

    #[must_use]
    pub fn ends_with_semi(&self) -> bool {
        self.ends_with_semi
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Function {
    parameters: Vec<FunctionParameter>,
    return_type: Type,
    attributes: Vec<Expr>,
    definition: Option<Expr>,
}

impl Function {
    #[must_use]
    pub(crate) fn new(
        parameters: Vec<FunctionParameter>,
        return_type: Type,
        attributes: Vec<Expr>,
        definition: Option<Expr>,
    ) -> Self {
        Function {
            parameters,
            return_type,
            attributes,
            definition,
        }
    }

    #[must_use]
    pub fn parameters(&self) -> &[FunctionParameter] {
        &self.parameters
    }

    #[must_use]
    pub fn parameters_mut(&mut self) -> &mut Vec<FunctionParameter> {
        &mut self.parameters
    }

    #[must_use]
    pub fn return_type(&self) -> &Type {
        &self.return_type
    }

    pub fn set_return_type(&mut self, ty: Type) {
        self.return_type = ty;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Expr] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<Expr> {
        &mut self.attributes
    }

    #[must_use]
    pub fn definition(&self) -> Option<&Expr> {
        self.definition.as_ref()
    }

    pub fn set_definition(&mut self, definition: Option<Expr>) {
        self.definition = definition;
    }

    #[must_use]
    pub fn is_definition(&self) -> bool {
        self.definition.is_some()
    }

    #[must_use]
    pub fn is_declaration(&self) -> bool {
        self.definition.is_none()
    }
}

#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Variable {
    kind: VariableKind,
    is_mutable: bool,
    attributes: Vec<Expr>,
    name: IString,
    var_type: Type,
    value: Option<Expr>,
}

impl Variable {
    #[must_use]
    pub(crate) fn new(
        kind: VariableKind,
        is_mutable: bool,
        attributes: Vec<Expr>,
        name: IString,
        var_type: Type,
        value: Option<Expr>,
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
    pub fn attributes(&self) -> &[Expr] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<Expr> {
        &mut self.attributes
    }

    #[must_use]
    pub fn name(&self) -> &IString {
        &self.name
    }

    pub fn set_name(&mut self, name: IString) {
        self.name = name;
    }

    #[must_use]
    pub fn get_type(&self) -> &Type {
        &self.var_type
    }

    pub fn set_type(&mut self, var_type: Type) {
        self.var_type = var_type;
    }

    #[must_use]
    pub fn value(&self) -> Option<&Expr> {
        self.value.as_ref()
    }

    pub fn set_value(&mut self, value: Option<Expr>) {
        self.value = value;
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IndexAccess {
    collection: Expr,
    index: Expr,
}

impl IndexAccess {
    #[must_use]
    pub(crate) fn new(collection: Expr, index: Expr) -> Self {
        IndexAccess { collection, index }
    }

    #[must_use]
    pub fn collection(&self) -> &Expr {
        &self.collection
    }

    pub fn set_collection(&mut self, collection: Expr) {
        self.collection = collection;
    }

    #[must_use]
    pub fn index(&self) -> &Expr {
        &self.index
    }

    pub fn set_index(&mut self, index: Expr) {
        self.index = index;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
pub struct QualifiedScope {
    scopes: SmallVec<[IString; 3]>,
}

impl QualifiedScope {
    #[must_use]
    pub fn new(scopes: SmallVec<[IString; 3]>) -> Self {
        Self { scopes }
    }

    #[must_use]
    pub fn parse(qualified_scope: IString) -> Self {
        let parts = qualified_scope
            .split("::")
            .filter(|s| !s.is_empty())
            .map(IString::from)
            .collect::<SmallVec<[IString; 3]>>();

        Self { scopes: parts }
    }

    #[must_use]
    pub fn is_root(&self) -> bool {
        self.scopes.is_empty()
    }

    pub fn pop(&mut self) {
        if !self.scopes.is_empty() {
            self.scopes.pop();
        }
    }

    pub fn push(&mut self, scope: IString) {
        self.scopes.push(scope);
    }

    #[must_use]
    pub fn scopes(&self) -> &[IString] {
        &self.scopes
    }
}

impl std::fmt::Display for QualifiedScope {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}",
            self.scopes()
                .iter()
                .map(Deref::deref)
                .collect::<Vec<&str>>()
                .join("::")
        )
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct If {
    condition: Expr,
    then_branch: Expr,
    else_branch: Option<Expr>,
}

impl If {
    #[must_use]
    pub(crate) fn new(condition: Expr, then_branch: Expr, else_branch: Option<Expr>) -> Self {
        If {
            condition,
            then_branch,
            else_branch,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr) {
        self.condition = condition;
    }

    #[must_use]
    pub fn then_branch(&self) -> &Expr {
        &self.then_branch
    }

    pub fn set_then_branch(&mut self, then_branch: Expr) {
        self.then_branch = then_branch;
    }

    #[must_use]
    pub fn else_branch(&self) -> Option<&Expr> {
        self.else_branch.as_ref()
    }

    pub fn set_else_branch(&mut self, else_branch: Option<Expr>) {
        self.else_branch = else_branch;
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WhileLoop {
    condition: Expr,
    body: Expr,
}

impl WhileLoop {
    #[must_use]
    pub(crate) fn new(condition: Expr, body: Expr) -> Self {
        WhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &Expr {
        &self.body
    }

    pub fn set_body(&mut self, body: Expr) {
        self.body = body;
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DoWhileLoop {
    condition: Expr,
    body: Expr,
}

impl DoWhileLoop {
    #[must_use]
    pub(crate) fn new(condition: Expr, body: Expr) -> Self {
        DoWhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &Expr {
        &self.body
    }

    pub fn set_body(&mut self, body: Expr) {
        self.body = body;
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Switch {
    condition: Expr,
    cases: Vec<(Expr, Expr)>,
    default_case: Option<Expr>,
}

impl Switch {
    #[must_use]
    pub(crate) fn new(
        condition: Expr,
        cases: Vec<(Expr, Expr)>,
        default_case: Option<Expr>,
    ) -> Self {
        Switch {
            condition,
            cases,
            default_case,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr) {
        self.condition = condition;
    }

    #[must_use]
    pub fn cases(&self) -> &[(Expr, Expr)] {
        &self.cases
    }

    #[must_use]
    pub fn cases_mut(&mut self) -> &mut Vec<(Expr, Expr)> {
        &mut self.cases
    }

    #[must_use]
    pub fn default_case(&self) -> Option<&Expr> {
        self.default_case.as_ref()
    }

    pub fn set_default_case(&mut self, default_case: Option<Expr>) {
        self.default_case = default_case;
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Break {
    label: Option<IString>,
}

impl Break {
    #[must_use]
    pub(crate) fn new(label: Option<IString>) -> Self {
        Break { label }
    }

    #[must_use]
    pub fn label(&self) -> Option<&IString> {
        self.label.as_ref()
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Continue {
    label: Option<IString>,
}

impl Continue {
    #[must_use]
    pub(crate) fn new(label: Option<IString>) -> Self {
        Continue { label }
    }

    #[must_use]
    pub fn label(&self) -> Option<&IString> {
        self.label.as_ref()
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Return {
    value: Option<Expr>,
}

impl Return {
    #[must_use]
    pub(crate) fn new(value: Option<Expr>) -> Self {
        Return { value }
    }

    #[must_use]
    pub fn value(&self) -> Option<&Expr> {
        self.value.as_ref()
    }

    pub fn set_value(&mut self, value: Option<Expr>) {
        self.value = value;
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ForEach {
    iterable: Expr,
    bindings: Vec<(IString, Option<Type>)>,
    body: Expr,
}

impl ForEach {
    #[must_use]
    pub(crate) fn new(bindings: Vec<(IString, Option<Type>)>, iterable: Expr, body: Expr) -> Self {
        ForEach {
            iterable,
            bindings,
            body,
        }
    }

    #[must_use]
    pub fn iterable(&self) -> &Expr {
        &self.iterable
    }

    pub fn set_iterable(&mut self, iterable: Expr) {
        self.iterable = iterable;
    }

    #[must_use]
    pub fn bindings(&self) -> &[(IString, Option<Type>)] {
        &self.bindings
    }

    #[must_use]
    pub fn bindings_mut(&mut self) -> &mut Vec<(IString, Option<Type>)> {
        &mut self.bindings
    }

    #[must_use]
    pub fn body(&self) -> &Expr {
        &self.body
    }

    pub fn set_body(&mut self, body: Expr) {
        self.body = body;
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Await {
    expression: Expr,
}

impl Await {
    #[must_use]
    pub(crate) fn new(expression: Expr) -> Self {
        Await { expression }
    }

    #[must_use]
    pub fn expression(&self) -> &Expr {
        &self.expression
    }

    pub fn set_expression(&mut self, expression: Expr) {
        self.expression = expression;
    }
}

pub type CallArguments = Vec<(Option<IString>, Expr)>;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Call {
    callee: Expr,
    arguments: CallArguments,
}

impl Call {
    #[must_use]
    pub fn new(callee: Expr, arguments: CallArguments) -> Self {
        Call { callee, arguments }
    }

    #[must_use]
    pub fn callee(&self) -> &Expr {
        &self.callee
    }

    pub fn set_callee(&mut self, callee: Expr) {
        self.callee = callee;
    }

    #[must_use]
    pub fn arguments(&self) -> &[(Option<IString>, Expr)] {
        &self.arguments
    }

    #[must_use]
    pub fn arguments_mut(&mut self) -> &mut CallArguments {
        &mut self.arguments
    }

    pub fn set_arguments(&mut self, arguments: CallArguments) {
        self.arguments = arguments;
    }
}

#[derive(Clone, Serialize, Deserialize)]
pub enum Expr {
    Discard,
    HasParentheses(Box<Expr>),

    Boolean(bool),
    Integer(Box<Integer>),
    Float(NotNan<f64>),
    String(IString),
    BString(Box<Vec<u8>>),
    Unit,

    TypeInfo(Type),
    List(Box<List>),
    Object(Box<Object>),
    UnaryExpr(Box<UnaryExpr>),
    BinExpr(Box<BinExpr>),
    Block(Box<Block>),

    Function(Box<Function>),
    Variable(Box<Variable>),
    Identifier(IString),
    IndexAccess(Box<IndexAccess>),

    If(Box<If>),
    WhileLoop(Box<WhileLoop>),
    DoWhileLoop(Box<DoWhileLoop>),
    Switch(Box<Switch>),
    Break(Box<Break>),
    Continue(Box<Continue>),
    Return(Box<Return>),
    ForEach(Box<ForEach>),
    Await(Box<Await>),
    Call(Box<Call>),
}

impl Expr {
    #[must_use]
    pub fn is_discard(&self) -> bool {
        matches!(self, Expr::Discard)
    }

    pub fn digest_128(&self) -> [u8; 16] {
        let mut hasher = blake3::Hasher::new();
        hasher.update(&serde_json::to_vec(self).unwrap_or_default());
        let hash = hasher.finalize();
        let bytes = hash.as_bytes();

        let mut result = [0u8; 16];
        result.copy_from_slice(&bytes[..16]);
        result
    }
}

impl std::fmt::Debug for Expr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Expr::Discard => write!(f, ""),
            Expr::HasParentheses(e) => f.debug_struct("Parentheses").field("expr", e).finish(),

            Expr::Boolean(e) => e.fmt(f),
            Expr::Integer(e) => e.fmt(f),
            Expr::Float(e) => e.fmt(f),
            Expr::String(e) => e.fmt(f),
            Expr::BString(e) => e.fmt(f),
            Expr::Unit => write!(f, "()"),

            Expr::TypeInfo(e) => f.debug_struct("TypeInfo").field("type", e).finish(),
            Expr::List(e) => e.fmt(f),
            Expr::Object(e) => e.fmt(f),
            Expr::UnaryExpr(e) => e.fmt(f),
            Expr::BinExpr(e) => e.fmt(f),
            Expr::Block(e) => e.fmt(f),

            Expr::Function(e) => e.fmt(f),
            Expr::Variable(e) => e.fmt(f),
            Expr::Identifier(e) => f.debug_struct("Identifier").field("name", &e).finish(),
            Expr::IndexAccess(e) => e.fmt(f),

            Expr::If(e) => e.fmt(f),
            Expr::WhileLoop(e) => e.fmt(f),
            Expr::DoWhileLoop(e) => e.fmt(f),
            Expr::Switch(e) => e.fmt(f),
            Expr::Break(e) => e.fmt(f),
            Expr::Continue(e) => e.fmt(f),
            Expr::Return(e) => e.fmt(f),
            Expr::ForEach(e) => e.fmt(f),
            Expr::Await(e) => e.fmt(f),
            Expr::Call(e) => e.fmt(f),
        }
    }
}
