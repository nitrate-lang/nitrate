use nitrate_parsetree::{
    Order, ParseTreeIter, RefNode,
    kind::{Enum, Function, Module, Struct, Trait, TypeAlias, Variable},
};

use std::collections::HashMap;
use std::sync::{Arc, RwLock};

#[derive(Debug)]
pub enum Symbol {
    TypeAlias(Arc<RwLock<TypeAlias>>),
    Struct(Arc<RwLock<Struct>>),
    Enum(Arc<RwLock<Enum>>),
    Trait(Arc<RwLock<Trait>>),
    Function(Arc<RwLock<Function>>),
    Variable(Arc<RwLock<Variable>>),
}

pub fn qualify_name(scope: &[String], name: &str) -> String {
    let mut qualified_name = scope.to_vec();
    qualified_name.push(name.to_string());
    qualified_name.join("::")
}

pub type SymbolTable<'a> = HashMap<String, Symbol>;

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

        RefNode::ItemFunction(sym) => symbol_table
            .entry(qualify_name(&scope_vec, &sym.read().unwrap().name))
            .or_insert_with(|| Symbol::Function(Arc::clone(sym))),

        RefNode::ItemVariable(sym) => symbol_table
            .entry(qualify_name(&scope_vec, &sym.read().unwrap().name))
            .or_insert_with(|| Symbol::Variable(Arc::clone(sym))),

        _ => return,
    };
}

pub fn build_symbol_table(module: &mut Module) -> SymbolTable {
    let mut symbol_table = SymbolTable::new();
    let mut scope_vec = Vec::new();

    module.depth_first_iter(&mut |order, node| {
        if let RefNode::ItemModule(module) = node {
            match order {
                Order::Enter => {
                    scope_vec.push(module.name.to_string());
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
