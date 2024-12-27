

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

// #include <cstring>
// #include <nitrate-core/Logger.hh>
// #include <nitrate-core/Macro.hh>
// #include <nitrate-ir/IR.hh>
// #include <nitrate-ir/IR/Nodes.hh>
// #include <nitrate-parser/Context.hh>
// #include <unordered_map>
// #include <unordered_set>

// using namespace ncc;
// using namespace ncc::ir;

// static Expr *IRCloneRecurse(const Expr *_node,
//                             std::unordered_map<const Expr *, Expr *> &map,
//                             std::unordered_set<Expr *> &in_visited) {
//   /// TODO: This code has bugs; fix it

// #define clone(X) static_cast<Expr *>(IRCloneRecurse(X, map, in_visited))

//   using namespace ncc::ir;

//   std::optional<Expr *> out;

//   if (!_node) {
//     return nullptr;
//   }

//   auto in = const_cast<Expr *>(_node);

//   switch (in->getKind()) {
//     case IR_eBIN: {
//       BinExpr *n = static_cast<BinExpr *>(in);
//       out = create<BinExpr>(clone(n->getLHS()), clone(n->getRHS()),
//       n->getOp()); break;
//     }
//     case IR_eUNARY: {
//       Unary *n = static_cast<Unary *>(in);
//       out = create<Unary>(clone(n->getExpr()), n->getOp(), n->isPostfix());
//       break;
//     }
//     case IR_eINT: {
//       Int *n = static_cast<Int *>(in);
//       out = create<Int>(n->getValue(), n->getSize());
//       break;
//     }
//     case IR_eFLOAT: {
//       Float *n = static_cast<Float *>(in);
//       out = create<Float>(n->getValue(), n->getSize());
//       break;
//     }
//     case IR_eLIST: {
//       List *n = static_cast<List *>(in);
//       ListItems items;
//       items.reserve(n->size());
//       for (const auto &item : *n) {
//         items.push_back(clone(item));
//       }
//       out = create<List>(std::move(items), n->isHomogenous());
//       break;
//     }
//     case IR_eCALL: {
//       Call *n = static_cast<Call *>(in);
//       CallArgs args;
//       for (auto arg : n->getArgs()) {
//         args.push_back(clone(arg));
//       }
//       out = create<Call>(clone(n->getTarget()), std::move(args));
//       break;
//     }
//     case IR_eSEQ: {
//       SeqItems items;
//       items.reserve(static_cast<Seq *>(in)->getItems().size());
//       for (auto item : static_cast<Seq *>(in)->getItems()) {
//         items.push_back(clone(item));
//       }
//       out = create<Seq>(std::move(items));
//       break;
//     }
//     case IR_eINDEX: {
//       Index *n = static_cast<Index *>(in);
//       out = create<Index>(clone(n->getExpr()), clone(n->getIndex()));
//       break;
//     }
//     case IR_eIDENT: {
//       Expr *bad_tmp_ref = static_cast<Ident *>(in)->getWhat();
//       out = create<Ident>(static_cast<Ident *>(in)->getName(), bad_tmp_ref);
//       break;
//     }
//     case IR_eEXTERN: {
//       Extern *n = static_cast<Extern *>(in);
//       out = create<Extern>(clone(n->getValue()), n->getAbiName());
//       break;
//     }
//     case IR_eLOCAL: {
//       Local *n = static_cast<Local *>(in);
//       out = create<Local>(n->getName(), clone(n->getValue()),
//       n->getAbiTag()); break;
//     }
//     case IR_eRET: {
//       out = create<Ret>(clone(static_cast<Ret *>(in)->getExpr()));
//       break;
//     }
//     case IR_eBRK: {
//       out = create<Brk>();
//       break;
//     }
//     case IR_eSKIP: {
//       out = create<Cont>();
//       break;
//     }
//     case IR_eIF: {
//       If *n = static_cast<If *>(in);
//       out = create<If>(clone(n->getCond()), clone(n->getThen()),
//                        clone(n->getElse()));
//       break;
//     }
//     case IR_eWHILE: {
//       While *n = static_cast<While *>(in);
//       out = create<While>(clone(n->getCond()),
//       clone(n->getBody())->as<Seq>()); break;
//     }
//     case IR_eFOR: {
//       For *n = static_cast<For *>(in);
//       out = create<For>(clone(n->getInit()), clone(n->getCond()),
//                         clone(n->getStep()), clone(n->getBody()));
//       break;
//     }
//     case IR_eCASE: {
//       Case *n = static_cast<Case *>(in);
//       out = create<Case>(clone(n->getCond()), clone(n->getBody()));
//       break;
//     }
//     case IR_eSWITCH: {
//       Switch *n = static_cast<Switch *>(in);
//       SwitchCases cases;
//       cases.reserve(n->getCases().size());
//       for (auto item : n->getCases()) {
//         cases.push_back(clone(item)->as<Case>());
//       }
//       out = create<Switch>(clone(n->getCond()), std::move(cases),
//                            clone(n->getDefault()));
//       break;
//     }
//     case IR_eFUNCTION: {
//       Fn *n = static_cast<Fn *>(in);
//       Params params;
//       params.reserve(n->getParams().size());
//       for (auto param : n->getParams()) {
//         params.push_back({clone(param.first)->asType(), param.second});
//       }

//       std::optional<Seq *> body;
//       if (n->getBody().has_value()) {
//         body = clone(n->getBody().value())->as<Seq>();
//       }

//       out = create<Fn>(n->getName(), std::move(params),
//                        clone(n->getReturn())->asType(), body,
//                        n->isVariadic(), n->getAbiTag());
//       break;
//     }
//     case IR_eASM: {
//       out = create<Asm>();
//       break;
//     }
//     case IR_eIGN: {
//       out = createIgn();
//       break;
//     }
//     case IR_tU1: {
//       out = create<U1Ty>();
//       break;
//     }
//     case IR_tU8: {
//       out = create<U8Ty>();
//       break;
//     }
//     case IR_tU16: {
//       out = create<U16Ty>();
//       break;
//     }
//     case IR_tU32: {
//       out = create<U32Ty>();
//       break;
//     }
//     case IR_tU64: {
//       out = create<U64Ty>();
//       break;
//     }
//     case IR_tU128: {
//       out = create<U128Ty>();
//       break;
//     }
//     case IR_tI8: {
//       out = create<I8Ty>();
//       break;
//     }
//     case IR_tI16: {
//       out = create<I16Ty>();
//       break;
//     }
//     case IR_tI32: {
//       out = create<I32Ty>();
//       break;
//     }
//     case IR_tI64: {
//       out = create<I64Ty>();
//       break;
//     }
//     case IR_tI128: {
//       out = create<I128Ty>();
//       break;
//     }
//     case IR_tF16_TY: {
//       out = create<F16Ty>();
//       break;
//     }
//     case IR_tF32_TY: {
//       out = create<F32Ty>();
//       break;
//     }
//     case IR_tF64_TY: {
//       out = create<F64Ty>();
//       break;
//     }
//     case IR_tF128_TY: {
//       out = create<F128Ty>();
//       break;
//     }
//     case IR_tVOID: {
//       out = create<VoidTy>();
//       break;
//     }
//     case IR_tPTR: {
//       PtrTy *n = static_cast<PtrTy *>(in);
//       out = create<PtrTy>(clone(n->getPointee())->asType());
//       break;
//     }
//     case IR_tCONST: {
//       ConstTy *n = static_cast<ConstTy *>(in);
//       out = create<ConstTy>(clone(n->getItem())->asType());
//       break;
//     }
//     case IR_tOPAQUE: {
//       OpaqueTy *n = static_cast<OpaqueTy *>(in);
//       out = create<OpaqueTy>(n->getName());
//       break;
//     }
//     case IR_tSTRUCT: {
//       StructFields fields;
//       fields.reserve(static_cast<StructTy *>(in)->getFields().size());
//       for (auto field : static_cast<StructTy *>(in)->getFields()) {
//         fields.push_back(clone(field)->asType());
//       }
//       out = create<StructTy>(std::move(fields));
//       break;
//     }
//     case IR_tUNION: {
//       UnionFields fields;
//       fields.reserve(static_cast<UnionTy *>(in)->getFields().size());
//       for (auto field : static_cast<UnionTy *>(in)->getFields()) {
//         fields.push_back(clone(field)->asType());
//       }
//       out = create<UnionTy>(std::move(fields));
//       break;
//     }
//     case IR_tARRAY: {
//       ArrayTy *n = static_cast<ArrayTy *>(in);
//       out = create<ArrayTy>(clone(n->getElement())->asType(), n->getCount());
//       break;
//     }
//     case IR_tFUNC: {
//       FnTy *n = static_cast<FnTy *>(in);
//       FnParams params;
//       params.reserve(n->getParams().size());
//       for (auto param : n->getParams()) {
//         params.push_back(clone(param)->asType());
//       }
//       out = create<FnTy>(std::move(params), clone(n->getReturn())->asType(),
//                          n->getAttrs());
//       break;
//     }
//     case IR_tTMP: {
//       Tmp *n = static_cast<Tmp *>(in);
//       switch (n->getTmpType()) {
//         case TmpType::NAMED_TYPE: {
//           std::string_view type_name =
//           std::get<std::string_view>(n->getData()); out =
//           create<Tmp>(TmpType::NAMED_TYPE, type_name); break;
//         }
//         case TmpType::DEFAULT_VALUE: {
//           std::string_view type_name =
//           std::get<std::string_view>(n->getData()); out =
//           create<Tmp>(TmpType::DEFAULT_VALUE, type_name); break;
//         }
//         case TmpType::CALL: {
//           const auto &data = std::get<CallArgsTmpNodeCradle>(n->getData());
//           CallArguments args;
//           for (auto arg : data.args) {
//             args.push_back({arg.first, clone(arg.second)});
//           }
//           Expr *base = clone(data.base);
//           out = create<Tmp>(TmpType::CALL, CallArgsTmpNodeCradle{base,
//           args}); break;
//         }
//       }
//       break;
//     }
//   }

//   map[in] = out.value();
//   in_visited.insert(in);

//   return out.value();
// }

// CPP_EXPORT Expr *Expr::cloneImpl() const {
//   std::unordered_map<const Expr *, Expr *> map;
//   std::unordered_set<Expr *> in_visited;

//   auto out = IRCloneRecurse(this, map, in_visited);

//   { /* Resolve Directed Acyclic* Graph Internal References */
//     using namespace ncc::ir;

//     Expr *out_expr = static_cast<Expr *>(out);
//     iterate<dfs_pre>(out_expr, [&](Expr *, Expr **_cur) -> IterOp {
//       Expr *cur = *_cur;

//       if (in_visited.contains(cur)) {
//         *_cur = static_cast<Expr *>(map[cur]);
//       }

//       if (cur->is(IR_eCALL)) [[unlikely]] {
//         Call *n = cur->as<Call>();
//         if (in_visited.contains(n->getTarget())) {
//           n->setTarget(static_cast<Expr *>(map[n->getTarget()]));
//         }
//       } else if (cur->is(IR_eIDENT)) [[unlikely]] {
//         Ident *n = cur->as<Ident>();
//         if (in_visited.contains(n->getWhat())) {
//           n->setWhat(static_cast<Expr *>(map[n->getWhat()]));
//         }
//       }

//       return IterOp::Proceed;
//     });
//     out = out_expr;
//   }

//   return out;
// }
