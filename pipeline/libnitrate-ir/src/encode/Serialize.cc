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
///   version 2.1 of the License, or (at your option) any later version-> ///
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
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/encode/Serialize.hh>
#include <nitrate-lexer/Lexer.hh>

using namespace ncc::ir::encode;

void IR_Writer::visit(FlowPtr<Expr> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  (void)m_include_source_location;

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<BinExpr> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Unary> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<U1Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<U8Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<U16Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<U32Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<U64Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<U128Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<I8Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<I16Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<I32Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<I64Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<I128Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<F16Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<F32Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<F64Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<F128Ty> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<VoidTy> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<PtrTy> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<ConstTy> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<OpaqueTy> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<StructTy> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<UnionTy> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<ArrayTy> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<FnTy> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Int> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Float> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<List> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Call> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Seq> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Index> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Ident> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Extern> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Local> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Ret> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Brk> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Cont> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<If> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<While> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<For> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Case> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Switch> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Function> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Asm> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}

void IR_Writer::visit(FlowPtr<Tmp> n) {
  begin_obj(1);

  string("kind");
  string(n->getKindName());

  /// TODO: Implement serialization for node

  end_obj();
}
