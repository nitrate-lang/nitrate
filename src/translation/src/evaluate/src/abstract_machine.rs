use hashbrown::HashMap;
use interned_string::{IString, Intern};
use nitrate_structure::{Builder, kind::Expr};
use std::rc::Rc;

#[derive(Debug, Default)]
pub(crate) struct CallFrame {
    local_variables: HashMap<IString, Expr>,
}

impl CallFrame {
    fn get(&self, name: &IString) -> Option<&Expr> {
        self.local_variables.get(name)
    }

    pub(crate) fn set(&mut self, name: IString, expr: Expr) {
        self.local_variables.insert(name, expr);
    }
}

#[derive(Debug)]
pub(crate) struct Task {
    call_stack: Vec<CallFrame>,
    task_locals: HashMap<IString, Expr>,
}

impl Task {
    fn new() -> Self {
        let call_stack = Vec::from([CallFrame::default()]);
        Task {
            call_stack,
            task_locals: HashMap::new(),
        }
    }

    pub(crate) fn callstack_mut(&mut self) -> &mut Vec<CallFrame> {
        &mut self.call_stack
    }

    pub(crate) fn in_function(&self) -> bool {
        self.call_stack.len() > 1
    }

    pub(crate) fn add_task_local(&mut self, name: IString, expr: Expr) {
        self.task_locals.insert(name, expr);
    }
}

#[derive(Debug)]
pub enum Unwind {
    FunctionReturn(Expr),
    TypeError,
    MissingArgument,
    UnknownCallee(IString),
    UnresolvedIdentifier(IString),
}

pub type IntrinsicFunction = Rc<dyn Fn(&mut AbstractMachine) -> Result<Expr, Unwind>>;

pub struct AbstractMachine {
    global_variables: HashMap<IString, Expr>,
    provided_functions: HashMap<IString, IntrinsicFunction>,
    tasks: Vec<Task>,
    current_task: usize,
}

impl Default for AbstractMachine {
    fn default() -> Self {
        Self::new()
    }
}

impl AbstractMachine {
    #[must_use]
    pub fn new() -> Self {
        let mut abstract_machine = AbstractMachine {
            global_variables: HashMap::new(),
            provided_functions: HashMap::new(),
            tasks: Vec::from([Task::new()]),
            current_task: 0,
        };

        abstract_machine.setup_builtins();
        abstract_machine
    }

    pub(crate) fn current_task(&self) -> &Task {
        &self.tasks[self.current_task]
    }

    pub(crate) fn current_task_mut(&mut self) -> &mut Task {
        &mut self.tasks[self.current_task]
    }

    pub(crate) fn resolve_intrinsic(&self, name: &IString) -> Option<IntrinsicFunction> {
        self.provided_functions.get(name).cloned()
    }

    pub(crate) fn resolve(&self, name: &IString) -> Option<&Expr> {
        // TODO: Verify and write tests

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

    pub fn get_parameter(&self, name: &IString) -> Option<&Expr> {
        self.current_task().call_stack.last()?.get(name)
    }

    pub fn provide_function<F>(&mut self, name: IString, callback: F)
    where
        F: Fn(&mut AbstractMachine) -> Result<Expr, Unwind> + 'static,
    {
        self.provided_functions.insert(name, Rc::new(callback));
    }

    pub fn setup_builtins(&mut self) {
        self.provide_function(
            "std::intrinsic::print".intern(),
            |m: &mut AbstractMachine| {
                let value = m
                    .get_parameter(&"message".intern())
                    .ok_or(Unwind::MissingArgument)?;

                if let Expr::String(string) = value {
                    print!("{}", string);
                } else {
                    print!("{:#?}", value);
                }

                Ok(Builder::create_unit())
            },
        );
    }

    pub fn evaluate(&mut self, expression: &Expr) -> Result<Expr, Unwind> {
        // TODO: Verify and write tests

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
            Expr::TypeName(t) => Ok(Expr::TypeName(t.to_owned())),
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
            Expr::StructType(t) => self.evaluate_struct_type(t).map(Into::into),
            Expr::LatentType(t) => self.evaluate_latent_type(t).map(Into::into),
            Expr::HasParenthesesType(t) => self.evaluate_type(t).map(Into::into),

            Expr::Discard => Ok(Expr::Discard),
            Expr::HasParentheses(e) => self.evaluate(e),

            Expr::Boolean(e) => Ok(Expr::Boolean(e.to_owned())),
            Expr::Integer(e) => Ok(Expr::Integer(e.clone())),
            Expr::Float(e) => Ok(Expr::Float(e.to_owned())),
            Expr::String(e) => Ok(Expr::String(e.clone())),
            Expr::BString(e) => Ok(Expr::BString(e.clone())),
            Expr::Unit => Ok(Expr::Unit),

            Expr::TypeEnvelop(t) => self.evaluate_type_envelop(t),
            Expr::List(e) => self.evaluate_list(e),
            Expr::Object(e) => self.evaluate_object(e),
            Expr::UnaryExpr(e) => self.evaluate_unaryexpr(e),
            Expr::BinExpr(e) => self.evaluate_binexpr(e),
            Expr::Block(e) => self.evaluate_block(e),

            Expr::Function(_) => Ok(expression.clone()),
            Expr::Variable(e) => self.evaluate_variable(e),
            Expr::Identifier(e) => self.evaluate_identifier(e),
            Expr::IndexAccess(e) => self.evaluate_index_access(e),
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
            Expr::Call(e) => self.evaluate_call(e),
        }
    }
}
