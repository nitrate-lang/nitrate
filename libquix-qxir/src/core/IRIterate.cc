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
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

/**
 * WARNING: This code technically uses unspecified behavior in C++. It assumes that
 * adding the __QXIR_NODE_REFLECT_IMPL__ define (which will change the visibility of)
 * private fields will not change the overall memory layout of any of the used polymorphic
 * class types. On the bright side, if this assumption is wrong, the code will certainly
 * crash, so it should be easy to detect.
 */

/* TODO: Optimize the heck out of this code */

#define __QXIR_IMPL__
#define __QXIR_NODE_REFLECT_IMPL__  // Make private fields accessible
#include <core/LibMacro.h>
#include <quix-qxir/Node.h>

#include <algorithm>
#include <future>
#include <list>
#include <queue>
#include <stack>

namespace qxir::detail {
  std::vector<Expr **> get_children_sorted(Expr *base, ChildSelect cs) {
    std::vector<Expr **> children;

    if (!base) {
      return children;
    }

    switch (base->getKind()) {
      case QIR_NODE_BINEXPR: {
        children.push_back(&base->as<BinExpr>()->m_lhs);
        children.push_back(&base->as<BinExpr>()->m_rhs);
        break;
      }
      case QIR_NODE_UNEXPR: {
        children.push_back(&base->as<UnExpr>()->m_expr);
        break;
      }
      case QIR_NODE_POST_UNEXPR: {
        children.push_back(&base->as<PostUnExpr>()->m_expr);
        break;
      }
      case QIR_NODE_INT: {
        break;
      }
      case QIR_NODE_FLOAT: {
        break;
      }
      case QIR_NODE_STRING: {
        break;
      }
      case QIR_NODE_LIST: {
        for (Expr *&child : base->as<List>()->m_items) {
          children.push_back(&child);
        }
        break;
      }
      case QIR_NODE_ALLOC: {
        children.push_back(reinterpret_cast<Expr **>(&base->as<Alloc>()->m_type));
        break;
      }
      case QIR_NODE_DCALL: {
        for (Expr *&child : base->as<DirectCall>()->m_args) {
          children.push_back(&child);
        }
        break;
      }
      case QIR_NODE_SEQ: {
        for (Expr *&child : base->as<Seq>()->m_items) {
          children.push_back(&child);
        }
        break;
      }
      case QIR_NODE_ASYNC: {
        for (Expr *&child : base->as<Async>()->m_items) {
          children.push_back(&child);
        }
        break;
      }
      case QIR_NODE_INDEX: {
        children.push_back(&base->as<Index>()->m_expr);
        children.push_back(&base->as<Index>()->m_index);
        break;
      }
      case QIR_NODE_IDENT: {
        // Don't print the m_what
        break;
      }
      case QIR_NODE_EXPORT: {
        children.push_back(&base->as<Export>()->m_value);
        break;
      }
      case QIR_NODE_LOCAL: {
        children.push_back(&base->as<Local>()->m_value);
        break;
      }
      case QIR_NODE_RET: {
        children.push_back(&base->as<Ret>()->m_expr);
        break;
      }
      case QIR_NODE_BRK: {
        break;
      }
      case QIR_NODE_CONT: {
        break;
      }
      case QIR_NODE_IF: {
        children.push_back(&base->as<If>()->m_cond);
        children.push_back(&base->as<If>()->m_then);
        children.push_back(&base->as<If>()->m_else);
        break;
      }
      case QIR_NODE_WHILE: {
        children.push_back(&base->as<While>()->m_cond);
        children.push_back(&base->as<While>()->m_body);
        break;
      }
      case QIR_NODE_FOR: {
        children.push_back(&base->as<For>()->m_init);
        children.push_back(&base->as<For>()->m_cond);
        children.push_back(&base->as<For>()->m_step);
        children.push_back(&base->as<For>()->m_body);
        break;
      }
      case QIR_NODE_FORM: {
        children.push_back(&base->as<Form>()->m_maxjobs);
        children.push_back(&base->as<Form>()->m_expr);
        children.push_back(reinterpret_cast<Expr **>(&base->as<Form>()->m_body));
        break;
      }
      case QIR_NODE_FOREACH: {
        children.push_back(&base->as<Foreach>()->m_expr);
        children.push_back(reinterpret_cast<Expr **>(&base->as<Foreach>()->m_body));
        break;
      }
      case QIR_NODE_CASE: {
        children.push_back(&base->as<Case>()->m_cond);
        children.push_back(&base->as<Case>()->m_body);
        break;
      }
      case QIR_NODE_SWITCH: {
        children.push_back(&base->as<Switch>()->m_cond);
        for (Case *&child : base->as<Switch>()->m_cases) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        children.push_back(reinterpret_cast<Expr **>(&base->as<Switch>()->m_default));
        break;
      }
      case QIR_NODE_FN: {
        for (Type *&child : base->as<Fn>()->m_params) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        children.push_back(reinterpret_cast<Expr **>(&base->as<Fn>()->m_body));
        break;
      }
      case QIR_NODE_ASM: {
        qcore_implement("QIR_NODE_ASM get_children_sorted");
        break;
      }
      case QIR_NODE_U1_TY: {
        break;
      }
      case QIR_NODE_U8_TY: {
        break;
      }
      case QIR_NODE_U16_TY: {
        break;
      }
      case QIR_NODE_U32_TY: {
        break;
      }
      case QIR_NODE_U64_TY: {
        break;
      }
      case QIR_NODE_U128_TY: {
        break;
      }
      case QIR_NODE_I8_TY: {
        break;
      }
      case QIR_NODE_I16_TY: {
        break;
      }
      case QIR_NODE_I32_TY: {
        break;
      }
      case QIR_NODE_I64_TY: {
        break;
      }
      case QIR_NODE_I128_TY: {
        break;
      }
      case QIR_NODE_F16_TY: {
        break;
      }
      case QIR_NODE_F32_TY: {
        break;
      }
      case QIR_NODE_F64_TY: {
        break;
      }
      case QIR_NODE_F128_TY: {
        break;
      }
      case QIR_NODE_VOID_TY: {
        break;
      }
      case QIR_NODE_PTR_TY: {
        children.push_back(reinterpret_cast<Expr **>(&base->as<PtrTy>()->m_pointee));
        break;
      }
      case QIR_NODE_OPAQUE_TY: {
        break;
      }
      case QIR_NODE_STRING_TY: {
        break;
      }
      case QIR_NODE_STRUCT_TY: {
        for (Type *&child : base->as<StructTy>()->m_fields) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        break;
      }
      case QIR_NODE_UNION_TY: {
        for (Type *&child : base->as<UnionTy>()->m_fields) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        break;
      }
      case QIR_NODE_ARRAY_TY: {
        children.push_back(reinterpret_cast<Expr **>(&base->as<ArrayTy>()->m_element));
        children.push_back(&base->as<ArrayTy>()->m_size);
        break;
      }
      case QIR_NODE_LIST_TY: {
        children.push_back(reinterpret_cast<Expr **>(&base->as<ListTy>()->m_element));
        break;
      }
      case QIR_NODE_INTRIN_TY: {
        break;
      }
      case QIR_NODE_FN_TY: {
        for (Type *&child : base->as<FnTy>()->m_params) {
          children.push_back(reinterpret_cast<Expr **>(&child));
        }
        children.push_back(reinterpret_cast<Expr **>(&base->as<FnTy>()->m_return));
        break;
      }
      case QIR_NODE_TMP: {
        break;
      }
      case QIR_NODE_BAD: {
        break;
      }
    }

    std::sort(children.begin(), children.end(), cs);

    return children;
  }

  class IterAbort {
  public:
    IterAbort() = default;
  };

  CPP_EXPORT void dfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs, bool parallel) {
    qcore_assert(base != nullptr && cb != nullptr, "dfs_pre_impl: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
    }

    typedef std::function<void(Expr **, const IterCallback &, const ChildSelect &)> IterFn;

    switch (cb(nullptr, base)) {
      case IterOp::Proceed:
        break;
      case IterOp::Abort:
        return;
      case IterOp::SkipChildren:
        return;
    }

    const IterFn syncfn = [&syncfn](Expr **n, const IterCallback &cb, const ChildSelect &cs) {
      for (Expr **child : get_children_sorted(*n, cs)) {
        switch (cb(*n, child)) {
          case IterOp::Proceed:
            syncfn(child, cb, cs);
            break;
          case IterOp::Abort:
            throw IterAbort();
          case IterOp::SkipChildren:
            break;
        }
      }
    };

    const IterFn asyncfn = [&asyncfn](Expr **n, IterCallback cb, ChildSelect cs) {
      std::vector<Expr **> children = get_children_sorted(*n, cs);

      std::list<std::future<void>> futures;
      for (Expr **child : children) {
        futures.push_back(std::async(std::launch::async, [&asyncfn, cb, child, cs]() {
          switch (cb(*child, child)) {
            case IterOp::Proceed:
              asyncfn(child, cb, cs);
              break;
            case IterOp::Abort:
              /// WARNING: This will be non-deterministic
              throw IterAbort();
            case IterOp::SkipChildren:
              break;
          }
        }));
      }

      for (std::future<void> &f : futures) {
        f.get();
      }
    };

    try {
      if (parallel) {
        asyncfn(base, cb, cs);
      } else {
        syncfn(base, cb, cs);
      }
    } catch (IterAbort &) {
      return;
    }
  }

  CPP_EXPORT void dfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs, bool parallel) {
    qcore_assert(base != nullptr && cb != nullptr, "dfs_post_impl: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
    }

    typedef std::function<void(Expr **, const IterCallback &, const ChildSelect &)> IterFn;

    const IterFn syncfn = [&syncfn](Expr **n, const IterCallback &cb, const ChildSelect &cs) {
      for (Expr **child : get_children_sorted(*n, cs)) {
        syncfn(child, cb, cs);
        switch (cb(*n, child)) {
          case IterOp::Proceed:
            break;
          case IterOp::Abort:
            throw IterAbort();
          case IterOp::SkipChildren:
            qcore_panic("dfs_post_impl: SkipChildren not supported");
            break;
        }
      }
    };

    const IterFn asyncfn = [&asyncfn](Expr **n, IterCallback cb, ChildSelect cs) {
      std::vector<Expr **> children = get_children_sorted(*n, cs);

      std::list<std::future<void>> futures;
      for (Expr **child : children) {
        futures.push_back(std::async(std::launch::async, [&asyncfn, cb, child, cs]() {
          asyncfn(child, cb, cs);
          switch (cb(*child, child)) {
            case IterOp::Proceed:
              break;
            case IterOp::Abort:
              /// WARNING: This will be non-deterministic
              throw IterAbort();
            case IterOp::SkipChildren:
              qcore_panic("dfs_post_impl: SkipChildren not supported");
              break;
          }
        }));
      }

      for (std::future<void> &f : futures) {
        f.get();
      }
    };

    try {
      if (parallel) {
        asyncfn(base, cb, cs);
      } else {
        syncfn(base, cb, cs);
      }

      cb(nullptr, base);

    } catch (IterAbort &) {
      return;
    }
  }

  CPP_EXPORT void bfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs, bool parallel) {
    /// TODO: Verify that this is correct
    qcore_assert(base != nullptr && cb != nullptr, "bfs_pre_impl: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
    }

    typedef std::function<void(Expr **, const IterCallback &, const ChildSelect &)> IterFn;

    cb(nullptr, base);

    const IterFn syncfn = [](Expr **n, const IterCallback &cb, const ChildSelect &cs) {
      std::queue<Expr *> q;
      q.push(*n);

      while (!q.empty()) {
        Expr *cur = q.front();
        q.pop();

        for (Expr **child : get_children_sorted(cur, cs)) {
          switch (cb(cur, child)) {
            case IterOp::Proceed:
              q.push(*child);
              break;
            case IterOp::Abort:
              throw IterAbort();
            case IterOp::SkipChildren:
              break;
          }
        }
      }
    };

    const IterFn asyncfn = [](Expr **n, IterCallback cb, ChildSelect cs) {
      std::queue<Expr *> q;
      q.push(*n);

      while (!q.empty()) {
        Expr *cur = q.front();
        q.pop();

        std::vector<Expr **> children = get_children_sorted(cur, cs);

        std::list<std::future<void>> futures;
        for (Expr **child : children) {
          futures.push_back(std::async(std::launch::async, [cb, child, &q]() {
            switch (cb(*child, child)) {
              case IterOp::Proceed:
                q.push(*child);
                break;
              case IterOp::Abort:
                /// WARNING: This will be non-deterministic
                throw IterAbort();
              case IterOp::SkipChildren:
                break;
            }
          }));
        }

        for (std::future<void> &f : futures) {
          f.get();
        }
      }
    };

    try {
      if (parallel) {
        asyncfn(base, cb, cs);
      } else {
        syncfn(base, cb, cs);
      }
    } catch (IterAbort &) {
      return;
    }
  }

  CPP_EXPORT void bfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs, bool parallel) {
    /// TODO: Verify that this is correct
    qcore_assert(base != nullptr && cb != nullptr, "bfs_post_impl: base and cb must not be null");

    if (!cs) { /* Iterate in the order the children are stored in the classes */
      cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
    }

    typedef std::function<void(Expr **, const IterCallback &, const ChildSelect &)> IterFn;

    const IterFn syncfn = [](Expr **n, const IterCallback &cb, const ChildSelect &cs) {
      std::queue<std::pair<Expr *, Expr **>> q;
      q.push({nullptr, n});

      while (!q.empty()) {
        Expr **cur = q.front().second;
        q.pop();

        for (Expr **child : get_children_sorted(*cur, cs)) {
          q.push({*cur, child});
        }

        cb(q.front().first, cur);
      }
    };

    const IterFn asyncfn = [](Expr **n, IterCallback cb, ChildSelect cs) {
      std::queue<std::pair<Expr *, Expr **>> q;
      q.push({nullptr, n});

      while (!q.empty()) {
        Expr **cur = q.front().second;
        q.pop();

        std::vector<Expr **> children = get_children_sorted(*cur, cs);

        std::list<std::future<void>> futures;
        for (Expr **child : children) {
          q.push({*cur, child});
        }

        for (std::future<void> &f : futures) {
          f.get();
        }

        cb(q.front().first, cur);
      }
    };

    try {
      if (parallel) {
        asyncfn(base, cb, cs);
      } else {
        syncfn(base, cb, cs);
      }
    } catch (IterAbort &) {
      return;
    }
  }

}  // namespace qxir::detail