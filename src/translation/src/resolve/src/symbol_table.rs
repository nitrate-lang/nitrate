use nitrate_parsetree::{
    Order, ParseTreeIter, RefNode,
    kind::{Enum, Module, NamedFunction, Struct, Trait, TypeAlias, Variable},
};

use std::collections::HashMap;
use std::sync::{Arc, RwLock};

#[derive(Debug)]
pub enum Symbol {
    TypeAlias(Arc<RwLock<TypeAlias>>),
    Struct(Arc<RwLock<Struct>>),
    Enum(Arc<RwLock<Enum>>),
    Trait(Arc<RwLock<Trait>>),
    NamedFunction(Arc<RwLock<NamedFunction>>),
    Variable(Arc<RwLock<Variable>>),
}

pub fn qualify_name(scope: &[String], name: &str) -> Vec<String> {
    let mut qualified_name = scope.to_vec();
    qualified_name.push(name.to_string());
    qualified_name
}

pub type SymbolTable<'a> = HashMap<Vec<String>, Symbol>;

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

        // Only add symbols on pre-order traversal.
        if order == Order::Enter {
            symbol_table_add(&mut symbol_table, &scope_vec, &node);
            return;
        }
    });

    symbol_table
}
