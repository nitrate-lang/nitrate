use super::expression::{Expr, Type};
use std::{rc::Rc, sync::Arc};

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
    bindings: Vec<(&'a str, Option<Rc<Type<'a>>>)>,
    body: Arc<Expr<'a>>,
}

impl<'a> ForEach<'a> {
    #[must_use]
    pub(crate) fn new(
        bindings: Vec<(&'a str, Option<Rc<Type<'a>>>)>,
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
    pub fn bindings(&self) -> &[(&'a str, Option<Rc<Type<'a>>>)] {
        &self.bindings
    }

    #[must_use]
    pub fn bindings_mut(&mut self) -> &mut Vec<(&'a str, Option<Rc<Type<'a>>>)> {
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
