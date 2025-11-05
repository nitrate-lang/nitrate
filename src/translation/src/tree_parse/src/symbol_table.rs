use nitrate_nstring::NString;
use nitrate_tree::{Order, ParseTreeIter, RefNode, ast::Module};
use std::collections::HashSet;

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

pub fn discover_symbols(module: &mut Module) -> HashSet<NString> {
    let mut symbol_set = HashSet::new();
    let mut scope_vec = Vec::new();

    module.depth_first_iter(&mut |order, node| {
        if order == Order::Enter {
            if let Some(symbol_name) = match node {
                RefNode::ItemTypeAlias(sym) => Some(qualify_name(&scope_vec, &sym.name)),
                RefNode::ItemStruct(sym) => Some(qualify_name(&scope_vec, &sym.name)),
                RefNode::ItemEnum(sym) => Some(qualify_name(&scope_vec, &sym.name)),
                RefNode::ItemTrait(sym) => Some(qualify_name(&scope_vec, &sym.name)),
                RefNode::ItemFunction(sym) => Some(qualify_name(&scope_vec, &sym.name)),
                RefNode::ItemGlobalVariable(sym) => Some(qualify_name(&scope_vec, &sym.name)),
                RefNode::ExprLocalVariable(sym) => Some(qualify_name(&scope_vec, &sym.name)),
                _ => None,
            } {
                symbol_set.insert(symbol_name);
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

    symbol_set
}
