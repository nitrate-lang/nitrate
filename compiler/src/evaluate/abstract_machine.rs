use std::ops::Deref;

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
        // self.provide_function("std::intrinsic::print", |m| {
        //     let string_to_print = m.tasks[m.current_task]
        //         .call_stack
        //         .last()
        //         .expect("No call frame found")
        //         .get("msg")
        //         .ok_or(FunctionError::MissingArgument)?
        //         .value()
        //         .ok_or(FunctionError::MissingArgument)?;

        //     match string_to_print.get(m.storage) {
        //         Expr::StringLit(string) => {
        //             print!("{}", string.get());
        //             Ok(Builder::new(m.storage).get_unit().into())
        //         }
        //         _ => Err(FunctionError::TypeError),
        //     }
        // });

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

    pub fn evaluate(&mut self, expression: Expr<'a>) -> Expr<'a> {
        match expression {
            Expr::Bool
            | Expr::UInt8
            | Expr::UInt16
            | Expr::UInt32
            | Expr::UInt64
            | Expr::UInt128
            | Expr::Int8
            | Expr::Int16
            | Expr::Int32
            | Expr::Int64
            | Expr::Int128
            | Expr::Float8
            | Expr::Float16
            | Expr::Float32
            | Expr::Float64
            | Expr::Float128
            | Expr::InferType
            | Expr::TypeName(_)
            | Expr::OpaqueType(_)
            | Expr::RefinementType(_)
            | Expr::TupleType(_)
            | Expr::ArrayType(_)
            | Expr::MapType(_)
            | Expr::SliceType(_)
            | Expr::FunctionType(_)
            | Expr::ManagedRefType(_)
            | Expr::UnmanagedRefType(_)
            | Expr::GenericType(_)
            | Expr::HasParenthesesType(_) => {
                let type_expression = expression.try_into().expect("Expected type expression");
                self.evaluate_type(type_expression).into()
            }

            Expr::Discard => {
                return Builder::new().get_discard();
            }
            Expr::HasParentheses(expr) => self.evaluate(expr.deref().clone()),

            Expr::BooleanLit(_)
            | Expr::IntegerLit(_)
            | Expr::FloatLit(_)
            | Expr::StringLit(_)
            | Expr::BStringLit(_) => expression,

            Expr::ListLit(list) => {
                let mut elements = list.elements().to_vec();

                for elem in &mut elements {
                    *elem = self.evaluate(elem.clone());
                }

                Builder::new().create_list().add_elements(elements).build()
            }

            Expr::ObjectLit(object) => {
                let mut fields = object.get().clone();

                for value in &mut fields.values_mut() {
                    *value = self.evaluate(value.clone());
                }

                Builder::new().create_object().add_fields(fields).build()
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
                self.evaluate(statement.get());
                Builder::new().get_unit().into()
            }

            Expr::Block(block) => {
                let mut result = None;
                for expr in block.elements().to_owned() {
                    result = Some(self.evaluate(expr));
                }

                result.unwrap_or_else(|| Builder::new().get_unit().into())
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

    pub fn evaluate_type(&mut self, type_expression: Type<'a>) -> Type<'a> {
        if self.already_evaluated_types.contains(&type_expression) {
            return type_expression;
        }

        let result = match type_expression {
            Type::Bool
            | Type::UInt8
            | Type::UInt16
            | Type::UInt32
            | Type::UInt64
            | Type::UInt128
            | Type::Int8
            | Type::Int16
            | Type::Int32
            | Type::Int64
            | Type::Int128
            | Type::Float8
            | Type::Float16
            | Type::Float32
            | Type::Float64
            | Type::Float128
            | Type::InferType
            | Type::TypeName(_)
            | Type::OpaqueType(_) => type_expression,

            Type::RefinementType(refinement_type) => {
                let (mut width, mut min, mut max, mut base_type) = (
                    refinement_type.width(),
                    refinement_type.min(),
                    refinement_type.max(),
                    refinement_type.base(),
                );

                width = width.map(|w| self.evaluate(w));
                min = min.map(|m| self.evaluate(m));
                max = max.map(|m| self.evaluate(m));
                base_type = self.evaluate_type(base_type);

                Builder::new()
                    .create_refinement_type()
                    .with_width(width)
                    .with_minimum(min)
                    .with_maximum(max)
                    .with_base(base_type)
                    .build()
            }

            Type::TupleType(tuple_type) => {
                let mut elements = tuple_type.elements().to_vec();
                for t in &mut elements {
                    *t = self.evaluate_type(t.clone());
                }

                Builder::new()
                    .create_tuple_type()
                    .add_elements(elements)
                    .build()
            }

            Type::ArrayType(array_type) => {
                let (mut element_type, mut count) = (array_type.element(), array_type.count());

                element_type = self.evaluate_type(element_type);
                count = self.evaluate(count);

                Builder::new()
                    .create_array_type()
                    .with_element(element_type)
                    .with_count(count)
                    .build()
            }

            Type::MapType(map_type) => {
                let (mut key_type, mut value_type) = (map_type.key(), map_type.value());

                key_type = self.evaluate_type(key_type);
                value_type = self.evaluate_type(value_type);

                Builder::new()
                    .create_map_type()
                    .with_key(key_type)
                    .with_value(value_type)
                    .build()
            }

            Type::SliceType(slice_type) => {
                let element_type = self.evaluate_type(slice_type.element());

                Builder::new()
                    .create_slice_type()
                    .with_element(element_type)
                    .build()
            }

            Type::FunctionType(function_type) => {
                let (mut attributes, mut parameters, mut return_type) = (
                    function_type.attributes().to_vec(),
                    function_type.parameters().to_vec(),
                    function_type.return_type(),
                );

                for attribute in &mut attributes {
                    *attribute = self.evaluate(attribute.to_owned());
                }

                for parameter in &mut parameters {
                    let type_ = self.evaluate_type(parameter.type_());
                    let default = parameter.default().map(|d| self.evaluate(d));
                    *parameter = FunctionParameter::new(parameter.name(), type_, default);
                }

                return_type = self.evaluate_type(return_type);

                Builder::new()
                    .create_function_type()
                    .add_attributes(attributes)
                    .add_parameters(parameters)
                    .with_return_type(return_type)
                    .build()
            }

            Type::ManagedRefType(ref_type) => {
                let is_mutable = ref_type.is_mutable();
                let target_type = self.evaluate_type(ref_type.target());

                Builder::new()
                    .create_managed_type()
                    .with_mutability(is_mutable)
                    .with_target(target_type)
                    .build()
            }

            Type::UnmanagedRefType(ref_type) => {
                let is_mutable = ref_type.is_mutable();
                let target_type = self.evaluate_type(ref_type.target());

                Builder::new()
                    .create_unmanaged_type()
                    .with_mutability(is_mutable)
                    .with_target(target_type)
                    .build()
            }

            Type::GenericType(generic_type) => {
                let (mut arguments, mut base) =
                    (generic_type.arguments().to_vec(), generic_type.base());

                for argument in &mut arguments {
                    *argument = (argument.0, self.evaluate(argument.1.to_owned()));
                }

                base = self.evaluate_type(base);

                Builder::new()
                    .create_generic_type()
                    .with_base(base)
                    .add_arguments(arguments)
                    .build()
            }

            Type::HasParenthesesType(inner_type) => {
                self.evaluate_type(inner_type.deref().to_owned())
            }
        };

        self.already_evaluated_types.insert(result.clone());

        result
    }
}
