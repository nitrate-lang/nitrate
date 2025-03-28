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

/// TODO: Deprecate this file

#include <boost/bind.hpp>
#include <cstring>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IR/Fwd.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/Init.hh>

using namespace ncc;
using namespace ncc::ir;

// struct ConvState {
//   int32_t indent;
//   size_t indent_width;
//   bool minify;
//   std::unordered_set<uint64_t> types;

//   ConvState(size_t index_width, bool minify)
//       : indent(0), indent_width(index_width), minify(minify) {}
// };

// template <typename L, typename R>
// boost::bimap<L, R> make_bimap(
//     std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
//   return boost::bimap<L, R>(list.begin(), list.end());
// }

// static void escape_string(std::ostream &ss, std::string_view input) {
//   ss << "\"";

//   for (char ch : input) {
//     switch (ch) {
//       case '"':
//         ss << "\\\"";
//         break;
//       case '\\':
//         ss << "\\\\";
//         break;
//       case '\b':
//         ss << "\\b";
//         break;
//       case '\f':
//         ss << "\\f";
//         break;
//       case '\n':
//         ss << "\\n";
//         break;
//       case '\r':
//         ss << "\\r";
//         break;
//       case '\t':
//         ss << "\\t";
//         break;
//       case '\0':
//         ss << "\\0";
//         break;
//       default:
//         if (ch >= 32 && ch < 127) {
//           ss << ch;
//         } else {
//           char hex[5];
//           snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
//           ss << hex;
//         }
//         break;
//     }
//   }

//   ss << "\"";
// }

// static void indent(std::ostream &ss, ConvState &state) {
//   if (state.minify) {
//     return;
//   }

//   ss << "\n";

//   if (state.indent > 0) {
//     ss << std::string(state.indent * state.indent_width, ' ');
//   }
// }

// static bool serialize_recurse(FlowPtr<Expr> n, std::ostream &ss,
//                               std::ostream &typedefs, ConvState &state
// #if !defined(NDEBUG)
//                               ,
//                               std::unordered_set<FlowPtr<Expr>> &visited,
//                               bool is_cylic
// #endif
// ) {
//   if (!n) { /* Nicely handle null nodes */
//     ss << "{?}";
//     return true;
//   }

// #if !defined(NDEBUG)
//   if (is_cylic) {
//     if (visited.contains(n)) {
//       ss << "{...}";
//       return true;
//     }
//     visited.insert(n);
//   }
// #define recurse(x) serialize_recurse(x, ss, typedefs, state, visited,
// is_cylic)
// #define recurse_ex(x, stream) \
//   serialize_recurse(x, stream, typedefs, state, visited, is_cylic)
// #else
// #define recurse(x) serialize_recurse(x, ss, typedefs, state)
// #define recurse_ex(x, stream) serialize_recurse(x, stream, typedefs, state)
// #endif

//   switch (n->GetKind()) {
//     case IR_eBIN: {
//       ss << "(";
//       recurse(n->As<Binary>()->getLHS());
//       ss << " ";
//       ss << n->As<Binary>()->getOp();
//       ss << " ";
//       recurse(n->As<Binary>()->getRHS());
//       ss << ")";
//       break;
//     }
//     case IR_eUNARY: {
//       ss << "(";
//       ss << n->As<Unary>()->getOp();
//       ss << "(";
//       recurse(n->As<Unary>()->getExpr());
//       ss << "))";
//       break;
//     }
//     case IR_eINT: {
//       recurse(n->As<Int>()->GetType().value_or(nullptr));
//       ss << " " << n->As<Int>()->getValueString();
//       break;
//     }
//     case IR_eFLOAT: {
//       recurse(n->As<Float>()->GetType().value_or(nullptr));
//       ss << " " << n->As<Float>()->getValue();
//       break;
//     }
//     case IR_eLIST: {
//       // Check if it matches the string literal pattern
//       List *L = n->As<List>();

//       recurse(L->GetType().value_or(nullptr));
//       ss << " ";

//       bool is_cstring = false;
//       std::string c_string;
//       for (size_t i = 0; i < L->size(); i++) {
//         if (!L.at(i)->is(IR_eINT)) {
//           break;
//         }

//         Int *C = L.at(i)->As<Int>();
//         if (C->getSize() != 8) {
//           break;
//         }

//         c_string.push_back((uint8_t)C->getValue());

//         if (i + 1 == L->size()) {  // Last item
//           if (C->getValue() != 0) {
//             break;
//           }

//           is_cstring = true;

//           escape_string(ss, c_string);
//           break;
//         }
//       }

//       if (!is_cstring) {
//         ss << "{";
//         for (auto it = L->begin(); it != L->end(); ++it) {
//           recurse(*it);
//           if (std::next(it) != L->end()) {
//             ss << ",";
//           }
//         }
//         ss << "}";
//       }
//       break;
//     }
//     case IR_eCALL: {
//       auto tkind = n->As<Call>()->getTarget()->GetKind();
//       if (tkind == IR_eLOCAL) {
//         ss << n->As<Call>()->getTarget()->As<Local>()->GetName();
//       } else if (tkind == IR_eFUNCTION) {
//         ss << n->As<Call>()->getTarget()->As<Function>()->GetName();
//       } else {
//         recurse(n->As<Call>()->getTarget());
//       }
//       ss << "(";
//       for (auto it = n->As<Call>()->getArgs().begin();
//            it != n->As<Call>()->getArgs().end(); ++it) {
//         recurse(*it);
//         if (std::next(it) != n->As<Call>()->getArgs().end()) {
//           ss << ", ";
//         }
//       }
//       ss << ")";
//       break;
//     }
//     case IR_eSEQ: {
//       ss << "seq {";
//       state.indent++;
//       indent(ss, state);
//       for (auto it = n->As<Seq>()->GetItems().begin();
//            it != n->As<Seq>()->GetItems().end(); ++it) {
//         if ((*it)->GetKind() == IR_eIGN) {
//           continue;
//         }

//         recurse(*it);
//         ss << ",";

//         if (std::next(it) != n->As<Seq>()->GetItems().end()) {
//           indent(ss, state);
//         }
//       }
//       state.indent--;
//       indent(ss, state);
//       ss << "}";
//       break;
//     }
//     case IR_eINDEX: {
//       recurse(n->As<Index>()->getExpr());
//       ss << "[";
//       recurse(n->As<Index>()->getIndex());
//       ss << "]";
//       break;
//     }
//     case IR_eIDENT: {
//       recurse(n->As<Identifier>()->GetType().value_or(nullptr));
//       ss << " " << n->As<Identifier>()->GetName();
//       break;
//     }
//     case IR_eEXTERN: {
//       ss << "extern ";
//       escape_string(ss, n->As<Extern>()->getAbiName());
//       ss << " ";
//       recurse(n->As<Extern>()->getValue());
//       break;
//     }
//     case IR_eLOCAL: {
//       ss << "local ";
//       ss << n->As<Local>()->GetName();
//       if (auto ty = n->As<Local>()->getValue()->GetType()) {
//         ss << ": ";
//         recurse(ty.value());
//       }

//       ss << " = ";
//       recurse(n->As<Local>()->getValue());
//       break;
//     }
//     case IR_eRET: {
//       ss << "ret ";
//       recurse(n->As<Ret>()->getExpr());
//       break;
//     }
//     case IR_eBRK: {
//       ss << "brk";
//       break;
//     }
//     case IR_eSKIP: {
//       ss << "cont";
//       break;
//     }

//     case IR_eIF: {
//       ss << "if (";
//       recurse(n->As<If>()->getCond());
//       ss << ") then ";
//       recurse(n->As<If>()->getThen());
//       ss << " else ";
//       recurse(n->As<If>()->getElse());
//       break;
//     }
//     case IR_eWHILE: {
//       ss << "while (";
//       recurse(n->As<While>()->getCond());
//       ss << ") ";
//       recurse(n->As<While>()->getBody());
//       break;
//     }
//     case IR_eFOR: {
//       ss << "for (";
//       recurse(n->As<For>()->getInit());
//       ss << "; ";
//       recurse(n->As<For>()->getCond());
//       ss << "; ";
//       recurse(n->As<For>()->getStep());
//       ss << ") ";
//       recurse(n->As<For>()->getBody());
//       break;
//     }
//     case IR_eCASE: {
//       ss << "case ";
//       recurse(n->As<Case>()->getCond());
//       ss << ": ";
//       recurse(n->As<Case>()->getBody());
//       break;
//     }
//     case IR_eSWITCH: {
//       ss << "switch (";
//       recurse(n->As<Switch>()->getCond());
//       ss << ") {";
//       state.indent++;
//       indent(ss, state);
//       for (auto it = n->As<Switch>()->getCases().begin();
//            it != n->As<Switch>()->getCases().end(); ++it) {
//         recurse(*it);
//         ss << ",";
//         indent(ss, state);
//       }
//       ss << "default: ";
//       recurse(n->As<Switch>()->getDefault());
//       state.indent--;
//       indent(ss, state);
//       ss << "}";
//       break;
//     }
//     case IR_eFUNCTION: {
//       ss << "fn ";
//       ss << n->As<Function>()->GetName();
//       ss << "(";
//       for (auto it = n->As<Function>()->getParams().begin();
//            it != n->As<Function>()->getParams().end(); ++it) {
//         ss << it->second << ": ";
//         recurse(it->first);
//         if (std::next(it) != n->As<Function>()->getParams().end() ||
//             n->As<Function>()->isVariadic()) {
//           ss << ", ";
//         }
//       }
//       if (n->As<Function>()->isVariadic()) {
//         ss << "...";
//       }
//       ss << ") -> ";
//       recurse(n->As<Function>()->getReturn());

//       if (n->As<Function>()->getBody().has_value()) {
//         ss << " ";
//         recurse(n->As<Function>()->getBody().value());
//       }
//       break;
//     }
//     case IR_eASM: {
//       qcore_implement();
//     }
//     case IR_eIGN: {
//       break;
//     }
//     case IR_tU1: {
//       ss << "u1";
//       break;
//     }
//     case IR_tU8: {
//       ss << "u8";
//       break;
//     }
//     case IR_tU16: {
//       ss << "u16";
//       break;
//     }
//     case IR_tU32: {
//       ss << "u32";
//       break;
//     }
//     case IR_tU64: {
//       ss << "u64";
//       break;
//     }
//     case IR_tU128: {
//       ss << "u128";
//       break;
//     }
//     case IR_tI8: {
//       ss << "i8";
//       break;
//     }
//     case IR_tI16: {
//       ss << "i16";
//       break;
//     }
//     case IR_tI32: {
//       ss << "i32";
//       break;
//     }
//     case IR_tI64: {
//       ss << "i64";
//       break;
//     }
//     case IR_tI128: {
//       ss << "i128";
//       break;
//     }
//     case IR_tF16_TY: {
//       ss << "f16";
//       break;
//     }
//     case IR_tF32_TY: {
//       ss << "f32";
//       break;
//     }
//     case IR_tF64_TY: {
//       ss << "f64";
//       break;
//     }
//     case IR_tF128_TY: {
//       ss << "f128";
//       break;
//     }
//     case IR_tVOID: {
//       ss << "void";
//       break;
//     }
//     case IR_tPTR: {
//       recurse(n->As<PtrTy>()->getPointee());
//       ss << "*";
//       break;
//     }
//     case IR_tCONST: {
//       ss << "const<";
//       recurse(n->As<ConstTy>()->GetItem());
//       ss << ">";
//       break;
//     }
//     case IR_tOPAQUE: {
//       ss << "opaque ";
//       ss << n->As<OpaqueTy>()->GetName();
//       break;
//     }
//     case IR_tSTRUCT: {
//       /// TODO: Implement
//       qcore_implement();

//       break;
//     }
//     case IR_tUNION: {
//       /// TODO: Implement
//       qcore_implement();

//       break;
//     }
//     case IR_tARRAY: {
//       ss << "[";
//       recurse(n->As<ArrayTy>()->getElement());
//       ss << "; " << n->As<ArrayTy>()->getCount();
//       ss << "]";
//       break;
//     }
//     case IR_tFUNC: {
//       ss << "fn (";
//       bool variadic = n->As<FnTy>()->isVariadic();
//       for (auto it = n->As<FnTy>()->getParams().begin();
//            it != n->As<FnTy>()->getParams().end(); ++it) {
//         recurse(*it);
//         if (std::next(it) != n->As<FnTy>()->getParams().end() || variadic) {
//           ss << ",";
//         }
//       }
//       if (variadic) {
//         ss << "...";
//       }
//       ss << "): ";
//       recurse(n->As<FnTy>()->getReturn());
//       break;
//     }
//     case IR_tTMP: {
//       ss << "`" << static_cast<uint64_t>(n->As<Tmp>()->getTmpType());
//       ss << ";" << n->As<Tmp>()->getData().index() << "`";
//       break;
//     }
//   }

//   return true;
// }

// static bool to_codeform(std::optional<IRModule *> mod, FlowPtr<Expr> node,
//                         bool minify, size_t indent_size, std::ostream &ss) {
//   ConvState state(indent_size, minify);

//   if (mod.has_value() && !minify) {
//     { /* Print the module name */
//       ss << "; Module: " << (mod.value())->GetName() << "\n";
//     }

//     { /* Print the mutation passes applied */
//       ss << "; Passes: [";
//       size_t i = 0;
//       for (auto it = mod.value()->getPassesApplied().begin();
//            it != mod.value()->getPassesApplied().end(); ++it) {
//         if (it->second != ModulePassType::Transform) {
//           continue;
//         }

//         ss << it->first;

//         if (std::next(it) != mod.value()->getPassesApplied().end()) {
//           ss << ",";

//           if (!minify) {
//             ss << " ";
//             if (i % 6 == 0 && i != 0) {
//               ss << "\n;          ";
//             }
//           }
//         }

//         i++;
//       }
//       ss << "]\n";
//     }

//     { /* Print the analysis passes applied */
//       ss << "; Checks: [";
//       size_t i = 0;
//       for (auto it = mod.value()->getPassesApplied().begin();
//            it != mod.value()->getPassesApplied().end(); ++it) {
//         if (it->second != ModulePassType::Check) {
//           continue;
//         }

//         ss << it->first;

//         if (std::next(it) != mod.value()->getPassesApplied().end()) {
//           ss << ",";

//           if (!minify) {
//             ss << " ";
//             if (i % 6 == 0 && i != 0) {
//               ss << "\n;          ";
//             }
//           }
//         }

//         i++;
//       }
//       ss << "]\n";
//     }

//     { /* Print other metadata */
//       auto now = std::chrono::system_clock::now();
//       auto in_time_t = std::chrono::system_clock::to_time_t(now);
//       std::stringstream tmp_ss;
//       tmp_ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
//       std::string datestamp = tmp_ss.str();

//       ss << "; Timestamp: " << datestamp << "\n";
//       ss << "; Compiler: " << IRLibrary.GetVersion() << "\n";
//       ss << "; Compiler invented by Wesley Jones\n\n";
//     }
//   }

// #if !defined(NDEBUG)
//   std::unordered_set<FlowPtr<Expr>> v;

//   bool is_cylic = !node->isAcyclic();
// #endif

//   std::stringstream body, typedefs;

//   /* Serialize the AST recursively */
//   bool result = serialize_recurse(node, body, typedefs, state
// #if !defined(NDEBUG)
//                                   ,
//                                   v, is_cylic
// #endif
//   );

//   if (!result) {
//     return false;
//   }

//   ss << typedefs.str();
//   ss << body.str();

//   return true;
// }

NCC_EXPORT void ir::NrWrite(IRModule *mod, NullableFlowPtr<Expr> node, std::ostream &out) {
  // if (!mod && !_node) {
  //   return;
  // }

  // FlowPtr<Expr> node = _node ? _node.value() : mod->getRoot();

  // if (mod) {
  //   to_codeform(mod, node, false, 2, out);

  // } else {
  //   to_codeform(std::nullopt, node, false, 2, out);
  // }

  /// TODO: Implement
  qcore_implement();
  (void)mod;
  (void)node;
  (void)out;
}

NCC_EXPORT void ir::detail::NodeDumpImpl(const Expr *e, std::ostream &os, bool is_for_debug) {
  (void)e;
  (void)os;
  (void)is_for_debug;
  /// TODO: Implement
  qcore_implement();
}
