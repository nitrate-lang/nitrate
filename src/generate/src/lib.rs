use nitrate_translation::parsetree::{PrettyPrint, PrintContext, SrcSpan, ast};
use std::{collections::HashSet, matches};

mod item;
mod rvalue;

struct Frame {
    pub(crate) locals: HashSet<String>,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct GenConfig {
    pub min_cyclomatic_complexity: u32,
    pub max_budget: u32,
    pub function_count: u32,
}

pub struct Gen {
    pub(crate) frames: Vec<Frame>,
    budget: u32,
    config: GenConfig,
}

impl Gen {
    pub fn new(config: GenConfig) -> Self {
        Self {
            frames: Vec::new(),
            budget: config.max_budget,
            config,
        }
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
