use std::collections::HashSet;

use nitrate_tree::{
    Order, ParseTreeIter, RefNode,
    ast::Module,
    tag::{ModuleNameId, intern_module_name},
};

fn qualify_name(scope: &[ModuleNameId], name: &str) -> String {
    let length = scope.iter().map(|s| s.len() + 2).sum::<usize>() + name.len();
    let mut qualified = String::with_capacity(length);

    for module in scope {
        qualified.push_str(&module);
        qualified.push_str("::");
    }

    qualified.push_str(name);
    qualified
}

pub type SymbolSet = HashSet<String>;

fn visit_node(symbol_set: &mut SymbolSet, scope_vec: &Vec<ModuleNameId>, node: &RefNode) {
    let name = match node {
        RefNode::ItemTypeAlias(sym) => qualify_name(&scope_vec, &sym.name),
        RefNode::ItemStruct(sym) => qualify_name(&scope_vec, &sym.name),
        RefNode::ItemEnum(sym) => qualify_name(&scope_vec, &sym.name),
        RefNode::ItemTrait(sym) => qualify_name(&scope_vec, &sym.name),
        RefNode::ItemFunction(sym) => qualify_name(&scope_vec, &sym.name),
        RefNode::ItemGlobalVariable(sym) => qualify_name(&scope_vec, &sym.name),
        _ => return,
    };

    symbol_set.insert(name);
}

pub fn discover_symbols(module: &mut Module) -> SymbolSet {
    let mut symbol_set = SymbolSet::new();
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

        visit_node(&mut symbol_set, &scope_vec, &node);
    });

    symbol_set
}
