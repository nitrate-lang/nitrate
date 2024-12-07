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
#include <nitrate-core/Macro.h>
#include <nitrate-parser/Node.h>

#include <algorithm>
#include <nitrate-parser/Writer.hh>

using namespace npar;

void AST_Writer::visit(npar_node_t& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ExprStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(StmtExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(TypeExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(NamedTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(InferTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(TemplType& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(U1& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(U8& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(U16& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(U32& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(U64& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(U128& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(I8& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(I16& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(I32& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(I64& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(I128& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(F16& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(F32& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(F64& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(F128& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(VoidTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(PtrTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(OpaqueTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(TupleTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ArrayTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(RefTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(StructTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(FuncTy& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(UnaryExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(BinExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(PostUnaryExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(TernaryExpr& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ConstInt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ConstFloat& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ConstBool& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ConstString& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ConstChar& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ConstNull& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ConstUndef& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(Call& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(TemplCall& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(List& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(Assoc& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(Field& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(Index& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(Slice& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(FString& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(Ident& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(SeqPoint& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(Block& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  { /* Write safety profile*/
    string("safe");

    switch (n.get_safety()) {
      case SafetyMode::Unknown:
        null();
        break;
      case SafetyMode::Safe:
        string("yes");
        break;
      case SafetyMode::Unsafe:
        string("no");
        break;
    }
  }

  { /* Write body */
    string("body");

    let items = n.get_items();
    begin_arr(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto& item) { item->accept(*this); });
    end_arr();
  }

  end_obj();
}

void AST_Writer::visit(VarDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(InlineAsm& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(IfStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(WhileStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ForStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ForeachStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(BreakStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ContinueStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ReturnStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ReturnIfStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(CaseStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(SwitchStmt& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(TypedefDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(FnDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(FnDef& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(StructField& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(StructDef& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(EnumDef& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ScopeDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}

void AST_Writer::visit(ExportDecl& n) {
  begin_obj();

  { /* Write kind */
    string("kind");
    string(n.getKindName());
  }

  /// TODO: Implement support for this node

  end_obj();
}
