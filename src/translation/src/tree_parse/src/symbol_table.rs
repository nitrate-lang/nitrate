use nitrate_nstring::NString;
use nitrate_tree::{
    Order, ParseTreeIter, RefNode,
    ast::{Module, SymbolKind},
};
use std::collections::{HashMap, HashSet};

fn qualify_name(scope: &[NString], name: &str) -> NString {
    let length = scope.iter().map(|s| s.len() + 2).sum::<usize>() + name.len();
    let mut qualified = String::with_capacity(length);

    for module in scope {
        qualified.push_str(&module);
        qualified.push_str("::");
    }

    qualified.push_str(name);
    qualified.into()
}

pub fn discover_symbols(module: &mut Module) -> HashMap<NString, SymbolKind> {
    let mut symbol_map = HashMap::new();
    let mut scope_vec = Vec::new();

    module.depth_first_iter(&mut |order, node| {
        if order == Order::Enter {
            match node {
                RefNode::ItemTypeAlias(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::TypeAlias);
                }

                RefNode::ItemStruct(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Struct);
                }

                RefNode::ItemEnum(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Enum);
                }

                RefNode::ItemTrait(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Trait);
                }

                RefNode::ItemFunction(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Function);
                }

                RefNode::ItemFuncParam(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Parameter);
                }

                RefNode::ItemGlobalVariable(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::GlobalVariable);
                }

                RefNode::ExprLocalVariable(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::LocalVariable);
                }

                _ => {}
            }
        }

        if let RefNode::ItemModule(module) = node {
            match order {
                Order::Enter => {
                    scope_vec.push(module.name.clone());
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }
        } else if let RefNode::ItemFunction(function) = node {
            match order {
                Order::Enter => {
                    scope_vec.push(function.name.to_string().into());
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }
        }
    });

    symbol_map
}
