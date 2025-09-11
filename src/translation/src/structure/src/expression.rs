use apint::UInt;
use interned_string::IString;
use nitrate_tokenize::{IntegerKind, Op};
use ordered_float::NotNan;
use serde::{Deserialize, Serialize};
use smallvec::SmallVec;
use std::collections::BTreeMap;
use std::ops::Deref;
use std::sync::{Arc, RwLock};

use super::types::{
    ArrayType, FunctionType, GenericType, ManagedRefType, MapType, RefinementType, SliceType,
    StructType, TupleType, Type, UnmanagedRefType,
};

#[derive(Clone, Serialize, Deserialize)]
pub struct Integer {
    value: UInt,
    kind: IntegerKind,
}

impl Integer {
    #[must_use]
    pub(crate) fn new(value: UInt, kind: IntegerKind) -> Option<Self> {
        if value.try_to_u128().is_ok() {
            Some(Integer { value, kind })
        } else {
            None
        }
    }

    #[must_use]
    pub fn get(&self) -> &UInt {
        &self.value
    }

    /// # Panics
    /// This function will panic if the value cannot be represented as a `u128`.
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

impl std::fmt::Debug for Integer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Integer({}, {:?})", self.get_u128(), self.kind())
    }
}

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
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
    Sizeof,  /* 'sizeof':     "Size Of Operator" */
    Alignof, /* 'alignof':    "Alignment Of Operator" */
    Typeof,  /* 'typeof':     "Type Of Operator" */
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
            Op::Sizeof => Ok(UnaryExprOp::Sizeof),
            Op::Typeof => Ok(UnaryExprOp::Typeof),
            Op::Alignof => Ok(UnaryExprOp::Alignof),

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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
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

            Op::BitNot | Op::LogicNot | Op::Sizeof | Op::Typeof | Op::Alignof => Err(()),
        }
    }
}

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
pub struct Statement {
    expr: Expr,
}

impl Statement {
    #[must_use]
    pub(crate) fn new(expr: Expr) -> Self {
        Statement { expr }
    }

    #[must_use]
    pub fn get(&self) -> &Expr {
        &self.expr
    }
}

#[derive(Debug, Clone)]
pub struct Block {
    elements: Vec<Expr>,
}

impl Block {
    #[must_use]
    pub(crate) fn new(items: Vec<Expr>) -> Self {
        Block { elements: items }
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

#[derive(Debug, Clone)]
pub struct FunctionParameter {
    name: IString,
    param_type: Type,
    default_value: Option<Expr>,
}

impl FunctionParameter {
    #[must_use]
    pub fn new(name: IString, param_type: Type, default_value: Option<Expr>) -> Self {
        FunctionParameter {
            name,
            param_type,
            default_value,
        }
    }

    #[must_use]
    pub fn name(&self) -> &IString {
        &self.name
    }

    #[must_use]
    pub fn type_(&self) -> &Type {
        &self.param_type
    }

    #[must_use]
    pub fn default(&self) -> Option<&Expr> {
        self.default_value.as_ref()
    }
}

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone, Copy)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
pub struct Identifier {
    full_name: IString,
    segments: Vec<IString>,
}

impl Identifier {
    #[must_use]
    pub(crate) fn new(segments: Vec<IString>) -> Self {
        Identifier {
            full_name: segments
                .iter()
                .map(Deref::deref)
                .collect::<Vec<&str>>()
                .join("::")
                .into(),
            segments,
        }
    }

    #[must_use]
    pub fn full_name(&self) -> &IString {
        &self.full_name
    }

    #[must_use]
    pub fn segments(&self) -> &[IString] {
        &self.segments
    }

    #[must_use]
    pub fn is_absolute(&self) -> bool {
        self.segments.first().map_or(false, |s| s.is_empty())
    }
}

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
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

#[derive(Debug, Clone)]
pub struct Scope {
    name: IString,
    attributes: Vec<Expr>,
    elements: Vec<Expr>,
}

impl Scope {
    #[must_use]
    pub fn new(name: IString, attributes: Vec<Expr>, elements: Vec<Expr>) -> Self {
        Scope {
            name,
            attributes,
            elements,
        }
    }

    #[must_use]
    pub fn name(&self) -> &IString {
        &self.name
    }

    pub fn set_name(&mut self, name: IString) {
        self.name = name;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Expr] {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Expr> {
        &mut self.attributes
    }

    #[must_use]
    pub fn elements(&self) -> &[Expr] {
        &self.elements
    }

    pub fn elements_mut(&mut self) -> &mut Vec<Expr> {
        &mut self.elements
    }
}

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
pub struct Assert {
    condition: Expr,
    message: Expr,
}

impl Assert {
    #[must_use]
    pub(crate) fn new(condition: Expr, message: Expr) -> Self {
        Assert { condition, message }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr) {
        self.condition = condition;
    }

    #[must_use]
    pub fn message(&self) -> &Expr {
        &self.message
    }

    pub fn set_message(&mut self, message: Expr) {
        self.message = message;
    }
}

pub type CallArguments = Vec<(Option<IString>, Expr)>;

#[derive(Debug, Clone)]
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

#[derive(Clone)]
pub enum Expr {
    Bool,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    UInt128,
    Int8,
    Int16,
    Int32,
    Int64,
    Int128,
    Float8,
    Float16,
    Float32,
    Float64,
    Float128,
    UnitType,
    InferType,
    TypeName(Arc<Identifier>),
    RefinementType(Arc<RefinementType>),
    TupleType(Arc<TupleType>),
    ArrayType(Arc<ArrayType>),
    MapType(Arc<MapType>),
    SliceType(Arc<SliceType>),
    FunctionType(Arc<FunctionType>),
    ManagedRefType(Arc<ManagedRefType>),
    UnmanagedRefType(Arc<UnmanagedRefType>),
    GenericType(Arc<GenericType>),
    OpaqueType(IString),
    StructType(Arc<StructType>),
    LatentType(Arc<Expr>),
    HasParenthesesType(Arc<Type>),

    Discard,
    HasParentheses(Arc<Expr>),

    Boolean(bool),
    Integer(Arc<Integer>),
    Float(NotNan<f64>),
    String(IString),
    BString(Arc<Vec<u8>>),
    Unit,

    TypeEnvelop(Arc<Type>),
    List(Arc<List>),
    Object(Arc<Object>),
    UnaryExpr(Arc<UnaryExpr>),
    BinExpr(Arc<BinExpr>),
    Statement(Arc<Statement>),
    Block(Arc<Block>),

    Function(Arc<RwLock<Function>>),
    Variable(Arc<Variable>),
    Identifier(Arc<Identifier>),
    IndexAccess(Arc<IndexAccess>),
    Scope(Arc<Scope>),

    If(Arc<If>),
    WhileLoop(Arc<WhileLoop>),
    DoWhileLoop(Arc<DoWhileLoop>),
    Switch(Arc<Switch>),
    Break(Arc<Break>),
    Continue(Arc<Continue>),
    Return(Arc<Return>),
    ForEach(Arc<ForEach>),
    Await(Arc<Await>),
    Assert(Arc<Assert>),
    Call(Arc<Call>),
}

impl TryInto<Type> for Expr {
    type Error = Self;

    fn try_into(self) -> Result<Type, Self::Error> {
        match self {
            Expr::Bool => Ok(Type::Bool),
            Expr::UInt8 => Ok(Type::UInt8),
            Expr::UInt16 => Ok(Type::UInt16),
            Expr::UInt32 => Ok(Type::UInt32),
            Expr::UInt64 => Ok(Type::UInt64),
            Expr::UInt128 => Ok(Type::UInt128),
            Expr::Int8 => Ok(Type::Int8),
            Expr::Int16 => Ok(Type::Int16),
            Expr::Int32 => Ok(Type::Int32),
            Expr::Int64 => Ok(Type::Int64),
            Expr::Int128 => Ok(Type::Int128),
            Expr::Float8 => Ok(Type::Float8),
            Expr::Float16 => Ok(Type::Float16),
            Expr::Float32 => Ok(Type::Float32),
            Expr::Float64 => Ok(Type::Float64),
            Expr::Float128 => Ok(Type::Float128),
            Expr::UnitType => Ok(Type::UnitType),

            Expr::InferType => Ok(Type::InferType),
            Expr::TypeName(x) => Ok(Type::TypeName(x)),
            Expr::RefinementType(x) => Ok(Type::RefinementType(x)),
            Expr::TupleType(x) => Ok(Type::TupleType(x)),
            Expr::ArrayType(x) => Ok(Type::ArrayType(x)),
            Expr::MapType(x) => Ok(Type::MapType(x)),
            Expr::SliceType(x) => Ok(Type::SliceType(x)),
            Expr::FunctionType(x) => Ok(Type::FunctionType(x)),
            Expr::ManagedRefType(x) => Ok(Type::ManagedRefType(x)),
            Expr::UnmanagedRefType(x) => Ok(Type::UnmanagedRefType(x)),
            Expr::GenericType(x) => Ok(Type::GenericType(x)),
            Expr::OpaqueType(x) => Ok(Type::OpaqueType(x)),
            Expr::StructType(x) => Ok(Type::StructType(x)),
            Expr::LatentType(x) => Ok(Type::LatentType(x)),
            Expr::HasParenthesesType(x) => Ok(Type::HasParenthesesType(x)),

            Expr::Discard
            | Expr::HasParentheses(_)
            | Expr::Boolean(_)
            | Expr::Integer(_)
            | Expr::Float(_)
            | Expr::String(_)
            | Expr::BString(_)
            | Expr::Unit
            | Expr::TypeEnvelop(_)
            | Expr::List(_)
            | Expr::Object(_)
            | Expr::UnaryExpr(_)
            | Expr::BinExpr(_)
            | Expr::Statement(_)
            | Expr::Block(_)
            | Expr::Function(_)
            | Expr::Variable(_)
            | Expr::Identifier(_)
            | Expr::IndexAccess(_)
            | Expr::Scope(_)
            | Expr::If(_)
            | Expr::WhileLoop(_)
            | Expr::DoWhileLoop(_)
            | Expr::Switch(_)
            | Expr::Break(_)
            | Expr::Continue(_)
            | Expr::Return(_)
            | Expr::ForEach(_)
            | Expr::Await(_)
            | Expr::Assert(_)
            | Expr::Call(_) => Err(self),
        }
    }
}

impl Expr {
    #[must_use]
    pub fn is_discard(&self) -> bool {
        matches!(self, Expr::Discard)
    }
}

impl std::fmt::Debug for Expr {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Expr::Bool => write!(f, "bool"),
            Expr::UInt8 => write!(f, "u8"),
            Expr::UInt16 => write!(f, "u16"),
            Expr::UInt32 => write!(f, "u32"),
            Expr::UInt64 => write!(f, "u64"),
            Expr::UInt128 => write!(f, "u128"),
            Expr::Int8 => write!(f, "i8"),
            Expr::Int16 => write!(f, "i16"),
            Expr::Int32 => write!(f, "i32"),
            Expr::Int64 => write!(f, "i64"),
            Expr::Int128 => write!(f, "i128"),
            Expr::Float8 => write!(f, "f8"),
            Expr::Float16 => write!(f, "f16"),
            Expr::Float32 => write!(f, "f32"),
            Expr::Float64 => write!(f, "f64"),
            Expr::Float128 => write!(f, "f128"),
            Expr::UnitType => write!(f, "()"),
            Expr::InferType => write!(f, "_"),
            Expr::TypeName(e) => f
                .debug_struct("TypeName")
                .field("name", &e.full_name())
                .finish(),
            Expr::RefinementType(e) => e.fmt(f),
            Expr::TupleType(e) => e.fmt(f),
            Expr::ArrayType(e) => e.fmt(f),
            Expr::MapType(e) => e.fmt(f),
            Expr::SliceType(e) => e.fmt(f),
            Expr::FunctionType(e) => e.fmt(f),
            Expr::ManagedRefType(e) => e.fmt(f),
            Expr::UnmanagedRefType(e) => e.fmt(f),
            Expr::GenericType(e) => e.fmt(f),
            Expr::OpaqueType(e) => f.debug_struct("OpaqueType").field("name", e).finish(),
            Expr::StructType(e) => e.fmt(f),
            Expr::LatentType(e) => f.debug_struct("LatentType").field("type", e).finish(),
            Expr::HasParenthesesType(e) => f.debug_struct("Parentheses").field("type", e).finish(),

            Expr::Discard => write!(f, ""),
            Expr::HasParentheses(e) => f.debug_struct("Parentheses").field("expr", e).finish(),

            Expr::Boolean(e) => e.fmt(f),
            Expr::Integer(e) => e.fmt(f),
            Expr::Float(e) => e.fmt(f),
            Expr::String(e) => e.fmt(f),
            Expr::BString(e) => e.fmt(f),
            Expr::Unit => write!(f, "()"),

            Expr::TypeEnvelop(e) => f.debug_struct("TypeEnvelop").field("type", e).finish(),
            Expr::List(e) => e.fmt(f),
            Expr::Object(e) => e.fmt(f),
            Expr::UnaryExpr(e) => e.fmt(f),
            Expr::BinExpr(e) => e.fmt(f),
            Expr::Statement(e) => e.fmt(f),
            Expr::Block(e) => e.fmt(f),

            Expr::Function(e) => e.fmt(f),
            Expr::Variable(e) => e.fmt(f),
            Expr::Identifier(e) => f
                .debug_struct("Identifier")
                .field("name", &e.full_name())
                .finish(),
            Expr::IndexAccess(e) => e.fmt(f),
            Expr::Scope(e) => e.fmt(f),

            Expr::If(e) => e.fmt(f),
            Expr::WhileLoop(e) => e.fmt(f),
            Expr::DoWhileLoop(e) => e.fmt(f),
            Expr::Switch(e) => e.fmt(f),
            Expr::Break(e) => e.fmt(f),
            Expr::Continue(e) => e.fmt(f),
            Expr::Return(e) => e.fmt(f),
            Expr::ForEach(e) => e.fmt(f),
            Expr::Await(e) => e.fmt(f),
            Expr::Assert(e) => e.fmt(f),
            Expr::Call(e) => e.fmt(f),
        }
    }
}
