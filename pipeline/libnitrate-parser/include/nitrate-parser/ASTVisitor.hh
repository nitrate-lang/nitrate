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

#ifndef __NITRATE_AST_VISTOR_H__
#define __NITRATE_AST_VISTOR_H__

#include <cassert>
#include <nitrate-parser/ASTCommon.hh>

namespace ncc::parse {
  template <class T>
  class RefNode;

  class ASTVisitor {
  public:
    virtual ~ASTVisitor() = default;

    virtual void visit(RefNode<const Base> n) = 0;
    virtual void visit(RefNode<const ExprStmt> n) = 0;
    virtual void visit(RefNode<const StmtExpr> n) = 0;
    virtual void visit(RefNode<const TypeExpr> n) = 0;
    virtual void visit(RefNode<const NamedTy> n) = 0;
    virtual void visit(RefNode<const InferTy> n) = 0;
    virtual void visit(RefNode<const TemplType> n) = 0;
    virtual void visit(RefNode<const U1> n) = 0;
    virtual void visit(RefNode<const U8> n) = 0;
    virtual void visit(RefNode<const U16> n) = 0;
    virtual void visit(RefNode<const U32> n) = 0;
    virtual void visit(RefNode<const U64> n) = 0;
    virtual void visit(RefNode<const U128> n) = 0;
    virtual void visit(RefNode<const I8> n) = 0;
    virtual void visit(RefNode<const I16> n) = 0;
    virtual void visit(RefNode<const I32> n) = 0;
    virtual void visit(RefNode<const I64> n) = 0;
    virtual void visit(RefNode<const I128> n) = 0;
    virtual void visit(RefNode<const F16> n) = 0;
    virtual void visit(RefNode<const F32> n) = 0;
    virtual void visit(RefNode<const F64> n) = 0;
    virtual void visit(RefNode<const F128> n) = 0;
    virtual void visit(RefNode<const VoidTy> n) = 0;
    virtual void visit(RefNode<const PtrTy> n) = 0;
    virtual void visit(RefNode<const OpaqueTy> n) = 0;
    virtual void visit(RefNode<const TupleTy> n) = 0;
    virtual void visit(RefNode<const ArrayTy> n) = 0;
    virtual void visit(RefNode<const RefTy> n) = 0;
    virtual void visit(RefNode<const FuncTy> n) = 0;
    virtual void visit(RefNode<const UnaryExpr> n) = 0;
    virtual void visit(RefNode<const BinExpr> n) = 0;
    virtual void visit(RefNode<const PostUnaryExpr> n) = 0;
    virtual void visit(RefNode<const TernaryExpr> n) = 0;
    virtual void visit(RefNode<const ConstInt> n) = 0;
    virtual void visit(RefNode<const ConstFloat> n) = 0;
    virtual void visit(RefNode<const ConstBool> n) = 0;
    virtual void visit(RefNode<const ConstString> n) = 0;
    virtual void visit(RefNode<const ConstChar> n) = 0;
    virtual void visit(RefNode<const ConstNull> n) = 0;
    virtual void visit(RefNode<const ConstUndef> n) = 0;
    virtual void visit(RefNode<const Call> n) = 0;
    virtual void visit(RefNode<const TemplCall> n) = 0;
    virtual void visit(RefNode<const List> n) = 0;
    virtual void visit(RefNode<const Assoc> n) = 0;
    virtual void visit(RefNode<const Index> n) = 0;
    virtual void visit(RefNode<const Slice> n) = 0;
    virtual void visit(RefNode<const FString> n) = 0;
    virtual void visit(RefNode<const Ident> n) = 0;
    virtual void visit(RefNode<const SeqPoint> n) = 0;
    virtual void visit(RefNode<const Block> n) = 0;
    virtual void visit(RefNode<const VarDecl> n) = 0;
    virtual void visit(RefNode<const InlineAsm> n) = 0;
    virtual void visit(RefNode<const IfStmt> n) = 0;
    virtual void visit(RefNode<const WhileStmt> n) = 0;
    virtual void visit(RefNode<const ForStmt> n) = 0;
    virtual void visit(RefNode<const ForeachStmt> n) = 0;
    virtual void visit(RefNode<const BreakStmt> n) = 0;
    virtual void visit(RefNode<const ContinueStmt> n) = 0;
    virtual void visit(RefNode<const ReturnStmt> n) = 0;
    virtual void visit(RefNode<const ReturnIfStmt> n) = 0;
    virtual void visit(RefNode<const CaseStmt> n) = 0;
    virtual void visit(RefNode<const SwitchStmt> n) = 0;
    virtual void visit(RefNode<const TypedefStmt> n) = 0;
    virtual void visit(RefNode<const Function> n) = 0;
    virtual void visit(RefNode<const StructDef> n) = 0;
    virtual void visit(RefNode<const EnumDef> n) = 0;
    virtual void visit(RefNode<const ScopeStmt> n) = 0;
    virtual void visit(RefNode<const ExportStmt> n) = 0;
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_VISTOR_H__
