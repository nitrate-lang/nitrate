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

#include <cstring>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-parser/Context.hh>

using namespace ncc;
using namespace ncc::ir;

class CloneVisitor : public IRVisitor<void> {
  std::optional<Expr *> R;

public:
  CloneVisitor() {}
  virtual ~CloneVisitor() = default;

  Expr *GetClone() { return R.value(); }

  void visit(FlowPtr<Expr> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Type> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<BinExpr> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Unary> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U1Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U8Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U16Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U32Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U64Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<U128Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I8Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I16Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I32Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I64Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<I128Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<F16Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<F32Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<F64Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<F128Ty> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<VoidTy> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<PtrTy> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<ConstTy> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<OpaqueTy> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<StructTy> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<UnionTy> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<ArrayTy> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<FnTy> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Int> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Float> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<List> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Call> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Seq> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Index> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Ident> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Extern> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Local> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Ret> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Brk> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Cont> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<If> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<While> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<For> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Case> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Switch> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Function> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Asm> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }

  void visit(FlowPtr<Tmp> n) override {
    /// TODO: Implement
    qcore_implement();
    (void)n;
  }
};

CPP_EXPORT Expr *detail::Expr_getCloneImpl(const Expr *self) {
  CloneVisitor V;
  V.dispatch(MakeFlowPtr(self));

  return V.GetClone();
}
