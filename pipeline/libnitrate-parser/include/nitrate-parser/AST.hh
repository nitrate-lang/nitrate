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

#include <functional>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>

namespace ncc::parse {
  /* This function takes template variadic arguments and forwards them into
   * the constructor of type T. If compiled with debugging, the source location
   * of the original call site is saved for the purposes of data-flow analysis
   * and AST debugging.
   */
  template <typename T, typename... Args>
  constexpr static inline auto make(Args &&...args) {
    return [&](
#if NITRATE_AST_TRACKING
               std::source_location origin = std::source_location::current()
#endif
           ) {
      FlowPtr<T> new_obj = MakeFlowPtr<T>(new (Arena<T>().allocate(1))
                                              T(std::forward<Args>(args)...));

      /// TODO: Implement node cache

#if NITRATE_AST_TRACKING
      new_obj.set_origin(origin);
#endif

      return new_obj;
    };
  }

  FlowPtr<Stmt> mock_stmt(std::optional<npar_ty_t> expected = std::nullopt);
  FlowPtr<Expr> mock_expr(std::optional<npar_ty_t> expected = std::nullopt);
  FlowPtr<Type> mock_type();
}  // namespace ncc::parse

namespace ncc::parse {
  enum IterMode {
    dfs_pre,
    dfs_post,
    bfs_pre,
    bfs_post,
    children,
  };

  enum class IterOp {
    Proceed,
    Abort,
    SkipChildren,
  };

  typedef std::function<IterOp(FlowPtr<Base> p, FlowPtr<Base> c)> IterCallback;

  namespace detail {
    void dfs_pre_impl(FlowPtr<Base> base, IterCallback cb);
    void dfs_post_impl(FlowPtr<Base> base, IterCallback cb);
    void bfs_pre_impl(FlowPtr<Base> base, IterCallback cb);
    void bfs_post_impl(FlowPtr<Base> base, IterCallback cb);
    void iter_children(FlowPtr<Base> base, IterCallback cb);
  }  // namespace detail

  template <IterMode mode, typename T>
  void iterate(FlowPtr<T> root, IterCallback cb) {
    if constexpr (mode == dfs_pre) {
      return detail::dfs_pre_impl(root, cb);
    } else if constexpr (mode == dfs_post) {
      return detail::dfs_post_impl(root, cb);
    } else if constexpr (mode == bfs_pre) {
      return detail::bfs_pre_impl(root, cb);
    } else if constexpr (mode == bfs_post) {
      return detail::bfs_post_impl(root, cb);
    } else if constexpr (mode == children) {
      return detail::iter_children(root, cb);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  template <auto mode = dfs_pre>
  void for_each(FlowPtr<Base> v,
                std::function<void(npar_ty_t, FlowPtr<Base>)> f) {
    iterate<mode>(v, [&](auto, FlowPtr<Base> c) -> IterOp {
      f(c->getKind(), c);

      return IterOp::Abort;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void for_each(FlowPtr<Base> v, std::function<void(FlowPtr<T>)> f) {
    iterate<mode>(v, [&](auto, FlowPtr<Base> c) -> IterOp {
      if (c->getKind() != Base::getTypeCode<T>()) {
        return IterOp::Proceed;
      }

      f(c.as<T>());

      return IterOp::Proceed;
    });
  }
}  // namespace ncc::parse

#endif
