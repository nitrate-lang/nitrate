// ////////////////////////////////////////////////////////////////////////////////
// /// ///
// ///     .-----------------.    .----------------.     .----------------. ///
// ///    | .--------------. |   | .--------------. |   | .--------------. | ///
// ///    | | ____  _____  | |   | |     ____     | |   | |    ______    | | ///
// ///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | | ///
// ///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | | ///
// ///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | | ///
// ///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | | ///
// ///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | | ///
// ///    | |              | |   | |              | |   | |              | | ///
// ///    | '--------------' |   | '--------------' |   | '--------------' | ///
// ///     '----------------'     '----------------'     '----------------' ///
// /// ///
// ///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language.
// ///
// ///   * Copyright (C) 2024 Wesley C. Jones ///
// /// ///
// ///   The Nitrate Toolchain is free software; you can redistribute it or ///
// ///   modify it under the terms of the GNU Lesser General Public ///
// ///   License as published by the Free Software Foundation; either ///
// ///   version 2.1 of the License, or (at your option) any later version. ///
// /// ///
// ///   The Nitrate Toolcain is distributed in the hope that it will be ///
// ///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// ///
// ///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU ///
// ///   Lesser General Public License for more details. ///
// /// ///
// ///   You should have received a copy of the GNU Lesser General Public ///
// ///   License along with the Nitrate Toolchain; if not, see ///
// ///   <https://www.gnu.org/licenses/>. ///
// /// ///
// ////////////////////////////////////////////////////////////////////////////////

// #define IRBUILDER_IMPL

// #include <algorithm>
// #include <cctype>
// #include <nitrate-core/Logger.hh>
// #include <nitrate-ir/IRB/Builder.hh>
// #include <nitrate-ir/IR/Fwd.hh>
// #include <nitrate-ir/IR/Nodes.hh>
// #include <string>
// #include <unordered_map>

// using namespace ncc::ir;

// void NRBuilder::flatten_symbols(Seq *root) {
//   std::unordered_set<Expr *> symbols;

//   iterate<dfs_post>(root, [&](Expr *P, Expr **C) -> IterOp {
//     if (!P) {
//       return IterOp::Proceed;
//     }

//     bool replace_with_ident = !P->is(IR_eSEQ);

//     if ((!P->is(IR_eEXTERN) && (*C)->is(IR_eFUNCTION)) ||
//         (*C)->is(IR_eEXTERN)) {
//       symbols.insert(*C);

//       if (replace_with_ident) {
//         *C = create<Identifier>((*C)->GetName(), *C);
//       } else {
//         *C = createIgn();
//       }
//     }

//     return IterOp::Proceed;
//   });

//   for (auto ele : symbols) {
//     root->GetItems().push_back(ele);
//   }
// }

// void NRBuilder::try_transform_alpha(Expr *root) {
//   /**
//    * @brief Resolve the `TmpType::NAMED_TYPE` nodes by replacing them with
//    the
//    * actual types they represent.
//    * @note Any nodes that fail to resolve are left alone.
//    */

//   iterate<dfs_pre>(root, [&](Expr *, Expr **C) -> IterOp {
//     Expr *N = *C;

//     if (!N->is(IR_tTMP)) {
//       return IterOp::Proceed;
//     }

//     bool is_default_value_expr =
//         N->is(IR_tTMP) && N->As<Tmp>()->getTmpType() ==
//         TmpType::DEFAULT_VALUE;

//     if (N->As<Tmp>()->getTmpType() == TmpType::NAMED_TYPE ||
//         is_default_value_expr) {
//       /* Get the fully-qualified type name */
//       std::string_view type_name =
//           std::get<std::string_view>(N->As<Tmp>()->getData());

//       auto result = resolve_name(type_name, Kind::TypeDef);
//       if (result.has_value()) [[likely]] {
//         /* Replace the current node */
//         *C = result.value().first;

//         if (is_default_value_expr) {
//           /* Replace the default value expression with the actual default
//           value
//            */
//           if (auto def = getDefaultValue(result.value().first->asType())) {
//             *C = def.value();
//           }
//         }
//       }
//     }

//     return IterOp::Proceed;
//   });
// }

// void NRBuilder::try_transform_beta(Expr *root) {
//   /**
//    * @brief Resolve identifiers by hooking them to the node they represent.
//    This
//    * may create cyclic references, which is okay because these hooks are not
//    * enumerated during normal iteration.
//    * @note Any nodes that fail to resolve are left alone.
//    */

//   iterate<dfs_pre>(root, [&](Expr *, Expr **C) -> IterOp {
//     Expr *N = *C;

//     if (N->is(IR_eIDENT) && N->As<Identifier>()->getWhat() == nullptr) {
//       Identifier *I = N->As<Identifier>();

//       if (auto enum_opt = resolve_name(I->GetName(), Kind::ScopedEnum)) {
//         *C = enum_opt.value().first;
//       } else if (auto var_opt = resolve_name(I->GetName(), Kind::Variable)) {
//         I->SetWhat(var_opt.value().first);
//         I->SetName(var_opt.value().second);
//       }
//     }

//     return IterOp::Proceed;
//   });
// }

// static void resolve_function_call(
//     Expr **C, Expr *callee_ref, FnTy *callee_ty,
//     const std::unordered_map<std::string_view, size_t> &name_index_map,
//     const std::optional<std::unordered_map<size_t, Expr *>>
//     &func_default_args, const GenericCallArguments &user_arguments) {
//   using namespace std;

//   unordered_map<size_t, Expr *> temporary_map(user_arguments.size());
//   auto callee_arg_count = callee_ty->getParams().size();

//   /* Allocate the user supplied arguments into the temporary map. Fail early
//   on
//    * error */
//   if (all_of(user_arguments.begin(), user_arguments.end(),
//              [&](auto user_argument) {
//                auto argument_name = user_argument.first;
//                qcore_assert(!argument_name.empty());
//                auto argument_value = user_argument.second;

//                bool is_positional = isdigit(argument_name[0]);

//                if (is_positional) {
//                  auto position = stoul(string(argument_name));

//                  /* Don't add the same argument twice */
//                  if (!temporary_map.contains(position)) [[likely]] {
//                    temporary_map[position] = argument_value;
//                    return true;
//                  }
//                } else if (auto it = name_index_map.find(argument_name);
//                           it != name_index_map.end()) {
//                  /* Don't add the same argument twice */
//                  if (!temporary_map.contains(it->second)) [[likely]] {
//                    temporary_map[it->second] = argument_value;
//                    return true;
//                  }
//                }

//                return false;
//              })) {
//     /* If default arguments were presented, use them to fill in any missing
//      * caller arguments */
//     if (func_default_args.has_value()) {
//       for (size_t i = 0; i < callee_arg_count; ++i) {
//         if (!temporary_map.contains(i)) {
//           auto it = func_default_args->find(i);
//           if (it != func_default_args->end()) {
//             temporary_map[i] = it->second;
//           }
//         }
//       }
//     }

//     bool is_variadic = callee_ty->isVariadic();
//     bool is_count_valid = is_variadic
//                               ? temporary_map.size() >= callee_arg_count
//                               : temporary_map.size() == callee_arg_count;

//     if (is_count_valid) {
//       /* Check if the arguments are contiguous and that the first one starts
//       at
//        * index 0 */
//       size_t i = 0;
//       bool is_contiguous_and_grounded =
//           all_of(temporary_map.begin(), temporary_map.end(),
//                  [&](auto &) { return temporary_map.contains(i++); });

//       /* Emit the Call IR expression */
//       if (is_contiguous_and_grounded) {
//         CallArgs flattened(temporary_map.size());
//         for (const auto &[index, value] : temporary_map) {
//           flattened[index] = value;
//         }

//         *C = create<Call>(callee_ref, std::move(flattened));
//       }
//     }
//   }
// }

// void NRBuilder::try_transform_gamma(Expr *root) {
//   using namespace std;

//   /* Foreach node in the IR Graph: if the node is a TMP CALL node, replace it
//    * with the appropriate call expression. On failture, skip the node leaving
//    it
//    * unchanged. */
//   iterate<dfs_pre>(root, [&](Expr *, Expr **C) -> IterOp {
//     auto N = *C;

//     if (N->is(IR_tTMP) && N->As<Tmp>()->getTmpType() == TmpType::CALL) {
//       /* The first stage of conversion stored this context information */
//       const auto &data = get<CallArgsTmpNodeCradle>(N->As<Tmp>()->getData());

//       qcore_assert(data.base != nullptr);

//       /* Currently, this code only supported direct function calls */
//       if (data.base->is(IR_eIDENT)) {
//         auto callee_name = data.base->As<Identifier>()->GetName();
//         qcore_assert(!callee_name.empty());

//         unordered_map<string_view, size_t> name_index_map;

//         /* Search the map of function defintions, conducting name resoltion
//         in
//          * the process */
//         if (auto callee_opt = resolve_name(callee_name, Kind::Function)) {
//           qcore_assert(callee_opt.value().first->is(IR_eFUNCTION));
//           auto callee_func_ptr = callee_opt.value().first->As<Function>();

//           /* This layer of indirection is needed to maintain the acylic
//            * properties */
//           auto callee_func =
//               create<Identifier>(callee_func_ptr->GetName(), callee_func_ptr);

//           /* Perform type inference on the callee node */
//           if (auto callee_type_opt = callee_func->GetType();
//               callee_type_opt.has_value() &&
//               callee_type_opt.value()->is_function()) {
//             auto callee_func_type = callee_type_opt.value()->As<FnTy>();

//             const auto &func_default_args =
//                 m_function_defaults.at(callee_func_ptr);

//             /* Use this lookup table to efficiently match named arguments to
//              * their index according to the callee's defintion */
//             auto param_count = callee_func_ptr->getParams().size();
//             name_index_map.reserve(param_count);
//             for (size_t i = 0; i < param_count; ++i) {
//               name_index_map[callee_func_ptr->getParams()[i].second] = i;
//             }

//             /* Do the actual IRGraph Call resoltuon */
//             resolve_function_call(C, callee_func, callee_func_type,
//                                   name_index_map, func_default_args,
//                                   data.args);
//           }
//         } else if (auto callee_opt =
//                        resolve_name(callee_name, Kind::Variable)) {
//           qcore_assert(callee_opt.value().first->is(IR_eLOCAL));

//           /* Check that the caller does not use any named arguments */
//           bool only_positional_args =
//               all_of(data.args.begin(), data.args.end(),
//                      [](auto x) { return isdigit(x.first.at(0)); });

//           if (only_positional_args) {
//             auto callee_local_ptr = callee_opt.value().first->As<Local>();

//             /* This layer of indirection is needed to maintain the acylic
//              * properties */
//             auto callee_local =
//                 create<Identifier>(callee_local_ptr->GetName(), callee_local_ptr);

//             /* Perform type inference on the callee node */
//             if (auto local_type = callee_local->GetType();
//                 local_type.has_value() && local_type.value()->is_function())
//                 {
//               auto callee_func_type = local_type.value()->As<FnTy>();

//               /* Create an identity map */
//               auto param_count = callee_func_type->getParams().size();
//               name_index_map.reserve(param_count);
//               for (size_t i = 0; i < param_count; ++i) {
//                 name_index_map[to_string(i)] = i;
//               }

//               /* Do the actual IRGraph Call resoltuon */
//               resolve_function_call(C, callee_local, callee_func_type,
//                                     name_index_map, nullopt, data.args);
//             }
//           }
//         }
//       }
//     }

//     return IterOp::Proceed;
//   });
// }

// void NRBuilder::connect_nodes(Seq *root) {
//   /* The order of the following matters */

//   try_transform_alpha(root);
//   try_transform_beta(root);
//   try_transform_gamma(root);

//   flatten_symbols(root);
// }

// void NRBuilder::remove_garbage(Seq *root) {
//   iterate<dfs_post>(root, [](Expr *, Expr **C) -> IterOp {
//     if ((*C)->is(IR_eSEQ)) {
//       static const std::unordered_set<nr_ty_t> non_functional_nodes = {
//           IR_eINT,     /* Integer literal */
//           IR_eFLOAT,   /* Floating-point literal */
//           IR_eIDENT,   /* Identifier */
//           IR_eIGN,     /* No-op */
//           IR_tU1,      /* 1-bit unsigned integer (boolean) */
//           IR_tU8,      /* 8-bit unsigned integer */
//           IR_tU16,     /* 16-bit unsigned integer */
//           IR_tU32,     /* 32-bit unsigned integer */
//           IR_tU64,     /* 64-bit unsigned integer */
//           IR_tU128,    /* 128-bit unsigned integer */
//           IR_tI8,      /* 8-bit signed integer */
//           IR_tI16,     /* 16-bit signed integer */
//           IR_tI32,     /* 32-bit signed integer */
//           IR_tI64,     /* 64-bit signed integer */
//           IR_tI128,    /* 128-bit signed integer */
//           IR_tF16_TY,  /* 16-bit floating-point */
//           IR_tF32_TY,  /* 32-bit floating-point */
//           IR_tF64_TY,  /* 64-bit floating-point */
//           IR_tF128_TY, /* 128-bit floating-point */
//           IR_tVOID,    /* Void type */
//           IR_tPTR,     /* Pointer type */
//           IR_tOPAQUE,  /* Opaque type */
//           IR_tSTRUCT,  /* Struct type */
//           IR_tUNION,   /* Union type */
//           IR_tARRAY,   /* Array type */
//           IR_tFUNC,    /* Function type */
//           IR_tCONST,   /* Constant wrapper type */
//       };

//       Seq *S = (*C)->As<Seq>();

//       size_t node_count = 0;
//       for (auto &I : S->GetItems()) {
//         if (non_functional_nodes.contains(I->GetKind())) {
//           I = createIgn();
//         } else {
//           node_count++;
//         }
//       }

//       if (node_count == 0) {
//         *C = createIgn();
//       }
//     }

//     return IterOp::Proceed;
//   });
// }
