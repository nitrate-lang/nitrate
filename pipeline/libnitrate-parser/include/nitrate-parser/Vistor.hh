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

struct npar_node_t;

namespace npar {
  class Stmt;
  class Type;
  class Expr;
  class ExprStmt;
  class StmtExpr;
  class TypeExpr;
  class NamedTy;
  class InferTy;
  class TemplType;
  class U1;
  class U8;
  class U16;
  class U32;
  class U64;
  class U128;
  class I8;
  class I16;
  class I32;
  class I64;
  class I128;
  class F16;
  class F32;
  class F64;
  class F128;
  class VoidTy;
  class PtrTy;
  class OpaqueTy;
  class TupleTy;
  class ArrayTy;
  class RefTy;
  class StructTy;
  class FuncTy;
  class UnaryExpr;
  class BinExpr;
  class PostUnaryExpr;
  class TernaryExpr;
  class ConstInt;
  class ConstFloat;
  class ConstBool;
  class ConstString;
  class ConstChar;
  class ConstNull;
  class ConstUndef;
  class Call;
  class TemplCall;
  class List;
  class Assoc;
  class Field;
  class Index;
  class Slice;
  class FString;
  class Ident;
  class SeqPoint;
  class Block;
  class VarDecl;
  class InlineAsm;
  class IfStmt;
  class WhileStmt;
  class ForStmt;
  class ForeachStmt;
  class BreakStmt;
  class ContinueStmt;
  class ReturnStmt;
  class ReturnIfStmt;
  class CaseStmt;
  class SwitchStmt;
  class TypedefStmt;
  class FnDecl;
  class FnDef;
  class StructField;
  class StructDef;
  class EnumDef;
  class ScopeStmt;
  class ExportStmt;

}  // namespace npar

namespace npar {
  class ASTVisitor {
  public:
    virtual void visit(npar_node_t& n) = 0;
    virtual void visit(ExprStmt& n) = 0;
    virtual void visit(StmtExpr& n) = 0;
    virtual void visit(TypeExpr& n) = 0;
    virtual void visit(NamedTy& n) = 0;
    virtual void visit(InferTy& n) = 0;
    virtual void visit(TemplType& n) = 0;
    virtual void visit(U1& n) = 0;
    virtual void visit(U8& n) = 0;
    virtual void visit(U16& n) = 0;
    virtual void visit(U32& n) = 0;
    virtual void visit(U64& n) = 0;
    virtual void visit(U128& n) = 0;
    virtual void visit(I8& n) = 0;
    virtual void visit(I16& n) = 0;
    virtual void visit(I32& n) = 0;
    virtual void visit(I64& n) = 0;
    virtual void visit(I128& n) = 0;
    virtual void visit(F16& n) = 0;
    virtual void visit(F32& n) = 0;
    virtual void visit(F64& n) = 0;
    virtual void visit(F128& n) = 0;
    virtual void visit(VoidTy& n) = 0;
    virtual void visit(PtrTy& n) = 0;
    virtual void visit(OpaqueTy& n) = 0;
    virtual void visit(TupleTy& n) = 0;
    virtual void visit(ArrayTy& n) = 0;
    virtual void visit(RefTy& n) = 0;
    virtual void visit(StructTy& n) = 0;
    virtual void visit(FuncTy& n) = 0;
    virtual void visit(UnaryExpr& n) = 0;
    virtual void visit(BinExpr& n) = 0;
    virtual void visit(PostUnaryExpr& n) = 0;
    virtual void visit(TernaryExpr& n) = 0;
    virtual void visit(ConstInt& n) = 0;
    virtual void visit(ConstFloat& n) = 0;
    virtual void visit(ConstBool& n) = 0;
    virtual void visit(ConstString& n) = 0;
    virtual void visit(ConstChar& n) = 0;
    virtual void visit(ConstNull& n) = 0;
    virtual void visit(ConstUndef& n) = 0;
    virtual void visit(Call& n) = 0;
    virtual void visit(TemplCall& n) = 0;
    virtual void visit(List& n) = 0;
    virtual void visit(Assoc& n) = 0;
    virtual void visit(Field& n) = 0;
    virtual void visit(Index& n) = 0;
    virtual void visit(Slice& n) = 0;
    virtual void visit(FString& n) = 0;
    virtual void visit(Ident& n) = 0;
    virtual void visit(SeqPoint& n) = 0;
    virtual void visit(Block& n) = 0;
    virtual void visit(VarDecl& n) = 0;
    virtual void visit(InlineAsm& n) = 0;
    virtual void visit(IfStmt& n) = 0;
    virtual void visit(WhileStmt& n) = 0;
    virtual void visit(ForStmt& n) = 0;
    virtual void visit(ForeachStmt& n) = 0;
    virtual void visit(BreakStmt& n) = 0;
    virtual void visit(ContinueStmt& n) = 0;
    virtual void visit(ReturnStmt& n) = 0;
    virtual void visit(ReturnIfStmt& n) = 0;
    virtual void visit(CaseStmt& n) = 0;
    virtual void visit(SwitchStmt& n) = 0;
    virtual void visit(TypedefStmt& n) = 0;
    virtual void visit(FnDecl& n) = 0;
    virtual void visit(FnDef& n) = 0;
    virtual void visit(StructField& n) = 0;
    virtual void visit(StructDef& n) = 0;
    virtual void visit(EnumDef& n) = 0;
    virtual void visit(ScopeStmt& n) = 0;
    virtual void visit(ExportStmt& n) = 0;
  };
}  // namespace npar

#endif  // __NITRATE_PARSER_VISTOR_H__
