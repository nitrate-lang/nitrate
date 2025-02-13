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
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <queue>
#include <ranges>
#include <stack>

#include "nitrate-core/Assert.hh"

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
    Add(n->GetWidth());
    Add(n->GetRangeBegin());
    Add(n->GetRangeEnd());
  }

  void Visit(FlowPtr<Base>) override {}
  void Visit(FlowPtr<ExprStmt> n) override { Add(n->GetExpr()); }
  void Visit(FlowPtr<StmtExpr> n) override { Add(n->GetStmt()); }
  void Visit(FlowPtr<TypeExpr> n) override { Add(n->GetType()); }
  void Visit(FlowPtr<NamedTy> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<InferTy> n) override { AddTypesuffix(n); }

  void Visit(FlowPtr<TemplateType> n) override {
    Add(n->GetTemplate());
    std::for_each(n->GetArgs().begin(), n->GetArgs().end(), [&](auto arg) { Add(arg.second); });

    AddTypesuffix(n);
  }

  void Visit(FlowPtr<U1> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<U8> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<U16> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<U32> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<U64> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<U128> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<I8> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<I16> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<I32> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<I64> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<I128> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<F16> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<F32> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<F64> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<F128> n) override { AddTypesuffix(n); }
  void Visit(FlowPtr<VoidTy> n) override { AddTypesuffix(n); }

  void Visit(FlowPtr<PtrTy> n) override {
    Add(n->GetItem());
    AddTypesuffix(n);
  }

  void Visit(FlowPtr<OpaqueTy> n) override { AddTypesuffix(n); }

  void Visit(FlowPtr<TupleTy> n) override {
    std::for_each(n->GetItems().begin(), n->GetItems().end(), [&](auto item) { Add(item); });

    AddTypesuffix(n);
  }

  void Visit(FlowPtr<ArrayTy> n) override {
    Add(n->GetItem());
    Add(n->GetSize());
    AddTypesuffix(n);
  }

  void Visit(FlowPtr<RefTy> n) override {
    Add(n->GetItem());
    AddTypesuffix(n);
  }

  void Visit(FlowPtr<FuncTy> n) override {
    std::for_each(n->GetAttributes().begin(), n->GetAttributes().end(), [&](auto attr) { Add(attr); });

    std::for_each(n->GetParams().begin(), n->GetParams().end(), [&](auto param) {
      Add(std::get<1>(param));
      std::get<2>(param);
    });

    Add(n->GetReturn());

    AddTypesuffix(n);
  }

  void Visit(FlowPtr<Unary> n) override { Add(n->GetRHS()); }

  void Visit(FlowPtr<Binary> n) override {
    Add(n->GetLHS());
    Add(n->GetRHS());
  }

  void Visit(FlowPtr<PostUnary> n) override { Add(n->GetLHS()); }

  void Visit(FlowPtr<Ternary> n) override {
    Add(n->GetCond());
    Add(n->GetLHS());
    Add(n->GetRHS());
  }

  void Visit(FlowPtr<Integer>) override {}
  void Visit(FlowPtr<Float>) override {}
  void Visit(FlowPtr<Boolean>) override {}
  void Visit(FlowPtr<parse::String>) override {}
  void Visit(FlowPtr<Character>) override {}
  void Visit(FlowPtr<Null>) override {}
  void Visit(FlowPtr<Undefined>) override {}

  void Visit(FlowPtr<Call> n) override {
    Add(n->GetFunc());
    std::for_each(n->GetArgs().begin(), n->GetArgs().end(), [&](auto arg) { Add(arg.second); });
  }

  void Visit(FlowPtr<TemplateCall> n) override {
    Add(n->GetFunc());
    std::for_each(n->GetTemplateArgs().begin(), n->GetTemplateArgs().end(), [&](auto arg) { Add(arg.second); });

    std::for_each(n->GetArgs().begin(), n->GetArgs().end(), [&](auto arg) { Add(arg.second); });
  }

  void Visit(FlowPtr<List> n) override {
    std::for_each(n->GetItems().begin(), n->GetItems().end(), [&](auto item) { Add(item); });
  }

  void Visit(FlowPtr<Assoc> n) override {
    Add(n->GetKey());
    Add(n->GetValue());
  }

  void Visit(FlowPtr<Index> n) override {
    Add(n->GetBase());
    Add(n->GetIndex());
  }

  void Visit(FlowPtr<Slice> n) override {
    Add(n->GetBase());
    Add(n->GetStart());
    Add(n->GetEnd());
  }

  void Visit(FlowPtr<FString> n) override {
    std::for_each(n->GetItems().begin(), n->GetItems().end(), [&](auto arg) {
      if (std::holds_alternative<FlowPtr<Expr>>(arg)) {
        Add(std::get<FlowPtr<Expr>>(arg));
      } else {
        qcore_assert(std::holds_alternative<string>(arg));
      }
    });
  }

  void Visit(FlowPtr<Identifier>) override {}

  void Visit(FlowPtr<Sequence> n) override {
    std::for_each(n->GetItems().begin(), n->GetItems().end(), [&](auto item) { Add(item); });
  }

  void Visit(FlowPtr<Block> n) override {
    std::for_each(n->GetItems().begin(), n->GetItems().end(), [&](auto item) { Add(item); });
  }

  void Visit(FlowPtr<Variable> n) override {
    std::for_each(n->GetAttributes().begin(), n->GetAttributes().end(), [&](auto attr) { Add(attr); });

    Add(n->GetType());
    Add(n->GetValue());
  }

  void Visit(FlowPtr<Assembly> n) override {
    std::for_each(n->GetArgs().begin(), n->GetArgs().end(), [&](auto arg) { Add(arg); });
  }

  void Visit(FlowPtr<If> n) override {
    Add(n->GetCond());
    Add(n->GetThen());
    Add(n->GetElse());
  }

  void Visit(FlowPtr<While> n) override {
    Add(n->GetCond());
    Add(n->GetBody());
  }

  void Visit(FlowPtr<For> n) override {
    Add(n->GetInit());
    Add(n->GetCond());
    Add(n->GetStep());
    Add(n->GetBody());
  }

  void Visit(FlowPtr<Foreach> n) override {
    Add(n->GetExpr());
    Add(n->GetBody());
  }

  void Visit(FlowPtr<Break>) override {}
  void Visit(FlowPtr<Continue>) override {}
  void Visit(FlowPtr<Return> n) override { Add(n->GetValue()); }

  void Visit(FlowPtr<ReturnIf> n) override {
    Add(n->GetCond());
    Add(n->GetValue());
  }

  void Visit(FlowPtr<Case> n) override {
    Add(n->GetCond());
    Add(n->GetBody());
  }

  void Visit(FlowPtr<Switch> n) override {
    Add(n->GetCond());
    std::for_each(n->GetCases().begin(), n->GetCases().end(), [&](auto c) { Add(c); });
    Add(n->GetDefault());
  }

  void Visit(FlowPtr<Typedef> n) override { Add(n->GetType()); }

  void Visit(FlowPtr<Function> n) override {
    std::for_each(n->GetAttributes().begin(), n->GetAttributes().end(), [&](auto attr) { Add(attr); });

    if (n->GetTemplateParams()) {
      std::for_each(n->GetTemplateParams()->begin(), n->GetTemplateParams()->end(), [&](auto param) {
        Add(std::get<1>(param));
        Add(std::get<2>(param));
      });
    }

    std::for_each(n->GetParams().begin(), n->GetParams().end(), [&](auto param) {
      Add(std::get<1>(param));
      std::get<2>(param);
    });

    Add(n->GetReturn());
    Add(n->GetPrecond());
    Add(n->GetPostcond());
    Add(n->GetBody());
  }

  void Visit(FlowPtr<Struct> n) override {
    std::for_each(n->GetAttributes().begin(), n->GetAttributes().end(), [&](auto attr) { Add(attr); });

    if (n->GetTemplateParams()) {
      std::for_each(n->GetTemplateParams()->begin(), n->GetTemplateParams()->end(), [&](auto param) {
        Add(std::get<1>(param));
        Add(std::get<2>(param));
      });
    }

    std::for_each(n->GetFields().begin(), n->GetFields().end(), [&](auto field) {
      Add(field.GetType());
      Add(field.GetValue());
    });

    std::for_each(n->GetMethods().begin(), n->GetMethods().end(), [&](auto method) { Add(method.m_func); });

    std::for_each(n->GetStaticMethods().begin(), n->GetStaticMethods().end(), [&](auto method) { Add(method.m_func); });
  }

  void Visit(FlowPtr<Enum> n) override {
    Add(n->GetType());

    std::for_each(n->GetItems().begin(), n->GetItems().end(), [&](auto item) { Add(item.second); });
  }

  void Visit(FlowPtr<Scope> n) override { Add(n->GetBody()); }

  void Visit(FlowPtr<Export> n) override {
    std::for_each(n->GetAttrs().begin(), n->GetAttrs().end(), [&](auto attr) { Add(attr); });

    Add(n->GetBody());
  }

public:
  IterVisitor(std::vector<FlowPtr<Base>>& children) : m_sub(children) {}
};

static NCC_FORCE_INLINE void GetChildrenSorted(FlowPtr<Base> base, std::vector<FlowPtr<Base>>& children) {
  children.clear();

  if (!base) [[unlikely]] {
    return;
  }

  IterVisitor v(children);
  base.Accept(v);
}

NCC_EXPORT void detail::DfsPreImpl(const FlowPtr<Base>& base, const IterCallback& cb) {
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

NCC_EXPORT void detail::DfsPostImpl(const FlowPtr<Base>& base, const IterCallback& cb) {
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

NCC_EXPORT void detail::BfsPreImpl(const FlowPtr<Base>& base, const IterCallback& cb) {
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

NCC_EXPORT void detail::BfsPostImpl(const FlowPtr<Base>& base, const IterCallback& cb) {
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

NCC_EXPORT void detail::IterChildren(const FlowPtr<Base>& base, const IterCallback& cb) {
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
