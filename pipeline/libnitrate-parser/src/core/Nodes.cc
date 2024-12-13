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
#include <nitrate-parser/Parser.h>

#include <core/Hash.hh>
#include <cstddef>
#include <cstring>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>

using namespace npar;

boost::flyweight<std::string> npar::SaveString(std::string_view str) {
  boost::flyweight<std::string> flyweight(str.data(), str.size());

  return flyweight;
};

///=============================================================================
namespace npar {
  void ArenaAllocatorImpl::swap(qcore_arena_t &arena) {
    std::swap(*m_arena.get(), arena);
  }

  CPP_EXPORT thread_local ArenaAllocatorImpl npar_arena;
}  // namespace npar

C_EXPORT void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;
  return qcore_arena_alloc_ex(m_arena.get(), size, alignment);
}

C_EXPORT void ArenaAllocatorImpl::deallocate(void *ptr) { (void)ptr; }

///=============================================================================

CPP_EXPORT std::ostream &npar_node_t::dump(std::ostream &os,
                                           bool isForDebug) const {
  (void)isForDebug;

  AST_JsonWriter writer(os);
  const_cast<npar_node_t *>(this)->accept(writer);

  return os;
}

CPP_EXPORT uint64_t npar_node_t::hash64() const {
  AST_Hash64 visitor;

  const_cast<npar_node_t *>(this)->accept(visitor);

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
