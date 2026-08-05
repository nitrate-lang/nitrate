use nitrate_translation::parsetree::{PrettyPrint, PrintContext, SrcSpan, ast};
use std::matches;

mod item;
mod rvalue;
mod ty;

/// A named symbol tracked in the generation scope, with its type.
#[derive(Debug, Clone)]
pub(crate) struct Symbol {
    pub name: String,
    pub kind: SymbolKind,
}

#[derive(Debug, Clone)]
pub(crate) enum SymbolKind {
    /// A local variable or function parameter.
    Local(ast::Type),
    /// A globally-declared function name.
    Function,
    /// A globally-declared struct name.
    Struct,
}

struct Frame {
    /// Local variable names in this scope.
    pub(crate) locals: Vec<Symbol>,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct GenConfig {
    pub min_cyclomatic_complexity: u32,
    pub max_budget: u32,
    pub function_count: u32,
    pub seed: u64,
}

pub struct Gen {
    pub(crate) frames: Vec<Frame>,
    budget: u32,
    config: GenConfig,
    rng: u64,
    /// Recursion depth for rvalue generation, prevents infinite recursion.
    rvalue_depth: u32,
    /// All globally-declared function names.
    known_functions: Vec<String>,
    /// All globally-declared struct names.
    known_structs: Vec<String>,
}

/// Maximum recursion depth for rvalue generation before forcing leaf expressions.
const MAX_RVALUE_DEPTH: u32 = 6;

impl Gen {
    pub fn new(config: GenConfig) -> Self {
        let seed = config.seed;
        Self {
            frames: Vec::new(),
            budget: config.max_budget,
            config,
            rng: splitmix64_init(seed),
            rvalue_depth: 0,
            known_functions: Vec::new(),
            known_structs: Vec::new(),
        }
    }

    /// Register a globally-declared function name.
    pub(crate) fn register_function(&mut self, name: String) {
        self.known_functions.push(name);
    }

    /// Register a globally-declared struct name.
    pub(crate) fn register_struct(&mut self, name: String) {
        self.known_structs.push(name);
    }

    /// Push a new scope frame (e.g., for a block or function body).
    pub(crate) fn push_frame(&mut self) {
        self.frames.push(Frame { locals: Vec::new() });
    }

    /// Pop the current scope frame.
    pub(crate) fn pop_frame(&mut self) {
        self.frames.pop();
    }

    /// Add a local variable to the current frame.
    pub(crate) fn add_local(&mut self, name: String, ty: ast::Type) {
        if let Some(frame) = self.frames.last_mut() {
            frame.locals.push(Symbol {
                name,
                kind: SymbolKind::Local(ty),
            });
        }
    }

    /// Check if any local variables are in scope.
    pub(crate) fn has_any_local(&self) -> bool {
        self.frames.iter().any(|f| !f.locals.is_empty())
    }

    /// Check if any functions are known.
    pub(crate) fn has_any_function(&self) -> bool {
        !self.known_functions.is_empty()
    }

    /// Check if any structs are known.
    pub(crate) fn has_any_struct(&self) -> bool {
        !self.known_structs.is_empty()
    }

    /// Splitmix64: advance the state and return a random u64.
    fn next_u64(&mut self) -> u64 {
        self.rng = self.rng.wrapping_add(0x9e3779b97f4a7c15);
        let mut z = self.rng;
        z = (z ^ (z >> 30)).wrapping_mul(0xbf58476d1ce4e5b9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94d049bb133111eb);
        z ^ (z >> 31)
    }

    /// Return a random u32.
    fn next_u32(&mut self) -> u32 {
        (self.next_u64() & 0xFFFFFFFF) as u32
    }

    /// Return a random boolean.
    fn next_bool(&mut self) -> bool {
        (self.next_u64() & 1) != 0
    }

    /// Return a random usize in `[0, max)`.
    fn gen_index(&mut self, max: usize) -> usize {
        if max == 0 {
            return 0;
        }
        (self.next_u64() as usize) % max
    }

    /// Return true if rvalue generation should force a leaf expression
    /// (due to recursion depth or budget exhaustion).
    fn force_leaf(&self) -> bool {
        self.rvalue_depth >= MAX_RVALUE_DEPTH || self.budget == 0
    }

    pub fn gen_program(&mut self) -> String {
        // TODO: Generate a main function that takes no arguments.
        // TODO: Generate other functions taking various arguments
        // TODO: Determine all types used in the program and generate debug printing helpers for them. Prepend helpers to the program.
        // TODO: Prepend the printf extern "C" declaration to the program.

        let mut items = Vec::new();
        let mut functions = 0;

        while functions < self.config.function_count && self.budget > 0 {
            let argc = if functions == 0 { Some(0) } else { None };
            let item = self.gen_item(argc);
            if matches!(item, ast::Item::Function(_)) {
                functions += 1;
            }
            items.push(item);
        }

        let module = ast::Module {
            span: SrcSpan::default(),
            items,
            name: "testcase".into(),
            visibility: Some(ast::Visibility::Public),
            attributes: None,
        };

        let mut printer = PrintContext::default();
        module.pretty_print(&mut printer).expect("failed to print AST")
    }
}

/// Initialize splitmix64 state from a seed.
fn splitmix64_init(seed: u64) -> u64 {
    let mut z = seed;
    z = (z ^ (z >> 30)).wrapping_mul(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94d049bb133111eb);
    z ^ (z >> 31)
}
