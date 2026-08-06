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
    /// Item-level recursion depth for module nesting.
    item_depth: u32,
    /// Type-generation recursion depth to prevent stack overflow in compound types.
    pub(crate) type_depth: u32,
    /// All globally-declared function names.
    known_functions: Vec<String>,
    /// All globally-declared struct names.
    known_structs: Vec<String>,
    /// Whether we are currently inside a loop (break/continue are valid).
    in_loop: bool,
    /// Unique name generation counter to avoid collisions.
    name_counter: u64,
}

/// Maximum recursion depth for rvalue generation before forcing leaf expressions.
const MAX_RVALUE_DEPTH: u32 = 6;
/// Maximum nesting depth for module-in-module generation.
pub(crate) const MAX_ITEM_DEPTH: u32 = 4;

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
        }
    }

    /// Spend one unit of budget. Returns true if a unit was successfully spent.
    pub(crate) fn spend_budget(&mut self) -> bool {
        if self.budget == 0 {
            return false;
        }
        self.budget = self.budget.saturating_sub(1);
        true
    }

    pub(crate) fn budget_left(&self) -> u32 {
        self.budget
    }

    pub(crate) fn register_function(&mut self, name: String) {
        // Deduplicate: don't register the same name twice.
        if !self.known_functions.contains(&name) {
            self.known_functions.push(name);
        }
    }

    pub(crate) fn register_struct(&mut self, name: String) {
        if !self.known_structs.contains(&name) {
            self.known_structs.push(name);
        }
    }

    pub(crate) fn push_frame(&mut self) {
        self.frames.push(Frame { locals: Vec::new() });
    }

    pub(crate) fn pop_frame(&mut self) {
        self.frames.pop();
    }

    pub(crate) fn add_local(&mut self, name: String, ty: ast::Type) {
        // Pre-compute dedup suffix before borrowing self.frames
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
        assert!(max > 0, "gen_index called with max=0");
        (self.next_u64() as usize) % max
    }

    fn force_leaf(&self) -> bool {
        self.rvalue_depth >= MAX_RVALUE_DEPTH || self.budget == 0
    }

    /// Emit a minimal struct declaration with four i32 fields for seed names
    /// so that type paths and struct init expressions referencing them are
    /// valid in the output (field count matches the 1+gen_index(4) range).
    fn gen_seed_struct_decl(&mut self, name: String) -> ast::Item {
        ast::Item::Struct(ast::Struct {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            fields: vec![
                ast::StructField {
                    span: SrcSpan::default(),
                    visibility: None,
                    attributes: None,
                    name: "field_0".into(),
                    ty: ast::Type::Int32(ast::Int32 {
                        span: SrcSpan::default(),
                    }),
                    default_value: None,
                },
                ast::StructField {
                    span: SrcSpan::default(),
                    visibility: None,
                    attributes: None,
                    name: "field_1".into(),
                    ty: ast::Type::Int32(ast::Int32 {
                        span: SrcSpan::default(),
                    }),
                    default_value: None,
                },
                ast::StructField {
                    span: SrcSpan::default(),
                    visibility: None,
                    attributes: None,
                    name: "field_2".into(),
                    ty: ast::Type::Int32(ast::Int32 {
                        span: SrcSpan::default(),
                    }),
                    default_value: None,
                },
                ast::StructField {
                    span: SrcSpan::default(),
                    visibility: None,
                    attributes: None,
                    name: "field_3".into(),
                    ty: ast::Type::Int32(ast::Int32 {
                        span: SrcSpan::default(),
                    }),
                    default_value: None,
                },
            ],
        })
    }

    /// Emit a minimal function stub for a seed function name so that
    /// function calls resolve to a declared function.
    fn gen_seed_function_stub(&mut self, name: String) -> ast::Item {
        ast::Item::Function(ast::Function {
            span: SrcSpan::default(),
            visibility: None,
            attributes: None,
            name: name.into(),
            generics: None,
            parameters: ast::FuncParams {
                span: SrcSpan::default(),
                params: Vec::new(),
                variadic: false,
            },
            return_type: Some(ast::Type::Int32(ast::Int32 {
                span: SrcSpan::default(),
            })),
            definition: Some(ast::Block {
                span: SrcSpan::default(),
                safety: None,
                elements: vec![ast::BlockItem::Expr(ast::Expr::Integer(Box::new(ast::IntegerLit {
                    span: SrcSpan::default(),
                    value: 0,
                    kind: nitrate_translation::token::IntegerKind::Dec,
                })))],
            }),
            abi: None,
        })
    }

    pub fn gen_program(&mut self) -> String {
        let mut items = Vec::new();
        let mut functions = 0;

        // Emit struct declarations for preset struct names so that type paths
        // always reference declared types.
        let struct_names = ["Vec", "Map", "Pair", "Data"];
        for name in struct_names {
            self.register_struct(name.to_string());
            items.push(self.gen_seed_struct_decl(name.to_string()));
        }

        // Emit stub functions for preset function names so that function calls
        // always reference declared functions.
        let builtin_names = ["add", "sub", "mul", "print", "len", "push", "pop", "map"];
        for name in builtin_names {
            self.register_function(name.to_string());
            items.push(self.gen_seed_function_stub(name.to_string()));
        }

        // Always emit a `main` function even if function_count is zero.
        // A program without an entry point is invalid.
        if self.config.function_count == 0 || self.budget_left() == 0 {
            items.push(self.gen_item_main());
            functions = 1;
        }

        // Generate the required number of user-defined functions.
        while functions < self.config.function_count && self.budget_left() > 0 {
            let item = if functions == 0 {
                self.gen_item_main()
            } else {
                self.gen_item(None)
            };
            if matches!(item, ast::Item::Function(_)) {
                functions += 1;
            }
            items.push(item);
        }

        // Generate additional random items to ensure all generators are exercised.
        let extra_items = 1 + self.gen_index(4);
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

/// Initialize splitmix64 state from a seed. The internal state for
/// splitmix64 is the raw seed value; the finalizer (xorshift-multiply)
/// is applied during each call to `next_u64` after advancing state with
/// the Weyl sequence constant.
fn splitmix64_init(seed: u64) -> u64 {
    seed
}
