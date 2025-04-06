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

#ifndef __NITRATE_AST_ALGORITHM_H__
#define __NITRATE_AST_ALGORITHM_H__

#include <functional>
#include <nitrate-parser/ASTBase.hh>

namespace ncc::parse {
  enum IterMode : uint8_t {
    dfs_pre,
    dfs_post,
    bfs_pre,
    bfs_post,
    children,
  };

  enum class IterOp : uint8_t {
    Proceed,
    Abort,
    SkipChildren,
  };

  using IterCallback = std::function<IterOp(NullableFlowPtr<Expr>, FlowPtr<Expr>)>;

  namespace detail {
    void DfsPreImpl(const FlowPtr<Expr> &base, const IterCallback &cb);
    void DfsPostImpl(const FlowPtr<Expr> &base, const IterCallback &cb);
    void BfsPreImpl(const FlowPtr<Expr> &base, const IterCallback &cb);
    void BfsPostImpl(const FlowPtr<Expr> &base, const IterCallback &cb);
    void IterChildren(const FlowPtr<Expr> &base, const IterCallback &cb);
  }  // namespace detail

  template <IterMode mode, typename T>
  void Traverse(FlowPtr<T> root, const IterCallback &cb) {
    if constexpr (mode == dfs_pre) {
      return detail::DfsPreImpl(root, cb);
    } else if constexpr (mode == dfs_post) {
      return detail::DfsPostImpl(root, cb);
    } else if constexpr (mode == bfs_pre) {
      return detail::BfsPreImpl(root, cb);
    } else if constexpr (mode == bfs_post) {
      return detail::BfsPostImpl(root, cb);
    } else if constexpr (mode == children) {
      return detail::IterChildren(root, cb);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  template <auto mode = dfs_pre>
  void ForEach(FlowPtr<Expr> v, const std::function<void(FlowPtr<Expr>)> &f) {
    Traverse<mode>(v, [&](auto, FlowPtr<Expr> c) -> IterOp {
      f(std::move(c));

      return IterOp::Proceed;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void ForEach(FlowPtr<Expr> v, std::function<void(FlowPtr<T>)> f) {
    Traverse<mode>(v, [&](auto, FlowPtr<Expr> c) -> IterOp {
      if (c->GetKind() != Expr::GetTypeCode<T>()) {
        return IterOp::Proceed;
      }

      f(c.As<T>());

      return IterOp::Proceed;
    });
  }
}  // namespace ncc::parse

#endif
