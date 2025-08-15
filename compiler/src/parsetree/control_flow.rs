use super::expression::{ExprOwned, TypeOwned};
use std::{rc::Rc, sync::Arc};

#[derive(Debug, Clone, PartialEq)]
pub struct If<'a> {
    condition: Arc<ExprOwned<'a>>,
    then_branch: Arc<ExprOwned<'a>>,
    else_branch: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> If<'a> {
    #[must_use]
    pub(crate) fn new(
        condition: Arc<ExprOwned<'a>>,
        then_branch: Arc<ExprOwned<'a>>,
        else_branch: Option<Arc<ExprOwned<'a>>>,
    ) -> Self {
        If {
            condition,
            then_branch,
            else_branch,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<ExprOwned<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<ExprOwned<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn then_branch(&self) -> &Arc<ExprOwned<'a>> {
        &self.then_branch
    }

    pub fn set_then_branch(&mut self, then_branch: Arc<ExprOwned<'a>>) {
        self.then_branch = then_branch;
    }

    #[must_use]
    pub fn else_branch(&self) -> Option<&Arc<ExprOwned<'a>>> {
        self.else_branch.as_ref()
    }

    pub fn set_else_branch(&mut self, else_branch: Option<Arc<ExprOwned<'a>>>) {
        self.else_branch = else_branch;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct WhileLoop<'a> {
    condition: Arc<ExprOwned<'a>>,
    body: Arc<ExprOwned<'a>>,
}

impl<'a> WhileLoop<'a> {
    #[must_use]
    pub(crate) fn new(condition: Arc<ExprOwned<'a>>, body: Arc<ExprOwned<'a>>) -> Self {
        WhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<ExprOwned<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<ExprOwned<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &Arc<ExprOwned<'a>> {
        &self.body
    }

    pub fn set_body(&mut self, body: Arc<ExprOwned<'a>>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct DoWhileLoop<'a> {
    condition: Arc<ExprOwned<'a>>,
    body: Arc<ExprOwned<'a>>,
}

impl<'a> DoWhileLoop<'a> {
    #[must_use]
    pub(crate) fn new(condition: Arc<ExprOwned<'a>>, body: Arc<ExprOwned<'a>>) -> Self {
        DoWhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<ExprOwned<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<ExprOwned<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &Arc<ExprOwned<'a>> {
        &self.body
    }

    pub fn set_body(&mut self, body: Arc<ExprOwned<'a>>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Switch<'a> {
    condition: Arc<ExprOwned<'a>>,
    cases: Vec<(Arc<ExprOwned<'a>>, Arc<ExprOwned<'a>>)>,
    default_case: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> Switch<'a> {
    #[must_use]
    pub(crate) fn new(
        condition: Arc<ExprOwned<'a>>,
        cases: Vec<(Arc<ExprOwned<'a>>, Arc<ExprOwned<'a>>)>,
        default_case: Option<Arc<ExprOwned<'a>>>,
    ) -> Self {
        Switch {
            condition,
            cases,
            default_case,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<ExprOwned<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<ExprOwned<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn cases(&self) -> &[(Arc<ExprOwned<'a>>, Arc<ExprOwned<'a>>)] {
        &self.cases
    }

    #[must_use]
    pub fn cases_mut(&mut self) -> &mut Vec<(Arc<ExprOwned<'a>>, Arc<ExprOwned<'a>>)> {
        &mut self.cases
    }

    #[must_use]
    pub fn default_case(&self) -> Option<&Arc<ExprOwned<'a>>> {
        self.default_case.as_ref()
    }

    pub fn set_default_case(&mut self, default_case: Option<Arc<ExprOwned<'a>>>) {
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
    value: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> Return<'a> {
    #[must_use]
    pub(crate) fn new(value: Option<Arc<ExprOwned<'a>>>) -> Self {
        Return { value }
    }

    #[must_use]
    pub fn value(&self) -> Option<Arc<ExprOwned<'a>>> {
        self.value.clone()
    }

    pub fn set_value(&mut self, value: Option<Arc<ExprOwned<'a>>>) {
        self.value = value;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct ForEach<'a> {
    iterable: Arc<ExprOwned<'a>>,
    bindings: Vec<(&'a str, Option<Rc<TypeOwned<'a>>>)>,
    body: Arc<ExprOwned<'a>>,
}

impl<'a> ForEach<'a> {
    #[must_use]
    pub(crate) fn new(
        bindings: Vec<(&'a str, Option<Rc<TypeOwned<'a>>>)>,
        iterable: Arc<ExprOwned<'a>>,
        body: Arc<ExprOwned<'a>>,
    ) -> Self {
        ForEach {
            iterable,
            bindings,
            body,
        }
    }

    #[must_use]
    pub fn iterable(&self) -> &Arc<ExprOwned<'a>> {
        &self.iterable
    }

    pub fn set_iterable(&mut self, iterable: Arc<ExprOwned<'a>>) {
        self.iterable = iterable;
    }

    #[must_use]
    pub fn bindings(&self) -> &[(&'a str, Option<Rc<TypeOwned<'a>>>)] {
        &self.bindings
    }

    #[must_use]
    pub fn bindings_mut(&mut self) -> &mut Vec<(&'a str, Option<Rc<TypeOwned<'a>>>)> {
        &mut self.bindings
    }

    #[must_use]
    pub fn body(&self) -> &Arc<ExprOwned<'a>> {
        &self.body
    }

    pub fn set_body(&mut self, body: Arc<ExprOwned<'a>>) {
        self.body = body;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Await<'a> {
    expression: Arc<ExprOwned<'a>>,
}

impl<'a> Await<'a> {
    #[must_use]
    pub(crate) fn new(expression: Arc<ExprOwned<'a>>) -> Self {
        Await { expression }
    }

    #[must_use]
    pub fn expression(&self) -> &Arc<ExprOwned<'a>> {
        &self.expression
    }

    pub fn set_expression(&mut self, expression: Arc<ExprOwned<'a>>) {
        self.expression = expression;
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Assert<'a> {
    condition: Arc<ExprOwned<'a>>,
    message: Option<Arc<ExprOwned<'a>>>,
}

impl<'a> Assert<'a> {
    #[must_use]
    pub(crate) fn new(condition: Arc<ExprOwned<'a>>, message: Option<Arc<ExprOwned<'a>>>) -> Self {
        Assert { condition, message }
    }

    #[must_use]
    pub fn condition(&self) -> &Arc<ExprOwned<'a>> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: Arc<ExprOwned<'a>>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn message(&self) -> Option<&Arc<ExprOwned<'a>>> {
        self.message.as_ref()
    }

    pub fn set_message(&mut self, message: Option<Arc<ExprOwned<'a>>>) {
        self.message = message;
    }
}
