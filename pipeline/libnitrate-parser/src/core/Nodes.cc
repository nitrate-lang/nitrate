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

#include <core/Hash.hh>
#include <cstring>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <sstream>

using namespace npar;

CPP_EXPORT thread_local std::unique_ptr<ncc::core::IMemory>
    npar::npar_allocator;

///=============================================================================

CPP_EXPORT std::ostream &Base::dump(std::ostream &os, bool isForDebug) const {
  (void)isForDebug;

  AST_JsonWriter writer(os);
  const_cast<Base *>(this)->accept(writer);

  return os;
}

CPP_EXPORT bool Base::isSame(const Base *o) const {
  if (this == o) {
    return true;
  }

  if (getKind() != o->getKind()) {
    return false;
  }

  std::stringstream ss1, ss2;
  AST_MsgPackWriter writer1(ss1, false), writer2(ss2, false);

  return ss1.str() == ss2.str();
}

CPP_EXPORT uint64_t Base::hash64() const {
  AST_Hash64 visitor;

  const_cast<Base *>(this)->accept(visitor);

  return visitor.get();
}

///=============================================================================

CPP_EXPORT bool Type::is_ptr_to(Type *type) const {
  if (!is_pointer()) {
    return false;
  }

  Type *item = as<PtrTy>()->get_item();
  while (item->is<RefTy>()) {
    item = item->as<RefTy>()->get_item();
  }

  return item->is(type->getKind());
}

Stmt *npar::mock_stmt(npar_ty_t expected) {
  (void)expected;

  static Stmt node(QAST_BASE);
  return &node;
}

Expr *npar::mock_expr(npar_ty_t expected) {
  (void)expected;

  static Expr node(QAST_BASE);
  return &node;
}

Type *npar::mock_type() {
  static Type node(QAST_BASE);
  return &node;
}
