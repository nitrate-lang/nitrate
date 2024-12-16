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

#ifndef __NITRATE_AST_AST_H__
#define __NITRATE_AST_AST_H__

#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>

namespace npar {
  template <typename T, typename... Args>
  static inline T *make(Args &&...args) {
    T *new_obj = new (Arena<T>().allocate(1)) T(std::forward<Args>(args)...);

    //  /// TODO: Cache nodes

    return new_obj;
  }

  Stmt *mock_stmt(npar_ty_t expected);
  Expr *mock_expr(npar_ty_t expected);
  Type *mock_type();
}  // namespace npar

namespace npar {
  enum IterMode {
    dfs_pre,
    dfs_post,
    bfs_pre,
    bfs_post,
    children,
    inorder,
  };

  enum class IterOp {
    Proceed,
    Abort,
    SkipChildren,
  };

  typedef std::function<IterOp(Expr *p, Expr **c)> IterCallback;
  typedef std::function<bool(Expr **a, Expr **b)> ChildSelect;

  namespace detail {
    void dfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void dfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void bfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void bfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void iter_children(Expr **base, IterCallback cb, ChildSelect cs);
    void inorder_impl(Expr **base, IterCallback cb, ChildSelect cs);
  }  // namespace detail

  template <IterMode mode, typename T>
  void iterate(T *&base, IterCallback cb, ChildSelect cs = nullptr) {
    if constexpr (mode == dfs_pre) {
      return detail::dfs_pre_impl((Expr **)&base, cb, cs);
    } else if constexpr (mode == dfs_post) {
      return detail::dfs_post_impl((Expr **)&base, cb, cs);
    } else if constexpr (mode == bfs_pre) {
      return detail::bfs_pre_impl((Expr **)&base, cb, cs);
    } else if constexpr (mode == bfs_post) {
      return detail::bfs_post_impl((Expr **)&base, cb, cs);
    } else if constexpr (mode == children) {
      return detail::iter_children((Expr **)&base, cb, cs);
    } else if constexpr (mode == inorder) {
      return detail::inorder_impl((Expr **)&base, cb, cs);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  template <auto mode = dfs_pre>
  void for_each(const npar_node_t *const v,
                std::function<void(npar_ty_t, const npar_node_t *const)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      f((*c)->getKind(), *c);

      return IterOp::Abort;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void for_each(const npar_node_t *const v, std::function<void(const T *)> f) {
    iterate<mode>(v, [&](auto, const npar_node_t *const *const c) -> IterOp {
      if ((*c)->getKind() != npar_node_t::getTypeCode<T>()) {
        return IterOp::Proceed;
      }

      f((*c)->as<T>());

      return IterOp::Proceed;
    });
  }
}  // namespace npar

#endif
