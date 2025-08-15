use crate::parsetree::{Builder, ExprKey, ExprRef, Storage, TypeKey, TypeRef, node::Variable};
use hashbrown::HashMap;

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

pub type Function<'a, 'storage> =
    fn(&mut AbstractMachine<'a, 'storage>) -> Result<ExprKey<'a>, FunctionError>;

pub struct AbstractMachine<'a, 'storage>
where
    'a: 'storage,
{
    storage: &'storage mut Storage<'a>,
    global_variables: HashMap<&'a str, Variable<'a>>,
    provided_functions: HashMap<&'a str, Function<'a, 'storage>>,
    tasks: Vec<Task<'a>>,
    current_task: usize,
}

impl<'a, 'storage> AbstractMachine<'a, 'storage> {
    pub fn new(s: &'storage mut Storage<'a>) -> Self {
        let mut abstract_machine = AbstractMachine {
            storage: s,
            global_variables: HashMap::new(),
            provided_functions: HashMap::new(),
            tasks: Vec::from([Task::new(0)]),
            current_task: 0,
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

            match string_to_print.get(m.storage) {
                ExprRef::StringLit(string) => {
                    print!("{}", string.get());
                    Ok(Builder::new(m.storage).get_unit().into())
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
        callback: Function<'a, 'storage>,
    ) -> Option<Function<'a, 'storage>> {
        self.provided_functions.insert(name, callback)
    }

    pub fn evaluate(&mut self, expression: ExprKey<'a>) -> ExprKey<'a> {
        match expression.get(self.storage) {
            ExprRef::Bool
            | ExprRef::UInt8
            | ExprRef::UInt16
            | ExprRef::UInt32
            | ExprRef::UInt64
            | ExprRef::UInt128
            | ExprRef::Int8
            | ExprRef::Int16
            | ExprRef::Int32
            | ExprRef::Int64
            | ExprRef::Int128
            | ExprRef::Float8
            | ExprRef::Float16
            | ExprRef::Float32
            | ExprRef::Float64
            | ExprRef::Float128
            | ExprRef::InferType
            | ExprRef::TypeName(_)
            | ExprRef::OpaqueType(_)
            | ExprRef::RefinementType(_)
            | ExprRef::TupleType(_)
            | ExprRef::ArrayType(_)
            | ExprRef::MapType(_)
            | ExprRef::SliceType(_)
            | ExprRef::FunctionType(_)
            | ExprRef::ManagedRefType(_)
            | ExprRef::UnmanagedRefType(_)
            | ExprRef::GenericType(_) => {
                let type_expression = expression.try_into().expect("Expected type expression");
                self.evaluate_type(type_expression).into()
            }

            ExprRef::Discard => {
                return Builder::new(self.storage).get_discard();
            }

            ExprRef::BooleanLit(_)
            | ExprRef::IntegerLit(_)
            | ExprRef::FloatLit(_)
            | ExprRef::StringLit(_)
            | ExprRef::BStringLit(_) => expression,

            ExprRef::ListLit(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::ObjectLit(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::UnaryExpr(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::BinExpr(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Statement(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Block(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Function(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Variable(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Identifier(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Scope(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::If(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::WhileLoop(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::DoWhileLoop(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Switch(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Break(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Continue(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Return(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::ForEach(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Await(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            ExprRef::Assert(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }
        }
    }

    pub fn evaluate_type(&mut self, type_expression: TypeKey<'a>) -> TypeKey<'a> {
        match type_expression.get(self.storage) {
            TypeRef::Bool
            | TypeRef::UInt8
            | TypeRef::UInt16
            | TypeRef::UInt32
            | TypeRef::UInt64
            | TypeRef::UInt128
            | TypeRef::Int8
            | TypeRef::Int16
            | TypeRef::Int32
            | TypeRef::Int64
            | TypeRef::Int128
            | TypeRef::Float8
            | TypeRef::Float16
            | TypeRef::Float32
            | TypeRef::Float64
            | TypeRef::Float128
            | TypeRef::InferType
            | TypeRef::TypeName(_)
            | TypeRef::OpaqueType(_) => type_expression,

            TypeRef::RefinementType(refinement_type) => {
                let (base_type, width, min, max) = (
                    refinement_type.base(),
                    refinement_type.width(),
                    refinement_type.min(),
                    refinement_type.max(),
                );

                let base_type = self.evaluate_type(base_type);
                let width = width.map(|w| self.evaluate(w));
                let min = min.map(|m| self.evaluate(m));
                let max = max.map(|m| self.evaluate(m));

                Builder::new(self.storage)
                    .create_refinement_type()
                    .with_base(base_type)
                    .with_width(width)
                    .with_minimum(min)
                    .with_maximum(max)
                    .build()
            }

            TypeRef::TupleType(tuple_type) => {
                let element_types = tuple_type
                    .elements()
                    .to_vec()
                    .iter_mut()
                    .map(|t| self.evaluate_type(*t))
                    .collect::<Vec<_>>();

                Builder::new(self.storage)
                    .create_tuple_type()
                    .add_elements(element_types)
                    .build()
            }

            TypeRef::ArrayType(array_type) => {
                let (element_type, count) = (array_type.element(), array_type.count());
                let element_type = self.evaluate_type(element_type);
                let count = self.evaluate(count);

                Builder::new(self.storage)
                    .create_array_type()
                    .with_element(element_type)
                    .with_count(count)
                    .build()
            }

            TypeRef::MapType(map_type) => {
                let (key_type, value_type) = (map_type.key(), map_type.value());
                let key_type = self.evaluate_type(key_type);
                let value_type = self.evaluate_type(value_type);

                Builder::new(self.storage)
                    .create_map_type()
                    .with_key(key_type)
                    .with_value(value_type)
                    .build()
            }

            TypeRef::SliceType(slice_type) => {
                let element_type = self.evaluate_type(slice_type.element());

                Builder::new(self.storage)
                    .create_slice_type()
                    .with_element(element_type)
                    .build()
            }

            TypeRef::FunctionType(_) => {
                // TODO: Evaluate variant
                unimplemented!()
            }

            TypeRef::ManagedRefType(ref_type) => {
                let is_mutable = ref_type.is_mutable();
                let target_type = self.evaluate_type(ref_type.target());

                Builder::new(self.storage)
                    .create_managed_type()
                    .with_mutability(is_mutable)
                    .with_target(target_type)
                    .build()
            }

            TypeRef::UnmanagedRefType(ref_type) => {
                let is_mutable = ref_type.is_mutable();
                let target_type = self.evaluate_type(ref_type.target());

                Builder::new(self.storage)
                    .create_unmanaged_type()
                    .with_mutability(is_mutable)
                    .with_target(target_type)
                    .build()
            }

            TypeRef::GenericType(generic_type) => {
                let (generic_base, mut generic_args) =
                    (generic_type.base(), generic_type.arguments().to_vec());

                let generic_args = generic_args
                    .iter_mut()
                    .map(|(name, arg)| (*name, self.evaluate(*arg)))
                    .collect::<Vec<_>>();

                let generic_base = self.evaluate_type(generic_base);

                Builder::new(self.storage)
                    .create_generic_type()
                    .with_base(generic_base)
                    .add_arguments(generic_args)
                    .build()
            }
        }
    }
}
