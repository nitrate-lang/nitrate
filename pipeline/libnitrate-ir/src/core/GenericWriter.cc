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
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Writer.hh>
#include <nitrate-lexer/Lexer.hh>

using namespace nr;

void NR_Writer::visit(Expr& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  (void)m_include_source_location;

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Type& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(BinExpr& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(UnExpr& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(PostUnExpr& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(U1Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(U8Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(U16Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(U32Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(U64Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(U128Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(I8Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(I16Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(I32Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(I64Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(I128Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(F16Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(F32Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(F64Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(F128Ty& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(VoidTy& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(PtrTy& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(ConstTy& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(OpaqueTy& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(StructTy& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(UnionTy& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(ArrayTy& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(FnTy& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Int& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Float& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(List& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Call& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Seq& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Index& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Ident& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Extern& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Local& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Ret& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Brk& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Cont& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(If& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(While& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(For& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Case& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Switch& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Fn& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Asm& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void NR_Writer::visit(Tmp& n) {
  begin_obj(1);

  string("kind");
  string(n.getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}
