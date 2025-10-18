use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::HirCtx;
use nitrate_parsetree::{
    Order, ParseTreeIter, RefNode,
    ast::{self, Enum, Function, Item, Struct, Trait, TypeAlias, Variable},
};
use std::collections::HashMap;
use std::sync::{Arc, RwLock};

#[derive(Debug, Clone)]
enum Symbol {
    TypeAlias(Arc<RwLock<ast::TypeAlias>>),
    Struct(Arc<RwLock<ast::Struct>>),
    Enum(Arc<RwLock<ast::Enum>>),
    Trait(Arc<RwLock<ast::Trait>>),
    Function(Arc<RwLock<ast::Function>>),
    Variable(Arc<RwLock<ast::Variable>>),
}

type SymbolName = String;
type SymbolTable<'a> = HashMap<SymbolName, Vec<Symbol>>;

fn qualify_name(scope: &[String], name: &str) -> SymbolName {
    let length = scope.iter().map(|s| s.len() + 2).sum::<usize>() + name.len();
    let mut qualified = String::with_capacity(length);

    for module in scope {
        qualified.push_str(&module);
        qualified.push_str("::");
    }

    qualified.push_str(name);

    qualified
}

fn symbol_table_add(symbol_table: &mut SymbolTable, scope_vec: &Vec<String>, node: &Item) {
    let (name, symbol) = match node {
        Item::TypeAlias(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::TypeAlias(Arc::clone(sym)),
        ),

        Item::Struct(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Struct(Arc::clone(sym)),
        ),

        Item::Enum(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Enum(Arc::clone(sym)),
        ),

        Item::Trait(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Trait(Arc::clone(sym)),
        ),

        Item::Function(sym) => (
            qualify_name(&scope_vec, &sym.read().unwrap().name),
            Symbol::Function(Arc::clone(sym)),
        ),

        Item::Variable(sym) => (
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
                        scope_vec.push(name.to_string());
                    } else {
                        scope_vec.push(String::default());
                    }

                    for item in &module.items {
                        symbol_table_add(&mut symbol_table, &scope_vec, item);
                    }
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }
        }
    });

    symbol_table
}

pub fn passover_module(module: &mut ast::Module, _ctx: &mut HirCtx, _log: &CompilerLog) {
    // TODO: Implement the first pass logic over items

    for (name, symbol) in build_symbol_table(module) {
        println!("Symbol: {} => {:#?}", name, symbol);
    }
}
