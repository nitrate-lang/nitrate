// use crate::parsetree::{
//     Builder, ExprKey, ExprRef, Storage, TypeKey, TypeRef,
//     node::{FunctionParameter, Variable},
// };
// use hashbrown::{HashMap, HashSet};

// #[derive(Debug, Default)]
// struct CallFrame<'a> {
//     function_locals: HashMap<&'a str, Variable<'a>>,
// }

// impl<'a> CallFrame<'a> {
//     fn get(&self, name: &str) -> Option<&Variable<'a>> {
//         self.function_locals.get(name)
//     }

//     fn set(&mut self, name: &'a str, variable: Variable<'a>) {
//         self.function_locals.insert(name, variable);
//     }
// }

// #[derive(Debug)]
// struct Task<'a> {
//     call_stack: Vec<CallFrame<'a>>,
//     task_locals: HashMap<&'a str, Variable<'a>>,
//     task_id: u64,
// }

// impl<'a> Task<'a> {
//     fn new(task_id: u64) -> Self {
//         let call_stack = Vec::from([CallFrame::default()]);
//         Task {
//             call_stack,
//             task_locals: HashMap::new(),
//             task_id,
//         }
//     }
// }

// pub enum FunctionError {
//     MissingArgument,
//     TypeError,
// }

// pub type Function<'a, 'storage> =
//     fn(&mut AbstractMachine<'a, 'storage>) -> Result<ExprKey<'a>, FunctionError>;

// pub struct AbstractMachine<'a, 'storage>
// where
//     'a: 'storage,
// {
//     global_variables: HashMap<&'a str, Variable<'a>>,
//     provided_functions: HashMap<&'a str, Function<'a, 'storage>>,
//     tasks: Vec<Task<'a>>,
//     current_task: usize,

//     already_evaluated_types: HashSet<TypeKey<'a>>,
// }

// impl<'a, 'storage> AbstractMachine<'a, 'storage> {
//     pub fn new() -> Self {
//         let mut abstract_machine = AbstractMachine {
//             global_variables: HashMap::new(),
//             provided_functions: HashMap::new(),
//             tasks: Vec::from([Task::new(0)]),
//             current_task: 0,
//             already_evaluated_types: HashSet::new(),
//         };

//         abstract_machine.setup_builtins();
//         abstract_machine.setup_runtime();
//         abstract_machine
//     }

//     fn setup_builtins(&mut self) {
//         // self.provide_function("std::intrinsic::print", |m| {
//         //     let string_to_print = m.tasks[m.current_task]
//         //         .call_stack
//         //         .last()
//         //         .expect("No call frame found")
//         //         .get("msg")
//         //         .ok_or(FunctionError::MissingArgument)?
//         //         .value()
//         //         .ok_or(FunctionError::MissingArgument)?;

//         //     match string_to_print.get(m.storage) {
//         //         ExprRef::StringLit(string) => {
//         //             print!("{}", string.get());
//         //             Ok(Builder::new(m.storage).get_unit().into())
//         //         }
//         //         _ => Err(FunctionError::TypeError),
//         //     }
//         // });

//         // TODO: Implement other intrinsic functions
//     }

//     fn setup_runtime(&mut self) {
//         // TODO: Implement runtime setup logic
//         // Create some global variables, garbage collector?, etc.
//     }

//     pub fn provide_function(
//         &mut self,
//         name: &'a str,
//         callback: Function<'a, 'storage>,
//     ) -> Option<Function<'a, 'storage>> {
//         self.provided_functions.insert(name, callback)
//     }

//     pub fn evaluate(&mut self, storage: &mut Storage<'a>, expression: ExprKey<'a>) -> ExprKey<'a> {
//         let s = storage;

//         match expression.get() {
//             ExprRef::Bool
//             | ExprRef::UInt8
//             | ExprRef::UInt16
//             | ExprRef::UInt32
//             | ExprRef::UInt64
//             | ExprRef::UInt128
//             | ExprRef::Int8
//             | ExprRef::Int16
//             | ExprRef::Int32
//             | ExprRef::Int64
//             | ExprRef::Int128
//             | ExprRef::Float8
//             | ExprRef::Float16
//             | ExprRef::Float32
//             | ExprRef::Float64
//             | ExprRef::Float128
//             | ExprRef::InferType
//             | ExprRef::TypeName(_)
//             | ExprRef::OpaqueType(_)
//             | ExprRef::RefinementType(_)
//             | ExprRef::TupleType(_)
//             | ExprRef::ArrayType(_)
//             | ExprRef::MapType(_)
//             | ExprRef::SliceType(_)
//             | ExprRef::FunctionType(_)
//             | ExprRef::ManagedRefType(_)
//             | ExprRef::UnmanagedRefType(_)
//             | ExprRef::GenericType(_) => {
//                 let type_expression = expression.try_into().expect("Expected type expression");
//                 self.evaluate_type(s, type_expression).into()
//             }

//             ExprRef::Discard => {
//                 return Builder::new().get_discard();
//             }

//             ExprRef::BooleanLit(_)
//             | ExprRef::IntegerLit(_)
//             | ExprRef::FloatLit(_)
//             | ExprRef::StringLit(_)
//             | ExprRef::BStringLit(_) => expression,

//             ExprRef::ListLit(list) => {
//                 let mut elements = list.elements().to_vec();

//                 for elem in &mut elements {
//                     *elem = self.evaluate(s, *elem);
//                 }

//                 Builder::new().create_list().add_elements(elements).build()
//             }

//             ExprRef::ObjectLit(object) => {
//                 let mut fields = object.get().clone();

//                 for value in &mut fields.values_mut() {
//                     *value = self.evaluate(s, *value);
//                 }

//                 Builder::new().create_object().add_fields(fields).build()
//             }

//             ExprRef::UnaryExpr(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::BinExpr(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Statement(statement) => {
//                 self.evaluate(s, statement.get());
//                 Builder::new().get_unit().into()
//             }

//             ExprRef::Block(block) => {
//                 let mut result = None;
//                 for expr in block.elements().to_owned() {
//                     result = Some(self.evaluate(s, expr));
//                 }

//                 result.unwrap_or_else(|| Builder::new().get_unit().into())
//             }

//             ExprRef::Function(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Variable(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Identifier(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Scope(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::If(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::WhileLoop(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::DoWhileLoop(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Switch(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Break(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Continue(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Return(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::ForEach(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Await(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }

//             ExprRef::Assert(_) => {
//                 // TODO: Evaluate variant
//                 unimplemented!()
//             }
//         }
//     }

//     pub fn evaluate_type(
//         &mut self,
//         storage: &mut Storage<'a>,
//         type_expression: TypeKey<'a>,
//     ) -> TypeKey<'a> {
//         let s = storage;

//         if self.already_evaluated_types.contains(&type_expression) {
//             return type_expression;
//         }

//         let result = match type_expression.get() {
//             TypeRef::Bool
//             | TypeRef::UInt8
//             | TypeRef::UInt16
//             | TypeRef::UInt32
//             | TypeRef::UInt64
//             | TypeRef::UInt128
//             | TypeRef::Int8
//             | TypeRef::Int16
//             | TypeRef::Int32
//             | TypeRef::Int64
//             | TypeRef::Int128
//             | TypeRef::Float8
//             | TypeRef::Float16
//             | TypeRef::Float32
//             | TypeRef::Float64
//             | TypeRef::Float128
//             | TypeRef::InferType
//             | TypeRef::TypeName(_)
//             | TypeRef::OpaqueType(_) => type_expression,

//             TypeRef::RefinementType(refinement_type) => {
//                 let (mut width, mut min, mut max, mut base_type) = (
//                     refinement_type.width(),
//                     refinement_type.min(),
//                     refinement_type.max(),
//                     refinement_type.base(),
//                 );

//                 width = width.map(|w| self.evaluate(s, w));
//                 min = min.map(|m| self.evaluate(s, m));
//                 max = max.map(|m| self.evaluate(s, m));
//                 base_type = self.evaluate_type(s, base_type);

//                 Builder::new()
//                     .create_refinement_type()
//                     .with_width(width)
//                     .with_minimum(min)
//                     .with_maximum(max)
//                     .with_base(base_type)
//                     .build()
//             }

//             TypeRef::TupleType(tuple_type) => {
//                 let mut elements = tuple_type.elements().to_vec();
//                 for t in &mut elements {
//                     *t = self.evaluate_type(s, *t);
//                 }

//                 Builder::new()
//                     .create_tuple_type()
//                     .add_elements(elements)
//                     .build()
//             }

//             TypeRef::ArrayType(array_type) => {
//                 let (mut element_type, mut count) = (array_type.element(), array_type.count());

//                 element_type = self.evaluate_type(s, element_type);
//                 count = self.evaluate(s, count);

//                 Builder::new()
//                     .create_array_type()
//                     .with_element(element_type)
//                     .with_count(count)
//                     .build()
//             }

//             TypeRef::MapType(map_type) => {
//                 let (mut key_type, mut value_type) = (map_type.key(), map_type.value());

//                 key_type = self.evaluate_type(s, key_type);
//                 value_type = self.evaluate_type(s, value_type);

//                 Builder::new()
//                     .create_map_type()
//                     .with_key(key_type)
//                     .with_value(value_type)
//                     .build()
//             }

//             TypeRef::SliceType(slice_type) => {
//                 let element_type = self.evaluate_type(s, slice_type.element());

//                 Builder::new()
//                     .create_slice_type()
//                     .with_element(element_type)
//                     .build()
//             }

//             TypeRef::FunctionType(function_type) => {
//                 let (mut attributes, mut parameters, mut return_type) = (
//                     function_type.attributes().to_vec(),
//                     function_type.parameters().to_vec(),
//                     function_type.return_type(),
//                 );

//                 for attribute in &mut attributes {
//                     *attribute = self.evaluate(s, *attribute);
//                 }

//                 for parameter in &mut parameters {
//                     let type_ = self.evaluate_type(s, parameter.type_());
//                     let default = parameter.default().map(|d| self.evaluate(s, d));
//                     *parameter = FunctionParameter::new(parameter.name(), type_, default);
//                 }

//                 return_type = self.evaluate_type(s, return_type);

//                 Builder::new()
//                     .create_function_type()
//                     .add_attributes(attributes)
//                     .add_parameters(parameters)
//                     .with_return_type(return_type)
//                     .build()
//             }

//             TypeRef::ManagedRefType(ref_type) => {
//                 let is_mutable = ref_type.is_mutable();
//                 let target_type = self.evaluate_type(s, ref_type.target());

//                 Builder::new()
//                     .create_managed_type()
//                     .with_mutability(is_mutable)
//                     .with_target(target_type)
//                     .build()
//             }

//             TypeRef::UnmanagedRefType(ref_type) => {
//                 let is_mutable = ref_type.is_mutable();
//                 let target_type = self.evaluate_type(s, ref_type.target());

//                 Builder::new()
//                     .create_unmanaged_type()
//                     .with_mutability(is_mutable)
//                     .with_target(target_type)
//                     .build()
//             }

//             TypeRef::GenericType(generic_type) => {
//                 let (mut arguments, mut base) =
//                     (generic_type.arguments().to_vec(), generic_type.base());

//                 for argument in &mut arguments {
//                     *argument = (argument.0, self.evaluate(s, argument.1));
//                 }

//                 base = self.evaluate_type(s, base);

//                 Builder::new()
//                     .create_generic_type()
//                     .with_base(base)
//                     .add_arguments(arguments)
//                     .build()
//             }
//         };

//         self.already_evaluated_types.insert(result);

//         result
//     }
// }
