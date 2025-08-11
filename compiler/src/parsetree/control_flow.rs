use super::storage::{ExprKey, TypeKey};

#[derive(Debug, Clone)]
pub struct If<'a> {
    condition: ExprKey<'a>,
    then_branch: ExprKey<'a>,
    else_branch: Option<ExprKey<'a>>,
}

impl<'a> If<'a> {
    #[must_use]
    pub(crate) fn new(
        condition: ExprKey<'a>,
        then_branch: ExprKey<'a>,
        else_branch: Option<ExprKey<'a>>,
    ) -> Self {
        If {
            condition,
            then_branch,
            else_branch,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &ExprKey<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: ExprKey<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn then_branch(&self) -> &ExprKey<'a> {
        &self.then_branch
    }

    pub fn set_then_branch(&mut self, then_branch: ExprKey<'a>) {
        self.then_branch = then_branch;
    }

    #[must_use]
    pub fn else_branch(&self) -> Option<&ExprKey<'a>> {
        self.else_branch.as_ref()
    }

    pub fn set_else_branch(&mut self, else_branch: Option<ExprKey<'a>>) {
        self.else_branch = else_branch;
    }
}

#[derive(Debug, Clone)]
pub struct WhileLoop<'a> {
    condition: ExprKey<'a>,
    body: ExprKey<'a>,
}

impl<'a> WhileLoop<'a> {
    #[must_use]
    pub(crate) fn new(condition: ExprKey<'a>, body: ExprKey<'a>) -> Self {
        WhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &ExprKey<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: ExprKey<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &ExprKey<'a> {
        &self.body
    }

    pub fn set_body(&mut self, body: ExprKey<'a>) {
        self.body = body;
    }
}

#[derive(Debug, Clone)]
pub struct DoWhileLoop<'a> {
    condition: ExprKey<'a>,
    body: ExprKey<'a>,
}

impl<'a> DoWhileLoop<'a> {
    #[must_use]
    pub(crate) fn new(condition: ExprKey<'a>, body: ExprKey<'a>) -> Self {
        DoWhileLoop { condition, body }
    }

    #[must_use]
    pub fn condition(&self) -> &ExprKey<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: ExprKey<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn body(&self) -> &ExprKey<'a> {
        &self.body
    }

    pub fn set_body(&mut self, body: ExprKey<'a>) {
        self.body = body;
    }
}

#[derive(Debug, Clone)]
pub struct Switch<'a> {
    condition: ExprKey<'a>,
    cases: Vec<(ExprKey<'a>, ExprKey<'a>)>,
    default_case: Option<ExprKey<'a>>,
}

impl<'a> Switch<'a> {
    #[must_use]
    pub(crate) fn new(
        condition: ExprKey<'a>,
        cases: Vec<(ExprKey<'a>, ExprKey<'a>)>,
        default_case: Option<ExprKey<'a>>,
    ) -> Self {
        Switch {
            condition,
            cases,
            default_case,
        }
    }

    #[must_use]
    pub fn condition(&self) -> &ExprKey<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: ExprKey<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn cases(&self) -> &[(ExprKey<'a>, ExprKey<'a>)] {
        &self.cases
    }

    #[must_use]
    pub fn cases_mut(&mut self) -> &mut Vec<(ExprKey<'a>, ExprKey<'a>)> {
        &mut self.cases
    }

    #[must_use]
    pub fn default_case(&self) -> Option<&ExprKey<'a>> {
        self.default_case.as_ref()
    }

    pub fn set_default_case(&mut self, default_case: Option<ExprKey<'a>>) {
        self.default_case = default_case;
    }
}

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
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

#[derive(Debug, Clone)]
pub struct Return<'a> {
    value: Option<ExprKey<'a>>,
}

impl<'a> Return<'a> {
    #[must_use]
    pub(crate) fn new(value: Option<ExprKey<'a>>) -> Self {
        Return { value }
    }

    #[must_use]
    pub fn value(&self) -> Option<ExprKey<'a>> {
        self.value
    }

    pub fn set_value(&mut self, value: Option<ExprKey<'a>>) {
        self.value = value;
    }
}

#[derive(Debug, Clone)]
pub struct ForEach<'a> {
    iterable: ExprKey<'a>,
    bindings: Vec<(&'a str, Option<TypeKey<'a>>)>,
    body: ExprKey<'a>,
}

impl<'a> ForEach<'a> {
    #[must_use]
    pub(crate) fn new(
        bindings: Vec<(&'a str, Option<TypeKey<'a>>)>,
        iterable: ExprKey<'a>,
        body: ExprKey<'a>,
    ) -> Self {
        ForEach {
            iterable,
            bindings,
            body,
        }
    }

    #[must_use]
    pub fn iterable(&self) -> &ExprKey<'a> {
        &self.iterable
    }

    pub fn set_iterable(&mut self, iterable: ExprKey<'a>) {
        self.iterable = iterable;
    }

    #[must_use]
    pub fn bindings(&self) -> &[(&'a str, Option<TypeKey<'a>>)] {
        &self.bindings
    }

    #[must_use]
    pub fn bindings_mut(&mut self) -> &mut Vec<(&'a str, Option<TypeKey<'a>>)> {
        &mut self.bindings
    }

    #[must_use]
    pub fn body(&self) -> &ExprKey<'a> {
        &self.body
    }

    pub fn set_body(&mut self, body: ExprKey<'a>) {
        self.body = body;
    }
}

#[derive(Debug, Clone)]
pub struct Await<'a> {
    expression: ExprKey<'a>,
}

impl<'a> Await<'a> {
    #[must_use]
    pub(crate) fn new(expression: ExprKey<'a>) -> Self {
        Await { expression }
    }

    #[must_use]
    pub fn expression(&self) -> &ExprKey<'a> {
        &self.expression
    }

    pub fn set_expression(&mut self, expression: ExprKey<'a>) {
        self.expression = expression;
    }
}

#[derive(Debug, Clone)]
pub struct Assert<'a> {
    condition: ExprKey<'a>,
    message: Option<ExprKey<'a>>,
}

impl<'a> Assert<'a> {
    #[must_use]
    pub(crate) fn new(condition: ExprKey<'a>, message: Option<ExprKey<'a>>) -> Self {
        Assert { condition, message }
    }

    #[must_use]
    pub fn condition(&self) -> &ExprKey<'a> {
        &self.condition
    }

    pub fn set_condition(&mut self, condition: ExprKey<'a>) {
        self.condition = condition;
    }

    #[must_use]
    pub fn message(&self) -> Option<&ExprKey<'a>> {
        self.message.as_ref()
    }

    pub fn set_message(&mut self, message: Option<ExprKey<'a>>) {
        self.message = message;
    }
}
