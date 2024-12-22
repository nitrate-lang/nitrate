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

using namespace ncc::parse;

class IterVisitor : public ASTVisitor {
  std::vector<Base**>& sub;

  template <class T>
  void add(T const& n) {
    using NoPtr = std::remove_pointer_t<T>;
    static_assert(std::is_base_of_v<Base, NoPtr>,
                  "T must be derived from Base");

    if (n == nullptr) {
      return;
    }

    // Hopefully this isn't UB
    let ptr = reinterpret_cast<const Base* const*>(&n);
    sub.push_back(const_cast<Base**>(ptr));
  }

  void add_typesuffix(Type const& n) {
    add(n.get_width());
    add(n.get_range().first);
    add(n.get_range().second);
  }

  void visit(Base const&) override {}
  void visit(ExprStmt const& n) override { add(n.get_expr()); }
  void visit(StmtExpr const& n) override { add(n.get_stmt()); }
  void visit(TypeExpr const& n) override { add(n.get_type()); }
  void visit(NamedTy const& n) override { add_typesuffix(n); }
  void visit(InferTy const& n) override { add_typesuffix(n); }

  void visit(TemplType const& n) override {
    add(n.get_template());
    std::for_each(n.get_args().begin(), n.get_args().end(),
                  [&](let arg) { add(arg.second); });

    add_typesuffix(n);
  }

  void visit(U1 const& n) override { add_typesuffix(n); }
  void visit(U8 const& n) override { add_typesuffix(n); }
  void visit(U16 const& n) override { add_typesuffix(n); }
  void visit(U32 const& n) override { add_typesuffix(n); }
  void visit(U64 const& n) override { add_typesuffix(n); }
  void visit(U128 const& n) override { add_typesuffix(n); }
  void visit(I8 const& n) override { add_typesuffix(n); }
  void visit(I16 const& n) override { add_typesuffix(n); }
  void visit(I32 const& n) override { add_typesuffix(n); }
  void visit(I64 const& n) override { add_typesuffix(n); }
  void visit(I128 const& n) override { add_typesuffix(n); }
  void visit(F16 const& n) override { add_typesuffix(n); }
  void visit(F32 const& n) override { add_typesuffix(n); }
  void visit(F64 const& n) override { add_typesuffix(n); }
  void visit(F128 const& n) override { add_typesuffix(n); }
  void visit(VoidTy const& n) override { add_typesuffix(n); }

  void visit(PtrTy const& n) override {
    add(n.get_item());
    add_typesuffix(n);
  }

  void visit(OpaqueTy const& n) override { add_typesuffix(n); }

  void visit(TupleTy const& n) override {
    std::for_each(n.get_items().begin(), n.get_items().end(),
                  [&](let item) { add(item); });

    add_typesuffix(n);
  }

  void visit(ArrayTy const& n) override {
    add(n.get_item());
    add(n.get_size());
    add_typesuffix(n);
  }

  void visit(RefTy const& n) override {
    add(n.get_item());
    add_typesuffix(n);
  }

  void visit(FuncTy const& n) override {
    std::for_each(n.get_attributes().begin(), n.get_attributes().end(),
                  [&](let attr) { add(attr); });

    std::for_each(n.get_params().params.begin(), n.get_params().params.end(),
                  [&](let param) {
                    add(std::get<1>(param));
                    std::get<2>(param);
                  });

    add(n.get_return());

    add_typesuffix(n);
  }

  void visit(UnaryExpr const& n) override { add(n.get_rhs()); }

  void visit(BinExpr const& n) override {
    add(n.get_lhs());
    add(n.get_rhs());
  }

  void visit(PostUnaryExpr const& n) override { add(n.get_lhs()); }

  void visit(TernaryExpr const& n) override {
    add(n.get_cond());
    add(n.get_lhs());
    add(n.get_rhs());
  }

  void visit(ConstInt const&) override {}
  void visit(ConstFloat const&) override {}
  void visit(ConstBool const&) override {}
  void visit(ConstString const&) override {}
  void visit(ConstChar const&) override {}
  void visit(ConstNull const&) override {}
  void visit(ConstUndef const&) override {}

  void visit(Call const& n) override {
    add(n.get_func());
    std::for_each(n.get_args().begin(), n.get_args().end(),
                  [&](let arg) { add(arg.second); });
  }

  void visit(TemplCall const& n) override {
    add(n.get_func());
    std::for_each(n.get_template_args().begin(), n.get_template_args().end(),
                  [&](let arg) { add(arg.second); });

    std::for_each(n.get_args().begin(), n.get_args().end(),
                  [&](let arg) { add(arg.second); });
  }

  void visit(List const& n) override {
    std::for_each(n.get_items().begin(), n.get_items().end(),
                  [&](let item) { add(item); });
  }

  void visit(Assoc const& n) override {
    add(n.get_key());
    add(n.get_value());
  }

  void visit(Index const& n) override {
    add(n.get_base());
    add(n.get_index());
  }

  void visit(Slice const& n) override {
    add(n.get_base());
    add(n.get_start());
    add(n.get_end());
  }

  void visit(FString const& n) override {
    std::for_each(n.get_items().begin(), n.get_items().end(), [&](let arg) {
      if (std::holds_alternative<Expr*>(arg)) {
        add(std::get<Expr*>(arg));
      } else if (std::holds_alternative<ncc::core::str_alias>(arg)) {
      } else {
        qcore_implement();
      }
    });
  }

  void visit(Ident const&) override {}

  void visit(SeqPoint const& n) override {
    std::for_each(n.get_items().begin(), n.get_items().end(),
                  [&](let item) { add(item); });
  }

  void visit(Block const& n) override {
    std::for_each(n.get_items().begin(), n.get_items().end(),
                  [&](let item) { add(item); });
  }

  void visit(VarDecl const& n) override {
    std::for_each(n.get_attributes().begin(), n.get_attributes().end(),
                  [&](let attr) { add(attr); });

    add(n.get_type());
    add(n.get_value());
  }

  void visit(InlineAsm const& n) override {
    std::for_each(n.get_args().begin(), n.get_args().end(),
                  [&](let arg) { add(arg); });
  }

  void visit(IfStmt const& n) override {
    add(n.get_cond());
    add(n.get_then());
    add(n.get_else());
  }

  void visit(WhileStmt const& n) override {
    add(n.get_cond());
    add(n.get_body());
  }

  void visit(ForStmt const& n) override {
    add(n.get_init().value_or(nullptr));
    add(n.get_cond().value_or(nullptr));
    add(n.get_step().value_or(nullptr));
    add(n.get_body());
  }

  void visit(ForeachStmt const& n) override {
    add(n.get_expr());
    add(n.get_body());
  }

  void visit(BreakStmt const&) override {}
  void visit(ContinueStmt const&) override {}
  void visit(ReturnStmt const& n) override {
    add(n.get_value().value_or(nullptr));
  }

  void visit(ReturnIfStmt const& n) override {
    add(n.get_cond());
    add(n.get_value());
  }

  void visit(CaseStmt const& n) override {
    add(n.get_cond());
    add(n.get_body());
  }

  void visit(SwitchStmt const& n) override {
    add(n.get_cond());
    std::for_each(n.get_cases().begin(), n.get_cases().end(),
                  [&](let c) { add(c); });
    add(n.get_default());
  }

  void visit(TypedefStmt const& n) override { add(n.get_type()); }

  void visit(Function const& n) override {
    std::for_each(n.get_attributes().begin(), n.get_attributes().end(),
                  [&](let attr) { add(attr); });

    if (n.get_template_params()) {
      std::for_each(n.get_template_params()->begin(),
                    n.get_template_params()->end(), [&](let param) {
                      add(std::get<1>(param));
                      add(std::get<2>(param));
                    });
    }

    std::for_each(n.get_params().params.begin(), n.get_params().params.end(),
                  [&](let param) {
                    add(std::get<1>(param));
                    std::get<2>(param);
                  });

    add(n.get_return());
    add(n.get_precond().value_or(nullptr));
    add(n.get_postcond().value_or(nullptr));
    add(n.get_body().value_or(nullptr));
  }

  void visit(StructDef const& n) override {
    std::for_each(n.get_attributes().begin(), n.get_attributes().end(),
                  [&](let attr) { add(attr); });

    if (n.get_template_params()) {
      std::for_each(n.get_template_params()->begin(),
                    n.get_template_params()->end(), [&](let param) {
                      add(std::get<1>(param));
                      add(std::get<2>(param));
                    });
    }

    std::for_each(n.get_fields().begin(), n.get_fields().end(), [&](let field) {
      add(field.get_type());
      add(field.get_value().value_or(nullptr));
    });

    std::for_each(n.get_methods().begin(), n.get_methods().end(),
                  [&](let method) { add(method.func); });

    std::for_each(n.get_static_methods().begin(), n.get_static_methods().end(),
                  [&](let method) { add(method.func); });
  }

  void visit(EnumDef const& n) override {
    add(n.get_type());

    std::for_each(n.get_items().begin(), n.get_items().end(),
                  [&](let item) { add(item.second); });
  }

  void visit(ScopeStmt const& n) override { add(n.get_body()); }

  void visit(ExportStmt const& n) override {
    std::for_each(n.get_attrs().begin(), n.get_attrs().end(),
                  [&](let attr) { add(attr); });

    add(n.get_body());
  }

public:
  IterVisitor(std::vector<Base**>& children) : sub(children) {}
};

static FORCE_INLINE void get_children_sorted(Base* base, ChildSelect cs,
                                             std::vector<Base**>& children) {
  children.clear();

  if (!base) [[unlikely]] {
    return;
  }

  IterVisitor v(children);
  base->accept(v);

  std::sort(children.begin(), children.end(), cs);

  return;
}

CPP_EXPORT void detail::dfs_pre_impl(Base** base, IterCallback cb,
                                     ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_pre_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Base** a, Base** b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
  }

  const auto syncfn = [](Base** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::stack<std::pair<Base*, Base**>> s;
    std::vector<Base**> children;

    s.push({nullptr, n});

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
        get_children_sorted(*cur.second, cs, children);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          s.push({*cur.second, *it});
        }
      }
    }
  };

  syncfn(base, cb, cs);
}

CPP_EXPORT void detail::dfs_post_impl(Base** base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_post_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Base** a, Base** b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
  }

  const auto syncfn = [](Base** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::stack<std::pair<Base*, Base**>> s;
    std::vector<Base**> children;

    s.push({nullptr, n});

    while (!s.empty()) {
      auto cur = s.top();
      s.pop();

      get_children_sorted(*cur.second, cs, children);
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        s.push({*cur.second, *it});
      }

      switch (cb(cur.first, cur.second)) {
        case IterOp::Proceed: {
          break;
        }

        case IterOp::Abort:
          [[unlikely]] { return; }

        case IterOp::SkipChildren: {
          qcore_panic("dfs_post_impl: IterOp::SkipChildren not supported");
          break;
        }
      }
    }
  };

  syncfn(base, cb, cs);
  cb(nullptr, base);
}

CPP_EXPORT void detail::bfs_pre_impl(Base** base, IterCallback cb,
                                     ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_pre_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Base** a, Base** b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
  }

  const auto syncfn = [](Base** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::queue<std::pair<Base*, Base**>> s;
    std::vector<Base**> children;

    s.push({nullptr, n});

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
        get_children_sorted(*cur.second, cs, children);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          s.push({*cur.second, *it});
        }
      }
    }
  };

  syncfn(base, cb, cs);
}

CPP_EXPORT void detail::bfs_post_impl(Base** base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_post_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Base** a, Base** b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
  }

  const auto syncfn = [](Base** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::queue<std::pair<Base*, Base**>> s;
    std::vector<Base**> children;

    s.push({nullptr, n});

    while (!s.empty()) {
      auto cur = s.front();
      s.pop();

      get_children_sorted(*cur.second, cs, children);
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        s.push({*cur.second, *it});
      }

      switch (cb(cur.first, cur.second)) {
        case IterOp::Proceed: {
          break;
        }

        case IterOp::Abort:
          [[unlikely]] { return; }

        case IterOp::SkipChildren: {
          qcore_assert(false,
                       "bfs_post_impl: IterOp::SkipChildren not supported");
          break;
        }
      }
    }
  };

  syncfn(base, cb, cs);
}

CPP_EXPORT void detail::iter_children(Base** base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "iter_children: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](Base** a, Base** b) -> bool { return (uintptr_t)a < (uintptr_t)b; };
  }

  const auto syncfn = [](Base** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::vector<Base**> children;
    get_children_sorted(*n, cs, children);

    for (Base** child : children) {
      switch (cb(*n, child)) {
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

  syncfn(base, cb, cs);
}
