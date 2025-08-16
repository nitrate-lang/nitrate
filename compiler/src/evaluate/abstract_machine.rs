use crate::parsetree::{nodes::*, *};
use hashbrown::{HashMap, HashSet};
use std::ops::Deref;

#[derive(Debug, Default)]
struct CallFrame<'a> {
    function_locals: HashMap<&'a str, Variable<'a>>,
}

impl<'a> CallFrame<'a> {
    fn get(&self, name: &str) -> Option<&Variable<'a>> {
        self.function_locals.get(name)
    }

    fn _set(&mut self, name: &'a str, variable: Variable<'a>) {
        self.function_locals.insert(name, variable);
    }
}

#[derive(Debug)]
struct Task<'a> {
    call_stack: Vec<CallFrame<'a>>,
    _task_locals: HashMap<&'a str, Variable<'a>>,
    _task_id: u64,
}

impl<'a> Task<'a> {
    fn new(task_id: u64) -> Self {
        let call_stack = Vec::from([CallFrame::default()]);
        Task {
            call_stack,
            _task_locals: HashMap::new(),
            _task_id: task_id,
        }
    }
}

pub enum FunctionError {
    MissingArgument,
    TypeError,
}

pub type Function<'a> = fn(&mut AbstractMachine<'a>) -> Result<Expr<'a>, FunctionError>;

pub enum EvalError {
    TypeError,
}

pub struct AbstractMachine<'a> {
    _global_variables: HashMap<&'a str, Variable<'a>>,
    provided_functions: HashMap<&'a str, Function<'a>>,
    tasks: Vec<Task<'a>>,
    current_task: usize,

    pub(crate) already_evaluated_types: HashSet<Type<'a>>,
}

impl<'a> AbstractMachine<'a> {
    pub fn new() -> Self {
        let mut abstract_machine = AbstractMachine {
            _global_variables: HashMap::new(),
            provided_functions: HashMap::new(),
            tasks: Vec::from([Task::new(0)]),
            current_task: 0,
            already_evaluated_types: HashSet::new(),
        };

        abstract_machine.setup_builtins();
        abstract_machine.setup_runtime();
        abstract_machine
    }

    fn setup_builtins(&mut self) {
        self.provide_function("std::intrinsic::print", |m| {
            let string_to_print = m.tasks[m.current_task]
                .call_stack
                .last()
                .expect("No call frame found")
                .get("msg")
                .ok_or(FunctionError::MissingArgument)?
                .value()
                .ok_or(FunctionError::MissingArgument)?;

            match string_to_print {
                Expr::StringLit(string) => {
                    print!("{}", string.get());
                    Ok(Builder::create_unit())
                }
                _ => Err(FunctionError::TypeError),
            }
        });

        // TODO: Implement other intrinsic functions
    }

    fn setup_runtime(&mut self) {
        // TODO: Implement runtime setup logic
        // Create some global variables, garbage collector?, etc.
    }

    pub fn provide_function(
        &mut self,
        name: &'a str,
        callback: Function<'a>,
    ) -> Option<Function<'a>> {
        self.provided_functions.insert(name, callback)
    }

    pub fn evaluate(&mut self, expression: &Expr<'a>) -> Result<Expr<'a>, EvalError> {
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
            Expr::RefinementType(t) => self.evaluate_refinement_type(t).map(|t| t.into()),
            Expr::TupleType(t) => self.evaluate_tuple_type(t).map(|t| t.into()),
            Expr::ArrayType(t) => self.evaluate_array_type(t).map(|t| t.into()),
            Expr::MapType(t) => self.evaluate_map_type(t).map(|t| t.into()),
            Expr::SliceType(t) => self.evaluate_slice_type(t).map(|t| t.into()),
            Expr::FunctionType(t) => self.evaluate_function_type(t).map(|t| t.into()),
            Expr::ManagedRefType(t) => self.evaluate_managed_ref_type(t).map(|t| t.into()),
            Expr::UnmanagedRefType(t) => self.evaluate_unmanaged_ref_type(t).map(|t| t.into()),
            Expr::GenericType(t) => self.evaluate_generic_type(t).map(|t| t.into()),
            Expr::HasParenthesesType(t) => self.evaluate_type(t).map(|t| t.into()),

            Expr::Discard => Ok(Expr::Discard),
            Expr::HasParentheses(e) => self.evaluate(e.deref()),

            Expr::BooleanLit(e) => Ok(Expr::BooleanLit(e.to_owned())),
            Expr::IntegerLit(e) => Ok(Expr::IntegerLit(e.clone())),
            Expr::FloatLit(e) => Ok(Expr::FloatLit(e.to_owned())),
            Expr::StringLit(e) => Ok(Expr::StringLit(e.clone())),
            Expr::BStringLit(e) => Ok(Expr::BStringLit(e.clone())),
            Expr::UnitLit => Ok(Expr::UnitLit),

            Expr::List(e) => self.evaluate_list(e),
            Expr::Object(e) => self.evaluate_object(e),
            Expr::UnaryExpr(e) => self.evaluate_unaryexpr(e),
            Expr::BinExpr(e) => self.evaluate_binexpr(e),
            Expr::Statement(e) => self.evaluate_statement(e),
            Expr::Block(e) => self.evaluate_block(e),

            Expr::Function(_) => {
                // TODO: Evaluate function definition
                unimplemented!()
            }

            Expr::Variable(_) => {
                // TODO: Evaluate variable declaration
                unimplemented!()
            }

            Expr::Identifier(_) => {
                // TODO: Evaluate identifier
                unimplemented!()
            }

            Expr::Scope(_) => {
                // TODO: Evaluate scope
                unimplemented!()
            }

            Expr::If(e) => self.evaluate_if(e),
            Expr::WhileLoop(e) => self.evaluate_while(e),
            Expr::DoWhileLoop(e) => self.evaluate_do_while(e),

            Expr::Switch(_) => {
                // TODO: Evaluate switch
                unimplemented!()
            }

            Expr::Break(_) => {
                // TODO: Evaluate break
                unimplemented!()
            }

            Expr::Continue(_) => {
                // TODO: Evaluate continue
                unimplemented!()
            }

            Expr::Return(_) => {
                // TODO: Evaluate return
                unimplemented!()
            }

            Expr::ForEach(_) => {
                // TODO: Evaluate for-each
                unimplemented!()
            }

            Expr::Await(_) => {
                // TODO: Evaluate await
                unimplemented!()
            }

            Expr::Assert(_) => {
                // TODO: Evaluate assert
                unimplemented!()
            }
        }
    }
}
