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

#include <nitrate-core/Macro.h>

#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <queue>

using namespace npar;

class IterVisitor : public ASTVisitor {
  std::vector<npar_node_t**>& sub;

  void visit(npar_node_t const& n) override {}

  void visit(ExprStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
    // sub.push_back(reinterpret_cast<npar_node_t**>(&n.get_expr()));
    (void)sub;
  }

  void visit(StmtExpr const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(TypeExpr const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(NamedTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(InferTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(TemplType const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(U1 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(U8 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(U16 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(U32 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(U64 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(U128 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(I8 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(I16 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(I32 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(I64 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(I128 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(F16 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(F32 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(F64 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(F128 const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(VoidTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(PtrTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(OpaqueTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(TupleTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ArrayTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(RefTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(FuncTy const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(UnaryExpr const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(BinExpr const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(PostUnaryExpr const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(TernaryExpr const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ConstInt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ConstFloat const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ConstBool const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ConstString const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ConstChar const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ConstNull const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ConstUndef const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(Call const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(TemplCall const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(List const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(Assoc const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(Field const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(Index const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(Slice const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(FString const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(Ident const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(SeqPoint const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(Block const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(VarDecl const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(InlineAsm const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(IfStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(WhileStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ForStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ForeachStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(BreakStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ContinueStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ReturnStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ReturnIfStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(CaseStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(SwitchStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(TypedefStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(Function const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(StructDef const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(EnumDef const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ScopeStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

  void visit(ExportStmt const& n) override {
    /// TODO: Implement IterVisitor
    qcore_implement();
  }

public:
  IterVisitor(std::vector<npar_node_t**>& children) : sub(children) {}
};

#define FORCE_INLINE __attribute__((always_inline)) inline

static FORCE_INLINE void get_children_sorted(
    npar_node_t* base, ChildSelect cs, std::vector<npar_node_t**>& children) {
  children.clear();

  if (!base) [[unlikely]] {
    return;
  }

  IterVisitor v(children);
  base->accept(v);

  std::sort(children.begin(), children.end(), cs);

  return;
}

CPP_EXPORT void detail::dfs_pre_impl(npar_node_t** base, IterCallback cb,
                                     ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_pre_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](npar_node_t** a, npar_node_t** b) -> bool {
      return (uintptr_t)a < (uintptr_t)b;
    };
  }

  const auto syncfn = [](npar_node_t** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::stack<std::pair<npar_node_t*, npar_node_t**>> s;
    std::vector<npar_node_t**> children;

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

CPP_EXPORT void detail::dfs_post_impl(npar_node_t** base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "dfs_post_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](npar_node_t** a, npar_node_t** b) -> bool {
      return (uintptr_t)a < (uintptr_t)b;
    };
  }

  const auto syncfn = [](npar_node_t** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::stack<std::pair<npar_node_t*, npar_node_t**>> s;
    std::vector<npar_node_t**> children;

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

CPP_EXPORT void detail::bfs_pre_impl(npar_node_t** base, IterCallback cb,
                                     ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_pre_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](npar_node_t** a, npar_node_t** b) -> bool {
      return (uintptr_t)a < (uintptr_t)b;
    };
  }

  const auto syncfn = [](npar_node_t** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::queue<std::pair<npar_node_t*, npar_node_t**>> s;
    std::vector<npar_node_t**> children;

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

CPP_EXPORT void detail::bfs_post_impl(npar_node_t** base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "bfs_post_impl: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](npar_node_t** a, npar_node_t** b) -> bool {
      return (uintptr_t)a < (uintptr_t)b;
    };
  }

  const auto syncfn = [](npar_node_t** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::queue<std::pair<npar_node_t*, npar_node_t**>> s;
    std::vector<npar_node_t**> children;

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

CPP_EXPORT void detail::iter_children(npar_node_t** base, IterCallback cb,
                                      ChildSelect cs) {
  qcore_assert(base != nullptr && cb != nullptr,
               "iter_children: base and cb must not be null");

  if (!cs) { /* Iterate in the order the children are stored in the classes */
    cs = [](npar_node_t** a, npar_node_t** b) -> bool {
      return (uintptr_t)a < (uintptr_t)b;
    };
  }

  const auto syncfn = [](npar_node_t** n, const IterCallback& cb,
                         const ChildSelect& cs) {
    std::vector<npar_node_t**> children;
    get_children_sorted(*n, cs, children);

    for (npar_node_t** child : children) {
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
