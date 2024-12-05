

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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-ir/IR.h>
#include <nitrate-parser/Parser.h>

#include <core/Config.hh>
#include <cstring>
#include <nitrate-ir/IRGraph.hh>
#include <unordered_map>
#include <unordered_set>

using namespace nr;

nr_node_t *nr_clone_impl(
    const nr_node_t *_node,
    std::unordered_map<const nr_node_t *, nr_node_t *> &map,
    std::unordered_set<nr_node_t *> &in_visited) {
#define clone(X) static_cast<Expr *>(nr_clone_impl(X, map, in_visited))

  using namespace nr;

  Expr *in;
  Expr *out;

  if (!_node) {
    return nullptr;
  }

  in = static_cast<Expr *>(const_cast<nr_node_t *>(_node));
  out = nullptr;

  switch (in->getKind()) {
    case NR_NODE_BINEXPR: {
      BinExpr *n = static_cast<BinExpr *>(in);
      out = create<BinExpr>(clone(n->getLHS()), clone(n->getRHS()), n->getOp());
      break;
    }
    case NR_NODE_UNEXPR: {
      UnExpr *n = static_cast<UnExpr *>(in);
      out = create<UnExpr>(clone(n->getExpr()), n->getOp());
      break;
    }
    case NR_NODE_POST_UNEXPR: {
      PostUnExpr *n = static_cast<PostUnExpr *>(in);
      out = create<PostUnExpr>(clone(n->getExpr()), n->getOp());
      break;
    }
    case NR_NODE_INT: {
      Int *n = static_cast<Int *>(in);
      out = create<Int>(n->getValue(), n->getSize());
      break;
    }
    case NR_NODE_FLOAT: {
      Float *n = static_cast<Float *>(in);
      out = create<Float>(n->getValue(), n->getSize());
      break;
    }
    case NR_NODE_LIST: {
      List *n = static_cast<List *>(in);
      ListItems items;
      items.reserve(n->size());
      for (const auto &item : *n) {
        items.push_back(clone(item));
      }
      out = create<List>(std::move(items), n->isHomogenous());
      break;
    }
    case NR_NODE_CALL: {
      Call *n = static_cast<Call *>(in);
      CallArgs args;
      for (auto arg : n->getArgs()) {
        args.push_back(clone(arg));
      }
      out = create<Call>(n->getTarget(), std::move(args));
      break;
    }
    case NR_NODE_SEQ: {
      SeqItems items;
      items.reserve(static_cast<Seq *>(in)->getItems().size());
      for (auto item : static_cast<Seq *>(in)->getItems()) {
        items.push_back(clone(item));
      }
      out = create<Seq>(std::move(items));
      break;
    }
    case NR_NODE_INDEX: {
      Index *n = static_cast<Index *>(in);
      out = create<Index>(clone(n->getExpr()), clone(n->getIndex()));
      break;
    }
    case NR_NODE_IDENT: {
      nr::Expr *bad_tmp_ref = static_cast<Ident *>(in)->getWhat();
      out = create<Ident>(static_cast<Ident *>(in)->getName(), bad_tmp_ref);
      break;
    }
    case NR_NODE_EXTERN: {
      Extern *n = static_cast<Extern *>(in);
      out = create<Extern>(clone(n->getValue()), n->getAbiName());
      break;
    }
    case NR_NODE_LOCAL: {
      Local *n = static_cast<Local *>(in);
      out = create<Local>(n->getName(), clone(n->getValue()), n->getAbiTag());
      break;
    }
    case NR_NODE_RET: {
      out = create<Ret>(clone(static_cast<Ret *>(in)->getExpr()));
      break;
    }
    case NR_NODE_BRK: {
      out = create<Brk>();
      break;
    }
    case NR_NODE_CONT: {
      out = create<Cont>();
      break;
    }
    case NR_NODE_IF: {
      If *n = static_cast<If *>(in);
      out = create<If>(clone(n->getCond()), clone(n->getThen()),
                       clone(n->getElse()));
      break;
    }
    case NR_NODE_WHILE: {
      While *n = static_cast<While *>(in);
      out = create<While>(clone(n->getCond()), clone(n->getBody())->as<Seq>());
      break;
    }
    case NR_NODE_FOR: {
      For *n = static_cast<For *>(in);
      out = create<For>(clone(n->getInit()), clone(n->getCond()),
                        clone(n->getStep()), clone(n->getBody()));
      break;
    }
    case NR_NODE_CASE: {
      Case *n = static_cast<Case *>(in);
      out = create<Case>(clone(n->getCond()), clone(n->getBody()));
      break;
    }
    case NR_NODE_SWITCH: {
      Switch *n = static_cast<Switch *>(in);
      SwitchCases cases;
      cases.reserve(n->getCases().size());
      for (auto item : n->getCases()) {
        cases.push_back(clone(item)->as<Case>());
      }
      out = create<Switch>(clone(n->getCond()), std::move(cases),
                           clone(n->getDefault()));
      break;
    }
    case NR_NODE_FN: {
      Fn *n = static_cast<Fn *>(in);
      Params params;
      params.reserve(n->getParams().size());
      for (auto param : n->getParams()) {
        params.push_back({clone(param.first)->asType(), param.second});
      }

      std::optional<Seq *> body;
      if (n->getBody().has_value()) {
        body = clone(n->getBody().value())->as<Seq>();
      }

      out = create<Fn>(n->getName(), std::move(params),
                       clone(n->getReturn())->asType(), body, n->isVariadic(),
                       n->getAbiTag());
      break;
    }
    case NR_NODE_ASM: {
      out = create<Asm>();
      break;
    }
    case NR_NODE_IGN: {
      out = createIgn();
      break;
    }
    case NR_NODE_U1_TY: {
      out = create<U1Ty>();
      break;
    }
    case NR_NODE_U8_TY: {
      out = create<U8Ty>();
      break;
    }
    case NR_NODE_U16_TY: {
      out = create<U16Ty>();
      break;
    }
    case NR_NODE_U32_TY: {
      out = create<U32Ty>();
      break;
    }
    case NR_NODE_U64_TY: {
      out = create<U64Ty>();
      break;
    }
    case NR_NODE_U128_TY: {
      out = create<U128Ty>();
      break;
    }
    case NR_NODE_I8_TY: {
      out = create<I8Ty>();
      break;
    }
    case NR_NODE_I16_TY: {
      out = create<I16Ty>();
      break;
    }
    case NR_NODE_I32_TY: {
      out = create<I32Ty>();
      break;
    }
    case NR_NODE_I64_TY: {
      out = create<I64Ty>();
      break;
    }
    case NR_NODE_I128_TY: {
      out = create<I128Ty>();
      break;
    }
    case NR_NODE_F16_TY: {
      out = create<F16Ty>();
      break;
    }
    case NR_NODE_F32_TY: {
      out = create<F32Ty>();
      break;
    }
    case NR_NODE_F64_TY: {
      out = create<F64Ty>();
      break;
    }
    case NR_NODE_F128_TY: {
      out = create<F128Ty>();
      break;
    }
    case NR_NODE_VOID_TY: {
      out = create<VoidTy>();
      break;
    }
    case NR_NODE_PTR_TY: {
      PtrTy *n = static_cast<PtrTy *>(in);
      out = create<PtrTy>(clone(n->getPointee())->asType());
      break;
    }
    case NR_NODE_OPAQUE_TY: {
      OpaqueTy *n = static_cast<OpaqueTy *>(in);
      out = create<OpaqueTy>(n->getName());
      break;
    }
    case NR_NODE_STRUCT_TY: {
      StructFields fields;
      fields.reserve(static_cast<StructTy *>(in)->getFields().size());
      for (auto field : static_cast<StructTy *>(in)->getFields()) {
        fields.push_back(clone(field)->asType());
      }
      out = create<StructTy>(std::move(fields));
      break;
    }
    case NR_NODE_UNION_TY: {
      UnionFields fields;
      fields.reserve(static_cast<UnionTy *>(in)->getFields().size());
      for (auto field : static_cast<UnionTy *>(in)->getFields()) {
        fields.push_back(clone(field)->asType());
      }
      out = create<UnionTy>(std::move(fields));
      break;
    }
    case NR_NODE_ARRAY_TY: {
      ArrayTy *n = static_cast<ArrayTy *>(in);
      out = create<ArrayTy>(clone(n->getElement())->asType(), n->getCount());
      break;
    }
    case NR_NODE_FN_TY: {
      FnTy *n = static_cast<FnTy *>(in);
      FnParams params;
      params.reserve(n->getParams().size());
      for (auto param : n->getParams()) {
        params.push_back(clone(param)->asType());
      }
      out = create<FnTy>(std::move(params), clone(n->getReturn())->asType(),
                         n->getAttrs());
      break;
    }
    case NR_NODE_TMP: {
      Tmp *n = static_cast<Tmp *>(in);
      switch (n->getTmpType()) {
        case TmpType::NAMED_TYPE: {
          std::string_view type_name = std::get<std::string_view>(n->getData());
          out = create<Tmp>(TmpType::NAMED_TYPE, type_name);
          break;
        }
        case TmpType::DEFAULT_VALUE: {
          std::string_view type_name = std::get<std::string_view>(n->getData());
          out = create<Tmp>(TmpType::DEFAULT_VALUE, type_name);
          break;
        }
        case TmpType::CALL: {
          const auto &data = std::get<CallArgsTmpNodeCradle>(n->getData());
          CallArguments args;
          for (auto arg : data.args) {
            args.push_back({arg.first, clone(arg.second)});
          }
          Expr *base = clone(data.base);
          out = create<Tmp>(TmpType::CALL, CallArgsTmpNodeCradle{base, args});
          break;
        }
      }
      break;
    }
  }

  qcore_assert(out != nullptr, "nr_clone: failed to clone node");

  map[in] = out;
  in_visited.insert(in);

  return static_cast<nr_node_t *>(out);
}

C_EXPORT nr_node_t *nr_clone(const nr_node_t *node) {
  nr_node_t *out;

  if (!node) {
    return nullptr;
  }

  out = nullptr;

  std::unordered_map<const nr_node_t *, nr_node_t *> map;
  std::unordered_set<nr_node_t *> in_visited;
  out = nr_clone_impl(node, map, in_visited);

  { /* Resolve Directed Acyclic* Graph Internal References */
    using namespace nr;

    Expr *out_expr = static_cast<Expr *>(out);
    iterate<dfs_pre>(out_expr, [&](Expr *, Expr **_cur) -> IterOp {
      Expr *cur = *_cur;

      if (in_visited.contains(cur)) {
        *_cur = static_cast<Expr *>(map[cur]);
      }

      if (cur->is(NR_NODE_CALL)) [[unlikely]] {
        Call *n = cur->as<Call>();
        if (in_visited.contains(n->getTarget())) {
          n->setTarget(static_cast<Expr *>(map[n->getTarget()]));
        }
      } else if (cur->is(NR_NODE_IDENT)) [[unlikely]] {
        Ident *n = cur->as<Ident>();
        if (in_visited.contains(n->getWhat())) {
          n->setWhat(static_cast<Expr *>(map[n->getWhat()]));
        }
      }

      return IterOp::Proceed;
    });
    out = out_expr;
  }

  return static_cast<nr_node_t *>(out);
}
