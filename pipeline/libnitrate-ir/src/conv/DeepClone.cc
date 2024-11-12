

////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
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

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <nitrate-ir/IR.h>
#include <nitrate-parser/Parser.h>

#include <core/Config.hh>
#include <cstring>
#include <nitrate-ir/IRGraph.hh>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

using namespace nr::diag;

static std::string_view intern(std::string_view sv) { return nr::current->internString(sv); }

nr_node_t *nr_clone_impl(const nr_node_t *_node,
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
    case QIR_NODE_BINEXPR: {
      BinExpr *n = static_cast<BinExpr *>(in);
      out = create<BinExpr>(clone(n->getLHS()), clone(n->getRHS()), n->getOp());
      break;
    }
    case QIR_NODE_UNEXPR: {
      UnExpr *n = static_cast<UnExpr *>(in);
      out = create<UnExpr>(clone(n->getExpr()), n->getOp());
      break;
    }
    case QIR_NODE_POST_UNEXPR: {
      PostUnExpr *n = static_cast<PostUnExpr *>(in);
      out = create<PostUnExpr>(clone(n->getExpr()), n->getOp());
      break;
    }
    case QIR_NODE_INT: {
      Int *n = static_cast<Int *>(in);
      out = create<Int>(n->getValue());
      break;
    }
    case QIR_NODE_FLOAT: {
      Float *n = static_cast<Float *>(in);
      out = create<Float>(n->getValue(), n->getSize());
      break;
    }
    case QIR_NODE_LIST: {
      ListItems items;
      items.reserve(static_cast<List *>(in)->getItems().size());
      for (auto item : static_cast<List *>(in)->getItems()) {
        items.push_back(clone(item));
      }
      out = create<List>(std::move(items));
      break;
    }
    case QIR_NODE_CALL: {
      Call *n = static_cast<Call *>(in);
      CallArgs args;
      for (auto arg : n->getArgs()) {
        args.push_back(clone(arg));
      }
      out = create<Call>(clone(n->getTarget()), std::move(args));
      break;
    }
    case QIR_NODE_SEQ: {
      SeqItems items;
      items.reserve(static_cast<Seq *>(in)->getItems().size());
      for (auto item : static_cast<Seq *>(in)->getItems()) {
        items.push_back(clone(item));
      }
      out = create<Seq>(std::move(items));
      break;
    }
    case QIR_NODE_INDEX: {
      Index *n = static_cast<Index *>(in);
      out = create<Index>(clone(n->getExpr()), clone(n->getIndex()));
      break;
    }
    case QIR_NODE_IDENT: {
      nr::Expr *bad_tmp_ref = static_cast<Ident *>(in)->getWhat();
      out = create<Ident>(intern(static_cast<Ident *>(in)->getName()), bad_tmp_ref);
      break;
    }
    case QIR_NODE_EXTERN: {
      Extern *n = static_cast<Extern *>(in);
      out = create<Extern>(clone(n->getValue()), intern(n->getAbiName()));
      break;
    }
    case QIR_NODE_LOCAL: {
      Local *n = static_cast<Local *>(in);
      out = create<Local>(intern(n->getName()), clone(n->getValue()), n->getAbiTag());
      break;
    }
    case QIR_NODE_RET: {
      out = create<Ret>(clone(static_cast<Ret *>(in)->getExpr()));
      break;
    }
    case QIR_NODE_BRK: {
      out = create<Brk>();
      break;
    }
    case QIR_NODE_CONT: {
      out = create<Cont>();
      break;
    }
    case QIR_NODE_IF: {
      If *n = static_cast<If *>(in);
      out = create<If>(clone(n->getCond()), clone(n->getThen()), clone(n->getElse()));
      break;
    }
    case QIR_NODE_WHILE: {
      While *n = static_cast<While *>(in);
      out = create<While>(clone(n->getCond()), clone(n->getBody())->as<Seq>());
      break;
    }
    case QIR_NODE_FOR: {
      For *n = static_cast<For *>(in);
      out = create<For>(clone(n->getInit()), clone(n->getCond()), clone(n->getStep()),
                        clone(n->getBody()));
      break;
    }
    case QIR_NODE_FORM: {
      Form *n = static_cast<Form *>(in);
      out = create<Form>(intern(n->getIdxIdent()), intern(n->getValIdent()), clone(n->getMaxJobs()),
                         clone(n->getExpr()), clone(n->getBody())->as<Seq>());
      break;
    }
    case QIR_NODE_CASE: {
      Case *n = static_cast<Case *>(in);
      out = create<Case>(clone(n->getCond()), clone(n->getBody()));
      break;
    }
    case QIR_NODE_SWITCH: {
      Switch *n = static_cast<Switch *>(in);
      SwitchCases cases;
      cases.reserve(n->getCases().size());
      for (auto item : n->getCases()) {
        cases.push_back(clone(item)->as<Case>());
      }
      out = create<Switch>(clone(n->getCond()), std::move(cases), clone(n->getDefault()));
      break;
    }
    case QIR_NODE_FN: {
      Fn *n = static_cast<Fn *>(in);
      Params params;
      params.reserve(n->getParams().size());
      for (auto param : n->getParams()) {
        params.push_back({clone(param.first)->asType(), intern(param.second)});
      }

      out = create<Fn>(intern(n->getName()), std::move(params), clone(n->getReturn())->asType(),
                       clone(n->getBody())->as<Seq>(), n->isVariadic(), n->getAbiTag());
      break;
    }
    case QIR_NODE_ASM: {
      qcore_implement("QIR_NODE_ASM cloning");
      break;
    }
    case QIR_NODE_IGN: {
      out = createIgn();
      break;
    }
    case QIR_NODE_U1_TY: {
      out = create<U1Ty>();
      break;
    }
    case QIR_NODE_U8_TY: {
      out = create<U8Ty>();
      break;
    }
    case QIR_NODE_U16_TY: {
      out = create<U16Ty>();
      break;
    }
    case QIR_NODE_U32_TY: {
      out = create<U32Ty>();
      break;
    }
    case QIR_NODE_U64_TY: {
      out = create<U64Ty>();
      break;
    }
    case QIR_NODE_U128_TY: {
      out = create<U128Ty>();
      break;
    }
    case QIR_NODE_I8_TY: {
      out = create<I8Ty>();
      break;
    }
    case QIR_NODE_I16_TY: {
      out = create<I16Ty>();
      break;
    }
    case QIR_NODE_I32_TY: {
      out = create<I32Ty>();
      break;
    }
    case QIR_NODE_I64_TY: {
      out = create<I64Ty>();
      break;
    }
    case QIR_NODE_I128_TY: {
      out = create<I128Ty>();
      break;
    }
    case QIR_NODE_F16_TY: {
      out = create<F16Ty>();
      break;
    }
    case QIR_NODE_F32_TY: {
      out = create<F32Ty>();
      break;
    }
    case QIR_NODE_F64_TY: {
      out = create<F64Ty>();
      break;
    }
    case QIR_NODE_F128_TY: {
      out = create<F128Ty>();
      break;
    }
    case QIR_NODE_VOID_TY: {
      out = create<VoidTy>();
      break;
    }
    case QIR_NODE_PTR_TY: {
      PtrTy *n = static_cast<PtrTy *>(in);
      out = create<PtrTy>(clone(n->getPointee())->asType());
      break;
    }
    case QIR_NODE_OPAQUE_TY: {
      OpaqueTy *n = static_cast<OpaqueTy *>(in);
      out = create<OpaqueTy>(intern(n->getName()));
      break;
    }
    case QIR_NODE_STRUCT_TY: {
      StructFields fields;
      fields.reserve(static_cast<StructTy *>(in)->getFields().size());
      for (auto field : static_cast<StructTy *>(in)->getFields()) {
        fields.push_back(clone(field)->asType());
      }
      out = create<StructTy>(std::move(fields));
      break;
    }
    case QIR_NODE_UNION_TY: {
      UnionFields fields;
      fields.reserve(static_cast<UnionTy *>(in)->getFields().size());
      for (auto field : static_cast<UnionTy *>(in)->getFields()) {
        fields.push_back(clone(field)->asType());
      }
      out = create<UnionTy>(std::move(fields));
      break;
    }
    case QIR_NODE_ARRAY_TY: {
      ArrayTy *n = static_cast<ArrayTy *>(in);
      out = create<ArrayTy>(clone(n->getElement())->asType(), n->getCount());
      break;
    }
    case QIR_NODE_FN_TY: {
      FnTy *n = static_cast<FnTy *>(in);
      FnParams params;
      params.reserve(n->getParams().size());
      for (auto param : n->getParams()) {
        params.push_back(clone(param)->asType());
      }
      out = create<FnTy>(std::move(params), clone(n->getReturn())->asType(), n->getAttrs());
      break;
    }
    case QIR_NODE_TMP: {
      Tmp *n = static_cast<Tmp *>(in);
      out = create<Tmp>(n->getTmpType(), n->getData());
      break;
    }
  }

  qcore_assert(out != nullptr, "nr_clone: failed to clone node");

  out->setLocDangerous(in->getLoc());

  map[in] = out;
  in_visited.insert(in);

  return static_cast<nr_node_t *>(out);
}

LIB_EXPORT nr_node_t *nr_clone(qmodule_t *dst, const nr_node_t *node) {
  nr_node_t *out;

  if (!node) {
    return nullptr;
  }

  if (!dst) {
    dst = static_cast<const nr::Expr *>(node)->getModule();
  }

  std::swap(nr::nr_arena.get(), dst->getNodeArena());
  std::swap(nr::current, dst);

  out = nullptr;

  try {
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

        return IterOp::Proceed;
      });
      out = out_expr;
    }
  } catch (...) {
    return nullptr;
  }

  std::swap(nr::current, dst);
  std::swap(nr::nr_arena.get(), dst->getNodeArena());

  return static_cast<nr_node_t *>(out);
}
