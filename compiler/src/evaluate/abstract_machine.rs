use crate::parsetree::{Builder, ExprKey, Storage, node::Variable};
use hashbrown::HashMap;

#[derive(Debug, Default)]
struct CallFrame<'a> {
    variables_and_parameters: HashMap<&'a str, Variable<'a>>,
}

impl<'a> CallFrame<'a> {
    fn get(&self, name: &str) -> Option<&Variable<'a>> {
        self.variables_and_parameters.get(name)
    }

    fn set(&mut self, name: &'a str, variable: Variable<'a>) {
        self.variables_and_parameters.insert(name, variable);
    }
}

#[derive(Debug)]
struct Task<'a> {
    call_stack: Vec<CallFrame<'a>>,
    task_locals: HashMap<&'a str, Variable<'a>>,
    task_id: u64,
}

impl<'a, 'storage> Task<'a> {
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

#[derive(Debug)]
pub struct AbstractMachine<'a, 'storage> {
    storage: &'storage mut Storage<'a>,
    program: ExprKey<'a>,

    global_variables: HashMap<&'a str, Variable<'a>>,
    provided_functions: HashMap<&'a str, Function<'a, 'storage>>,
    tasks: Vec<Task<'a>>,
    current_task: usize,
}

impl<'a, 'storage> AbstractMachine<'a, 'storage> {
    pub fn new(program: ExprKey<'a>, storage: &'storage mut Storage<'a>) -> Self {
        let mut abstract_machine = AbstractMachine {
            storage,
            program,
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
        self.provide_function("print", |m| {
            let value_to_print = m.tasks[m.current_task]
                .call_stack
                .last()
                .expect("No call frame found")
                .get("value")
                .ok_or(FunctionError::MissingArgument)?
                .value()
                .ok_or(FunctionError::MissingArgument)?;

            // FIXME: Lookup the ToString trait implementation for the type of value_to_print
            println!("{:?}", value_to_print.as_printable(m.storage));

            Ok(Builder::new(m.storage).get_unit().into())
        });

        // TODO: Implement default intrinsic functions
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

    pub fn evaluate(&mut self) -> Option<ExprKey<'a>> {
        // TODO: Interpret the program starting from self.program
        None
    }
}
