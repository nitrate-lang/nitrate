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
  constexpr void add(FlowPtr<T>& n) {
    auto unsafe_casted_ptr = reinterpret_cast<FlowPtr<Expr>*>(&n);
    m_children.push_back(unsafe_casted_ptr);
  }

  template <class T>
  constexpr void add(NullableFlowPtr<T>& n) {
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

  void visit(FlowPtr<IR_Vertex_Expr<A>>) override {}
  void visit(FlowPtr<IR_Vertex_Type<A>>) override {}

  void visit(FlowPtr<IR_Vertex_BinExpr<A>> n) override {
    add(n->m_lhs);
    add(n->m_rhs);
  }

  void visit(FlowPtr<IR_Vertex_Unary<A>> n) override { add(n->m_expr); }

  void visit(FlowPtr<IR_Vertex_U1Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_U8Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_U16Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_U32Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_U64Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_U128Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_I8Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_I16Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_I32Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_I64Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_I128Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_F16Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_F32Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_F64Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_F128Ty<A>>) override {}
  void visit(FlowPtr<IR_Vertex_VoidTy<A>>) override {}

  void visit(FlowPtr<IR_Vertex_PtrTy<A>> n) override { add(n->m_pointee); }

  void visit(FlowPtr<IR_Vertex_ConstTy<A>> n) override { add(n->m_item); }

  void visit(FlowPtr<IR_Vertex_OpaqueTy<A>>) override {}

  void visit(FlowPtr<IR_Vertex_StructTy<A>> n) override {
    std::for_each(n->m_fields.begin(), n->m_fields.end(),
                  [&](auto& f) { add(f); });
  }

  void visit(FlowPtr<IR_Vertex_UnionTy<A>> n) override {
    std::for_each(n->m_fields.begin(), n->m_fields.end(),
                  [&](auto& f) { add(f); });
  }

  void visit(FlowPtr<IR_Vertex_ArrayTy<A>> n) override { add(n->m_element); }

  void visit(FlowPtr<IR_Vertex_FnTy<A>> n) override {
    std::for_each(n->m_params.begin(), n->m_params.end(),
                  [&](auto& p) { add(p); });
    add(n->m_return);
  }

  void visit(FlowPtr<IR_Vertex_Int<A>>) override {}
  void visit(FlowPtr<IR_Vertex_Float<A>>) override {}

  void visit(FlowPtr<IR_Vertex_List<A>> n) override {
    std::for_each(n->m_items.begin(), n->m_items.end(),
                  [&](auto& i) { add(i); });
  }

  void visit(FlowPtr<IR_Vertex_Call<A>> n) override {
    std::for_each(n->m_args.begin(), n->m_args.end(), [&](auto& a) { add(a); });
  }

  void visit(FlowPtr<IR_Vertex_Seq<A>> n) override {
    std::for_each(n->m_items.begin(), n->m_items.end(),
                  [&](auto& i) { add(i); });
  }

  void visit(FlowPtr<IR_Vertex_Index<A>> n) override {
    add(n->m_expr);
    add(n->m_index);
  }

  void visit(FlowPtr<IR_Vertex_Ident<A>>) override {}

  void visit(FlowPtr<IR_Vertex_Extern<A>> n) override { add(n->m_value); }

  void visit(FlowPtr<IR_Vertex_Local<A>> n) override { add(n->m_value); }

  void visit(FlowPtr<IR_Vertex_Ret<A>> n) override { add(n->m_expr); }

  void visit(FlowPtr<IR_Vertex_Brk<A>>) override {}
  void visit(FlowPtr<IR_Vertex_Cont<A>>) override {}

  void visit(FlowPtr<IR_Vertex_If<A>> n) override {
    add(n->m_cond);
    add(n->m_then);
    add(n->m_else);
  }

  void visit(FlowPtr<IR_Vertex_While<A>> n) override {
    add(n->m_cond);
    add(n->m_body);
  }

  void visit(FlowPtr<IR_Vertex_For<A>> n) override {
    add(n->m_init);
    add(n->m_cond);
    add(n->m_step);
    add(n->m_body);
  }

  void visit(FlowPtr<IR_Vertex_Case<A>> n) override {
    add(n->m_cond);
    add(n->m_body);
  }

  void visit(FlowPtr<IR_Vertex_Switch<A>> n) override {
    add(n->m_cond);
    add(n->m_default);
    std::for_each(n->m_cases.begin(), n->m_cases.end(),
                  [&](auto& c) { add(c); });
  }

  void visit(FlowPtr<IR_Vertex_Function<A>> n) override {
    std::for_each(n->m_params.begin(), n->m_params.end(),
                  [&](auto& p) { add(p.first); });
    add(n->m_body);
    add(n->m_return);
  }

  void visit(FlowPtr<IR_Vertex_Asm<A>>) override {}

  void visit(FlowPtr<IR_Vertex_Tmp<A>> n) override {
    if (std::holds_alternative<IR_Vertex_CallArgsTmpNodeCradle<A>>(
            n->getData())) {
      auto cradle = std::get<IR_Vertex_CallArgsTmpNodeCradle<A>>(n->getData());
      add(cradle.base);
      std::for_each(cradle.args.begin(), cradle.args.end(),
                    [&](auto& a) { add(a.second); });
    } else if (std::holds_alternative<string>(n->getData())) {
    } else {
      qcore_panic("GetNodeChildren: unknown TmpNodeCradle type");
    }
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
