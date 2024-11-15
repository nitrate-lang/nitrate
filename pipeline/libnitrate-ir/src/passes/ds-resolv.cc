////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <nitrate-ir/IR.h>

#include <boost/bimap.hpp>
#include <nitrate-ir/IRGraph.hh>
#include <passes/PassList.hh>
#include <string_view>

// /// TODO: Worst code ever written. Refactor this.

// /**
//  * @brief Remove the temporary nodes and replace them with a resolved node.
//  *
//  * @timecomplexity O(n)
//  * @spacecomplexity O(n)
//  */

// using namespace nr;

// static std::pair<std::vector<std::string>, std::string>
// split_ns(std::string_view the) {
//   std::vector<std::string> ns;

//   size_t start = 0;
//   size_t end = the.find("::");

//   while (end != std::string::npos) {
//     ns.push_back(std::string(the.substr(start, end - start)));
//     start = end + 2;
//     end = the.find("::", start);
//   }

//   ns.push_back(std::string(the.substr(start)));

//   std::string name = ns.back();
//   ns.pop_back();

//   return {ns, name};
// }

// static std::string join_ns(const std::vector<std::string> &ns, const
// std::string &name) {
//   std::string result;

//   for (size_t i = 0; i < ns.size(); i++) {
//     result += ns[i] + "::";
//   }

//   result += name;
//   return result;
// }

// enum class IdentWhat {
//   Variable,
//   NamedConstant,
//   Function,
//   Typename,
// };

// static std::optional<std::pair<std::string, IdentWhat>>
// resolve_name(qmodule_t *mod,
//                                                                      std::string_view
//                                                                      name) {
//   // This algorithm is SLOOOW; FIXME: Make it faster
//   auto ns = split_ns(name);

//   {  // Resolve attempt 1
//     while (true) {
//       std::string n = join_ns(ns.first, ns.second);

//       if (mod->getGlobalVariables().left.count(n) > 0) {
//         return std::make_pair(n, IdentWhat::Variable);
//       }

//       if (mod->getNamedConstants().count(n) > 0) {
//         return std::make_pair(n, IdentWhat::NamedConstant);
//       }

//       if (mod->getFunctions().left.count(n) > 0) {
//         return std::make_pair(n, IdentWhat::Function);
//       }

//       if (mod->getTypeMap().count(n) > 0) {
//         return std::make_pair(n, IdentWhat::Typename);
//       }

//       if (ns.first.empty()) {
//         break;
//       }

//       ns.first.pop_back();
//     }
//   }

//   ns = split_ns(name);

//   {  // Resolve attempt 2
//     while (true) {
//       std::string n = join_ns(ns.first, ns.second);

//       if (mod->getGlobalVariables().left.count(n) > 0) {
//         return std::make_pair(n, IdentWhat::Variable);
//       }

//       if (mod->getNamedConstants().count(n) > 0) {
//         return std::make_pair(n, IdentWhat::NamedConstant);
//       }

//       if (mod->getFunctions().left.count(n) > 0) {
//         return std::make_pair(n, IdentWhat::Function);
//       }

//       if (mod->getTypeMap().count(n) > 0) {
//         return std::make_pair(n, IdentWhat::Typename);
//       }

//       if (ns.first.empty()) {
//         break;
//       }

//       ns.first.erase(ns.first.begin());
//     }
//   }

//   return std::nullopt;
// }

// static bool resolve_node(nr::Expr **_cur) {
//   if ((*_cur)->getKind() != QIR_NODE_TMP) {
//     return true;
//   }

//   using namespace nr;

//   Tmp *cur = (*_cur)->as<Tmp>();
//   TmpType tt = cur->getTmpType();
//   qmodule_t *mod = cur->getModule();
//   bool error = false;

//   switch (tt) {
//     case TmpType::CALL: {
//       const auto &info = std::get<CallArgsTmpNodeCradle>(cur->getData());
//       const Expr *base = std::get<0>(info);
//       const auto &user_args = std::get<1>(info);

//       if (base->getKind() == QIR_NODE_IDENT) { /* Direct call */
//         std::string funcname = std::string(base->as<Ident>()->getName());

//         { /* Resolve the function */
//           auto resolved = resolve_name(mod, funcname);
//           if (!resolved) {
//             NO_MATCHING_FUNCTION(funcname);
//             error = true;
//             break;
//           }

//           if (resolved->second != IdentWhat::Function) {
//             NO_MATCHING_FUNCTION(funcname);
//             error = true;
//             break;
//           }

//           funcname = resolved->first;
//         }

//         qcore_assert(mod->getParameterMap().contains(funcname));

//         struct PMapVal {
//           size_t pos;
//           Type *type;
//         };

//         /* Some faster data structures */
//         Expr *target = mod->getFunctions().left.at(funcname).second;
//         FnTy *ft = mod->getFunctions().left.at(funcname).first;
//         std::unordered_map<std::string_view, PMapVal> param_map;
//         CallArgs arguments;
//         size_t pos_i = 0;

//         /* Calculate the number of arguments to be passed */
//         arguments.resize(std::max(user_args.size(), ft->getParams().size()));

//         const auto &params = mod->getParameterMap().at(funcname);
//         if (params.size() != ft->getParams().size()) {
//           error = true;
//           report(UnknownArgument, IC::Error, funcname);
//           break;
//         }

//         /* Create a map of parameter names to their positions and types */
//         for (size_t i = 0; i < params.size(); i++) {
//           param_map[std::get<0>(params[i])] = {i, std::get<1>(params[i])};
//         }

//         { /* Check for too many arguments */
//           if (user_args.size() > params.size() &&
//           !ft->getAttrs().contains(FnAttr::Variadic)) {
//             TOO_MANY_ARGUMENTS(funcname);
//             error = true;
//             break;
//           }
//         }

//         { /* Allocate arguments into function parameters */
//           for (const auto &[name, arg] : user_args) {
//             /* Handle positional argument */
//             if (std::isdigit(name.at(0))) {
//               if (pos_i < params.size()) {
//                 BinExpr *casted = create<BinExpr>(arg,
//                 std::get<1>(params.at(pos_i)), Op::CastAs);
//                 arguments.at(pos_i) = casted;
//               } else {
//                 // This must be a variadic argument. Casting is not possible.
//                 arguments.at(pos_i) = arg;
//               }
//               pos_i++;
//             } else { /* Handle named argument */
//               if (!param_map.contains(name)) {
//                 NO_MATCHING_PARAMETER(funcname, name);
//                 error = true;
//                 break;
//               }

//               BinExpr *casted = create<BinExpr>(arg, param_map[name].type,
//               Op::CastAs); arguments[param_map[name].pos] = casted;

//               /* Handle mixed named and positional arguments */
//               if (param_map[name].pos <= pos_i) {
//                 pos_i++;
//               }
//             }
//           }
//           if (error) {
//             break;
//           }

//           /* Assign default arguments and do checking */
//           for (size_t i = 0; i < arguments.size(); i++) {
//             if (arguments[i]) {
//               continue;
//             }

//             Expr *_default = std::get<2>(params.at(i));
//             if (_default) {
//               arguments[i] = create<BinExpr>(_default,
//               std::get<1>(params.at(i)), Op::CastAs);
//             } else {
//               NO_MATCHING_PARAMETER(funcname, std::get<0>(params.at(i)));
//               error = true;
//               break;
//             }
//           }

//           if (error) {
//             break;
//           }
//         }

//         /* Create the call */
//         *_cur = create<Call>(target, std::move(arguments));
//       } else {
//         std::cout << "Indirect call not supported yet for type: " <<
//         base->getKindName()
//                   << std::endl;
//         error = true;
//       }
//       break;
//     }

//     case TmpType::ENUM: {
//       std::string_view name =
//       std::get<std::string_view>(cur->as<Tmp>()->getData());

//       if (!mod->getTypeMap().contains(name)) {
//         report(UnknownType, IC::Error, name);
//         error = true;
//         break;
//       }

//       Type *type = mod->getTypeMap().at(name);

//       if (type->getKind() != QIR_NODE_TMP) {
//         *_cur = type;
//         break;
//       }

//       for (auto &[const_name, val] : mod->getNamedConstants()) {
//         if (!const_name.starts_with(name)) {
//           CONV_DEBUG("Skipping constant: " + std::string(const_name));
//           continue;
//         }

//         nr_node_t *inferred = nr_infer(val);
//         if (!inferred) {
//           error = true;
//           break;
//         }

//         *_cur = static_cast<Expr *>(inferred);
//         break;
//       }

//       break;
//     }

//     case TmpType::NAMED_TYPE: {
//       std::string_view name =
//       std::get<std::string_view>(cur->as<Tmp>()->getData());

//       auto resolved = resolve_name(mod, name);

//       if (resolved && resolved->second == IdentWhat::Typename) {
//         *_cur = mod->getTypeMap().at(resolved->first);
//       } else {
//         report(UnknownType, IC::Error, name);
//         error = true;
//       }
//       break;
//     }
//   }

//   return !error;
// }

// static bool recursive_resolve(nr::Expr **base) {
//   using namespace nr;

//   bool error = false;

//   const auto cb = [&error](Expr *, Expr **_cur) -> IterOp {
//     if (!resolve_node(_cur)) {
//       error = true;
//     }

//     /// TODO: BUG: IMPORTANT: Fix the resolution pass to guarantee that
//     cyclic references are not
//     /// created

//     // if (!(*_cur)->isAcyclic()) {
//     //   (*_cur)->getModule()->getDiag().push(
//     //       QXIR_AUDIT_CONV,
//     //       DiagMessage(
//     //           "Cyclic polymorphic node reference detected in module IR
//     data structure.",
//     //           IC::FatalError, DSPolyCyclicRef));
//     //   error = true;
//     //   return IterOp::Abort;
//     // }

//     return IterOp::Proceed;
//   };

//   iterate<IterMode::dfs_pre>(*base, cb);

//   return !error;
// }

// static bool alpha_pass(qmodule_t *mod) {
//   using namespace nr;

//   bool error = false;

//   { /* Phase 1 */
//     for (auto it = mod->getParameterMap().begin(); it !=
//     mod->getParameterMap().end(); it++) {
//       for (auto &[name, type, expr] : it->second) {
//         Expr *tmp = type;
//         if (!recursive_resolve(&tmp)) {
//           error = true;
//         }
//         type = tmp->asType();

//         if (expr) {
//           tmp = expr;
//           if (!recursive_resolve(&tmp)) {
//             error = true;
//           }

//           expr = tmp;
//         }
//       }
//     }
//   }

//   { /* Phase 2 */
//     std::vector<std::pair<std::string_view, std::pair<FnTy *, Fn *>>> simped;

//     for (auto it = mod->getFunctions().begin(); it !=
//     mod->getFunctions().end(); it++) {
//       simped.push_back({(*it).left, {(*it).right.first,
//       (*it).right.second}});
//     }

//     for (auto &[name, func] : simped) {
//       Expr *fn = func.second;
//       Expr *ft = func.first;

//       error |= !recursive_resolve(&fn) || !recursive_resolve(&ft);
//     }

//     mod->getFunctions().clear();
//     for (auto &[name, func] : simped) {
//       mod->getFunctions().insert({name, func});
//     }
//   }

//   { /* Phase 3 */
//     for (auto it = mod->getCompositeFields().begin(); it !=
//     mod->getCompositeFields().end(); it++) {
//       for (auto &[name, type, expr] : it->second) {
//         Expr *tmp = type;
//         if (!recursive_resolve(&tmp)) {
//           error = true;
//         }
//         type = tmp->asType();

//         if (expr) {
//           tmp = expr;
//           if (!recursive_resolve(&tmp)) {
//             error = true;
//           }

//           expr = tmp;
//         }
//       }
//     }
//   }

//   { /* Phase 4 */
//     for (auto &[name, expr] : mod->getNamedConstants()) {
//       Expr *tmp = expr;
//       if (!recursive_resolve(&tmp)) {
//         error = true;
//       }
//       expr = tmp;
//     }
//   }

//   { /* Phase 5 */
//     for (auto &[name, type] : mod->getTypeMap()) {
//       Expr *tmp = type;
//       if (!recursive_resolve(&tmp)) {
//         error = true;
//       }
//       type = tmp->asType();
//     }
//   }

//   { /* Phase 6 */
//     if (!recursive_resolve(&mod->getRoot())) {
//       error = true;
//     }
//   }

//   return !error;
// }

// static bool beta_pass(qmodule_t *mod) {
//   using namespace nr;

//   bool error = false;

//   auto cb = [&](Expr *, Expr **_cur) -> IterOp {
//     if ((*_cur)->getKind() != QIR_NODE_IDENT) {
//       return IterOp::Proceed;
//     }

//     Ident *cur = (*_cur)->as<Ident>();
//     if (cur->getWhat()) {
//       return IterOp::Proceed;
//     }

//     std::string_view name = cur->getName();

//     auto resolved = resolve_name(mod, name);

//     if (resolved) {
//       if (resolved->second == IdentWhat::Variable) {
//         cur->setWhat(mod->getGlobalVariables().left.at(resolved->first));
//       } else if (resolved->second == IdentWhat::NamedConstant) {
//         cur->setWhat(mod->getNamedConstants().at(resolved->first));
//       } else if (resolved->second == IdentWhat::Function) {
//         cur->setWhat(mod->getFunctions().left.at(resolved->first).first);
//       }

//       qcore_assert(!resolved->first.empty());
//       cur->setName(cur->getModule()->internString(resolved->first));

//       return IterOp::Proceed;
//     }

//     return IterOp::Proceed;
//   };

//   iterate<IterMode::dfs_pre>(mod->getRoot(), cb);

//   ///=============================================================================
//   /// Resolve function parameters

//   auto cb2 = [&](Expr *, Expr **_cur) -> IterOp {
//     if ((*_cur)->getKind() != QIR_NODE_FN) {
//       return IterOp::Proceed;
//     }

//     Fn *cur = (*_cur)->as<Fn>();
//     auto name = cur->getName();

//     auto inner = [name, mod](Expr *, Expr **_cur) -> IterOp {
//       if ((*_cur)->getKind() == QIR_NODE_FN) {
//         return IterOp::SkipChildren;
//       }

//       if ((*_cur)->getKind() != QIR_NODE_IDENT) {
//         return IterOp::Proceed;
//       }

//       if ((*_cur)->as<Ident>()->getWhat()) {
//         return IterOp::Proceed;
//       }

//       std::string_view ident_name = (*_cur)->as<Ident>()->getName();
//       size_t l = ident_name.find_last_of("::");
//       if (l != std::string::npos) {
//         ident_name = ident_name.substr(l + 1);
//       }

//       for (auto &[param_name, param_type, param_default] :
//       mod->getParameterMap().at(name)) {
//         if (param_name == ident_name) {
//           /// FIXME: Should point to a symbol
//           // (*_cur)->as<Ident>()->setWhat(param_type);
//           return IterOp::Proceed;
//         }
//       }

//       auto cur = (*_cur)->as<Ident>();

//       report(UnresolvedIdentifier, IC::Error, ident_name, cur->locBeg(),
//              cur->locEnd());

//       return IterOp::Proceed;
//     };

//     Expr *body = cur->getBody();
//     iterate<IterMode::dfs_pre>(body, inner);

//     return IterOp::Proceed;
//   };

//   iterate<IterMode::dfs_pre>(mod->getRoot(), cb2);

//   ///=============================================================================

//   auto cb3 = [&](Expr *, Expr **_cur) -> IterOp {
//     if ((*_cur)->getKind() != QIR_NODE_IDENT) {
//       return IterOp::Proceed;
//     }

//     Ident *cur = (*_cur)->as<Ident>();

//     if (!cur->getWhat()) {
//       report(UnresolvedIdentifier, IC::Error, cur->getName(), cur->locBeg(),
//              cur->locEnd());
//     }

//     return IterOp::Proceed;
//   };

//   iterate<IterMode::dfs_pre>(mod->getRoot(), cb3);

//   ///=============================================================================

//   return !error;
// }

bool nr::pass::ds_resolv(qmodule_t *, IReport *) {
  bool error = false;

  // error |= !alpha_pass(mod);
  // error |= !beta_pass(mod);

  // for (auto &[name, fn] : mod->getFunctions()) {
  //   CONV_DEBUG("Found function: " + std::string(name));
  // }
  // for (auto &[name, var] : mod->getGlobalVariables()) {
  //   CONV_DEBUG("Found variable: " + std::string(name));
  // }

  return !error;
}
