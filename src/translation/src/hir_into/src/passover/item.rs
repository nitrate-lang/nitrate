use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::HirCtx;
use nitrate_parsetree::{
    Order, ParseTreeIter, RefNode,
    ast::{self, Enum, Function, Struct, Trait, TypeAlias, Variable},
    tag::{ModuleNameId, intern_module_name},
};
use std::collections::HashMap;
use std::sync::{Arc, RwLock};

#[derive(Debug, Clone)]
enum Symbol {
    TypeAlias(Arc<RwLock<TypeAlias>>),
    Struct(Arc<RwLock<Struct>>),
    Enum(Arc<RwLock<Enum>>),
    Trait(Arc<RwLock<Trait>>),
    Function(Arc<RwLock<Function>>),
    Variable(Arc<RwLock<Variable>>),
}

fn qualify_name(scope: &[ModuleNameId], name: &str) -> SymbolName {
    let length = scope.iter().map(|s| s.len() + 2).sum::<usize>() + name.len();
    let mut qualified = String::with_capacity(length);

    for module in scope {
        qualified.push_str(&module);
        qualified.push_str("::");
    }

    qualified.push_str(name);

    qualified
}

type SymbolName = String;
type SymbolTable<'a> = HashMap<SymbolName, Vec<Symbol>>;

fn symbol_table_add(symbol_table: &mut SymbolTable, scope_vec: &Vec<ModuleNameId>, node: &RefNode) {
    let (name, symbol) = match node {
        RefNode::ItemTypeAlias(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::TypeAlias(Arc::clone(sym)),
        ),

        RefNode::ItemStruct(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Struct(Arc::clone(sym)),
        ),

        RefNode::ItemEnum(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Enum(Arc::clone(sym)),
        ),

        RefNode::ItemTrait(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Trait(Arc::clone(sym)),
        ),

        RefNode::ItemFunction(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Function(Arc::clone(sym)),
        ),

        RefNode::ItemVariable(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Variable(Arc::clone(sym)),
        ),

        _ => return,
    };

    symbol_table
        .entry(name.clone())
        .or_insert_with(Vec::new)
        .push(symbol);
}

fn build_symbol_table(module: &mut ast::Module) -> SymbolTable {
    let mut symbol_table = SymbolTable::new();
    let mut scope_vec = Vec::new();

    module.depth_first_iter(&mut |order, node| {
        if let RefNode::ItemModule(module) = node {
            match order {
                Order::Enter => {
                    if let Some(name) = &module.name {
                        scope_vec.push(name.clone());
                    } else {
                        scope_vec.push(intern_module_name("".to_string()));
                    }
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }

            return;
        }

        if order != Order::Enter {
            return;
        }

        symbol_table_add(&mut symbol_table, &scope_vec, &node);
    });

    symbol_table
}

pub fn passover_module(module: &mut ast::Module, _ctx: &mut HirCtx, _log: &CompilerLog) {
    let _symbol_table = build_symbol_table(module);
    // TODO: Implement the first pass logic over items
}
