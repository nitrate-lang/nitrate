////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define __QXIR_IMPL__

#include <quix-core/Error.h>
#include <quix-qxir/Node.h>

#include <cstring>

using namespace qxir;

///=============================================================================
namespace qxir {
  thread_local ArenaAllocatorImpl qxir_arena;
}

ArenaAllocatorImpl::ArenaAllocatorImpl() { qcore_arena_open(&m_arena); }

ArenaAllocatorImpl::~ArenaAllocatorImpl() { qcore_arena_close(&m_arena); }

void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;
  return qcore_arena_alloc_ex(&m_arena, size, alignment);
}

void ArenaAllocatorImpl::deallocate(void *ptr) noexcept { (void)ptr; }

///=============================================================================

uint32_t Expr::thisSizeOf() const noexcept {
#define SIZEOF_ROW(__type) \
  { typeid(__type).hash_code(), sizeof(__type) }

  static const std::unordered_map<size_t, uint32_t> sizes = {
      SIZEOF_ROW(BinExpr),  SIZEOF_ROW(UnExpr),   SIZEOF_ROW(PostUnExpr), SIZEOF_ROW(Int),
      SIZEOF_ROW(Float),    SIZEOF_ROW(String),   SIZEOF_ROW(List),       SIZEOF_ROW(Alloc),
      SIZEOF_ROW(Call),     SIZEOF_ROW(Seq),      SIZEOF_ROW(Async),      SIZEOF_ROW(Index),
      SIZEOF_ROW(Ident),    SIZEOF_ROW(Global),   SIZEOF_ROW(Ret),        SIZEOF_ROW(Brk),
      SIZEOF_ROW(Cont),     SIZEOF_ROW(If),       SIZEOF_ROW(While),      SIZEOF_ROW(For),
      SIZEOF_ROW(Form),     SIZEOF_ROW(Foreach),  SIZEOF_ROW(Case),       SIZEOF_ROW(Switch),
      SIZEOF_ROW(Fn),       SIZEOF_ROW(Asm),      SIZEOF_ROW(U1Ty),       SIZEOF_ROW(U8Ty),
      SIZEOF_ROW(U16Ty),    SIZEOF_ROW(U32Ty),    SIZEOF_ROW(U64Ty),      SIZEOF_ROW(U128Ty),
      SIZEOF_ROW(I8Ty),     SIZEOF_ROW(I16Ty),    SIZEOF_ROW(I32Ty),      SIZEOF_ROW(I64Ty),
      SIZEOF_ROW(I128Ty),   SIZEOF_ROW(F16Ty),    SIZEOF_ROW(F32Ty),      SIZEOF_ROW(F64Ty),
      SIZEOF_ROW(F128Ty),   SIZEOF_ROW(VoidTy),   SIZEOF_ROW(PtrTy),      SIZEOF_ROW(OpaqueTy),
      SIZEOF_ROW(StringTy), SIZEOF_ROW(StructTy), SIZEOF_ROW(UnionTy),    SIZEOF_ROW(ArrayTy),
      SIZEOF_ROW(ListTy),   SIZEOF_ROW(IntrinTy), SIZEOF_ROW(FnTy),
  };

  qcore_assert(sizes.size() == QIR_NODE_COUNT, "Polymorphic type size lookup table is incomplete");

  size_t id = typeid(*this).hash_code();
  qcore_assert(sizes.contains(id));

  return sizes.at(id);
}

qxir_ty_t Expr::thisTypeId() const noexcept { return m_node_type; }

const char *Expr::thisTypeName() const noexcept {
#define NAMEOF_ROW(__name) \
  { QIR_NODE_##__name, "QIR_NODE_" #__name }

  static const std::unordered_map<qxir_ty_t, const char *> names = {
      NAMEOF_ROW(BINEXPR),   NAMEOF_ROW(UNEXPR),    NAMEOF_ROW(POST_UNEXPR), NAMEOF_ROW(INT),
      NAMEOF_ROW(FLOAT),     NAMEOF_ROW(STRING),    NAMEOF_ROW(LIST),        NAMEOF_ROW(ALLOC),
      NAMEOF_ROW(CALL),      NAMEOF_ROW(SEQ),       NAMEOF_ROW(ASYNC),       NAMEOF_ROW(INDEX),
      NAMEOF_ROW(IDENT),     NAMEOF_ROW(GLOBAL),    NAMEOF_ROW(RET),         NAMEOF_ROW(BRK),
      NAMEOF_ROW(CONT),      NAMEOF_ROW(IF),        NAMEOF_ROW(WHILE),       NAMEOF_ROW(FOR),
      NAMEOF_ROW(FORM),      NAMEOF_ROW(FOREACH),   NAMEOF_ROW(CASE),        NAMEOF_ROW(SWITCH),
      NAMEOF_ROW(FN),        NAMEOF_ROW(ASM),       NAMEOF_ROW(U1_TY),       NAMEOF_ROW(U8_TY),
      NAMEOF_ROW(U16_TY),    NAMEOF_ROW(U32_TY),    NAMEOF_ROW(U64_TY),      NAMEOF_ROW(U128_TY),
      NAMEOF_ROW(I8_TY),     NAMEOF_ROW(I16_TY),    NAMEOF_ROW(I32_TY),      NAMEOF_ROW(I64_TY),
      NAMEOF_ROW(I128_TY),   NAMEOF_ROW(F16_TY),    NAMEOF_ROW(F32_TY),      NAMEOF_ROW(F64_TY),
      NAMEOF_ROW(F128_TY),   NAMEOF_ROW(VOID_TY),   NAMEOF_ROW(PTR_TY),      NAMEOF_ROW(OPAQUE_TY),
      NAMEOF_ROW(STRING_TY), NAMEOF_ROW(STRUCT_TY), NAMEOF_ROW(UNION_TY),    NAMEOF_ROW(ARRAY_TY),
      NAMEOF_ROW(LIST_TY),   NAMEOF_ROW(INTRIN_TY), NAMEOF_ROW(FN_TY),
  };

  qxir_ty_t type = thisTypeId();

  qcore_assert(names.size() == QIR_NODE_COUNT, "Polymorphic type size lookup table is incomplete");
  qcore_assert(names.contains(type));

  return names.at(type);
}

bool Expr::isType() const noexcept {
  switch (thisTypeId()) {
    case QIR_NODE_U1_TY:
    case QIR_NODE_U8_TY:
    case QIR_NODE_U16_TY:
    case QIR_NODE_U32_TY:
    case QIR_NODE_U64_TY:
    case QIR_NODE_U128_TY:
    case QIR_NODE_I8_TY:
    case QIR_NODE_I16_TY:
    case QIR_NODE_I32_TY:
    case QIR_NODE_I64_TY:
    case QIR_NODE_I128_TY:
    case QIR_NODE_F16_TY:
    case QIR_NODE_F32_TY:
    case QIR_NODE_F64_TY:
    case QIR_NODE_F128_TY:
    case QIR_NODE_VOID_TY:
    case QIR_NODE_PTR_TY:
    case QIR_NODE_OPAQUE_TY:
    case QIR_NODE_STRING_TY:
    case QIR_NODE_STRUCT_TY:
    case QIR_NODE_UNION_TY:
    case QIR_NODE_ARRAY_TY:
    case QIR_NODE_LIST_TY:
    case QIR_NODE_INTRIN_TY:
    case QIR_NODE_FN_TY:
      return true;
    default:
      return false;
  }
}

qxir::Type *qxir::Expr::getType() noexcept { return getModule()->getType(m_type_idx); }

bool Expr::is(qxir_ty_t type) const noexcept { return type == thisTypeId(); }

void qxir::Expr::dump(std::ostream &os, bool isForDebug) const {
  (void)isForDebug;
  (void)os;
  /// TODO:
  throw std::runtime_error("qxir::Expr::dump() not implemented");
}

boost::uuids::uuid qxir::Expr::hash() noexcept {
  /// TODO:
  qcore_panic("qxir::Expr::hash() not implemented");
}

qxir::Module *qxir::Expr::getModule() noexcept { return ::getModule(m_module_idx); }
