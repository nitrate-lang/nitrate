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

#ifndef __NITRATE_PARSER_VISTOR_H__
#define __NITRATE_PARSER_VISTOR_H__

#include <cassert>
#include <nitrate-parser/ASTCommon.hh>

namespace npar {
  class ASTVisitor {
  public:
    virtual ~ASTVisitor() = default;

    virtual void visit(npar_node_t const &n) = 0;
    virtual void visit(ExprStmt const &n) = 0;
    virtual void visit(StmtExpr const &n) = 0;
    virtual void visit(TypeExpr const &n) = 0;
    virtual void visit(NamedTy const &n) = 0;
    virtual void visit(InferTy const &n) = 0;
    virtual void visit(TemplType const &n) = 0;
    virtual void visit(U1 const &n) = 0;
    virtual void visit(U8 const &n) = 0;
    virtual void visit(U16 const &n) = 0;
    virtual void visit(U32 const &n) = 0;
    virtual void visit(U64 const &n) = 0;
    virtual void visit(U128 const &n) = 0;
    virtual void visit(I8 const &n) = 0;
    virtual void visit(I16 const &n) = 0;
    virtual void visit(I32 const &n) = 0;
    virtual void visit(I64 const &n) = 0;
    virtual void visit(I128 const &n) = 0;
    virtual void visit(F16 const &n) = 0;
    virtual void visit(F32 const &n) = 0;
    virtual void visit(F64 const &n) = 0;
    virtual void visit(F128 const &n) = 0;
    virtual void visit(VoidTy const &n) = 0;
    virtual void visit(PtrTy const &n) = 0;
    virtual void visit(OpaqueTy const &n) = 0;
    virtual void visit(TupleTy const &n) = 0;
    virtual void visit(ArrayTy const &n) = 0;
    virtual void visit(RefTy const &n) = 0;
    virtual void visit(FuncTy const &n) = 0;
    virtual void visit(UnaryExpr const &n) = 0;
    virtual void visit(BinExpr const &n) = 0;
    virtual void visit(PostUnaryExpr const &n) = 0;
    virtual void visit(TernaryExpr const &n) = 0;
    virtual void visit(ConstInt const &n) = 0;
    virtual void visit(ConstFloat const &n) = 0;
    virtual void visit(ConstBool const &n) = 0;
    virtual void visit(ConstString const &n) = 0;
    virtual void visit(ConstChar const &n) = 0;
    virtual void visit(ConstNull const &n) = 0;
    virtual void visit(ConstUndef const &n) = 0;
    virtual void visit(Call const &n) = 0;
    virtual void visit(TemplCall const &n) = 0;
    virtual void visit(List const &n) = 0;
    virtual void visit(Assoc const &n) = 0;
    virtual void visit(Field const &n) = 0;
    virtual void visit(Index const &n) = 0;
    virtual void visit(Slice const &n) = 0;
    virtual void visit(FString const &n) = 0;
    virtual void visit(Ident const &n) = 0;
    virtual void visit(SeqPoint const &n) = 0;
    virtual void visit(Block const &n) = 0;
    virtual void visit(VarDecl const &n) = 0;
    virtual void visit(InlineAsm const &n) = 0;
    virtual void visit(IfStmt const &n) = 0;
    virtual void visit(WhileStmt const &n) = 0;
    virtual void visit(ForStmt const &n) = 0;
    virtual void visit(ForeachStmt const &n) = 0;
    virtual void visit(BreakStmt const &n) = 0;
    virtual void visit(ContinueStmt const &n) = 0;
    virtual void visit(ReturnStmt const &n) = 0;
    virtual void visit(ReturnIfStmt const &n) = 0;
    virtual void visit(CaseStmt const &n) = 0;
    virtual void visit(SwitchStmt const &n) = 0;
    virtual void visit(TypedefStmt const &n) = 0;
    virtual void visit(FnDef const &n) = 0;
    virtual void visit(StructField const &n) = 0;
    virtual void visit(StructDef const &n) = 0;
    virtual void visit(EnumDef const &n) = 0;
    virtual void visit(ScopeStmt const &n) = 0;
    virtual void visit(ExportStmt const &n) = 0;
  };
}  // namespace npar

#endif  // __NITRATE_PARSER_VISTOR_H__
