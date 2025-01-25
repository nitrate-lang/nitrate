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
#include <variant>

using namespace ncc;
using namespace ncc::ir;

class GetNodeChildren final : public IRVisitor<GetNodeChildren> {
  using A = GetNodeChildren;

  std::vector<FlowPtr<Expr>*>& m_children;

  template <class T>
  constexpr void Add(FlowPtr<T>& n) {
    auto unsafe_casted_ptr = reinterpret_cast<FlowPtr<Expr>*>(&n);
    m_children.push_back(unsafe_casted_ptr);
  }

  template <class T>
  constexpr void Add(NullableFlowPtr<T>& n) {
    if (!n.has_value()) {
      return;
    }

    auto unsafe_casted_ptr = reinterpret_cast<FlowPtr<Expr>*>(&n.value());
    m_children.push_back(unsafe_casted_ptr);
  }

public:
  GetNodeChildren(std::vector<FlowPtr<Expr>*>& children)
      : m_children(children) {}
  virtual ~GetNodeChildren() = default;

  void Visit(FlowPtr<GenericExpr<A>>) override {}

  void Visit(FlowPtr<GenericBinaryExpression<A>> n) override {
    Add(n->m_lhs);
    Add(n->m_rhs);
  }

  void Visit(FlowPtr<GenericUnary<A>> n) override { Add(n->m_expr); }

  void Visit(FlowPtr<GenericU1Ty<A>>) override {}
  void Visit(FlowPtr<GenericU8Ty<A>>) override {}
  void Visit(FlowPtr<GenericU16Ty<A>>) override {}
  void Visit(FlowPtr<GenericU32Ty<A>>) override {}
  void Visit(FlowPtr<GenericU64Ty<A>>) override {}
  void Visit(FlowPtr<GenericU128Ty<A>>) override {}
  void Visit(FlowPtr<GenericI8Ty<A>>) override {}
  void Visit(FlowPtr<GenericI16Ty<A>>) override {}
  void Visit(FlowPtr<GenericI32Ty<A>>) override {}
  void Visit(FlowPtr<GenericI64Ty<A>>) override {}
  void Visit(FlowPtr<GenericI128Ty<A>>) override {}
  void Visit(FlowPtr<GenericF16Ty<A>>) override {}
  void Visit(FlowPtr<GenericF32Ty<A>>) override {}
  void Visit(FlowPtr<GenericF64Ty<A>>) override {}
  void Visit(FlowPtr<GenericF128Ty<A>>) override {}
  void Visit(FlowPtr<GenericVoidTy<A>>) override {}

  void Visit(FlowPtr<GenericPtrTy<A>> n) override { Add(n->m_pointee); }

  void Visit(FlowPtr<GenericConstTy<A>> n) override { Add(n->m_item); }

  void Visit(FlowPtr<GenericOpaqueTy<A>>) override {}

  void Visit(FlowPtr<GenericStructTy<A>> n) override {
    std::for_each(n->m_fields.begin(), n->m_fields.end(),
                  [&](auto& f) { Add(f); });
  }

  void Visit(FlowPtr<GenericUnionTy<A>> n) override {
    std::for_each(n->m_fields.begin(), n->m_fields.end(),
                  [&](auto& f) { Add(f); });
  }

  void Visit(FlowPtr<GenericArrayTy<A>> n) override { Add(n->m_element); }

  void Visit(FlowPtr<GenericFnTy<A>> n) override {
    std::for_each(n->m_params.begin(), n->m_params.end(),
                  [&](auto& p) { Add(p); });
    Add(n->m_return);
  }

  void Visit(FlowPtr<GenericInt<A>>) override {}
  void Visit(FlowPtr<GenericFloat<A>>) override {}

  void Visit(FlowPtr<GenericList<A>> n) override {
    std::for_each(n->m_items.begin(), n->m_items.end(),
                  [&](auto& i) { Add(i); });
  }

  void Visit(FlowPtr<GenericCall<A>> n) override {
    std::for_each(n->m_args.begin(), n->m_args.end(), [&](auto& a) { Add(a); });
  }

  void Visit(FlowPtr<GenericSeq<A>> n) override {
    std::for_each(n->m_items.begin(), n->m_items.end(),
                  [&](auto& i) { Add(i); });
  }

  void Visit(FlowPtr<GenericIndex<A>> n) override {
    Add(n->m_expr);
    Add(n->m_index);
  }

  void Visit(FlowPtr<GenericIdentifier<A>>) override {}

  void Visit(FlowPtr<GenericExtern<A>> n) override { Add(n->m_value); }

  void Visit(FlowPtr<GenericLocal<A>> n) override { Add(n->m_value); }

  void Visit(FlowPtr<GenericRet<A>> n) override { Add(n->m_expr); }

  void Visit(FlowPtr<GenericBrk<A>>) override {}
  void Visit(FlowPtr<GenericCont<A>>) override {}

  void Visit(FlowPtr<GenericIf<A>> n) override {
    Add(n->m_cond);
    Add(n->m_then);
    Add(n->m_else);
  }

  void Visit(FlowPtr<GenericWhile<A>> n) override {
    Add(n->m_cond);
    Add(n->m_body);
  }

  void Visit(FlowPtr<GenericFor<A>> n) override {
    Add(n->m_init);
    Add(n->m_cond);
    Add(n->m_step);
    Add(n->m_body);
  }

  void Visit(FlowPtr<GenericCase<A>> n) override {
    Add(n->m_cond);
    Add(n->m_body);
  }

  void Visit(FlowPtr<GenericSwitch<A>> n) override {
    Add(n->m_cond);
    Add(n->m_default);
    std::for_each(n->m_cases.begin(), n->m_cases.end(),
                  [&](auto& c) { Add(c); });
  }

  void Visit(FlowPtr<GenericFunction<A>> n) override {
    std::for_each(n->m_params.begin(), n->m_params.end(),
                  [&](auto& p) { Add(p.first); });
    Add(n->m_body);
    Add(n->m_return);
  }

  void Visit(FlowPtr<GenericAsm<A>>) override {}

  void Visit(FlowPtr<GenericTmp<A>> n) override {
    if (std::holds_alternative<GenericCallArgsTmpNodeCradle<A>>(n->GetData())) {
      auto cradle = std::get<GenericCallArgsTmpNodeCradle<A>>(n->GetData());
      Add(cradle.m_base);
      std::for_each(cradle.m_args.begin(), cradle.m_args.end(),
                    [&](auto& a) { Add(a.second); });
    } else if (std::holds_alternative<string>(n->GetData())) {
    } else {
      qcore_panic("GetNodeChildren: unknown TmpNodeCradle type");
    }
  }
};

static void GetChildrenSorted(FlowPtr<Expr> base, ChildSelect cs,
                              std::vector<FlowPtr<Expr>*>& children) {
  GetNodeChildren gnc(children);
  base.Accept(gnc);

  std::sort(children.begin(), children.end(), cs);
}

NCC_EXPORT void detail::DfsPreImpl(FlowPtr<Expr>* base, IterCallback cb,
                                   ChildSelect cs) {
  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto kSyncfn = [](auto n, auto cb, auto cs) {
    std::stack<std::pair<NullableFlowPtr<Expr>, FlowPtr<Expr>*>> s;
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
        GetChildrenSorted(*cur.second, cs, children);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          s.push({*cur.second, *it});
        }
      }
    }
  };

  kSyncfn(base, cb, cs);
}

NCC_EXPORT void detail::DfsPostImpl(FlowPtr<Expr>* base, IterCallback cb,
                                    ChildSelect cs) {
  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto kSyncfn = [](auto n, auto cb, auto cs) {
    std::stack<std::pair<NullableFlowPtr<Expr>, FlowPtr<Expr>*>> s;
    std::vector<FlowPtr<Expr>*> children;

    s.push({nullptr, n});

    while (!s.empty()) {
      auto cur = s.top();
      s.pop();

      GetChildrenSorted(*cur.second, cs, children);
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

  kSyncfn(base, cb, cs);
  cb(nullptr, base);
}

NCC_EXPORT void detail::BfsPreImpl(FlowPtr<Expr>* base, IterCallback cb,
                                   ChildSelect cs) {
  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto kSyncfn = [](auto n, auto cb, auto cs) {
    std::queue<std::pair<NullableFlowPtr<Expr>, FlowPtr<Expr>*>> s;
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
        GetChildrenSorted(*cur.second, cs, children);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          s.push({*cur.second, *it});
        }
      }
    }
  };

  kSyncfn(base, cb, cs);
}

NCC_EXPORT void detail::BfsPostImpl(FlowPtr<Expr>* base, IterCallback cb,
                                    ChildSelect cs) {
  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto kSyncfn = [](auto n, auto cb, auto cs) {
    std::queue<std::pair<NullableFlowPtr<Expr>, FlowPtr<Expr>*>> s;
    std::vector<FlowPtr<Expr>*> children;

    s.push({nullptr, n});

    while (!s.empty()) {
      auto cur = s.front();
      s.pop();

      GetChildrenSorted(*cur.second, cs, children);
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        s.push({*cur.second, *it});
      }

      switch (cb(cur.first, cur.second)) {
        case IterOp::Proceed:
          break;
        case IterOp::Abort:
          return;
        case IterOp::SkipChildren:
          qcore_panic("bfs_post_impl: IterOp::SkipChildren not supported");
          break;
      }
    }
  };

  kSyncfn(base, cb, cs);
}

NCC_EXPORT void detail::IterChildren(FlowPtr<Expr>* base, IterCallback cb,
                                     ChildSelect cs) {
  if (!cs) {
    cs = [](auto a, auto b) { return (uintptr_t)a < (uintptr_t)b; };
  }

  constexpr auto kSyncfn = [](auto n, auto cb, auto cs) {
    std::vector<FlowPtr<Expr>*> children;
    GetChildrenSorted(*n, cs, children);

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

  kSyncfn(base, cb, cs);
}
