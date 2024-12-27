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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <queue>
#include <stack>

using namespace ncc;
using namespace ncc::ir;

class GetNodeChildren final : public IRVisitor {
  std::vector<FlowPtr<Expr>*>& children;

public:
  GetNodeChildren(std::vector<FlowPtr<Expr>*>& children) : children(children) {}
  virtual ~GetNodeChildren() = default;

  void visit(FlowPtr<Expr> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;

    (void)children;
  }

  void visit(FlowPtr<Type> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<BinExpr> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Unary> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U1Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U8Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U16Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U32Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U64Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U128Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I8Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I16Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I32Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I64Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I128Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<F16Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<F32Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<F64Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<F128Ty> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<VoidTy> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<PtrTy> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<ConstTy> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<OpaqueTy> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<StructTy> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<UnionTy> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<ArrayTy> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<FnTy> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Int> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Float> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<List> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Call> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Seq> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Index> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Ident> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Extern> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Local> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Ret> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Brk> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Cont> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<If> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<While> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<For> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Case> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Switch> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Function> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Asm> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Tmp> n) override {
    /// TODO: Implement this function
    qcore_implement();
    (void)n;
  }
};

static void get_children_sorted(FlowPtr<Expr> base, ChildSelect cs,
                                std::vector<FlowPtr<Expr>*>& children) {
  GetNodeChildren gnc(children);
  base.accept(gnc);

  std::sort(children.begin(), children.end(), cs);
}

CPP_EXPORT void detail::dfs_pre_impl(FlowPtr<Expr>* base, IterCallback cb,
                                     ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_pre_impl: base and cb must not be null");

  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto syncfn = [](auto n, auto cb, auto cs) {
    std::stack<std::pair<FlowPtr<Expr>, FlowPtr<Expr>*>> s;
    std::vector<FlowPtr<Expr>*> children;

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

CPP_EXPORT void detail::dfs_post_impl(FlowPtr<Expr>* base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_post_impl: base and cb must not be null");

  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto syncfn = [](auto n, auto cb, auto cs) {
    std::stack<std::pair<FlowPtr<Expr>, FlowPtr<Expr>*>> s;
    std::vector<FlowPtr<Expr>*> children;

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

CPP_EXPORT void detail::bfs_pre_impl(FlowPtr<Expr>* base, IterCallback cb,
                                     ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_pre_impl: base and cb must not be null");

  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto syncfn = [](auto n, auto cb, auto cs) {
    std::queue<std::pair<FlowPtr<Expr>, FlowPtr<Expr>*>> s;
    std::vector<FlowPtr<Expr>*> children;

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

CPP_EXPORT void detail::bfs_post_impl(FlowPtr<Expr>* base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_post_impl: base and cb must not be null");

  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto syncfn = [](auto n, auto cb, auto cs) {
    std::queue<std::pair<FlowPtr<Expr>, FlowPtr<Expr>*>> s;
    std::vector<FlowPtr<Expr>*> children;

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

CPP_EXPORT void detail::iter_children(FlowPtr<Expr>* base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "iter_children: base and cb must not be null");

  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto syncfn = [](auto n, auto cb, auto cs) {
    std::vector<FlowPtr<Expr>*> children;
    get_children_sorted(*n, cs, children);

    for (FlowPtr<Expr>* child : children) {
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
