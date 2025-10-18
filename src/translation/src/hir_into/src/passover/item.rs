use nitrate_diagnosis::CompilerLog;
use nitrate_hir::hir::HirCtx;
use nitrate_parsetree::{
    Order, ParseTreeIter, RefNode,
    ast::{self, Item},
};

fn qualify_name(scope: &[String], name: &str) -> String {
    let length = scope.iter().map(|s| s.len() + 2).sum::<usize>() + name.len();
    let mut qualified = String::with_capacity(length);

    for module in scope {
        qualified.push_str(&module);
        qualified.push_str("::");
    }

    qualified.push_str(name);
    qualified
}

fn visit_item(_ctx: &mut HirCtx, scope_vec: &Vec<String>, node: &Item) {
    match node {
        Item::TypeAlias(sym) => {
            let _name = qualify_name(&scope_vec, &sym.read().unwrap().name);
        }

        Item::Struct(sym) => {
            let _name = qualify_name(&scope_vec, &sym.read().unwrap().name);
        }

        Item::Enum(sym) => {
            let _name = qualify_name(&scope_vec, &sym.read().unwrap().name);
        }

        Item::Trait(sym) => {
            let _name = qualify_name(&scope_vec, &sym.read().unwrap().name);
        }

        Item::Function(sym) => {
            let _name = qualify_name(&scope_vec, &sym.read().unwrap().name);
        }

        Item::Variable(sym) => {
            let _name = qualify_name(&scope_vec, &sym.read().unwrap().name);
        }

        _ => return,
    }
}

pub fn passover_module(module: &mut ast::Module, ctx: &mut HirCtx, _log: &CompilerLog) {
    // TODO: Implement the first pass logic over items

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
                        visit_item(ctx, &scope_vec, item);
                    }
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }
        }
    });
}
