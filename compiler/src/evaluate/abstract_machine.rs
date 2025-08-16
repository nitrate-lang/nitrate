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

pub enum EvalError {
    TypeError,
}

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
            Expr::InferType => Ok(Expr::InferType),
            Expr::TypeName(str) => Ok(Expr::TypeName(str)),
            Expr::OpaqueType(identity) => Ok(Expr::OpaqueType(identity.clone())),

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
                self.evaluate_type(&type_expression).map(|t| t.into())
            }

            Expr::Discard => Ok(Expr::Discard),
            Expr::HasParentheses(expr) => self.evaluate(expr.deref()),

            Expr::BooleanLit(bool) => Ok(Expr::BooleanLit(bool.to_owned())),
            Expr::IntegerLit(int) => Ok(Expr::IntegerLit(int.clone())),
            Expr::FloatLit(float) => Ok(Expr::FloatLit(float.to_owned())),
            Expr::StringLit(string) => Ok(Expr::StringLit(string.clone())),
            Expr::BStringLit(bstring) => Ok(Expr::BStringLit(bstring.clone())),

            Expr::ListLit(list) => {
                let mut elements = Vec::new();
                elements.reserve(list.elements().len());

                for element in list.elements() {
                    elements.push(self.evaluate(element)?);
                }

                Ok(Builder::create_list().add_elements(elements).build())
            }

            Expr::ObjectLit(object) => {
                let mut fields = BTreeMap::new();
                for (key, value) in object.get() {
                    fields.insert(*key, self.evaluate(value)?);
                }

                Ok(Builder::create_object().add_fields(fields).build())
            }

            Expr::UnaryExpr(_) => {
                // TODO: Evaluate unary expression
                unimplemented!()
            }

            Expr::BinExpr(_) => {
                // TODO: Evaluate binary expression
                unimplemented!()
            }

            Expr::Statement(statement) => {
                self.evaluate(&statement.get())?;
                Ok(Builder::get_unit().into())
            }

            Expr::Block(block) => {
                let mut result = None;
                for element in block.elements() {
                    result = Some(self.evaluate(element)?);
                }

                Ok(result.unwrap_or_else(|| Builder::get_unit().into()))
            }

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

            Expr::If(if_expr) => match self.evaluate(&if_expr.condition())? {
                Expr::BooleanLit(b) => {
                    if b {
                        self.evaluate(&if_expr.then_branch())
                    } else if let Some(else_branch) = if_expr.else_branch() {
                        self.evaluate(else_branch)
                    } else {
                        Ok(Builder::get_unit().into())
                    }
                }

                _ => panic!("Condition did not evaluate to a boolean"),
            },

            Expr::WhileLoop(_) => {
                // TODO: Evaluate while loop
                unimplemented!()
            }

            Expr::DoWhileLoop(_) => {
                // TODO: Evaluate do-while loop
                unimplemented!()
            }

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

    pub fn evaluate_type(&mut self, type_expression: &Type<'a>) -> Result<Type<'a>, EvalError> {
        if self.already_evaluated_types.contains(type_expression) {
            return Ok(type_expression.to_owned());
        }

        let result = match type_expression {
            Type::Bool => Ok(Type::Bool),
            Type::UInt8 => Ok(Type::UInt8),
            Type::UInt16 => Ok(Type::UInt16),
            Type::UInt32 => Ok(Type::UInt32),
            Type::UInt64 => Ok(Type::UInt64),
            Type::UInt128 => Ok(Type::UInt128),
            Type::Int8 => Ok(Type::Int8),
            Type::Int16 => Ok(Type::Int16),
            Type::Int32 => Ok(Type::Int32),
            Type::Int64 => Ok(Type::Int64),
            Type::Int128 => Ok(Type::Int128),
            Type::Float8 => Ok(Type::Float8),
            Type::Float16 => Ok(Type::Float16),
            Type::Float32 => Ok(Type::Float32),
            Type::Float64 => Ok(Type::Float64),
            Type::Float128 => Ok(Type::Float128),
            Type::InferType => Ok(Type::InferType),
            Type::TypeName(name) => Ok(Type::TypeName(*name)),
            Type::OpaqueType(identity) => Ok(Type::OpaqueType(identity.clone())),

            Type::RefinementType(refinement) => {
                let width = match refinement.width() {
                    Some(w) => Some(self.evaluate(&w)?),
                    None => None,
                };

                let min = match refinement.min() {
                    Some(m) => Some(self.evaluate(&m)?),
                    None => None,
                };

                let max = match refinement.max() {
                    Some(m) => Some(self.evaluate(&m)?),
                    None => None,
                };

                let base = self.evaluate_type(&refinement.base())?;

                Ok(Builder::create_refinement_type()
                    .with_width(width)
                    .with_minimum(min)
                    .with_maximum(max)
                    .with_base(base)
                    .build())
            }

            Type::TupleType(tuple) => {
                let mut elements = Vec::new();
                elements.reserve(tuple.elements().len());

                for element in tuple.elements() {
                    elements.push(self.evaluate_type(element)?);
                }

                Ok(Builder::create_tuple_type().add_elements(elements).build())
            }

            Type::ArrayType(array) => {
                let element = self.evaluate_type(&array.element())?;
                let count = self.evaluate(&array.count())?;

                Ok(Builder::create_array_type()
                    .with_element(element)
                    .with_count(count)
                    .build())
            }

            Type::MapType(map) => {
                let key = self.evaluate_type(&map.key())?;
                let value = self.evaluate_type(&map.value())?;

                Ok(Builder::create_map_type()
                    .with_key(key)
                    .with_value(value)
                    .build())
            }

            Type::SliceType(slice) => {
                let element = self.evaluate_type(&slice.element())?;
                Ok(Builder::create_slice_type().with_element(element).build())
            }

            Type::FunctionType(function) => {
                let attributes = function.attributes().to_vec();

                let mut parameters = Vec::new();
                parameters.reserve(function.parameters().len());

                for parameter in function.parameters() {
                    let type_ = self.evaluate_type(&parameter.type_())?;
                    let default = parameter.default();
                    parameters.push(FunctionParameter::new(parameter.name(), type_, default));
                }

                let return_type = self.evaluate_type(&function.return_type())?;

                Ok(Builder::create_function_type()
                    .add_attributes(attributes)
                    .add_parameters(parameters)
                    .with_return_type(return_type)
                    .build())
            }

            Type::ManagedRefType(reference) => {
                let is_mutable = reference.is_mutable();
                let target_type = self.evaluate_type(&reference.target())?;

                Ok(Builder::create_managed_type()
                    .with_mutability(is_mutable)
                    .with_target(target_type)
                    .build())
            }

            Type::UnmanagedRefType(reference) => {
                let is_mutable = reference.is_mutable();
                let target_type = self.evaluate_type(&reference.target())?;

                Ok(Builder::create_unmanaged_type()
                    .with_mutability(is_mutable)
                    .with_target(target_type)
                    .build())
            }

            Type::GenericType(generic) => {
                // TODO: Instantiate generic base with type arguments

                let mut arguments = Vec::new();
                arguments.reserve(generic.arguments().len());

                for (name, value) in generic.arguments() {
                    let evaluated_type = self.evaluate(value)?;
                    arguments.push((*name, evaluated_type));
                }

                let base = self.evaluate_type(&generic.base())?;

                Ok(Builder::create_generic_type()
                    .add_arguments(arguments)
                    .with_base(base)
                    .build())
            }

            Type::HasParenthesesType(inner) => self.evaluate_type(inner),
        };

        result.inspect(|t| {
            self.already_evaluated_types.insert(t.clone());
        })
    }
}
