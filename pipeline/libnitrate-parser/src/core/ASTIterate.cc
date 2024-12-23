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

using namespace ncc;
using namespace ncc::parse;

class IterVisitor : public ASTVisitor {
  std::vector<RefNode<Base>>& sub;

  template <class T>
  void add(RefNode<T> const& n) {
    if (n == nullptr) {
      return;
    }

    sub.push_back(n);
  }

  void add_typesuffix(RefNode<const Type> n) {
    add(n->get_width());
    add(n->get_range().first);
    add(n->get_range().second);
  }

  void visit(RefNode<const Base>) override {}
  void visit(RefNode<const ExprStmt> n) override { add(n->get_expr()); }
  void visit(RefNode<const StmtExpr> n) override { add(n->get_stmt()); }
  void visit(RefNode<const TypeExpr> n) override { add(n->get_type()); }
  void visit(RefNode<const NamedTy> n) override { add_typesuffix(n); }
  void visit(RefNode<const InferTy> n) override { add_typesuffix(n); }

  void visit(RefNode<const TemplType> n) override {
    add(n->get_template());
    std::for_each(n->get_args().begin(), n->get_args().end(),
                  [&](let arg) { add(arg.second); });

    add_typesuffix(n);
  }

  void visit(RefNode<const U1> n) override { add_typesuffix(n); }
  void visit(RefNode<const U8> n) override { add_typesuffix(n); }
  void visit(RefNode<const U16> n) override { add_typesuffix(n); }
  void visit(RefNode<const U32> n) override { add_typesuffix(n); }
  void visit(RefNode<const U64> n) override { add_typesuffix(n); }
  void visit(RefNode<const U128> n) override { add_typesuffix(n); }
  void visit(RefNode<const I8> n) override { add_typesuffix(n); }
  void visit(RefNode<const I16> n) override { add_typesuffix(n); }
  void visit(RefNode<const I32> n) override { add_typesuffix(n); }
  void visit(RefNode<const I64> n) override { add_typesuffix(n); }
  void visit(RefNode<const I128> n) override { add_typesuffix(n); }
  void visit(RefNode<const F16> n) override { add_typesuffix(n); }
  void visit(RefNode<const F32> n) override { add_typesuffix(n); }
  void visit(RefNode<const F64> n) override { add_typesuffix(n); }
  void visit(RefNode<const F128> n) override { add_typesuffix(n); }
  void visit(RefNode<const VoidTy> n) override { add_typesuffix(n); }

  void visit(RefNode<const PtrTy> n) override {
    add(n->get_item());
    add_typesuffix(n);
  }

  void visit(RefNode<const OpaqueTy> n) override { add_typesuffix(n); }

  void visit(RefNode<const TupleTy> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](let item) { add(item); });

    add_typesuffix(n);
  }

  void visit(RefNode<const ArrayTy> n) override {
    add(n->get_item());
    add(n->get_size());
    add_typesuffix(n);
  }

  void visit(RefNode<const RefTy> n) override {
    add(n->get_item());
    add_typesuffix(n);
  }

  void visit(RefNode<const FuncTy> n) override {
    std::for_each(n->get_attributes().begin(), n->get_attributes().end(),
                  [&](let attr) { add(attr); });

    std::for_each(n->get_params().params.begin(), n->get_params().params.end(),
                  [&](let param) {
                    add(std::get<1>(param));
                    std::get<2>(param);
                  });

    add(n->get_return());

    add_typesuffix(n);
  }

  void visit(RefNode<const UnaryExpr> n) override { add(n->get_rhs()); }

  void visit(RefNode<const BinExpr> n) override {
    add(n->get_lhs());
    add(n->get_rhs());
  }

  void visit(RefNode<const PostUnaryExpr> n) override { add(n->get_lhs()); }

  void visit(RefNode<const TernaryExpr> n) override {
    add(n->get_cond());
    add(n->get_lhs());
    add(n->get_rhs());
  }

  void visit(RefNode<const ConstInt>) override {}
  void visit(RefNode<const ConstFloat>) override {}
  void visit(RefNode<const ConstBool>) override {}
  void visit(RefNode<const ConstString>) override {}
  void visit(RefNode<const ConstChar>) override {}
  void visit(RefNode<const ConstNull>) override {}
  void visit(RefNode<const ConstUndef>) override {}

  void visit(RefNode<const Call> n) override {
    add(n->get_func());
    std::for_each(n->get_args().begin(), n->get_args().end(),
                  [&](let arg) { add(arg.second); });
  }

  void visit(RefNode<const TemplCall> n) override {
    add(n->get_func());
    std::for_each(n->get_template_args().begin(), n->get_template_args().end(),
                  [&](let arg) { add(arg.second); });

    std::for_each(n->get_args().begin(), n->get_args().end(),
                  [&](let arg) { add(arg.second); });
  }

  void visit(RefNode<const List> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](let item) { add(item); });
  }

  void visit(RefNode<const Assoc> n) override {
    add(n->get_key());
    add(n->get_value());
  }

  void visit(RefNode<const Index> n) override {
    add(n->get_base());
    add(n->get_index());
  }

  void visit(RefNode<const Slice> n) override {
    add(n->get_base());
    add(n->get_start());
    add(n->get_end());
  }

  void visit(RefNode<const FString> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(), [&](let arg) {
      if (std::holds_alternative<RefNode<Expr>>(arg)) {
        add(std::get<RefNode<Expr>>(arg));
      } else if (std::holds_alternative<string>(arg)) {
      } else {
        qcore_implement();
      }
    });
  }

  void visit(RefNode<const Ident>) override {}

  void visit(RefNode<const SeqPoint> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](let item) { add(item); });
  }

  void visit(RefNode<const Block> n) override {
    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](let item) { add(item); });
  }

  void visit(RefNode<const VarDecl> n) override {
    std::for_each(n->get_attributes().begin(), n->get_attributes().end(),
                  [&](let attr) { add(attr); });

    add(n->get_type());
    add(n->get_value());
  }

  void visit(RefNode<const InlineAsm> n) override {
    std::for_each(n->get_args().begin(), n->get_args().end(),
                  [&](let arg) { add(arg); });
  }

  void visit(RefNode<const IfStmt> n) override {
    add(n->get_cond());
    add(n->get_then());
    add(n->get_else());
  }

  void visit(RefNode<const WhileStmt> n) override {
    add(n->get_cond());
    add(n->get_body());
  }

  void visit(RefNode<const ForStmt> n) override {
    add(n->get_init().value_or(nullptr));
    add(n->get_cond().value_or(nullptr));
    add(n->get_step().value_or(nullptr));
    add(n->get_body());
  }

  void visit(RefNode<const ForeachStmt> n) override {
    add(n->get_expr());
    add(n->get_body());
  }

  void visit(RefNode<const BreakStmt>) override {}
  void visit(RefNode<const ContinueStmt>) override {}
  void visit(RefNode<const ReturnStmt> n) override {
    add(n->get_value().value_or(nullptr));
  }

  void visit(RefNode<const ReturnIfStmt> n) override {
    add(n->get_cond());
    add(n->get_value());
  }

  void visit(RefNode<const CaseStmt> n) override {
    add(n->get_cond());
    add(n->get_body());
  }

  void visit(RefNode<const SwitchStmt> n) override {
    add(n->get_cond());
    std::for_each(n->get_cases().begin(), n->get_cases().end(),
                  [&](let c) { add(c); });
    add(n->get_default());
  }

  void visit(RefNode<const TypedefStmt> n) override { add(n->get_type()); }

  void visit(RefNode<const Function> n) override {
    std::for_each(n->get_attributes().begin(), n->get_attributes().end(),
                  [&](let attr) { add(attr); });

    if (n->get_template_params()) {
      std::for_each(n->get_template_params()->begin(),
                    n->get_template_params()->end(), [&](let param) {
                      add(std::get<1>(param));
                      add(std::get<2>(param));
                    });
    }

    std::for_each(n->get_params().params.begin(), n->get_params().params.end(),
                  [&](let param) {
                    add(std::get<1>(param));
                    std::get<2>(param);
                  });

    add(n->get_return());
    add(n->get_precond().value_or(nullptr));
    add(n->get_postcond().value_or(nullptr));
    add(n->get_body().value_or(nullptr));
  }

  void visit(RefNode<const StructDef> n) override {
    std::for_each(n->get_attributes().begin(), n->get_attributes().end(),
                  [&](let attr) { add(attr); });

    if (n->get_template_params()) {
      std::for_each(n->get_template_params()->begin(),
                    n->get_template_params()->end(), [&](let param) {
                      add(std::get<1>(param));
                      add(std::get<2>(param));
                    });
    }

    std::for_each(n->get_fields().begin(), n->get_fields().end(),
                  [&](let field) {
                    add(field.get_type());
                    add(field.get_value().value_or(nullptr));
                  });

    std::for_each(n->get_methods().begin(), n->get_methods().end(),
                  [&](let method) { add(method.func); });

    std::for_each(n->get_static_methods().begin(),
                  n->get_static_methods().end(),
                  [&](let method) { add(method.func); });
  }

  void visit(RefNode<const EnumDef> n) override {
    add(n->get_type());

    std::for_each(n->get_items().begin(), n->get_items().end(),
                  [&](let item) { add(item.second); });
  }

  void visit(RefNode<const ScopeStmt> n) override { add(n->get_body()); }

  void visit(RefNode<const ExportStmt> n) override {
    std::for_each(n->get_attrs().begin(), n->get_attrs().end(),
                  [&](let attr) { add(attr); });

    add(n->get_body());
  }

public:
  IterVisitor(std::vector<RefNode<Base>>& children) : sub(children) {}
};

static FORCE_INLINE void get_children_sorted(
    RefNode<Base> base, std::vector<RefNode<Base>>& children) {
  children.clear();

  if (!base) [[unlikely]] {
    return;
  }

  IterVisitor v(children);
  base.accept(v);

  return;
}

CPP_EXPORT void detail::dfs_pre_impl(RefNode<Base> base, IterCallback cb) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_pre_impl: base and cb must not be null");

  auto syncfn = [](RefNode<Base> n, IterCallback& cb) {
    std::stack<std::pair<RefNode<Base>, RefNode<Base>>> s;
    std::vector<RefNode<Base>> children;

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
        get_children_sorted(cur.second, children);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          s.push({cur.second, *it});
        }
      }
    }
  };

  syncfn(base, cb);
}

CPP_EXPORT void detail::dfs_post_impl(RefNode<Base> base, IterCallback cb) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_post_impl: base and cb must not be null");

  auto syncfn = [](RefNode<Base> n, IterCallback& cb) {
    std::stack<std::pair<RefNode<Base>, RefNode<Base>>> s;
    std::vector<RefNode<Base>> children;

    s.push({nullptr, n});

    while (!s.empty()) {
      auto cur = s.top();
      s.pop();

      get_children_sorted(cur.second, children);
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        s.push({cur.second, *it});
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

  syncfn(base, cb);
  cb(nullptr, base);
}

CPP_EXPORT void detail::bfs_pre_impl(RefNode<Base> base, IterCallback cb) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_pre_impl: base and cb must not be null");

  auto syncfn = [](RefNode<Base> n, IterCallback& cb) {
    std::queue<std::pair<RefNode<Base>, RefNode<Base>>> s;
    std::vector<RefNode<Base>> children;

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
        get_children_sorted(cur.second, children);
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
          s.push({cur.second, *it});
        }
      }
    }
  };

  syncfn(base, cb);
}

CPP_EXPORT void detail::bfs_post_impl(RefNode<Base> base, IterCallback cb) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_post_impl: base and cb must not be null");

  auto syncfn = [](RefNode<Base> n, IterCallback& cb) {
    std::queue<std::pair<RefNode<Base>, RefNode<Base>>> s;
    std::vector<RefNode<Base>> children;

    s.push({nullptr, n});

    while (!s.empty()) {
      auto cur = s.front();
      s.pop();

      get_children_sorted(cur.second, children);
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        s.push({cur.second, *it});
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

  syncfn(base, cb);
}

CPP_EXPORT void detail::iter_children(RefNode<Base> base, IterCallback cb) {
  qcore_assert(base != nullptr && cb != nullptr,
               "iter_children: base and cb must not be null");

  auto syncfn = [](RefNode<Base> n, IterCallback& cb) {
    std::vector<RefNode<Base>> children;
    get_children_sorted(n, children);

    for (RefNode<Base> child : children) {
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
