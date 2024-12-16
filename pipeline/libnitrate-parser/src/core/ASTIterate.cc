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
#include <nitrate-lexer/Lexer.h>

#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <queue>

using namespace npar;

static void get_children_sorted(Expr *base, ChildSelect cs,
                                std::vector<Expr **> &children)

{
  /// TODO: Implement get_children_sorted
  qcore_implement();
}

CPP_EXPORT void detail::dfs_pre_impl(Expr **base, IterCallback cb,
                                     ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_pre_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
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

CPP_EXPORT void detail::dfs_post_impl(Expr **base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_post_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
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

CPP_EXPORT void detail::bfs_pre_impl(Expr **base, IterCallback cb,
                                     ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_pre_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
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

CPP_EXPORT void detail::bfs_post_impl(Expr **base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_post_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
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

CPP_EXPORT void detail::iter_children(Expr **base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "iter_children: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Expr **a, Expr **b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
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
