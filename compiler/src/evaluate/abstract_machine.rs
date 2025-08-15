use std::{collections::BTreeMap, ops::Deref};

use crate::parsetree::{nodes::*, *};
use hashbrown::{HashMap, HashSet};

#[derive(Debug, Default)]
struct CallFrame<'a> {
    function_locals: HashMap<&'a str, Variable<'a>>,
}

impl<'a> CallFrame<'a> {
    fn get(&self, name: &str) -> Option<&Variable<'a>> {
        self.function_locals.get(name)
    }

    fn set(&mut self, name: &'a str, variable: Variable<'a>) {
        self.function_locals.insert(name, variable);
    }
}

#[derive(Debug)]
struct Task<'a> {
    call_stack: Vec<CallFrame<'a>>,
    task_locals: HashMap<&'a str, Variable<'a>>,
    task_id: u64,
}

impl<'a> Task<'a> {
    fn new(task_id: u64) -> Self {
        let call_stack = Vec::from([CallFrame::default()]);
        Task {
            call_stack,
            task_locals: HashMap::new(),
            task_id,
        }
    }
}

pub enum FunctionError {
    MissingArgument,
    TypeError,
}

pub type Function<'a> = fn(&mut AbstractMachine<'a>) -> Result<Expr<'a>, FunctionError>;

pub struct AbstractMachine<'a> {
    global_variables: HashMap<&'a str, Variable<'a>>,
    provided_functions: HashMap<&'a str, Function<'a>>,
    tasks: Vec<Task<'a>>,
    current_task: usize,

    already_evaluated_types: HashSet<Type<'a>>,
}

impl<'a> AbstractMachine<'a> {
    pub fn new() -> Self {
        let mut abstract_machine = AbstractMachine {
            global_variables: HashMap::new(),
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
                    Ok(Builder::get_unit().into())
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

    pub fn evaluate(&mut self, expression: &Expr<'a>) -> Expr<'a> {
        match expression {
            Expr::Bool => Expr::Bool,
            Expr::UInt8 => Expr::UInt8,
            Expr::UInt16 => Expr::UInt16,
            Expr::UInt32 => Expr::UInt32,
            Expr::UInt64 => Expr::UInt64,
            Expr::UInt128 => Expr::UInt128,
            Expr::Int8 => Expr::Int8,
            Expr::Int16 => Expr::Int16,
            Expr::Int32 => Expr::Int32,
            Expr::Int64 => Expr::Int64,
            Expr::Int128 => Expr::Int128,
            Expr::Float8 => Expr::Float8,
            Expr::Float16 => Expr::Float16,
            Expr::Float32 => Expr::Float32,
            Expr::Float64 => Expr::Float64,
            Expr::Float128 => Expr::Float128,
            Expr::InferType => Expr::InferType,
            Expr::TypeName(str) => Expr::TypeName(str),
            Expr::OpaqueType(identity) => Expr::OpaqueType(identity.clone()),

            Expr::RefinementType(_)
            | Expr::TupleType(_)
            | Expr::ArrayType(_)
            | Expr::MapType(_)
            | Expr::SliceType(_)
            | Expr::FunctionType(_)
            | Expr::ManagedRefType(_)
            | Expr::UnmanagedRefType(_)
            | Expr::GenericType(_)
            | Expr::HasParenthesesType(_) => {
                let type_expression = expression
                    .clone()
                    .try_into()
                    .expect("Expected type expression");
                self.evaluate_type(&type_expression).into()
            }

            Expr::Discard => Expr::Discard,
            Expr::HasParentheses(expr) => self.evaluate(expr.deref()),

            Expr::BooleanLit(bool) => Expr::BooleanLit(bool.to_owned()),
            Expr::IntegerLit(int) => Expr::IntegerLit(int.clone()),
            Expr::FloatLit(float) => Expr::FloatLit(float.to_owned()),
            Expr::StringLit(string) => Expr::StringLit(string.clone()),
            Expr::BStringLit(bstring) => Expr::BStringLit(bstring.clone()),

            Expr::ListLit(list) => {
                let elements = list
                    .elements()
                    .iter()
                    .map(|e| self.evaluate(e))
                    .collect::<Vec<_>>();

                Builder::create_list().add_elements(elements).build()
            }

            Expr::ObjectLit(object) => {
                let fields = object
                    .get()
                    .iter()
                    .map(|(k, v)| (*k, self.evaluate(v)))
                    .collect::<BTreeMap<_, _>>();

                Builder::create_object().add_fields(fields).build()
            }

            Expr::UnaryExpr(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::BinExpr(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Statement(statement) => {
                self.evaluate(&statement.get());
                Builder::get_unit().into()
            }

            Expr::Block(block) => {
                let mut result = None;
                for element in block.elements() {
                    result = Some(self.evaluate(element));
                }

                result.unwrap_or_else(|| Builder::get_unit().into())
            }

            Expr::Function(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Variable(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Identifier(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Scope(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::If(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::WhileLoop(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::DoWhileLoop(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Switch(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Break(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Continue(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Return(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::ForEach(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Await(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            Expr::Assert(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }
        }
    }

    pub fn evaluate_type(&mut self, type_expression: &Type<'a>) -> Type<'a> {
        if self.already_evaluated_types.contains(type_expression) {
            return type_expression.to_owned();
        }

        let result = match type_expression {
            Type::Bool => Type::Bool,
            Type::UInt8 => Type::UInt8,
            Type::UInt16 => Type::UInt16,
            Type::UInt32 => Type::UInt32,
            Type::UInt64 => Type::UInt64,
            Type::UInt128 => Type::UInt128,
            Type::Int8 => Type::Int8,
            Type::Int16 => Type::Int16,
            Type::Int32 => Type::Int32,
            Type::Int64 => Type::Int64,
            Type::Int128 => Type::Int128,
            Type::Float8 => Type::Float8,
            Type::Float16 => Type::Float16,
            Type::Float32 => Type::Float32,
            Type::Float64 => Type::Float64,
            Type::Float128 => Type::Float128,
            Type::InferType => Type::InferType,
            Type::TypeName(name) => Type::TypeName(*name),
            Type::OpaqueType(identity) => Type::OpaqueType(identity.clone()),

            Type::RefinementType(refinement) => {
                let width = refinement.width().map(|w| self.evaluate(&w));
                let min = refinement.min().map(|m| self.evaluate(&m));
                let max = refinement.max().map(|m| self.evaluate(&m));
                let base = self.evaluate_type(&refinement.base());

                Builder::create_refinement_type()
                    .with_width(width)
                    .with_minimum(min)
                    .with_maximum(max)
                    .with_base(base)
                    .build()
            }

            Type::TupleType(tuple) => {
                let elements = tuple
                    .elements()
                    .iter()
                    .map(|t| self.evaluate_type(t))
                    .collect::<Vec<_>>();

                Builder::create_tuple_type().add_elements(elements).build()
            }

            Type::ArrayType(array) => {
                let element = self.evaluate_type(&array.element());
                let count = self.evaluate(&array.count());

                Builder::create_array_type()
                    .with_element(element)
                    .with_count(count)
                    .build()
            }

            Type::MapType(map) => {
                let key = self.evaluate_type(&map.key());
                let value = self.evaluate_type(&map.value());

                Builder::create_map_type()
                    .with_key(key)
                    .with_value(value)
                    .build()
            }

            Type::SliceType(slice) => {
                let element = self.evaluate_type(&slice.element());
                Builder::create_slice_type().with_element(element).build()
            }

            Type::FunctionType(function) => {
                let attributes = function.attributes().to_vec();

                let parameters = function
                    .parameters()
                    .iter()
                    .map(|p| {
                        let type_ = self.evaluate_type(&p.type_());
                        let default = p.default();
                        FunctionParameter::new(p.name(), type_, default)
                    })
                    .collect::<Vec<_>>();

                let return_type = self.evaluate_type(&function.return_type());

                Builder::create_function_type()
                    .add_attributes(attributes)
                    .add_parameters(parameters)
                    .with_return_type(return_type)
                    .build()
            }

            Type::ManagedRefType(reference) => {
                let is_mutable = reference.is_mutable();
                let target_type = self.evaluate_type(&reference.target());

                Builder::create_managed_type()
                    .with_mutability(is_mutable)
                    .with_target(target_type)
                    .build()
            }

            Type::UnmanagedRefType(reference) => {
                let is_mutable = reference.is_mutable();
                let target_type = self.evaluate_type(&reference.target());

                Builder::create_unmanaged_type()
                    .with_mutability(is_mutable)
                    .with_target(target_type)
                    .build()
            }

            Type::GenericType(generic) => {
                // TODO: Instantiate generic base with type arguments

                let arguments = generic
                    .arguments()
                    .iter()
                    .map(|(name, type_)| (*name, self.evaluate(type_)))
                    .collect::<Vec<_>>();

                let base = self.evaluate_type(&generic.base());

                Builder::create_generic_type()
                    .add_arguments(arguments)
                    .with_base(base)
                    .build()
            }

            Type::HasParenthesesType(inner) => self.evaluate_type(inner),
        };

        self.already_evaluated_types.insert(result.clone());

        result
    }
}
