use apint::UInt;
use nitrate_lexical::IntegerKind;
use nitrate_lexical::{BStringData, StringData};
use ordered_float::NotNan;
use smallvec::SmallVec;
use std::{collections::BTreeMap, rc::Rc};

use super::types::{
    ArrayType, FunctionType, GenericType, ManagedRefType, MapType, RefinementType, SliceType,
    StructType, TupleType, Type, UnmanagedRefType,
};

#[derive(Clone, PartialEq, Eq, Hash)]
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

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct List<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> List<'a> {
    #[must_use]
    pub(crate) fn new(elements: Vec<Expr<'a>>) -> Self {
        List { elements }
    }

    #[must_use]
    pub fn elements(&self) -> &[Expr<'a>] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.elements
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Object<'a> {
    fields: BTreeMap<&'a str, Expr<'a>>,
}

impl<'a> Object<'a> {
    #[must_use]
    pub(crate) fn new(fields: BTreeMap<&'a str, Expr<'a>>) -> Self {
        Object { fields }
    }

    #[must_use]
    pub fn fields(&self) -> &BTreeMap<&'a str, Expr<'a>> {
        &self.fields
    }

    #[must_use]
    pub fn fields_mut(&mut self) -> &mut BTreeMap<&'a str, Expr<'a>> {
        &mut self.fields
    }

    #[must_use]
    pub fn access(&self, key: &'a str) -> Option<&Expr<'a>> {
        self.fields.get(key)
    }

    #[must_use]
    pub fn access_mut(&mut self, key: &'a str) -> Option<&mut Expr<'a>> {
        self.fields_mut().get_mut(key)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
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

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct UnaryExpr<'a> {
    operand: Expr<'a>,
    operator: UnaryExprOp,
    is_postfix: bool,
}

impl<'a> UnaryExpr<'a> {
    #[must_use]
    pub(crate) fn new(operand: Expr<'a>, operator: UnaryExprOp, is_postfix: bool) -> Self {
        UnaryExpr {
            operand,
            operator,
            is_postfix,
        }
    }

    #[must_use]
    pub fn operand(&self) -> &Expr<'a> {
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
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
    Spaceship, /* '<=>':        "Spaceship Operator" */
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct BinExpr<'a> {
    left: Expr<'a>,
    right: Expr<'a>,
    operator: BinExprOp,
}

impl<'a> BinExpr<'a> {
    #[must_use]
    pub(crate) fn new(left: Expr<'a>, operator: BinExprOp, right: Expr<'a>) -> Self {
        BinExpr {
            left,
            right,
            operator,
        }
    }

    #[must_use]
    pub fn left(&self) -> &Expr<'a> {
        &self.left
    }

    #[must_use]
    pub fn op(&self) -> BinExprOp {
        self.operator
    }

    #[must_use]
    pub fn right(&self) -> &Expr<'a> {
        &self.right
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Statement<'a> {
    expr: Expr<'a>,
}

impl<'a> Statement<'a> {
    #[must_use]
    pub(crate) fn new(expr: Expr<'a>) -> Self {
        Statement { expr }
    }

    #[must_use]
    pub fn get(&self) -> &Expr<'a> {
        &self.expr
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Block<'a> {
    elements: Vec<Expr<'a>>,
}

impl<'a> Block<'a> {
    #[must_use]
    pub(crate) fn new(items: Vec<Expr<'a>>) -> Self {
        Block { elements: items }
    }

    #[must_use]
    pub fn elements(&self) -> &[Expr<'a>] {
        &self.elements
    }

    #[must_use]
    pub fn elements_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.elements
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct FunctionParameter<'a> {
    name: &'a str,
    param_type: Type<'a>,
    default_value: Option<Expr<'a>>,
}

impl<'a> FunctionParameter<'a> {
    #[must_use]
    pub fn new(name: &'a str, param_type: Type<'a>, default_value: Option<Expr<'a>>) -> Self {
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
    pub fn type_(&self) -> &Type<'a> {
        &self.param_type
    }

    #[must_use]
    pub fn default(&self) -> Option<&Expr<'a>> {
        self.default_value.as_ref()
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Function<'a> {
    parameters: Vec<FunctionParameter<'a>>,
    return_type: Type<'a>,
    attributes: Vec<Expr<'a>>,
    definition: Option<Expr<'a>>,
}

impl<'a> Function<'a> {
    #[must_use]
    pub(crate) fn new(
        parameters: Vec<FunctionParameter<'a>>,
        return_type: Type<'a>,
        attributes: Vec<Expr<'a>>,
        definition: Option<Expr<'a>>,
    ) -> Self {
        Function {
            parameters,
            return_type,
            attributes,
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
    pub fn return_type(&self) -> &Type<'a> {
        &self.return_type
    }

    pub fn set_return_type(&mut self, ty: Type<'a>) {
        self.return_type = ty;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Expr<'a>] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn definition(&self) -> Option<&Expr<'a>> {
        self.definition.as_ref()
    }

    pub fn set_definition(&mut self, definition: Option<Expr<'a>>) {
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum VariableKind {
    Let,
    Var,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Variable<'a> {
    kind: VariableKind,
    is_mutable: bool,
    attributes: Vec<Expr<'a>>,
    name: &'a str,
    var_type: Type<'a>,
    value: Option<Expr<'a>>,
}

impl<'a> Variable<'a> {
    #[must_use]
    pub(crate) fn new(
        kind: VariableKind,
        is_mutable: bool,
        attributes: Vec<Expr<'a>>,
        name: &'a str,
        var_type: Type<'a>,
        value: Option<Expr<'a>>,
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
    pub fn attributes(&self) -> &[Expr<'a>] {
        &self.attributes
    }

    #[must_use]
    pub fn attributes_mut(&mut self) -> &mut Vec<Expr<'a>> {
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
    pub fn get_type(&self) -> &Type<'a> {
        &self.var_type
    }

    pub fn set_type(&mut self, var_type: Type<'a>) {
        self.var_type = var_type;
    }

    #[must_use]
    pub fn value(&self) -> Option<&Expr<'a>> {
        self.value.as_ref()
    }

    pub fn set_value(&mut self, value: Option<Expr<'a>>) {
        self.value = value;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Identifier<'a> {
    full_name: String,
    segments: Vec<&'a str>,
}

impl<'a> Identifier<'a> {
    #[must_use]
    pub(crate) fn new(segments: Vec<&'a str>) -> Self {
        Identifier {
            full_name: segments.join("::"),
            segments,
        }
    }

    #[must_use]
    pub fn full_name(&self) -> &str {
        &self.full_name
    }

    #[must_use]
    pub fn segments(&self) -> &[&'a str] {
        &self.segments
    }

    #[must_use]
    pub fn is_absolute(&self) -> bool {
        self.segments.first() == Some(&"")
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct QualifiedScope<'a> {
    scopes: SmallVec<[&'a str; 3]>,
}

impl<'a> QualifiedScope<'a> {
    #[must_use]
    pub fn new(scopes: SmallVec<[&'a str; 3]>) -> Self {
        Self { scopes }
    }

    #[must_use]
    pub fn parse(qualified_scope: &'a str) -> Self {
        let parts = qualified_scope
            .split("::")
            .filter(|s| !s.is_empty())
            .collect::<SmallVec<[&'a str; 3]>>();
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

    pub fn push(&mut self, scope: &'a str) {
        self.scopes.push(scope);
    }

    #[must_use]
    pub fn scopes(&self) -> &[&'a str] {
        &self.scopes
    }
}

impl std::fmt::Display for QualifiedScope<'_> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.scopes().to_vec().join("::"))
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Scope<'a> {
    name: &'a str,
    attributes: Vec<Expr<'a>>,
    elements: Vec<Expr<'a>>,
}

impl<'a> Scope<'a> {
    #[must_use]
    pub fn new(name: &'a str, attributes: Vec<Expr<'a>>, elements: Vec<Expr<'a>>) -> Self {
        Scope {
            name,
            attributes,
            elements,
        }
    }

    #[must_use]
    pub fn name(&self) -> &'a str {
        self.name
    }

    pub fn set_name(&mut self, name: &'a str) {
        self.name = name;
    }

    #[must_use]
    pub fn attributes(&self) -> &[Expr<'a>] {
        &self.attributes
    }

    pub fn attributes_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.attributes
    }

    #[must_use]
    pub fn elements(&self) -> &[Expr<'a>] {
        &self.elements
    }

    pub fn elements_mut(&mut self) -> &mut Vec<Expr<'a>> {
        &mut self.elements
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct If<'a> {
    condition: Expr<'a>,
    then_branch: Expr<'a>,
    else_branch: Option<Expr<'a>>,
}

impl<'a> If<'a> {
    #[must_use]
    pub(crate) fn new(
        condition: Expr<'a>,
        then_branch: Expr<'a>,
        else_branch: Option<Expr<'a>>,
    ) -> Self {
        If {
            condition,
            then_branch,
            else_branch,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn then_branch(&self) -> &Expr<'a> {
        &self.then_branch
    }

    pub fn set_then_branch(&mut self, then_branch: Expr<'a>) {
        self.then_branch = then_branch;
    }

    #[must_use]
    pub fn else_branch(&self) -> Option<&Expr<'a>> {
        self.else_branch.as_ref()
    }

    pub fn set_else_branch(&mut self, else_branch: Option<Expr<'a>>) {
        self.else_branch = else_branch;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct WhileLoop<'a> {
    condition: Expr<'a>,
    body: Expr<'a>,
}

impl<'a> WhileLoop<'a> {
    #[must_use]
    pub(crate) fn new(condition: Expr<'a>, body: Expr<'a>) -> Self {
        WhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &Expr<'a> {
        &self.body
    }

    pub fn set_body(&mut self, body: Expr<'a>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct DoWhileLoop<'a> {
    condition: Expr<'a>,
    body: Expr<'a>,
}

impl<'a> DoWhileLoop<'a> {
    #[must_use]
    pub(crate) fn new(condition: Expr<'a>, body: Expr<'a>) -> Self {
        DoWhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &Expr<'a> {
        &self.body
    }

    pub fn set_body(&mut self, body: Expr<'a>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Switch<'a> {
    condition: Expr<'a>,
    cases: Vec<(Expr<'a>, Expr<'a>)>,
    default_case: Option<Expr<'a>>,
}

impl<'a> Switch<'a> {
    #[must_use]
    pub(crate) fn new(
        condition: Expr<'a>,
        cases: Vec<(Expr<'a>, Expr<'a>)>,
        default_case: Option<Expr<'a>>,
    ) -> Self {
        Switch {
            condition,
            cases,
            default_case,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn cases(&self) -> &[(Expr<'a>, Expr<'a>)] {
        &self.cases
    }

    #[must_use]
    pub fn cases_mut(&mut self) -> &mut Vec<(Expr<'a>, Expr<'a>)> {
        &mut self.cases
    }

    #[must_use]
    pub fn default_case(&self) -> Option<&Expr<'a>> {
        self.default_case.as_ref()
    }

    pub fn set_default_case(&mut self, default_case: Option<Expr<'a>>) {
        self.default_case = default_case;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
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
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
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
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Return<'a> {
    value: Option<Expr<'a>>,
}

impl<'a> Return<'a> {
    #[must_use]
    pub(crate) fn new(value: Option<Expr<'a>>) -> Self {
        Return { value }
    }

    #[must_use]
    pub fn value(&self) -> Option<&Expr<'a>> {
        self.value.as_ref()
    }

    pub fn set_value(&mut self, value: Option<Expr<'a>>) {
        self.value = value;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct ForEach<'a> {
    iterable: Expr<'a>,
    bindings: Vec<(&'a str, Option<Type<'a>>)>,
    body: Expr<'a>,
}

impl<'a> ForEach<'a> {
    #[must_use]
    pub(crate) fn new(
        bindings: Vec<(&'a str, Option<Type<'a>>)>,
        iterable: Expr<'a>,
        body: Expr<'a>,
    ) -> Self {
        ForEach {
            iterable,
            bindings,
            body,
        }
    }

    #[must_use]
    pub fn iterable(&self) -> &Expr<'a> {
        &self.iterable
    }

    pub fn set_iterable(&mut self, iterable: Expr<'a>) {
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
    pub fn body(&self) -> &Expr<'a> {
        &self.body
    }

    pub fn set_body(&mut self, body: Expr<'a>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Await<'a> {
    expression: Expr<'a>,
}

impl<'a> Await<'a> {
    #[must_use]
    pub(crate) fn new(expression: Expr<'a>) -> Self {
        Await { expression }
    }

    #[must_use]
    pub fn expression(&self) -> &Expr<'a> {
        &self.expression
    }

    pub fn set_expression(&mut self, expression: Expr<'a>) {
        self.expression = expression;
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Assert<'a> {
    condition: Expr<'a>,
    message: Expr<'a>,
}

impl<'a> Assert<'a> {
    #[must_use]
    pub(crate) fn new(condition: Expr<'a>, message: Expr<'a>) -> Self {
        Assert { condition, message }
    }

    #[must_use]
    pub fn condition(&self) -> &Expr<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Expr<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn message(&self) -> &Expr<'a> {
        &self.message
    }

    pub fn set_message(&mut self, message: Expr<'a>) {
        self.message = message;
    }
}

pub type CallArguments<'a> = Vec<(Option<&'a str>, Expr<'a>)>;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Call<'a> {
    callee: Expr<'a>,
    arguments: CallArguments<'a>,
}

impl<'a> Call<'a> {
    #[must_use]
    pub fn new(callee: Expr<'a>, arguments: CallArguments<'a>) -> Self {
        Call { callee, arguments }
    }

    #[must_use]
    pub fn callee(&self) -> &Expr<'a> {
        &self.callee
    }

    pub fn set_callee(&mut self, callee: Expr<'a>) {
        self.callee = callee;
    }

    #[must_use]
    pub fn arguments(&self) -> &[(Option<&'a str>, Expr<'a>)] {
        &self.arguments
    }

    #[must_use]
    pub fn arguments_mut(&mut self) -> &mut CallArguments<'a> {
        &mut self.arguments
    }

    pub fn set_arguments(&mut self, arguments: CallArguments<'a>) {
        self.arguments = arguments;
    }
}

#[derive(Clone, PartialEq, Eq, Hash)]
pub enum Expr<'a> {
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
    TypeName(Rc<Identifier<'a>>),
    RefinementType(Rc<RefinementType<'a>>),
    TupleType(Rc<TupleType<'a>>),
    ArrayType(Rc<ArrayType<'a>>),
    MapType(Rc<MapType<'a>>),
    SliceType(Rc<SliceType<'a>>),
    FunctionType(Rc<FunctionType<'a>>),
    ManagedRefType(Rc<ManagedRefType<'a>>),
    UnmanagedRefType(Rc<UnmanagedRefType<'a>>),
    GenericType(Rc<GenericType<'a>>),
    OpaqueType(Rc<StringData<'a>>),
    StructType(Rc<StructType<'a>>),
    LatentType(Rc<Expr<'a>>),
    HasParenthesesType(Rc<Type<'a>>),

    Discard,
    HasParentheses(Rc<Expr<'a>>),

    Boolean(bool),
    Integer(Rc<Integer>),
    Float(NotNan<f64>),
    String(Rc<StringData<'a>>),
    BString(Rc<BStringData<'a>>),
    Unit,

    TypeEnvelop(Rc<Type<'a>>),
    List(Rc<List<'a>>),
    Object(Rc<Object<'a>>),
    UnaryExpr(Rc<UnaryExpr<'a>>),
    BinExpr(Rc<BinExpr<'a>>),
    Statement(Rc<Statement<'a>>),
    Block(Rc<Block<'a>>),

    Function(Rc<Function<'a>>),
    Variable(Rc<Variable<'a>>),
    Identifier(Rc<Identifier<'a>>),
    Scope(Rc<Scope<'a>>),

    If(Rc<If<'a>>),
    WhileLoop(Rc<WhileLoop<'a>>),
    DoWhileLoop(Rc<DoWhileLoop<'a>>),
    Switch(Rc<Switch<'a>>),
    Break(Rc<Break<'a>>),
    Continue(Rc<Continue<'a>>),
    Return(Rc<Return<'a>>),
    ForEach(Rc<ForEach<'a>>),
    Await(Rc<Await<'a>>),
    Assert(Rc<Assert<'a>>),
    Call(Rc<Call<'a>>),
}

impl<'a> TryInto<Type<'a>> for Expr<'a> {
    type Error = Self;

    fn try_into(self) -> Result<Type<'a>, Self::Error> {
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

impl Expr<'_> {
    #[must_use]
    pub fn is_discard(&self) -> bool {
        matches!(self, Expr::Discard)
    }
}

impl<'a> std::fmt::Debug for Expr<'a> {
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
