use crate::parsetree::{Builder, Expr, Type};
use hashbrown::{HashMap, HashSet};
use std::rc::Rc;

#[derive(Debug, Default)]
pub(crate) struct CallFrame<'a> {
    local_variables: HashMap<&'a str, Expr<'a>>,
}

impl<'a> CallFrame<'a> {
    fn get(&self, name: &str) -> Option<&Expr<'a>> {
        self.local_variables.get(name)
    }

    pub(crate) fn set(&mut self, name: &'a str, expr: Expr<'a>) {
        self.local_variables.insert(name, expr);
    }
}

#[derive(Debug)]
pub(crate) struct Task<'a> {
    call_stack: Vec<CallFrame<'a>>,
    task_locals: HashMap<&'a str, Expr<'a>>,
}

impl<'a> Task<'a> {
    fn new() -> Self {
        let call_stack = Vec::from([CallFrame::default()]);
        Task {
            call_stack,
            task_locals: HashMap::new(),
        }
    }

    pub(crate) fn callstack_mut(&mut self) -> &mut Vec<CallFrame<'a>> {
        &mut self.call_stack
    }

    pub(crate) fn in_function(&self) -> bool {
        self.call_stack.len() > 1
    }

    pub(crate) fn add_task_local(&mut self, name: &'a str, expr: Expr<'a>) {
        self.task_locals.insert(name, expr);
    }
}

#[derive(Debug)]
pub enum Unwind<'a> {
    FunctionReturn(Expr<'a>),
    TypeError,
    MissingArgument,
    UnknownCallee(&'a str),
    UnresolvedIdentifier(&'a str),
    ProgramaticAssertionFailed(String),
}

pub type IntrinsicFunction<'a> =
    Rc<dyn Fn(&mut AbstractMachine<'a>) -> Result<Expr<'a>, Unwind<'a>>>;

pub struct AbstractMachine<'a> {
    global_variables: HashMap<&'a str, Expr<'a>>,
    provided_functions: HashMap<&'a str, IntrinsicFunction<'a>>,
    tasks: Vec<Task<'a>>,
    current_task: usize,

    pub(crate) already_evaluated_types: HashSet<Type<'a>>,
}

impl Default for AbstractMachine<'_> {
    fn default() -> Self {
        Self::new()
    }
}

impl<'a> AbstractMachine<'a> {
    #[must_use]
    pub fn new() -> Self {
        let mut abstract_machine = AbstractMachine {
            global_variables: HashMap::new(),
            provided_functions: HashMap::new(),
            tasks: Vec::from([Task::new()]),
            current_task: 0,
            already_evaluated_types: HashSet::new(),
        };

        abstract_machine.setup_builtins();
        abstract_machine
    }

    fn setup_builtins(&mut self) {
        self.provide_function("std::intrinsic::print", |m: &mut AbstractMachine<'a>| {
            let Expr::String(string) = m.get_parameter("message").ok_or(Unwind::MissingArgument)?
            else {
                return Err(Unwind::TypeError);
            };

            print!("{}", string.get());

            Ok(Builder::create_unit())
        });
    }

    pub(crate) fn current_task(&self) -> &Task<'a> {
        &self.tasks[self.current_task]
    }

    pub(crate) fn current_task_mut(&mut self) -> &mut Task<'a> {
        &mut self.tasks[self.current_task]
    }

    pub fn resolve(&self, name: &str) -> Option<&Expr<'a>> {
        // TODO: Write tests
        // TODO: Verify logic

        if let Some(local_var) = self
            .current_task()
            .call_stack
            .last()
            .and_then(|frame| frame.get(name))
        {
            return Some(local_var);
        }

        if let Some(task_local_var) = self.current_task().task_locals.get(name) {
            return Some(task_local_var);
        }

        if let Some(global_var) = self.global_variables.get(name) {
            return Some(global_var);
        }

        None
    }

    pub fn resolve_intrinsic(&self, name: &str) -> Option<IntrinsicFunction<'a>> {
        self.provided_functions.get(name).cloned()
    }

    pub fn get_parameter(&self, name: &str) -> Option<&Expr<'a>> {
        self.current_task().call_stack.last()?.get(name)
    }

    pub fn provide_function<F>(&mut self, name: &'a str, callback: F)
    where
        F: Fn(&mut AbstractMachine<'a>) -> Result<Expr<'a>, Unwind<'a>> + 'static,
    {
        self.provided_functions.insert(name, Rc::new(callback));
    }

    pub fn evaluate(&mut self, expression: &Expr<'a>) -> Result<Expr<'a>, Unwind<'a>> {
        // TODO: Write tests
        // TODO: Verify logic

        match expression {
            Expr::Bool => Ok(Expr::Bool),
            Expr::UInt8 => Ok(Expr::UInt8),
            Expr::UInt16 => Ok(Expr::UInt16),
            Expr::UInt32 => Ok(Expr::UInt32),
            Expr::UInt64 => Ok(Expr::UInt64),
            Expr::UInt128 => Ok(Expr::UInt128),
            Expr::Int8 => Ok(Expr::Int8),
            Expr::Int16 => Ok(Expr::Int16),
            Expr::Int32 => Ok(Expr::Int32),
            Expr::Int64 => Ok(Expr::Int64),
            Expr::Int128 => Ok(Expr::Int128),
            Expr::Float8 => Ok(Expr::Float8),
            Expr::Float16 => Ok(Expr::Float16),
            Expr::Float32 => Ok(Expr::Float32),
            Expr::Float64 => Ok(Expr::Float64),
            Expr::Float128 => Ok(Expr::Float128),
            Expr::UnitType => Ok(Expr::UnitType),
            Expr::InferType => Ok(Expr::InferType),
            Expr::TypeName(t) => Ok(Expr::TypeName(t)),
            Expr::OpaqueType(t) => Ok(Expr::OpaqueType(t.clone())),
            Expr::RefinementType(t) => self.evaluate_refinement_type(t).map(Into::into),
            Expr::TupleType(t) => self.evaluate_tuple_type(t).map(Into::into),
            Expr::ArrayType(t) => self.evaluate_array_type(t).map(Into::into),
            Expr::MapType(t) => self.evaluate_map_type(t).map(Into::into),
            Expr::SliceType(t) => self.evaluate_slice_type(t).map(Into::into),
            Expr::FunctionType(t) => self.evaluate_function_type(t).map(Into::into),
            Expr::ManagedRefType(t) => self.evaluate_managed_ref_type(t).map(Into::into),
            Expr::UnmanagedRefType(t) => self.evaluate_unmanaged_ref_type(t).map(Into::into),
            Expr::GenericType(t) => self.evaluate_generic_type(t).map(Into::into),
            Expr::StructType(t) => self.evaluate_struct_type(t.to_owned()).map(Into::into),
            Expr::LatentType(t) => self.evaluate_latent_type(t.to_owned()).map(Into::into),
            Expr::HasParenthesesType(t) => self.evaluate_type(t).map(Into::into),

            Expr::Discard => Ok(Expr::Discard),
            Expr::HasParentheses(e) => self.evaluate(e),

            Expr::Boolean(e) => Ok(Expr::Boolean(e.to_owned())),
            Expr::Integer(e) => Ok(Expr::Integer(e.clone())),
            Expr::Float(e) => Ok(Expr::Float(e.to_owned())),
            Expr::String(e) => Ok(Expr::String(e.clone())),
            Expr::BString(e) => Ok(Expr::BString(e.clone())),
            Expr::Unit => Ok(Expr::Unit),

            Expr::List(e) => self.evaluate_list(e),
            Expr::Object(e) => self.evaluate_object(e),
            Expr::UnaryExpr(e) => self.evaluate_unaryexpr(e),
            Expr::BinExpr(e) => self.evaluate_binexpr(e),
            Expr::Statement(e) => self.evaluate_statement(e),
            Expr::Block(e) => self.evaluate_block(e),

            Expr::Function(_) => Ok(expression.clone()),
            Expr::Variable(e) => self.evaluate_variable(e),
            Expr::Identifier(e) => self.evaluate_identifier(e),
            Expr::Scope(e) => self.evaluate_scope(e),

            Expr::If(e) => self.evaluate_if(e),
            Expr::WhileLoop(e) => self.evaluate_while(e),
            Expr::DoWhileLoop(e) => self.evaluate_do_while(e),
            Expr::Switch(e) => self.evaluate_switch(e),
            Expr::Break(e) => self.evaluate_break(e),
            Expr::Continue(e) => self.evaluate_continue(e),
            Expr::Return(e) => self.evaluate_return(e),
            Expr::ForEach(e) => self.evaluate_for_each(e),
            Expr::Await(e) => self.evaluate_await(e),
            Expr::Assert(e) => self.evaluate_assert(e),
            Expr::Call(e) => self.evaluate_call(e),
        }
    }
}
