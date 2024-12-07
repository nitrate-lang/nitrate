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

#include <nitrate-core/Error.h>

#include <nitrate-parser/Writer.hh>

using namespace npar;

void AST_Writer::visit(npar_node_t&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ExprStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(StmtExpr&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(TypeExpr&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(NamedTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(InferTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(TemplType&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(U1&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(U8&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(U16&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(U32&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(U64&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(U128&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(I8&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(I16&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(I32&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(I64&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(I128&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(F16&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(F32&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(F64&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(F128&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(VoidTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(PtrTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ConstTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(OpaqueTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(TupleTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ArrayTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(RefTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(StructTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(FuncTy&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(UnaryExpr&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(BinExpr&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(PostUnaryExpr&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(TernaryExpr&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ConstInt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ConstFloat&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ConstBool&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ConstString&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ConstChar&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ConstNull&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ConstUndef&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(Call&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(TemplCall&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(List&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(Assoc&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(Field&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(Index&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(Slice&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(FString&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(Ident&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(SeqPoint&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(Block&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(VarDecl&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(InlineAsm&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(IfStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(WhileStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ForStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ForeachStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(BreakStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ContinueStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ReturnStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ReturnIfStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(CaseStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(SwitchStmt&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(TypedefDecl&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(FnDecl&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(FnDef&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(StructField&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(StructDef&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(EnumDef&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ScopeDecl&) {
  /// TODO: Implement support for this node
  qcore_implement();
}

void AST_Writer::visit(ExportDecl&) {
  /// TODO: Implement support for this node
  qcore_implement();
}
