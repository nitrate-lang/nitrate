use nitrate_nstring::NString;
use nitrate_tree::{
    Order, ParseTreeIter, RefNode,
    ast::{Generics, Module, SymbolKind},
};
use std::collections::HashMap;

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

fn enumerate_generics(
    scope_vec: &mut Vec<NString>,
    name: NString,
    generics: &Option<Generics>,
    symbol_map: &mut HashMap<NString, SymbolKind>,
) {
    if let Some(generics) = generics {
        scope_vec.push(name.clone());

        for generic in &generics.params {
            let generic_name = qualify_name(&scope_vec, &generic.name);
            symbol_map.insert(generic_name, SymbolKind::GenericParameter);
        }

        scope_vec.pop();
    }
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
                    enumerate_generics(&mut scope_vec, sym.name.clone(), &sym.generics, &mut symbol_map);
                }

                RefNode::ItemEnum(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Enum);
                    enumerate_generics(&mut scope_vec, sym.name.clone(), &sym.generics, &mut symbol_map);
                }

                RefNode::ItemEnumVariant(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::EnumVariant);
                }

                RefNode::ItemTrait(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Trait);
                    enumerate_generics(&mut scope_vec, sym.name.clone(), &sym.generics, &mut symbol_map);
                }

                RefNode::ItemFunction(sym) => {
                    let name = qualify_name(&scope_vec, &sym.name);
                    symbol_map.insert(name, SymbolKind::Function);
                    enumerate_generics(&mut scope_vec, sym.name.clone(), &sym.generics, &mut symbol_map);
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

        let scope_add = match node {
            RefNode::ItemTypeAlias(type_alias) => Some(type_alias.name.clone()),
            RefNode::ItemStruct(struct_def) => Some(struct_def.name.to_string().into()),
            RefNode::ItemEnum(enum_def) => Some(enum_def.name.to_string().into()),
            RefNode::ItemTrait(trait_def) => Some(trait_def.name.to_string().into()),
            RefNode::ItemFunction(function) => Some(function.name.to_string().into()),
            RefNode::ItemModule(module) => Some(module.name.clone()),
            _ => None,
        };

        if let Some(name) = scope_add {
            match order {
                Order::Enter => {
                    scope_vec.push(name);
                }

                Order::Leave => {
                    scope_vec.pop();
                }
            }
        }
    });

    symbol_map
}
