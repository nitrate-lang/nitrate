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

#include <nitrate-ir/IRBase.hh>
#include <nitrate-ir/IRExpr.hh>
#include <nitrate-ir/IRType.hh>

namespace ncc::ir {
  Expr *createIgn();

  namespace mem {
    extern Brk static_IR_eBRK;
    extern Cont static_IR_eSKIP;
    extern Expr static_IR_eIGN;

  };  // namespace mem

  template <typename T, typename... Args>
  static constexpr inline T *create(Args &&...args) {
    /**
     * Create nodes and minimizes the number of allocations by reusing
     * immutable items.
     */

#define NORMAL_ALLOC(NAME)                                              \
  if constexpr (ty == NAME) {                                           \
    return new (Arena<T>().allocate(1)) T(std::forward<Args>(args)...); \
  }

    constexpr nr_ty_t ty = Expr::getTypeCode<T>();

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
    NORMAL_ALLOC(IR_tU1);
    NORMAL_ALLOC(IR_tU8);
    NORMAL_ALLOC(IR_tU16);
    NORMAL_ALLOC(IR_tU32);
    NORMAL_ALLOC(IR_tU64);
    NORMAL_ALLOC(IR_tU128);
    NORMAL_ALLOC(IR_tI8);
    NORMAL_ALLOC(IR_tI16);
    NORMAL_ALLOC(IR_tI32);
    NORMAL_ALLOC(IR_tI64);
    NORMAL_ALLOC(IR_tI128);
    NORMAL_ALLOC(IR_tF16_TY);
    NORMAL_ALLOC(IR_tF32_TY);
    NORMAL_ALLOC(IR_tF64_TY);
    NORMAL_ALLOC(IR_tF128_TY);
    NORMAL_ALLOC(IR_tVOID);
    NORMAL_ALLOC(IR_tPTR);
    NORMAL_ALLOC(IR_tCONST);
    NORMAL_ALLOC(IR_tOPAQUE);
    NORMAL_ALLOC(IR_tSTRUCT);
    NORMAL_ALLOC(IR_tUNION);
    NORMAL_ALLOC(IR_tARRAY);
    NORMAL_ALLOC(IR_tFUNC);
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

  typedef std::function<IterOp(Expr *p, Expr **c)> IterCallback;
  typedef std::function<bool(Expr **a, Expr **b)> ChildSelect;

  typedef std::function<IterOp(const Expr *const p, const Expr *const *const c)>
      ConstIterCallback;
  typedef std::function<bool(const Expr *const *const a,
                             const Expr *const *const b)>
      ConstChildSelect;

  namespace detail {
    void dfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void dfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void bfs_pre_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void bfs_post_impl(Expr **base, IterCallback cb, ChildSelect cs);
    void iter_children(Expr **base, IterCallback cb, ChildSelect cs);
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
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  template <IterMode mode, typename T>
  void iterate(const T *base, ConstIterCallback cb,
               ConstChildSelect cs = nullptr) {
    T *ref = const_cast<T *>(base);

    const auto const_cb = cb != nullptr ? [&](Expr *p, Expr **c) -> IterOp {
      return cb(static_cast<const Expr *const>(p),
                const_cast<const Expr *const *const>(c));
    }
    : IterCallback(nullptr);

    const auto const_cs = cs != nullptr ? [&](Expr **a, Expr **b) -> bool {
      return cs(const_cast<const Expr *const *const>(a),
                const_cast<const Expr *const *const>(b));
    }
    : ChildSelect(nullptr);

    if constexpr (mode == dfs_pre) {
      return detail::dfs_pre_impl((Expr **)&ref, const_cb, const_cs);
    } else if constexpr (mode == dfs_post) {
      return detail::dfs_post_impl((Expr **)&ref, const_cb, const_cs);
    } else if constexpr (mode == bfs_pre) {
      return detail::bfs_pre_impl((Expr **)&ref, const_cb, const_cs);
    } else if constexpr (mode == bfs_post) {
      return detail::bfs_pre_impl((Expr **)&ref, const_cb, const_cs);
    } else if constexpr (mode == children) {
      return detail::iter_children((Expr **)&ref, const_cb, const_cs);
    } else {
      static_assert(mode != mode, "Invalid iteration mode.");
    }
  }

  std::optional<Expr *> comptime_impl(
      Expr *x, std::optional<std::function<void(std::string_view)>> eprintn =
                   std::nullopt);
  /** Add source debugging information to an IR node */
  template <typename T>
  static inline T *debug_info(T *N, uint32_t line, uint32_t col) {
    /// TODO: Store source location information
    (void)line;
    (void)col;

    return N;
  }

  template <auto mode = dfs_pre>
  void for_each(const Expr *const v,
                std::function<void(nr_ty_t, const Expr *const)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      f((*c)->getKind(), *c);

      return IterOp::Abort;
    });
  }

  template <auto mode = dfs_pre>
  void transform(Expr *v, std::function<bool(nr_ty_t, Expr **)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      return f((*c)->getKind(), c) ? IterOp::Proceed : IterOp::Abort;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void for_each(const Expr *const v, std::function<void(const T *)> f) {
    iterate<mode>(v, [&](auto, const Expr *const *const c) -> IterOp {
      if ((*c)->getKind() != Expr::getTypeCode<T>()) {
        return IterOp::Proceed;
      }

      f((*c)->as<T>());

      return IterOp::Proceed;
    });
  }

  template <typename T, auto mode = dfs_pre>
  void transform(Expr *v, std::function<bool(T **)> f) {
    iterate<mode>(v, [&](auto, auto c) -> IterOp {
      if ((*c)->getKind() != Expr::getTypeCode<T>()) {
        return IterOp::Proceed;
      }

      return f(reinterpret_cast<T **>(c)) ? IterOp::Proceed : IterOp::Abort;
    });
  }
}  // namespace ncc::ir

#endif  // __NITRATE_IR_GRAPH_H__
