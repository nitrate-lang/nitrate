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
#include <nitrate-parser/Parser.h>

#include <cstddef>
#include <cstring>

using namespace qparse;

///=============================================================================
namespace qparse {
  void ArenaAllocatorImpl::swap(qcore_arena_t &arena) {
    std::swap(*m_arena.get(), arena);
  }

  CPP_EXPORT thread_local ArenaAllocatorImpl qparse_arena;
}  // namespace qparse

C_EXPORT void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;
  return qcore_arena_alloc_ex(m_arena.get(), size, alignment);
}

C_EXPORT void ArenaAllocatorImpl::deallocate(void *ptr) noexcept { (void)ptr; }

///=============================================================================

CPP_EXPORT std::ostream &Node::dump(std::ostream &os,
                                    bool isForDebug) const noexcept {
  (void)isForDebug;

  size_t size = 0;
  char *buf = qparse_repr(this, false, 2, &size);

  os << std::string_view(buf, size);

  return os;
}

///=============================================================================

CPP_EXPORT bool Type::is_ptr_to(Type *type) noexcept {
  if (!is_pointer()) {
    return false;
  }

  Type *item = as<PtrTy>()->get_item();
  while (item->is<RefTy>()) {
    item = item->as<RefTy>()->get_item();
  }

  return item->is(type->getKind());
}

Stmt *qparse::mock_stmt(qparse_ty_t expected) {
  (void)expected;

  static Stmt node(QAST_NODE_NODE);
  return &node;
}

Expr *qparse::mock_expr(qparse_ty_t expected) {
  (void)expected;

  static Expr node(QAST_NODE_NODE);
  return &node;
}

Type *qparse::mock_type(qparse_ty_t expected) {
  (void)expected;

  static Type node(QAST_NODE_NODE);
  return &node;
}

Decl *qparse::mock_decl(qparse_ty_t expected) {
  (void)expected;

  static Decl node(QAST_NODE_NODE);
  return &node;
}

Block *qparse::mock_block(qparse_ty_t expected) {
  (void)expected;

  static Block node;
  return &node;
}
