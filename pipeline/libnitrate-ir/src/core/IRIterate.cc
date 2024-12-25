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

/**
 * WARNING: This code technically uses unspecified behavior in C++. It assumes
 * that adding the __IR_NODE_REFLECT_IMPL__ define, which will change the
 * visibility of private fields will not change the overall memory layout of any
 * of the used polymorphic class types. On the bright side, if this assumption
 * is wrong, the code will certainly crash on test cases, so it should be easy
 * to detect faults.
 */

#define __IR_NODE_REFLECT_IMPL__  // Make private fields accessible

#include <algorithm>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IRGraph.hh>
#include <queue>
#include <stack>

using namespace ncc::ir;

namespace ncc::ir::detail {
  void get_children_sorted(Expr *base, ChildSelect cs,
                           std::vector<Expr **> &children) {
    children.clear();

    if (!base) {
      return;
    }

    switch (base->getKind()) {
      case IR_eBIN: {
        children.push_back(&base->as<BinExpr>()->m_lhs);
        children.push_back(&base->as<BinExpr>()->m_rhs);
        break;
      }
      case IR_eUNARY: {
        children.push_back(&base->as<UnExpr>()->m_expr);
        break;
      }
      case IR_ePOST_UNEXPR: {
        children.push_back(&base->as<PostUnExpr>()->m_expr);
        break;
      }
      case IR_eINT: {
        break;
      }
      case IR_eFLOAT: {
        break;
      }
      case IR_eLIST: {
        children.reserve(base->as<List>()->m_items.size());
        for (Expr *&child : base->as<List>()->m_items) {
          children.push_back(&child);
        }
        break;
      }
      case IR_eCALL: {
        children.push_back(&base->as<Call>()->m_iref);
        children.reserve(base->as<Call>()->m_args.size());
        for (Expr *&child : base->as<Call>()->m_args) {
          children.push_back(&child);
        }
        break;
      }
      case IR_eSEQ: {
        children.reserve(base->as<Seq>()->m_items.size());
        for (Expr *&child : base->as<Seq>()->m_items) {
          children.push_back(&child);
        }
        break;
      }
      case IR_eINDEX: {
        children.push_back(&base->as<Index>()->m_expr);
        children.push_back(&base->as<Index>()->m_index);
        break;
      }
      case IR_eIDENT: {
        break;
      }
      case IR_eEXTERN: {
        children.push_back(&base->as<Extern>()->m_value);
        break;
      }
      case IR_eLOCAL: {
        children.push_back(&base->as<Local>()->m_value);
        break;
      }
      case IR_eRET: {
        children.push_back(&base->as<Ret>()->m_expr);
        break;
      }
      case IR_eBRK: {
        break;
      }
      case IR_eSKIP: {
        break;
      }
      case IR_eIF: {
        children.push_back(&base->as<If>()->m_cond);
        children.push_back(&base->as<If>()->m_then);
        children.push_back(&base->as<If>()->m_else);
        break;
      }
      case IR_eWHILE: {
        children.push_back(&base->as<While>()->m_cond);
        children.push_back(
            reinterpret_cast<Expr **>(&base->as<While>()->m_body));
        break;
      }
      case IR_eFOR: {
        children.push_back(&base->as<For>()->m_init);
        children.push_back(&base->as<For>()->m_cond);
        children.push_back(&base->as<For>()->m_step);
        children.push_back(&base->as<For>()->m_body);
        break;
      }
      case IR_eCASE: {
        children.push_back(&base->as<Case>()->m_cond);
        children.push_back(&base->as<Case>()->m_body);
        break;
      }
      case IR_eSWITCH: {
        children.push_back(&base->as<Switch>()->m_cond);
        children.reserve(base->as<Switch>()->m_cases.size() + 2);
        for (Case *&child : base->as<Switch>()->m_cases) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        children.push_back(
            reinterpret_cast<Expr **>(&base->as<Switch>()->m_default));
        break;
      }
      case IR_eFUNCTION: {
        children.reserve(base->as<Fn>()->m_params.size() + 1);
        for (auto &child : base->as<Fn>()->m_params) {
          children.push_back(reinterpret_cast<Expr **>(&child.first));
        }
        children.push_back(
            reinterpret_cast<Expr **>(&base->as<Fn>()->m_return));
        if (base->as<Fn>()->m_body.has_value()) {
          children.push_back(
              reinterpret_cast<Expr **>(&base->as<Fn>()->m_body.value()));
        }
        break;
      }
      case IR_eASM: {
        qcore_implement();
        break;
      }
      case IR_eIGN: {
        break;
      }
      case IR_tU1: {
        break;
      }
      case IR_tU8: {
        break;
      }
      case IR_tU16: {
        break;
      }
      case IR_tU32: {
        break;
      }
      case IR_tU64: {
        break;
      }
      case IR_tU128: {
        break;
      }
      case IR_tI8: {
        break;
      }
      case IR_tI16: {
        break;
      }
      case IR_tI32: {
        break;
      }
      case IR_tI64: {
        break;
      }
      case IR_tI128: {
        break;
      }
      case IR_tF16_TY: {
        break;
      }
      case IR_tF32_TY: {
        break;
      }
      case IR_tF64_TY: {
        break;
      }
      case IR_tF128_TY: {
        break;
      }
      case IR_tVOID: {
        break;
      }
      case IR_tPTR: {
        children.push_back(
            reinterpret_cast<Expr **>(&base->as<PtrTy>()->m_pointee));
        break;
      }
      case IR_tCONST: {
        children.push_back(
            reinterpret_cast<Expr **>(&base->as<ConstTy>()->m_item));
        break;
      }
      case IR_tOPAQUE: {
        break;
      }
      case IR_tSTRUCT: {
        children.reserve(base->as<StructTy>()->m_fields.size());
        for (Type *&child : base->as<StructTy>()->m_fields) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        break;
      }
      case IR_tUNION: {
        children.reserve(base->as<UnionTy>()->m_fields.size());
        for (Type *&child : base->as<UnionTy>()->m_fields) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        break;
      }
      case IR_tARRAY: {
        children.push_back(
            reinterpret_cast<Expr **>(&base->as<ArrayTy>()->m_element));
        break;
      }
      case IR_tFUNC: {
        children.reserve(base->as<FnTy>()->m_params.size() + 1);
        for (Type *&child : base->as<FnTy>()->m_params) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        children.push_back(
            reinterpret_cast<Expr **>(&base->as<FnTy>()->m_return));
        break;
      }
      case IR_tTMP: {
        Tmp *tmp = base->as<Tmp>();
        switch (tmp->getTmpType()) {
          case TmpType::NAMED_TYPE: {
            break;
          }

          case TmpType::DEFAULT_VALUE: {
            break;
          }

          case TmpType::CALL: {
            auto &data = get<CallArgsTmpNodeCradle>(tmp->getData());

            children.push_back(&data.base);
            for (auto &arg : data.args) {
              children.push_back(&arg.second);
            }
            break;
          }
        }
        break;
      }
    }

    std::sort(children.begin(), children.end(), cs);

    return;
  }

  CPP_EXPORT void dfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs) {
    qcore_assert(base != nullptr && cb != nullptr,
                 "dfs_pre_impl: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool {
        return (uintptr_t)a < (uintptr_t)b;
      };
    }

    const auto syncfn = [](Expr **n, const IterCallback &cb,
                           const ChildSelect &cs) {
      std::stack<std::pair<Expr *, Expr **>> s;
      std::vector<Expr **> children;

      s.push({nullptr, n});

      while (!s.empty()) {
        auto cur = s.top();
        s.pop();

        bool skip = false;

        switch (cb(cur.first, cur.second)) {
          case IterOp::Proceed:
            break;
          case IterOp::Abort:
            return;
          case IterOp::SkipChildren:
            skip = true;
            break;
        }

        if (!skip) {
          get_children_sorted(*cur.second, cs, children);
          for (auto it = children.rbegin(); it != children.rend(); ++it) {
            s.push({*cur.second, *it});
          }
        }
      }
    };

    syncfn(base, cb, cs);
  }

  CPP_EXPORT void dfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs) {
    qcore_assert(base != nullptr && cb != nullptr,
                 "dfs_post_impl: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool {
        return (uintptr_t)a < (uintptr_t)b;
      };
    }

    const auto syncfn = [](Expr **n, const IterCallback &cb,
                           const ChildSelect &cs) {
      std::stack<std::pair<Expr *, Expr **>> s;
      std::vector<Expr **> children;

      s.push({nullptr, n});

      while (!s.empty()) {
        auto cur = s.top();
        s.pop();

        get_children_sorted(*cur.second, cs, children);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          s.push({*cur.second, *it});
        }

        switch (cb(cur.first, cur.second)) {
          case IterOp::Proceed:
            break;
          case IterOp::Abort:
            return;
          case IterOp::SkipChildren:
            qcore_panic("dfs_post_impl: IterOp::SkipChildren not supported");
            break;
        }
      }
    };

    syncfn(base, cb, cs);
    cb(nullptr, base);
  }

  CPP_EXPORT void bfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs) {
    qcore_assert(base != nullptr && cb != nullptr,
                 "bfs_pre_impl: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool {
        return (uintptr_t)a < (uintptr_t)b;
      };
    }

    const auto syncfn = [](Expr **n, const IterCallback &cb,
                           const ChildSelect &cs) {
      std::queue<std::pair<Expr *, Expr **>> s;
      std::vector<Expr **> children;

      s.push({nullptr, n});

      while (!s.empty()) {
        auto cur = s.front();
        s.pop();

        bool skip = false;

        switch (cb(cur.first, cur.second)) {
          case IterOp::Proceed:
            break;
          case IterOp::Abort:
            return;
          case IterOp::SkipChildren:
            skip = true;
            break;
        }

        if (!skip) {
          get_children_sorted(*cur.second, cs, children);
          for (auto it = children.rbegin(); it != children.rend(); ++it) {
            s.push({*cur.second, *it});
          }
        }
      }
    };

    syncfn(base, cb, cs);
  }

  CPP_EXPORT void bfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs) {
    qcore_assert(base != nullptr && cb != nullptr,
                 "bfs_post_impl: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool {
        return (uintptr_t)a < (uintptr_t)b;
      };
    }

    const auto syncfn = [](Expr **n, const IterCallback &cb,
                           const ChildSelect &cs) {
      std::queue<std::pair<Expr *, Expr **>> s;
      std::vector<Expr **> children;

      s.push({nullptr, n});

      while (!s.empty()) {
        auto cur = s.front();
        s.pop();

        get_children_sorted(*cur.second, cs, children);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          s.push({*cur.second, *it});
        }

        switch (cb(cur.first, cur.second)) {
          case IterOp::Proceed:
            break;
          case IterOp::Abort:
            return;
          case IterOp::SkipChildren:
            qcore_assert(false,
                         "bfs_post_impl: IterOp::SkipChildren not supported");
            break;
        }
      }
    };

    syncfn(base, cb, cs);
  }

  CPP_EXPORT void iter_children(Expr **base, IterCallback cb, ChildSelect cs) {
    qcore_assert(base != nullptr && cb != nullptr,
                 "iter_children: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool {
        return (uintptr_t)a < (uintptr_t)b;
      };
    }

    const auto syncfn = [](Expr **n, const IterCallback &cb,
                           const ChildSelect &cs) {
      std::vector<Expr **> children;
      get_children_sorted(*n, cs, children);

      for (Expr **child : children) {
        switch (cb(*n, child)) {
          case IterOp::Proceed:
            break;
          case IterOp::Abort:
            return;
          case IterOp::SkipChildren:
            return;
        }
      }
    };

    syncfn(base, cb, cs);
  }
}  // namespace ncc::ir::detail
