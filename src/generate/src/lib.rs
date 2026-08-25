use nitrate_translation::parsetree::{PrettyPrint, PrintContext, SrcSpan, ast};
use std::format;

mod item;
mod rvalue;
mod ty;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct GenConfig {
    pub min_cyclomatic_complexity: u32,
    pub max_budget: u32,
    pub function_count: u32,
    pub seed: u64,
}

/// Generator state. RNG, budget, and recursion depth counters are kept here.
pub struct Gen {
    budget: u32,
    config: GenConfig,
    rng: u64,
    /// Recursion depth for value generation, prevents infinite recursion.
    rvalue_depth: u32,
    /// Item-level recursion depth for module nesting.
    item_depth: u32,
    /// Type-generation recursion depth to prevent stack overflow in compound types.
    pub(crate) type_depth: u32,
    /// Unique name generation counter to avoid collisions.
    name_counter: u64,
}

/// Maximum recursion depth for value generation before forcing leaf expressions.
pub(crate) const MAX_RVALUE_DEPTH: u32 = 5;
/// Maximum nesting depth for module-in-module generation.
pub(crate) const MAX_ITEM_DEPTH: u32 = 3;

impl Gen {
    pub fn new(config: GenConfig) -> Self {
        let seed = config.seed;
        Self {
            budget: config.max_budget,
            config,
            rng: splitmix64_init(seed),
            rvalue_depth: 0,
            item_depth: 0,
            type_depth: 0,
            name_counter: 0,
        }
    }

    /// Spend exact amount. Fails (returns false) if insufficient budget remains.
    pub(crate) fn spend_budget(&mut self, amount: u32) -> bool {
        if self.budget < amount {
            return false;
        }
        self.budget -= amount;
        true
    }

    pub(crate) fn budget_left(&self) -> u32 {
        self.budget
    }

    /// Force leaf generation when deeply nested or out of budget.
    pub(crate) fn force_leaf(&self) -> bool {
        self.rvalue_depth >= MAX_RVALUE_DEPTH || self.budget_left() < 2
    }

    pub(crate) fn inc_rvalue_depth(&mut self) {
        self.rvalue_depth += 1;
    }

    pub(crate) fn dec_rvalue_depth(&mut self) {
        self.rvalue_depth = self.rvalue_depth.saturating_sub(1);
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

    /// Weighted choice: returns one of the supplied `code` values,
    /// each associated with a positive weight.
    pub(crate) fn pick_weighted(&mut self, options: &[(u32, u8)]) -> u8 {
        assert!(!options.is_empty(), "pick_weighted called with no options");
        let total: u32 = options.iter().map(|(w, _)| w).sum();
        let mut roll = self.next_u64() as u32 % total;
        for (weight, code) in options {
            if roll < *weight {
                return *code;
            }
            roll -= *weight;
        }
        options[0].1
    }

    /// Produce a unique name with the given prefix.
    pub(crate) fn gen_unique_name(&mut self, prefix: &str) -> String {
        let counter = self.name_counter;
        self.name_counter = self.name_counter.wrapping_add(1);
        format!("{}_{}", prefix, counter)
    }

    pub fn gen_program(&mut self) -> String {
        let mut items = Vec::new();

        // Always emit a main function.
        items.push(self.gen_item_main());

        // Emit the requested number of additional functions, subject to budget.
        let mut extra_functions = self.config.function_count.saturating_sub(1);
        while extra_functions > 0 && self.budget_left() >= 10 {
            items.push(self.gen_item_function());
            extra_functions -= 1;
        }

        // Emit additional random items (structs, enums, consts, type aliases, modules).
        let extra_ceiling = 2 + self.gen_index(5);
        for _ in 0..extra_ceiling {
            if self.budget_left() < 3 {
                break;
            }
            items.push(self.gen_item());
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
