use std::{collections::HashMap, ops::Deref};

use crate::{Ast2HirCtx, diagnosis::HirErr, lower::Ast2Hir};
use interned_string::IString;
use nitrate_diagnosis::CompilerLog;
use nitrate_hir::prelude::*;
use nitrate_source::ast::CallArgument;

// fn construct_call(
//     ctx: &mut Ast2HirCtx,
//     log: &CompilerLog,
//     function_type: &FunctionType,
//     arguments: &[(IString, ValueId)],
// ) -> Result<Vec<ValueId>, ()> {
//     fn find_first_hole(arguments: &Vec<Option<ValueId>>) -> usize {
//         if let Some((index, _)) = arguments.iter().enumerate().find(|x| x.1.is_none()) {
//             return index;
//         }

//         // Must be a variadic argument
//         arguments.len()
//     }

//     fn place_argument(
//         position: usize,
//         value: ValueId,
//         log: &CompilerLog,
//         call_arguments: &mut Vec<Option<ValueId>>,
//     ) -> Result<(), ()> {
//         // Push variadic argument
//         if position == call_arguments.len() {
//             call_arguments.push(Some(value));
//             return Ok(());
//         }

//         if let Some(_) = call_arguments[position] {
//             log.report(&HirErr::DuplicateFunctionArguments);
//             return Err(());
//         }

//         call_arguments[position] = Some(value);
//         Ok(())
//     }

//     let mut name_to_pos = HashMap::new();
//     for (index, param) in function_type.params.iter().enumerate() {
//         let param = ctx[param].borrow();
//         name_to_pos.insert(param.name.to_string(), index);
//     }

//     let mut next_pos = 0;
//     let mut call_arguments: Vec<Option<ValueId>> = Vec::new();
//     call_arguments.resize(function_type.params.len(), None);

//     for (name, value) in arguments {
//         match name {
//             Some(name) => {
//                 if let Some(position) = name_to_pos.get(name.deref()) {
//                     let value = value.ast2hir(ctx, log)?.into_id(&ctx.store);
//                     place_argument(position.to_owned(), value, log, &mut call_arguments)?;
//                     next_pos = find_first_hole(&call_arguments);
//                 } else {
//                     log.report(&HirErr::NoSuchParameter(name.to_string()));
//                     return Err(());
//                 }
//             }

//             None => {
//                 let value = value.ast2hir(ctx, log)?.into_id(&ctx.store);
//                 place_argument(next_pos, value, log, &mut call_arguments)?;
//                 next_pos = find_first_hole(&call_arguments);
//             }
//         };
//     }

//     for (i, arg) in call_arguments.iter_mut().enumerate() {
//         if arg.is_none() {
//             if let Some(parameter) = function_type.params.get(i) {
//                 let parameter = ctx[parameter].borrow();
//                 if let Some(default_value) = &parameter.default_value {
//                     if arg.is_none() {
//                         *arg = Some(default_value.clone());
//                         continue;
//                     }
//                 }
//             }

//             log.report(&HirErr::MissingFunctionArguments);
//             return Err(());
//         }
//     }

//     if !function_type
//         .attributes
//         .contains(&FunctionAttribute::Variadic)
//     {
//         if call_arguments.len() > function_type.params.len() {
//             log.report(&HirErr::TooManyFunctionArguments);
//             return Err(());
//         }
//     }

//     Ok(call_arguments
//         .into_iter()
//         .map(|v| v.unwrap())
//         .collect::<Vec<ValueId>>())
// }

pub(crate) fn module_put_defaults(module: &mut Module, ctx: &mut Ast2HirCtx, _log: &CompilerLog) {
    module
        .iter_mut()
        .for_each_value_mut(&ctx.store, &mut |value| match value {
            Value::Call {
                callee: _,
                positional_arguments: _,
                named_arguments: _,
            } => {
                // arguments
                // TODO: Place default values for arguments
            }

            Value::MethodCall {
                object: _,
                method: _,
                arguments: _,
            } => {
                // TODO: Place default values for arguments
            }

            _ => {}
        });
}
