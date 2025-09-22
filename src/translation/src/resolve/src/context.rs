use nitrate_diagnosis::{
    DiagnosticCollector, DiagnosticGroupId, DiagnosticInfo, FormattableDiagnosticGroup,
};

use nitrate_parsetree::{
    Order, ParseTreeIter, RefNode,
    kind::{Enum, Item, Module, NamedFunction, Path, Struct, Trait, TypeAlias, Variable},
};

use std::collections::HashMap;
use std::sync::{Arc, RwLock};

pub enum ResolveIssue {
    NotFound(Path),
    Ambiguous(Path, Vec<Item>),
}

impl FormattableDiagnosticGroup for ResolveIssue {
    fn group_id(&self) -> DiagnosticGroupId {
        DiagnosticGroupId::Resolve
    }

    fn variant_id(&self) -> u16 {
        match self {
            ResolveIssue::NotFound(_) => 1,
            ResolveIssue::Ambiguous(_, _) => 2,
        }
    }

    fn format(&self) -> DiagnosticInfo {
        match self {
            ResolveIssue::NotFound(path) => DiagnosticInfo {
                origin: nitrate_diagnosis::Origin::None,
                message: format!("Symbol not found: {}", path.path.join("::")),
            },

            ResolveIssue::Ambiguous(path, candidates) => {
                // FIXME: Improve formatting.

                let mut message = format!(
                    "Ambiguous symbol reference: {}\nCandidates:",
                    path.path.join("::")
                );

                for candidate in candidates {
                    message.push_str(&format!("\n - {:?}", candidate));
                }

                DiagnosticInfo {
                    origin: nitrate_diagnosis::Origin::None,
                    message,
                }
            }
        }
    }
}

#[derive(Debug)]
enum Symbol {
    TypeAlias(Arc<RwLock<TypeAlias>>),
    Struct(Arc<RwLock<Struct>>),
    Enum(Arc<RwLock<Enum>>),
    Trait(Arc<RwLock<Trait>>),
    NamedFunction(Arc<RwLock<NamedFunction>>),
    Variable(Arc<RwLock<Variable>>),
}

fn qualify_name(scope: &[String], name: &str) -> Vec<String> {
    let mut qualified_name = scope.to_vec();
    qualified_name.push(name.to_string());
    qualified_name
}

type SymbolTable<'a> = HashMap<Vec<String>, Symbol>;

fn symbol_table_add(symbol_table: &mut SymbolTable, scope_vec: &Vec<String>, node: &RefNode) {
    match node {
        RefNode::ItemTypeAlias(sym) => symbol_table
            .entry(qualify_name(&scope_vec, &sym.read().unwrap().name))
            .or_insert_with(|| Symbol::TypeAlias(Arc::clone(sym))),

        RefNode::ItemStruct(sym) => symbol_table
            .entry(qualify_name(&scope_vec, &sym.read().unwrap().name))
            .or_insert_with(|| Symbol::Struct(Arc::clone(sym))),

        RefNode::ItemEnum(sym) => symbol_table
            .entry(qualify_name(&scope_vec, &sym.read().unwrap().name))
            .or_insert_with(|| Symbol::Enum(Arc::clone(sym))),

        RefNode::ItemTrait(sym) => symbol_table
            .entry(qualify_name(&scope_vec, &sym.read().unwrap().name))
            .or_insert_with(|| Symbol::Trait(Arc::clone(sym))),

        RefNode::ItemNamedFunction(sym) => symbol_table
            .entry(qualify_name(&scope_vec, &sym.read().unwrap().name))
            .or_insert_with(|| Symbol::NamedFunction(Arc::clone(sym))),

        RefNode::ItemVariable(sym) => symbol_table
            .entry(qualify_name(&scope_vec, &sym.read().unwrap().name))
            .or_insert_with(|| Symbol::Variable(Arc::clone(sym))),

        _ => return,
    };
}

fn build_symbol_table(module: &mut Module) -> SymbolTable {
    let mut symbol_table = SymbolTable::new();
    let mut scope_vec = Vec::new();

    module.depth_first_iter(&mut |order, node| {
        if let RefNode::ItemModule(module) = node {
            match order {
                Order::Pre => {
                    scope_vec.push(module.name.to_string());
                }

                Order::Post => {
                    scope_vec.pop();
                }
            }

            return;
        }

        // Only add symbols on pre-order traversal.
        if order == Order::Pre {
            symbol_table_add(&mut symbol_table, &scope_vec, &node);
            return;
        }
    });

    symbol_table
}

pub fn resolve(module: &mut Module, _bugs: &DiagnosticCollector) {
    let symbol_table = build_symbol_table(module);

    for (entry, _) in &symbol_table {
        println!("Symbol: {:?} => {:?}", entry, ());
    }
}
