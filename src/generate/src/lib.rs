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
}

struct Frame {
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

    pub(crate) fn register_function(&mut self, name: String) {
        self.known_functions.push(name);
    }

    pub(crate) fn register_struct(&mut self, name: String) {
        self.known_structs.push(name);
    }

    pub(crate) fn push_frame(&mut self) {
        self.frames.push(Frame { locals: Vec::new() });
    }

    pub(crate) fn pop_frame(&mut self) {
        self.frames.pop();
    }

    pub(crate) fn add_local(&mut self, name: String, ty: ast::Type) {
        if let Some(frame) = self.frames.last_mut() {
            frame.locals.push(Symbol {
                name,
                kind: SymbolKind::Local(ty),
            });
        }
    }

    pub(crate) fn has_any_local(&self) -> bool {
        self.frames.iter().any(|f| !f.locals.is_empty())
    }

    pub(crate) fn has_any_function(&self) -> bool {
        !self.known_functions.is_empty()
    }

    pub(crate) fn has_any_struct(&self) -> bool {
        !self.known_structs.is_empty()
    }

    fn next_u64(&mut self) -> u64 {
        self.rng = self.rng.wrapping_add(0x9e3779b97f4a7c15);
        let mut z = self.rng;
        z = (z ^ (z >> 30)).wrapping_mul(0xbf58476d1ce4e5b9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94d049bb133111eb);
        z ^ (z >> 31)
    }

    fn next_bool(&mut self) -> bool {
        (self.next_u64() & 1) != 0
    }

    fn gen_index(&mut self, max: usize) -> usize {
        if max == 0 {
            return 0;
        }
        (self.next_u64() as usize) % max
    }

    fn force_leaf(&self) -> bool {
        self.rvalue_depth >= MAX_RVALUE_DEPTH || self.budget == 0
    }

    /// Pre-seed the symbol table with a baseline set of functions and
    /// structs so that rvalue generation can reference valid names.
    fn seed_symbols(&mut self) {
        let builtin_names = ["add", "sub", "mul", "print", "len", "push", "pop", "map"];
        for name in builtin_names {
            self.register_function(name.to_string());
        }
        let struct_names = ["Vec", "Map", "Pair", "Data"];
        for name in struct_names {
            self.register_struct(name.to_string());
        }
    }

    /// Emit a minimal struct declaration for a seed name so that type paths
    /// referencing it are valid in the output.
    fn gen_seed_struct_decl(&mut self, name: String) -> ast::Item {
        ast::Item::Struct(ast::Struct {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            fields: vec![ast::StructField {
                span: SrcSpan::default(),
                visibility: None,
                attributes: None,
                name: "field_0".into(),
                ty: ast::Type::Int32(ast::Int32 {
                    span: SrcSpan::default(),
                }),
                default_value: None,
            }],
        })
    }

    pub fn gen_program(&mut self) -> String {
        // Pre-seed symbols so rvalue generation can produce valid paths.
        self.seed_symbols();

        let mut items = Vec::new();

        // Emit struct declarations for every registered (seeded + generated)
        // name so that type paths always reference declared types.  We copy
        // the list first because gen_item() may append to it.
        {
            let known = self.known_structs.clone();
            for name in known {
                items.push(self.gen_seed_struct_decl(name));
            }
        }

        let mut functions = 0;

        // Generate the required number of functions, plus random extra items.
        // The first function is always main with 0 arguments.
        while functions < self.config.function_count && self.budget > 0 {
            let argc = if functions == 0 { Some(0) } else { None };
            let item = self.gen_item(argc);
            if matches!(item, ast::Item::Function(_)) {
                functions += 1;
            }
            items.push(item);
        }

        // Generate additional random items to ensure all generators are exercised.
        let extra_items = self.gen_index(5);
        for _ in 0..extra_items {
            if self.budget > 0 {
                items.push(self.gen_item(None));
            }
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
