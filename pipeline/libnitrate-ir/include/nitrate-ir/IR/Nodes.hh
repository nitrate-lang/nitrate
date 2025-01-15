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

#ifndef __NITRATE_IR_GRAPH_H__
#define __NITRATE_IR_GRAPH_H__

#include <nitrate-ir/IR/Base.hh>
#include <nitrate-ir/IR/Expression.hh>
#include <nitrate-ir/IR/Type.hh>

namespace ncc::ir {
  FlowPtr<Expr> CreateIgn();

  template <typename T, typename... Args>
  static constexpr inline T *Create(Args &&...args) {
    /**
     * Create nodes and minimizes the number of allocations by reusing
     * immutable items.
     */

#define NORMAL_ALLOC(NAME)                                              \
  if constexpr (ty == NAME) {                                           \
    return new (Arena<T>().allocate(1)) T(std::forward<Args>(args)...); \
  }

    constexpr NrTyT ty = Expr::GetTypeCode<T>();

    NORMAL_ALLOC(IR_eBIN);
    NORMAL_ALLOC(IR_eUNARY);
    NORMAL_ALLOC(IR_eINT);
    NORMAL_ALLOC(IR_eFLOAT);
    NORMAL_ALLOC(IR_eLIST);
    NORMAL_ALLOC(IR_eCALL);
    NORMAL_ALLOC(IR_eSEQ);
    NORMAL_ALLOC(IR_eINDEX);
    NORMAL_ALLOC(IR_eIDENT);
    NORMAL_ALLOC(IR_eEXTERN);
    NORMAL_ALLOC(IR_eLOCAL);
    NORMAL_ALLOC(IR_eRET);
    NORMAL_ALLOC(IR_eBRK);
    NORMAL_ALLOC(IR_eSKIP);
    NORMAL_ALLOC(IR_eIF);
    NORMAL_ALLOC(IR_eWHILE);
    NORMAL_ALLOC(IR_eFOR);
    NORMAL_ALLOC(IR_eCASE);
    NORMAL_ALLOC(IR_eSWITCH);
    NORMAL_ALLOC(IR_eFUNCTION);
    NORMAL_ALLOC(IR_eASM);
    NORMAL_ALLOC(IR_eIGN);
    NORMAL_ALLOC(IR_tTMP);

#undef NORMAL_ALLOC
  }

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

  typedef std::function<IterOp(NullableFlowPtr<Expr> p, FlowPtr<Expr> *c)>
      IterCallback;
  typedef std::function<bool(FlowPtr<Expr> *a, FlowPtr<Expr> *b)> ChildSelect;

  namespace detail {
    void DfsPreImpl(FlowPtr<Expr> *base, IterCallback cb, ChildSelect cs);
    void DfsPostImpl(FlowPtr<Expr> *base, IterCallback cb, ChildSelect cs);
    void BfsPreImpl(FlowPtr<Expr> *base, IterCallback cb, ChildSelect cs);
    void BfsPostImpl(FlowPtr<Expr> *base, IterCallback cb, ChildSelect cs);
    void IterChildren(FlowPtr<Expr> *base, IterCallback cb, ChildSelect cs);
  }  // namespace detail

  template <IterMode mode, typename T>
  void Iterate(FlowPtr<T> &base, IterCallback cb, ChildSelect cs = nullptr) {
    if constexpr (mode == dfs_pre) {
      return detail::DfsPreImpl((FlowPtr<Expr> *)&base, cb, cs);
    } else if constexpr (mode == dfs_post) {
      return detail::DfsPostImpl((FlowPtr<Expr> *)&base, cb, cs);
    } else if constexpr (mode == bfs_pre) {
      return detail::BfsPreImpl((FlowPtr<Expr> *)&base, cb, cs);
    } else if constexpr (mode == bfs_post) {
      return detail::BfsPostImpl((FlowPtr<Expr> *)&base, cb, cs);
    } else if constexpr (mode == children) {
      return detail::IterChildren((FlowPtr<Expr> *)&base, cb, cs);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  // std::optional<FlowPtr<Expr>> comptime_impl(
  //     FlowPtr<Expr> x,
  //     std::optional<std::function<void(std::string_view)>> eprintn =
  //         std::nullopt);

  /** Add source debugging information to an IR node */
  template <typename T>
  static inline T *DebugInfo(T *n, uint32_t, uint32_t) {
    /// TODO: Store source location information
    return n;
  }

  template <auto mode = dfs_pre>
  void ForEach(FlowPtr<Expr> v,
                std::function<void(NrTyT, FlowPtr<Expr>)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      f((*c)->GetKind(), *c);

      return IterOp::Proceed;
    });
  }

  template <auto mode = dfs_pre>
  void Transform(FlowPtr<Expr> v,
                 std::function<bool(NrTyT, FlowPtr<Expr> *)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      return f((*c)->GetKind(), c) ? IterOp::Proceed : IterOp::Abort;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void ForEach(FlowPtr<Expr> v, std::function<void(FlowPtr<T>)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      if ((*c)->GetKind() != Expr::GetTypeCode<T>()) {
        return IterOp::Proceed;
      }

      f((*c)->template as<T>());

      return IterOp::Proceed;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void Transform(FlowPtr<Expr> v, std::function<bool(FlowPtr<T> *)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      if ((*c)->GetKind() != Expr::GetTypeCode<T>()) {
        return IterOp::Proceed;
      }

      return f(reinterpret_cast<FlowPtr<T> *>(c)) ? IterOp::Proceed
                                                  : IterOp::Abort;
    });
  }
}  // namespace ncc::ir

#endif  // __NITRATE_IR_GRAPH_H__
