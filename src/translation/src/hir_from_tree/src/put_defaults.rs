use crate::Ast2HirCtx;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_hir_get_type::HirGetType;
use std::collections::HashMap;

pub(crate) fn module_put_defaults(module: &mut Module, ctx: &mut Ast2HirCtx, _log: &CompilerLog) {
    module
        .iter_mut()
        .for_each_value_mut(&ctx.store, &mut |value| match value {
            Value::Call {
                callee,
                positional,
                named,
            } => match &*ctx[callee as &ValueId].borrow() {
                Value::FunctionSymbol { id } => {
                    let function = ctx[id].borrow();

                    let mut augmented = Vec::new();
                    augmented.extend(positional.iter().cloned());

                    let mut callee_map = HashMap::new();
                    for (name, value) in named.iter() {
                        callee_map.insert(name.clone(), value.clone());
                    }

                    for signature_param in function.params.iter().skip(positional.len()) {
                        let signature_param = ctx[signature_param].borrow();

                        if let Some(arg_value) = callee_map.get(&signature_param.name) {
                            augmented.push(arg_value.clone());
                        } else if let Some(default_value) = &signature_param.default_value {
                            augmented.push(default_value.clone());
                        } else {
                            // Missing argument without default value
                        }
                    }

                    *positional = augmented.into();
                    named.clear();
                }

                _ => {}
            },

            Value::MethodCall {
                object,
                method_name,
                positional,
                named,
            } => {
                let object = &ctx.store[object as &ValueId].borrow();

                if let Ok(object_type) = object
                    .get_type(&ctx.store, &ctx.tab)
                    .map(|t| t.into_id(&ctx.store))
                {
                    if let Some(method) = ctx.tab.get_method(&object_type, method_name) {
                        let function = &ctx.store[method].borrow();

                        let mut augmented = Vec::new();
                        augmented.extend(positional.iter().cloned());

                        let mut callee_map = HashMap::new();
                        for (name, value) in named.iter() {
                            callee_map.insert(name.clone(), value.clone());
                        }

                        for signature_param in function.params.iter().skip(positional.len()) {
                            let signature_param = ctx[signature_param].borrow();

                            if let Some(arg_value) = callee_map.get(&signature_param.name) {
                                augmented.push(arg_value.clone());
                            } else if let Some(default_value) = &signature_param.default_value {
                                augmented.push(default_value.clone());
                            } else {
                                // Missing argument without default value
                            }
                        }

                        *positional = augmented.into();
                        named.clear();
                    }
                }
            }

            _ => {}
        });
}
