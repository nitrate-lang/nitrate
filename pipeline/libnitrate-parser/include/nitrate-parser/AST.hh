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
  constexpr static inline auto make(Args &&...args) {  // NOLINT
    return [&](std::source_location origin = std::source_location::current()) {
      FlowPtr<T> new_obj = MakeFlowPtr<T>(new (Arena<T>().allocate(1)) T(
          std::forward<Args>(args)...));  // NOLINT

      new_obj.SetTracking(origin);

      return new_obj;
    };
  }
}  // namespace ncc::parse

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

  using IterCallback =
      std::function<IterOp(NullableFlowPtr<Base>, FlowPtr<Base>)>;

  namespace detail {
    void DfsPreImpl(const FlowPtr<Base> &base, const IterCallback &cb);
    void DfsPostImpl(const FlowPtr<Base> &base, const IterCallback &cb);
    void BfsPreImpl(const FlowPtr<Base> &base, const IterCallback &cb);
    void BfsPostImpl(const FlowPtr<Base> &base, const IterCallback &cb);
    void IterChildren(const FlowPtr<Base> &base, const IterCallback &cb);
  }  // namespace detail

  template <IterMode mode, typename T>
  void iterate(FlowPtr<T> root, const IterCallback &cb) {  // NOLINT
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
  void for_each(FlowPtr<Base> v,  // NOLINT
                const std::function<void(npar_ty_t, FlowPtr<Base>)> &f) {
    iterate<mode>(v, [&](auto, const FlowPtr<Base> &c) -> IterOp {
      f(c->GetKind(), c);

      return IterOp::Proceed;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void for_each(FlowPtr<Base> v,  // NOLINT
                std::function<void(FlowPtr<T>)> f) {
    iterate<mode>(v, [&](auto, FlowPtr<Base> c) -> IterOp {
      if (c->GetKind() != Base::GetTypeCode<T>()) {
        return IterOp::Proceed;
      }

      f(c.As<T>());

      return IterOp::Proceed;
    });
  }
}  // namespace ncc::parse

#endif
