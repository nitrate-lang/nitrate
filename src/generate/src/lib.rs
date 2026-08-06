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

/// Metadata about a function known to the generator, so that call-sites
/// can emit type-compatible arguments.
#[derive(Debug, Clone)]
pub(crate) struct FuncInfo {
    pub name: String,
    pub params: Vec<ast::Type>,
    pub return_type: Option<ast::Type>,
}

/// Metadata about a struct known to the generator, so that struct-inits
/// and field-accesses can produce compatible value expressions.
#[derive(Debug, Clone)]
pub(crate) struct StructInfo {
    pub name: String,
    pub fields: Vec<(String, ast::Type)>,
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
    /// Item-level recursion depth for module nesting.
    item_depth: u32,
    /// Type-generation recursion depth to prevent stack overflow in compound types.
    pub(crate) type_depth: u32,
    /// All globally-declared function signatures.
    known_functions: Vec<FuncInfo>,
    /// All globally-declared struct definitions.
    known_structs: Vec<StructInfo>,
    /// Whether we are currently inside a loop (break/continue are valid).
    in_loop: bool,
    /// Unique name generation counter to avoid collisions.
    name_counter: u64,
    /// Whether a `main` function has been emitted.
    has_main: bool,
}

/// Maximum recursion depth for rvalue generation before forcing leaf expressions.
const MAX_RVALUE_DEPTH: u32 = 5;
/// Maximum nesting depth for module-in-module generation.
pub(crate) const MAX_ITEM_DEPTH: u32 = 3;

impl Gen {
    pub fn new(config: GenConfig) -> Self {
        let seed = config.seed;
        Self {
            frames: Vec::new(),
            budget: config.max_budget,
            config,
            rng: splitmix64_init(seed),
            rvalue_depth: 0,
            item_depth: 0,
            type_depth: 0,
            known_functions: Vec::new(),
            known_structs: Vec::new(),
            in_loop: false,
            name_counter: 0,
            has_main: false,
        }
    }

    /// Spend budget. Amount scales by the weight of what we're generating.
    pub(crate) fn spend_budget(&mut self, amount: u32) -> bool {
        let amt = amount.max(1);
        if self.budget == 0 {
            return false;
        }
        self.budget = self.budget.saturating_sub(amt);
        true
    }

    pub(crate) fn budget_left(&self) -> u32 {
        self.budget
    }

    pub(crate) fn register_function(&mut self, info: FuncInfo) {
        if !self.known_functions.iter().any(|f| f.name == info.name) {
            self.known_functions.push(info);
        }
    }

    pub(crate) fn register_struct(&mut self, info: StructInfo) {
        if !self.known_structs.iter().any(|s| s.name == info.name) {
            self.known_structs.push(info);
        }
    }

    pub(crate) fn push_frame(&mut self) {
        self.frames.push(Frame { locals: Vec::new() });
    }

    pub(crate) fn pop_frame(&mut self) {
        self.frames.pop();
    }

    pub(crate) fn add_local(&mut self, name: String, ty: ast::Type) {
        let suffix: Option<u64> = if self
            .frames
            .last()
            .map_or(false, |f| f.locals.iter().any(|s| s.name == name))
        {
            Some(self.next_u64() & 0xFFF)
        } else {
            None
        };
        if let Some(frame) = self.frames.last_mut() {
            let unique_name = match suffix {
                Some(s) => format!("{}_{}", name, s),
                None => name,
            };
            frame.locals.push(Symbol {
                name: unique_name,
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

    /// Pick a random known function info.
    pub(crate) fn pick_function(&mut self) -> FuncInfo {
        debug_assert!(self.has_any_function());
        let idx = self.gen_index(self.known_functions.len());
        self.known_functions[idx].clone()
    }

    /// Pick a random known struct info.
    pub(crate) fn pick_struct(&mut self) -> StructInfo {
        debug_assert!(self.has_any_struct());
        let idx = self.gen_index(self.known_structs.len());
        self.known_structs[idx].clone()
    }

    pub(crate) fn find_struct_by_name(&self, name: &str) -> Option<&StructInfo> {
        self.known_structs.iter().find(|s| s.name == name)
    }

    pub(crate) fn next_u64(&mut self) -> u64 {
        self.rng = self.rng.wrapping_add(0x9e3779b97f4a7c15);
        let mut z = self.rng;
        z = (z ^ (z >> 30)).wrapping_mul(0xbf58476d1ce4e5b9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94d049bb133111eb);
        z ^ (z >> 31)
    }

    pub(crate) fn next_bool(&mut self) -> bool {
        (self.next_u64() & 1) != 0
    }

    pub(crate) fn gen_index(&mut self, max: usize) -> usize {
        assert!(max > 0, "gen_index called with max=0");
        (self.next_u64() as usize) % max
    }

    pub(crate) fn force_leaf(&self) -> bool {
        self.rvalue_depth >= MAX_RVALUE_DEPTH || self.budget == 0
    }

    pub(crate) fn set_in_loop(&mut self, val: bool) {
        self.in_loop = val;
    }

    pub(crate) fn in_loop(&self) -> bool {
        self.in_loop
    }

    pub(crate) fn inc_rvalue_depth(&mut self) {
        self.rvalue_depth += 1;
    }

    pub(crate) fn dec_rvalue_depth(&mut self) {
        self.rvalue_depth = self.rvalue_depth.saturating_sub(1);
    }

    pub fn gen_program(&mut self) -> String {
        let mut items = Vec::new();
        let mut functions_generated = 0u32;

        // Generate the required number of user-defined functions.
        // First function is always `main` if function_count > 0.
        while functions_generated < self.config.function_count && self.budget_left() > 0 {
            let item = if functions_generated == 0 {
                self.gen_item_main()
            } else {
                self.gen_item(None)
            };
            if matches!(item, ast::Item::Function(_)) {
                functions_generated += 1;
            }
            items.push(item);
        }

        // If no main was generated (e.g. first item wasn't a fn), add one.
        if !self.has_main && self.budget_left() > 0 {
            items.insert(0, self.gen_item_main());
        }

        // Generate additional random items to ensure all generators are exercised.
        let extra_items = 1 + self.gen_index(3);
        for _ in 0..extra_items {
            if self.budget_left() > 0 {
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
    seed
}
