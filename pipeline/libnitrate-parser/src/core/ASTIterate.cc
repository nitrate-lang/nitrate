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
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <queue>
#include <ranges>

using namespace ncc;
using namespace ncc::parse;

class IterVisitor : public ASTVisitor {
  std::vector<FlowPtr<Base>>& m_sub;

  template <class T>
  constexpr void Add(FlowPtr<T> n) {
    if (n == nullptr) {
      return;
    }

    m_sub.push_back(n);
  }

  template <class T>
  constexpr void Add(NullableFlowPtr<T> n) {
    if (!n.has_value() || n == nullptr) {
      return;
    }

    m_sub.push_back(n.value());
  }

  void AddTypesuffix(const FlowPtr<Type>& n) {
    Add(n->get_width());
    Add(n->get_range_begin());
    Add(n->get_range_end());
  }

  void visit(FlowPtr<Base>) override {}
  void visit(FlowPtr<ExprStmt> n) override { Add(n->get_expr()); }
  void visit(FlowPtr<StmtExpr> n) override { Add(n->get_stmt()); }
  void visit(FlowPtr<TypeExpr> n) override { Add(n->get_type()); }
  void visit(FlowPtr<NamedTy> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<InferTy> n) override { AddTypesuffix(n); }

  void visit(FlowPtr<TemplType> n) override {
    Add(n->get_template());
    std::for_each(n->get_args().begin(), n->get_args().end(),
                  [&](auto arg) { Add(arg.second); });

    AddTypesuffix(n);
  }

  void visit(FlowPtr<U1> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<U8> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<U16> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<U32> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<U64> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<U128> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<I8> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<I16> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<I32> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<I64> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<I128> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<F16> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<F32> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<F64> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<F128> n) override { AddTypesuffix(n); }
  void visit(FlowPtr<VoidTy> n) override { AddTypesuffix(n); }

  void visit(FlowPtr<PtrTy> n) override {
    Add(n->get_item());
    AddTypesuffix(n);
  }

  void visit(FlowPtr<OpaqueTy> n) override { AddTypesuffix(n); }

  void visit(FlowPtr<TupleTy> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](auto item) { Add(item); });

    AddTypesuffix(n);
  }

  void visit(FlowPtr<ArrayTy> n) override {
    Add(n->get_item());
    Add(n->get_size());
    AddTypesuffix(n);
  }

  void visit(FlowPtr<RefTy> n) override {
    Add(n->get_item());
    AddTypesuffix(n);
  }

  void visit(FlowPtr<FuncTy> n) override {
    std::for_each(n->get_attributes().begin(), n->get_attributes().end(),
                  [&](auto attr) { Add(attr); });

    std::for_each(n->get_params().begin(), n->get_params().end(),
                  [&](auto param) {
                    Add(std::get<1>(param));
                    std::get<2>(param);
                  });

    Add(n->get_return());

    AddTypesuffix(n);
  }

  void visit(FlowPtr<UnaryExpr> n) override { Add(n->get_rhs()); }

  void visit(FlowPtr<BinExpr> n) override {
    Add(n->get_lhs());
    Add(n->get_rhs());
  }

  void visit(FlowPtr<PostUnaryExpr> n) override { Add(n->get_lhs()); }

  void visit(FlowPtr<TernaryExpr> n) override {
    Add(n->get_cond());
    Add(n->get_lhs());
    Add(n->get_rhs());
  }

  void visit(FlowPtr<ConstInt>) override {}
  void visit(FlowPtr<ConstFloat>) override {}
  void visit(FlowPtr<ConstBool>) override {}
  void visit(FlowPtr<ConstString>) override {}
  void visit(FlowPtr<ConstChar>) override {}
  void visit(FlowPtr<ConstNull>) override {}
  void visit(FlowPtr<ConstUndef>) override {}

  void visit(FlowPtr<Call> n) override {
    Add(n->get_func());
    std::for_each(n->get_args().begin(), n->get_args().end(),
                  [&](auto arg) { Add(arg.second); });
  }

  void visit(FlowPtr<TemplCall> n) override {
    Add(n->get_func());
    std::for_each(n->get_template_args().begin(), n->get_template_args().end(),
                  [&](auto arg) { Add(arg.second); });

    std::for_each(n->get_args().begin(), n->get_args().end(),
                  [&](auto arg) { Add(arg.second); });
  }

  void visit(FlowPtr<List> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](auto item) { Add(item); });
  }

  void visit(FlowPtr<Assoc> n) override {
    Add(n->get_key());
    Add(n->get_value());
  }

  void visit(FlowPtr<Index> n) override {
    Add(n->get_base());
    Add(n->get_index());
  }

  void visit(FlowPtr<Slice> n) override {
    Add(n->get_base());
    Add(n->get_start());
    Add(n->get_end());
  }

  void visit(FlowPtr<FString> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(), [&](auto arg) {
      if (std::holds_alternative<FlowPtr<Expr>>(arg)) {
        Add(std::get<FlowPtr<Expr>>(arg));
      } else if (std::holds_alternative<string>(arg)) {
      } else {
        qcore_implement();
      }
    });
  }

  void visit(FlowPtr<Ident>) override {}

  void visit(FlowPtr<SeqPoint> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](auto item) { Add(item); });
  }

  void visit(FlowPtr<Block> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](auto item) { Add(item); });
  }

  void visit(FlowPtr<VarDecl> n) override {
    std::for_each(n->get_attributes().begin(), n->get_attributes().end(),
                  [&](auto attr) { Add(attr); });

    Add(n->get_type());
    Add(n->get_value());
  }

  void visit(FlowPtr<InlineAsm> n) override {
    std::for_each(n->get_args().begin(), n->get_args().end(),
                  [&](auto arg) { Add(arg); });
  }

  void visit(FlowPtr<IfStmt> n) override {
    Add(n->get_cond());
    Add(n->get_then());
    Add(n->get_else());
  }

  void visit(FlowPtr<WhileStmt> n) override {
    Add(n->get_cond());
    Add(n->get_body());
  }

  void visit(FlowPtr<ForStmt> n) override {
    Add(n->get_init());
    Add(n->get_cond());
    Add(n->get_step());
    Add(n->get_body());
  }

  void visit(FlowPtr<ForeachStmt> n) override {
    Add(n->get_expr());
    Add(n->get_body());
  }

  void visit(FlowPtr<BreakStmt>) override {}
  void visit(FlowPtr<ContinueStmt>) override {}
  void visit(FlowPtr<ReturnStmt> n) override { Add(n->get_value()); }

  void visit(FlowPtr<ReturnIfStmt> n) override {
    Add(n->get_cond());
    Add(n->get_value());
  }

  void visit(FlowPtr<CaseStmt> n) override {
    Add(n->get_cond());
    Add(n->get_body());
  }

  void visit(FlowPtr<SwitchStmt> n) override {
    Add(n->get_cond());
    std::for_each(n->get_cases().begin(), n->get_cases().end(),
                  [&](auto c) { Add(c); });
    Add(n->get_default());
  }

  void visit(FlowPtr<TypedefStmt> n) override { Add(n->get_type()); }

  void visit(FlowPtr<Function> n) override {
    std::for_each(n->get_attributes().begin(), n->get_attributes().end(),
                  [&](auto attr) { Add(attr); });

    if (n->get_template_params()) {
      std::for_each(n->get_template_params()->begin(),
                    n->get_template_params()->end(), [&](auto param) {
                      Add(std::get<1>(param));
                      Add(std::get<2>(param));
                    });
    }

    std::for_each(n->get_params().begin(), n->get_params().end(),
                  [&](auto param) {
                    Add(std::get<1>(param));
                    std::get<2>(param);
                  });

    Add(n->get_return());
    Add(n->get_precond());
    Add(n->get_postcond());
    Add(n->get_body());
  }

  void visit(FlowPtr<StructDef> n) override {
    std::for_each(n->get_attributes().begin(), n->get_attributes().end(),
                  [&](auto attr) { Add(attr); });

    if (n->get_template_params()) {
      std::for_each(n->get_template_params()->begin(),
                    n->get_template_params()->end(), [&](auto param) {
                      Add(std::get<1>(param));
                      Add(std::get<2>(param));
                    });
    }

    std::for_each(n->get_fields().begin(), n->get_fields().end(),
                  [&](auto field) {
                    Add(field.get_type());
                    Add(field.get_value());
                  });

    std::for_each(n->get_methods().begin(), n->get_methods().end(),
                  [&](auto method) { Add(method.func); });

    std::for_each(n->get_static_methods().begin(),
                  n->get_static_methods().end(),
                  [&](auto method) { Add(method.func); });
  }

  void visit(FlowPtr<EnumDef> n) override {
    Add(n->get_type());

    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](auto item) { Add(item.second); });
  }

  void visit(FlowPtr<ScopeStmt> n) override { Add(n->get_body()); }

  void visit(FlowPtr<ExportStmt> n) override {
    std::for_each(n->get_attrs().begin(), n->get_attrs().end(),
                  [&](auto attr) { Add(attr); });

    Add(n->get_body());
  }

public:
  IterVisitor(std::vector<FlowPtr<Base>>& children) : m_sub(children) {}
};

static NCC_FORCE_INLINE void GetChildrenSorted(
    FlowPtr<Base> base, std::vector<FlowPtr<Base>>& children) {
  children.clear();

  if (!base) [[unlikely]] {
    return;
  }

  IterVisitor v(children);
  base.Accept(v);
}

NCC_EXPORT void detail::DfsPreImpl(const FlowPtr<Base>& base,
                                   const IterCallback& cb) {
  auto syncfn = [](const FlowPtr<Base>& n, const IterCallback& cb) {
    std::stack<std::pair<NullableFlowPtr<Base>, FlowPtr<Base>>> s;
    std::vector<FlowPtr<Base>> children;

    s.emplace(nullptr, n);

    while (!s.empty()) {
      auto cur = s.top();
      s.pop();

      bool skip = false;

      switch (cb(cur.first, cur.second)) {
        case IterOp::Proceed: {
          break;
        }

        case IterOp::Abort:
          [[unlikely]] { return; }

        case IterOp::SkipChildren: {
          skip = true;
          break;
        }
      }

      if (!skip) [[likely]] {
        GetChildrenSorted(cur.second, children);
        for (auto& it : std::ranges::reverse_view(children)) {
          s.emplace(cur.second, it);
        }
      }
    }
  };

  syncfn(base, cb);
}

NCC_EXPORT void detail::DfsPostImpl(const FlowPtr<Base>& base,
                                    const IterCallback& cb) {
  auto syncfn = [](const FlowPtr<Base>& n, const IterCallback& cb) {
    std::stack<std::pair<NullableFlowPtr<Base>, FlowPtr<Base>>> s;
    std::vector<FlowPtr<Base>> children;

    s.emplace(nullptr, n);

    while (!s.empty()) {
      auto cur = s.top();
      s.pop();

      GetChildrenSorted(cur.second, children);
      for (auto& it : std::ranges::reverse_view(children)) {
        s.emplace(cur.second, it);
      }

      switch (cb(cur.first, cur.second)) {
        case IterOp::Proceed: {
          break;
        }

        case IterOp::Abort:
          [[unlikely]] { return; }

        case IterOp::SkipChildren: {
          qcore_panic("dfs_post_impl: IterOp::SkipChildren not supported");
        }
      }
    }
  };

  syncfn(base, cb);
  cb(nullptr, base);
}

NCC_EXPORT void detail::BfsPreImpl(const FlowPtr<Base>& base,
                                   const IterCallback& cb) {
  auto syncfn = [](const FlowPtr<Base>& n, const IterCallback& cb) {
    std::queue<std::pair<NullableFlowPtr<Base>, FlowPtr<Base>>> s;
    std::vector<FlowPtr<Base>> children;

    s.emplace(nullptr, n);

    while (!s.empty()) {
      auto cur = s.front();
      s.pop();

      bool skip = false;

      switch (cb(cur.first, cur.second)) {
        case IterOp::Proceed: {
          break;
        }

        case IterOp::Abort:
          [[unlikely]] { return; }

        case IterOp::SkipChildren: {
          skip = true;
          break;
        }
      }

      if (!skip) [[likely]] {
        GetChildrenSorted(cur.second, children);
        for (auto& it : std::ranges::reverse_view(children)) {
          s.emplace(cur.second, it);
        }
      }
    }
  };

  syncfn(base, cb);
}

NCC_EXPORT void detail::BfsPostImpl(const FlowPtr<Base>& base,
                                    const IterCallback& cb) {
  auto syncfn = [](const FlowPtr<Base>& n, const IterCallback& cb) {
    std::queue<std::pair<NullableFlowPtr<Base>, FlowPtr<Base>>> s;
    std::vector<FlowPtr<Base>> children;

    s.emplace(nullptr, n);

    while (!s.empty()) {
      auto cur = s.front();
      s.pop();

      GetChildrenSorted(cur.second, children);
      for (auto& it : std::ranges::reverse_view(children)) {
        s.emplace(cur.second, it);
      }

      switch (cb(cur.first, cur.second)) {
        case IterOp::Proceed: {
          break;
        }

        case IterOp::Abort:
          [[unlikely]] { return; }

        case IterOp::SkipChildren: {
          qcore_panic("bfs_post_impl: IterOp::SkipChildren not supported");
        }
      }
    }
  };

  syncfn(base, cb);
}

NCC_EXPORT void detail::IterChildren(const FlowPtr<Base>& base,
                                     const IterCallback& cb) {
  auto syncfn = [](const FlowPtr<Base>& n, const IterCallback& cb) {
    std::vector<FlowPtr<Base>> children;
    GetChildrenSorted(n, children);

    for (const FlowPtr<Base>& child : children) {
      switch (cb(n, child)) {
        case IterOp::Proceed: {
          break;
        }

        case IterOp::Abort: {
          return;
        }

        case IterOp::SkipChildren: {
          return;
        }
      }
    }
  };

  syncfn(base, cb);
}
