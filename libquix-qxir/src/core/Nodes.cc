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

#include <algorithm>
#include <variant>
#define __QUIX_IMPL__
#define __QXIR_NODE_REFLECT_IMPL__  // Make private fields accessible

#include <core/LibMacro.h>
#include <openssl/evp.h>
#include <quix-core/Error.h>
#include <quix-qxir/Inference.h>
#include <quix-qxir/Module.h>
#include <quix-qxir/Node.h>

#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

using namespace qxir;

///=============================================================================
namespace qxir {
  thread_local ArenaAllocatorImpl qxir_arena;
}

void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;
  return qcore_arena_alloc_ex(m_arena.get(), size, alignment);
}

void ArenaAllocatorImpl::deallocate(void *ptr) noexcept { (void)ptr; }

///=============================================================================

CPP_EXPORT uint32_t Expr::getKindSize() const noexcept {
#define SIZEOF_ROW(__type) \
  { typeid(__type).hash_code(), sizeof(__type) }

  static const std::unordered_map<size_t, uint32_t> sizes = {
      SIZEOF_ROW(BinExpr), SIZEOF_ROW(UnExpr),   SIZEOF_ROW(PostUnExpr), SIZEOF_ROW(Int),
      SIZEOF_ROW(Float),   SIZEOF_ROW(String),   SIZEOF_ROW(List),       SIZEOF_ROW(Call),
      SIZEOF_ROW(Seq),     SIZEOF_ROW(Index),    SIZEOF_ROW(Index),      SIZEOF_ROW(Extern),
      SIZEOF_ROW(Local),   SIZEOF_ROW(Ret),      SIZEOF_ROW(Brk),        SIZEOF_ROW(Cont),
      SIZEOF_ROW(If),      SIZEOF_ROW(While),    SIZEOF_ROW(For),        SIZEOF_ROW(Form),
      SIZEOF_ROW(Case),    SIZEOF_ROW(Switch),   SIZEOF_ROW(Fn),         SIZEOF_ROW(Asm),
      SIZEOF_ROW(U1Ty),    SIZEOF_ROW(U8Ty),     SIZEOF_ROW(U16Ty),      SIZEOF_ROW(U32Ty),
      SIZEOF_ROW(U64Ty),   SIZEOF_ROW(U128Ty),   SIZEOF_ROW(I8Ty),       SIZEOF_ROW(I16Ty),
      SIZEOF_ROW(I32Ty),   SIZEOF_ROW(I64Ty),    SIZEOF_ROW(I128Ty),     SIZEOF_ROW(F16Ty),
      SIZEOF_ROW(F32Ty),   SIZEOF_ROW(F64Ty),    SIZEOF_ROW(F128Ty),     SIZEOF_ROW(VoidTy),
      SIZEOF_ROW(PtrTy),   SIZEOF_ROW(OpaqueTy), SIZEOF_ROW(StructTy),   SIZEOF_ROW(UnionTy),
      SIZEOF_ROW(ArrayTy), SIZEOF_ROW(FnTy),     SIZEOF_ROW(Tmp),
  };

  qcore_assert(sizes.size() == QIR_NODE_COUNT, "Polymorphic type size lookup table is incomplete");

  size_t id = typeid(*this).hash_code();
  qcore_assert(sizes.contains(id));

  return sizes.at(id);
}

CPP_EXPORT const char *Expr::getKindName() const noexcept {
#define NAMEOF_ROW(__name) \
  { QIR_NODE_##__name, "QIR_NODE_" #__name }

  static const std::unordered_map<qxir_ty_t, const char *> names = {
      NAMEOF_ROW(BINEXPR),  NAMEOF_ROW(UNEXPR),    NAMEOF_ROW(POST_UNEXPR), NAMEOF_ROW(INT),
      NAMEOF_ROW(FLOAT),    NAMEOF_ROW(STRING),    NAMEOF_ROW(LIST),        NAMEOF_ROW(CALL),
      NAMEOF_ROW(SEQ),      NAMEOF_ROW(INDEX),     NAMEOF_ROW(IDENT),       NAMEOF_ROW(EXTERN),
      NAMEOF_ROW(LOCAL),    NAMEOF_ROW(RET),       NAMEOF_ROW(BRK),         NAMEOF_ROW(CONT),
      NAMEOF_ROW(IF),       NAMEOF_ROW(WHILE),     NAMEOF_ROW(FOR),         NAMEOF_ROW(FORM),
      NAMEOF_ROW(CASE),     NAMEOF_ROW(SWITCH),    NAMEOF_ROW(FN),          NAMEOF_ROW(ASM),
      NAMEOF_ROW(U1_TY),    NAMEOF_ROW(U8_TY),     NAMEOF_ROW(U16_TY),      NAMEOF_ROW(U32_TY),
      NAMEOF_ROW(U64_TY),   NAMEOF_ROW(U128_TY),   NAMEOF_ROW(I8_TY),       NAMEOF_ROW(I16_TY),
      NAMEOF_ROW(I32_TY),   NAMEOF_ROW(I64_TY),    NAMEOF_ROW(I128_TY),     NAMEOF_ROW(F16_TY),
      NAMEOF_ROW(F32_TY),   NAMEOF_ROW(F64_TY),    NAMEOF_ROW(F128_TY),     NAMEOF_ROW(VOID_TY),
      NAMEOF_ROW(PTR_TY),   NAMEOF_ROW(OPAQUE_TY), NAMEOF_ROW(STRUCT_TY),   NAMEOF_ROW(UNION_TY),
      NAMEOF_ROW(ARRAY_TY), NAMEOF_ROW(FN_TY),     NAMEOF_ROW(TMP),
  };

  qxir_ty_t type = getKind();

  qcore_assert(names.size() == QIR_NODE_COUNT, "Polymorphic type size lookup table is incomplete");
  qcore_assert(names.contains(type));

  return names.at(type);
}

CPP_EXPORT bool Expr::isType() const noexcept {
  switch (getKind()) {
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
    case QIR_NODE_STRUCT_TY:
    case QIR_NODE_UNION_TY:
    case QIR_NODE_ARRAY_TY:
    case QIR_NODE_FN_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT qxir::Type *qxir::Expr::getType() noexcept {
  return static_cast<Type *>(qxir_infer(this));
}

CPP_EXPORT bool qxir::Expr::cmp_eq(const qxir::Expr *other) const {
  qxir_ty_t kind = getKind();

  if (kind != other->getKind()) {
    return false;
  }

  switch (kind) {
    case QIR_NODE_BINEXPR: {
      auto a = as<BinExpr>();
      auto b = other->as<BinExpr>();
      if (a->m_op != b->m_op) {
        return false;
      }
      return a->m_lhs->cmp_eq(b->m_lhs) && a->m_rhs->cmp_eq(b->m_rhs);
    }
    case QIR_NODE_UNEXPR: {
      auto a = as<UnExpr>();
      auto b = other->as<UnExpr>();
      if (a->m_op != b->m_op) {
        return false;
      }
      return a->m_expr->cmp_eq(b->m_expr);
    }
    case QIR_NODE_POST_UNEXPR: {
      auto a = as<PostUnExpr>();
      auto b = other->as<PostUnExpr>();
      if (a->m_op != b->m_op) {
        return false;
      }
      return a->m_expr->cmp_eq(b->m_expr);
    }
    case QIR_NODE_INT: {
      return as<Int>()->getValue() == other->as<Int>()->getValue();
    }
    case QIR_NODE_FLOAT: {
      return as<Float>()->getValue() == other->as<Float>()->getValue();
    }
    case QIR_NODE_STRING: {
      return as<String>()->m_data == other->as<String>()->m_data;
    }
    case QIR_NODE_LIST: {
      auto a = as<List>();
      auto b = other->as<List>();
      if (a->m_items.size() != b->m_items.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_items.size(); i++) {
        if (!a->m_items[i]->cmp_eq(b->m_items[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_CALL: {
      auto a = as<Call>();
      auto b = other->as<Call>();
      if (!a->m_iref->cmp_eq(b->m_iref)) {
        return false;
      }
      if (a->m_args.size() != b->m_args.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_args.size(); i++) {
        if (!a->m_args[i]->cmp_eq(b->m_args[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_SEQ: {
      auto a = as<Seq>();
      auto b = other->as<Seq>();
      if (a->m_items.size() != b->m_items.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_items.size(); i++) {
        if (!a->m_items[i]->cmp_eq(b->m_items[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_INDEX: {
      auto a = as<Index>();
      auto b = other->as<Index>();
      if (!a->m_expr->cmp_eq(b->m_expr)) {
        return false;
      }
      if (!a->m_index->cmp_eq(b->m_index)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_IDENT: {
      return as<Ident>()->m_name == other->as<Ident>()->m_name;
    }
    case QIR_NODE_EXTERN: {
      auto a = as<Extern>();
      auto b = other->as<Extern>();
      if (a->m_abi_name != b->m_abi_name) {
        return false;
      }
      return a->m_value->cmp_eq(b->m_value);
    }
    case QIR_NODE_LOCAL: {
      auto a = as<Local>();
      auto b = other->as<Local>();
      if (a->m_name != b->m_name) {
        return false;
      }
      return a->m_value->cmp_eq(b->m_value);
    }
    case QIR_NODE_RET: {
      return as<Ret>()->m_expr->cmp_eq(other->as<Ret>()->m_expr);
    }
    case QIR_NODE_BRK: {
      return true;
    }
    case QIR_NODE_CONT: {
      return true;
    }
    case QIR_NODE_IF: {
      auto a = as<If>();
      auto b = other->as<If>();
      if (!a->m_cond->cmp_eq(b->m_cond)) {
        return false;
      }
      if (!a->m_then->cmp_eq(b->m_then)) {
        return false;
      }
      if (!a->m_else->cmp_eq(b->m_else)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_WHILE: {
      auto a = as<While>();
      auto b = other->as<While>();
      if (!a->m_cond->cmp_eq(b->m_cond)) {
        return false;
      }
      if (!a->m_body->cmp_eq(b->m_body)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_FOR: {
      auto a = as<For>();
      auto b = other->as<For>();
      if (!a->m_init->cmp_eq(b->m_init)) {
        return false;
      }
      if (!a->m_cond->cmp_eq(b->m_cond)) {
        return false;
      }
      if (!a->m_step->cmp_eq(b->m_step)) {
        return false;
      }
      if (!a->m_body->cmp_eq(b->m_body)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_FORM: {
      auto a = as<Form>();
      auto b = other->as<Form>();
      if (!a->m_maxjobs->cmp_eq(b->m_maxjobs)) {
        return false;
      }
      if (a->m_idx_ident != b->m_idx_ident) {
        return false;
      }
      if (a->m_val_ident != b->m_val_ident) {
        return false;
      }
      if (!a->m_expr->cmp_eq(b->m_expr)) {
        return false;
      }
      if (!a->m_body->cmp_eq(b->m_body)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_CASE: {
      auto a = as<Case>();
      auto b = other->as<Case>();
      if (!a->m_cond->cmp_eq(b->m_cond)) {
        return false;
      }
      if (!a->m_body->cmp_eq(b->m_body)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_SWITCH: {
      auto a = as<Switch>();
      auto b = other->as<Switch>();
      if (!a->m_cond->cmp_eq(b->m_cond)) {
        return false;
      }
      if (!a->m_default->cmp_eq(b->m_default)) {
        return false;
      }
      if (a->m_cases.size() != b->m_cases.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_cases.size(); i++) {
        if (!a->m_cases[i]->cmp_eq(b->m_cases[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_FN: {
      auto a = as<Fn>();
      auto b = other->as<Fn>();
      if (a->m_name != b->m_name) {
        return false;
      }
      if (!a->m_return->cmp_eq(b->m_return)) {
        return false;
      }
      if (a->m_params.size() != b->m_params.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_params.size(); i++) {
        if (a->m_params[i].second != b->m_params[i].second) {
          return false;
        }
        if (!a->m_params[i].first->cmp_eq(b->m_params[i].first)) {
          return false;
        }
      }
      if (!a->m_body->cmp_eq(b->m_body)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_ASM: {
      qcore_implement("Expr::cmp_eq for QIR_NODE_ASM");
      break;
    }
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
      return true;
    case QIR_NODE_PTR_TY: {
      return as<PtrTy>()->m_pointee->cmp_eq(other->as<PtrTy>()->m_pointee);
    }
    case QIR_NODE_OPAQUE_TY: {
      return as<OpaqueTy>()->m_name == other->as<OpaqueTy>()->m_name;
    }
    case QIR_NODE_STRUCT_TY: {
      auto a = as<StructTy>();
      auto b = other->as<StructTy>();
      if (a->m_fields.size() != b->m_fields.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_fields.size(); i++) {
        if (!a->m_fields[i]->cmp_eq(b->m_fields[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_UNION_TY: {
      auto a = as<UnionTy>();
      auto b = other->as<UnionTy>();
      if (a->m_fields.size() != b->m_fields.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_fields.size(); i++) {
        if (!a->m_fields[i]->cmp_eq(b->m_fields[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_ARRAY_TY: {
      auto a = as<ArrayTy>();
      auto b = other->as<ArrayTy>();
      if (!a->m_element->cmp_eq(b->m_element)) {
        return false;
      }
      if (a->m_size != b->m_size) {
        return false;
      }
      return true;
    }
    case QIR_NODE_FN_TY: {
      auto a = as<FnTy>();
      auto b = other->as<FnTy>();
      if (a->m_params.size() != b->m_params.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_params.size(); i++) {
        if (!a->m_params[i]->cmp_eq(b->m_params[i])) {
          return false;
        }
      }
      if (!a->m_return->cmp_eq(b->m_return)) {
        return false;
      }
      if (a->m_attrs != b->m_attrs) {
        return false;
      }
      return true;
    }
    case QIR_NODE_TMP: {
      auto a = as<Tmp>();
      auto b = other->as<Tmp>();
      if (a->m_type != b->m_type) {
        return false;
      }
      if (a->m_data != b->m_data) {
        return false;
      }

      throw std::runtime_error("Expr::cmp_eq: attempt to compare fine structure of QIR_NODE_TMP");
      break;
    }
  }

  __builtin_unreachable();
}

static bool isCyclicUtil(qxir::Expr *base, std::unordered_set<qxir::Expr *> &visited,
                         std::unordered_set<qxir::Expr *> &recStack) {
  bool has_cycle = false;

  if (!visited.contains(base)) {
    // Mark the current node as visited
    // and part of recursion stack
    visited.insert(base);
    recStack.insert(base);

    // Recur for all the vertices adjacent
    // to this vertex
    iterate<IterMode::children>(base, [&](qxir::Expr *par, qxir::Expr **cur) -> IterOp {
      if (!visited.contains(*cur) && isCyclicUtil(*cur, visited, recStack)) [[unlikely]] {
        has_cycle = true;
        return IterOp::Abort;
      } else if (recStack.contains(*cur)) [[unlikely]] {
        has_cycle = true;
        return IterOp::Abort;
      }

      return IterOp::Proceed;
    });
  }

  // Remove the vertex from recursion stack
  recStack.erase(base);
  return has_cycle;
}

CPP_EXPORT bool qxir::Expr::is_acyclic() const noexcept {
  std::unordered_set<Expr *> visited, recStack;
  bool has_cycle = false;

  Expr *ptr = const_cast<Expr *>(this);
  iterate<IterMode::children>(ptr, [&](Expr *par, Expr **cur) -> IterOp {
    if (!visited.contains(*cur) && isCyclicUtil(*cur, visited, recStack)) [[unlikely]] {
      has_cycle = true;
      return IterOp::Abort;
    }

    return IterOp::Proceed;
  });

  return !has_cycle;
}

CPP_EXPORT std::string_view qxir::Expr::getName() const noexcept {
  std::string_view R = "";

  switch (this->getKind()) {
    case QIR_NODE_BINEXPR: {
      break;
    }

    case QIR_NODE_UNEXPR: {
      break;
    }

    case QIR_NODE_POST_UNEXPR: {
      break;
    }

    case QIR_NODE_INT: {
      break;
    }

    case QIR_NODE_FLOAT: {
      break;
    }

    case QIR_NODE_STRING: {
      break;
    }

    case QIR_NODE_LIST: {
      break;
    }

    case QIR_NODE_CALL: {
      break;
    }

    case QIR_NODE_SEQ: {
      break;
    }

    case QIR_NODE_INDEX: {
      break;
    }

    case QIR_NODE_IDENT: {
      R = as<Ident>()->m_name;
      break;
    }

    case QIR_NODE_EXTERN: {
      break;
    }

    case QIR_NODE_LOCAL: {
      R = as<Local>()->m_name;
      break;
    }

    case QIR_NODE_RET: {
      break;
    }

    case QIR_NODE_BRK: {
      break;
    }

    case QIR_NODE_CONT: {
      break;
    }

    case QIR_NODE_IF: {
      break;
    }

    case QIR_NODE_WHILE: {
      break;
    }

    case QIR_NODE_FOR: {
      break;
    }

    case QIR_NODE_FORM: {
      break;
    }

    case QIR_NODE_CASE: {
      break;
    }

    case QIR_NODE_SWITCH: {
      break;
    }

    case QIR_NODE_FN: {
      R = as<Fn>()->m_name;
      break;
    }

    case QIR_NODE_ASM: {
      qcore_implement("Expr::getName for QIR_NODE_ASM");
      break;
    }

    case QIR_NODE_U1_TY: {
      break;
    }

    case QIR_NODE_U8_TY: {
      break;
    }

    case QIR_NODE_U16_TY: {
      break;
    }

    case QIR_NODE_U32_TY: {
      break;
    }

    case QIR_NODE_U64_TY: {
      break;
    }

    case QIR_NODE_U128_TY: {
      break;
    }

    case QIR_NODE_I8_TY: {
      break;
    }

    case QIR_NODE_I16_TY: {
      break;
    }

    case QIR_NODE_I32_TY: {
      break;
    }

    case QIR_NODE_I64_TY: {
      break;
    }

    case QIR_NODE_I128_TY: {
      break;
    }

    case QIR_NODE_F16_TY: {
      break;
    }

    case QIR_NODE_F32_TY: {
      break;
    }

    case QIR_NODE_F64_TY: {
      break;
    }

    case QIR_NODE_F128_TY: {
      break;
    }

    case QIR_NODE_VOID_TY: {
      break;
    }

    case QIR_NODE_PTR_TY: {
      break;
    }

    case QIR_NODE_OPAQUE_TY: {
      R = as<OpaqueTy>()->m_name;
      break;
    }

    case QIR_NODE_STRUCT_TY: {
      break;
    }

    case QIR_NODE_UNION_TY: {
      break;
    }

    case QIR_NODE_ARRAY_TY: {
      break;
    }

    case QIR_NODE_FN_TY: {
      break;
    }

    case QIR_NODE_TMP: {
      break;
    }
  }

  return R;
}

CPP_EXPORT std::pair<qlex_loc_t, qlex_loc_t> qxir::Expr::getLoc() const noexcept {
  qmodule_t *mod = getModule();
  qcore_assert(mod != nullptr, "Module is not set");

  qlex_t *lexer = mod->getLexer();
  qcore_assert(lexer != nullptr, "Lexer is not set");

  return {m_start_loc, qlex_offset(lexer, m_start_loc, m_loc_size)};
}

CPP_EXPORT qlex_loc_t qxir::Expr::locBeg() const noexcept {
  qmodule_t *mod = getModule();
  qcore_assert(mod != nullptr, "Module is not set");

  qlex_t *lexer = mod->getLexer();
  qcore_assert(lexer != nullptr, "Lexer is not set");

  return m_start_loc;
}

CPP_EXPORT qlex_loc_t qxir::Expr::locEnd() const noexcept {
  qmodule_t *mod = getModule();
  qcore_assert(mod != nullptr, "Module is not set");

  qlex_t *lexer = mod->getLexer();
  qcore_assert(lexer != nullptr, "Lexer is not set");

  return qlex_offset(lexer, m_start_loc, m_loc_size);
}

CPP_EXPORT void qxir::Expr::setLoc(std::pair<qlex_loc_t, qlex_loc_t> loc) noexcept {
  qmodule_t *mod = getModule();
  qcore_assert(mod != nullptr, "Module is not set");

  qlex_t *lexer = mod->getLexer();
  qcore_assert(lexer != nullptr, "Lexer is not set");

  m_start_loc = loc.first;
  m_loc_size = qlex_span(lexer, loc.first, loc.second);
}

CPP_EXPORT bool Expr::is(qxir_ty_t type) const noexcept { return type == getKind(); }

CPP_EXPORT void qxir::Expr::dump(std::ostream &os, bool isForDebug) const {
  (void)isForDebug;

  char *cstr = nullptr;
  size_t len = 0;

  FILE *fmembuf = open_memstream(&cstr, &len);
  if (!qxir_write(this, QXIR_SERIAL_CODE_MIN, fmembuf, nullptr, 0)) {
    qcore_panic("Failed to dump expression");
  }
  fflush(fmembuf);

  os.write(cstr, len);

  fclose(fmembuf);
  free(cstr);
}

CPP_EXPORT boost::uuids::uuid qxir::Expr::hash() noexcept {
  const EVP_MD *md = EVP_sha256();
  std::array<uint8_t, EVP_MAX_MD_SIZE> hash;

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (!ctx) {
    qcore_panic("Failed to create EVP_MD_CTX");
  }

  if (EVP_DigestInit(ctx, md) != 1) {
    qcore_panic("Failed to initialize EVP_MD_CTX");
  }

  Expr *ptr = this;
  iterate<IterMode::dfs_pre>(ptr, [ctx](Expr *, Expr **_cur) -> IterOp {
    Expr *cur = *_cur;
    uint8_t kind = static_cast<uint8_t>(cur->getKind());

    if (EVP_DigestUpdate(ctx, &kind, sizeof(kind)) != 1) {
      qcore_panic("Failed to update EVP_MD_CTX");
    }

#define MIXIN_PRIMITIVE(x) EVP_DigestUpdate(ctx, &x, sizeof(x))
#define MIXIN_STRING(x) EVP_DigestUpdate(ctx, x.data(), x.size())

    switch (kind) {
      case QIR_NODE_BINEXPR: {
        MIXIN_PRIMITIVE(cur->as<BinExpr>()->m_op);
        break;
      }
      case QIR_NODE_UNEXPR: {
        MIXIN_PRIMITIVE(cur->as<UnExpr>()->m_op);
        break;
      }
      case QIR_NODE_POST_UNEXPR: {
        MIXIN_PRIMITIVE(cur->as<PostUnExpr>()->m_op);
        break;
      }
      case QIR_NODE_INT: {
        MIXIN_STRING(cur->as<Int>()->getValue());
        break;
      }
      case QIR_NODE_FLOAT: {
        MIXIN_STRING(cur->as<Float>()->getValue());
        break;
      }
      case QIR_NODE_STRING: {
        MIXIN_STRING(cur->as<String>()->m_data);
        break;
      }
      case QIR_NODE_LIST: {
        break;
      }
      case QIR_NODE_CALL: {
        break;
      }
      case QIR_NODE_SEQ: {
        break;
      }
      case QIR_NODE_INDEX: {
        break;
      }
      case QIR_NODE_IDENT: {
        MIXIN_STRING(cur->as<Ident>()->m_name);
        break;
      }
      case QIR_NODE_EXTERN: {
        MIXIN_STRING(cur->as<Extern>()->m_abi_name);
        break;
      }
      case QIR_NODE_LOCAL: {
        MIXIN_STRING(cur->as<Local>()->m_name);
        break;
      }
      case QIR_NODE_RET: {
        break;
      }
      case QIR_NODE_BRK: {
        break;
      }
      case QIR_NODE_CONT: {
        break;
      }
      case QIR_NODE_IF: {
        break;
      }
      case QIR_NODE_WHILE: {
        break;
      }
      case QIR_NODE_FOR: {
        break;
      }
      case QIR_NODE_FORM: {
        MIXIN_STRING(cur->as<Form>()->m_idx_ident);
        MIXIN_STRING(cur->as<Form>()->m_val_ident);
        break;
      }
      case QIR_NODE_CASE: {
        break;
      }
      case QIR_NODE_SWITCH: {
        break;
      }
      case QIR_NODE_FN: {
        MIXIN_STRING(cur->as<Fn>()->m_name);
        break;
      }
      case QIR_NODE_ASM: {
        qcore_implement("QIR_NODE_ASM node hashing");
        break;
      }
      case QIR_NODE_U1_TY: {
        break;
      }
      case QIR_NODE_U8_TY: {
        break;
      }
      case QIR_NODE_U16_TY: {
        break;
      }
      case QIR_NODE_U32_TY: {
        break;
      }
      case QIR_NODE_U64_TY: {
        break;
      }
      case QIR_NODE_U128_TY: {
        break;
      }
      case QIR_NODE_I8_TY: {
        break;
      }
      case QIR_NODE_I16_TY: {
        break;
      }
      case QIR_NODE_I32_TY: {
        break;
      }
      case QIR_NODE_I64_TY: {
        break;
      }
      case QIR_NODE_I128_TY: {
        break;
      }
      case QIR_NODE_F16_TY: {
        break;
      }
      case QIR_NODE_F32_TY: {
        break;
      }
      case QIR_NODE_F64_TY: {
        break;
      }
      case QIR_NODE_F128_TY: {
        break;
      }
      case QIR_NODE_VOID_TY: {
        break;
      }
      case QIR_NODE_PTR_TY: {
        break;
      }
      case QIR_NODE_OPAQUE_TY: {
        MIXIN_STRING(cur->as<OpaqueTy>()->m_name);
        break;
      }
      case QIR_NODE_STRUCT_TY: {
        break;
      }
      case QIR_NODE_UNION_TY: {
        break;
      }
      case QIR_NODE_ARRAY_TY: {
        break;
      }
      case QIR_NODE_FN_TY: {
        std::set<FnAttr> tags;
        for (auto &tag : cur->as<FnTy>()->m_attrs) {
          tags.insert(tag);
        }
        for (auto tag : tags) {
          MIXIN_PRIMITIVE(tag);
        }
        break;
      }
      case QIR_NODE_TMP: {
        MIXIN_PRIMITIVE(cur->as<Tmp>()->m_type);

        if (std::holds_alternative<LetTmpNodeCradle>(cur->as<Tmp>()->m_data)) {
          static_assert(std::tuple_size_v<LetTmpNodeCradle> == 2);

          LetTmpNodeCradle &data = std::get<LetTmpNodeCradle>(cur->as<Tmp>()->m_data);
          MIXIN_STRING(std::get<0>(data));
          if (std::get<1>(data) != nullptr) {
            MIXIN_STRING(std::get<1>(data)->getUniqueUUID());
          }
        } else if (std::holds_alternative<CallArgsTmpNodeCradle>(cur->as<Tmp>()->m_data)) {
          static_assert(std::tuple_size_v<CallArgsTmpNodeCradle> == 2);
          CallArgsTmpNodeCradle &data = std::get<CallArgsTmpNodeCradle>(cur->as<Tmp>()->m_data);
          if (std::get<0>(data) != nullptr) {
            MIXIN_STRING(std::get<0>(data)->getUniqueUUID());
          }
          for (auto &arg : std::get<1>(data)) {
            MIXIN_STRING(arg.first);
            MIXIN_STRING(arg.second->getUniqueUUID());
          }
        } else if (std::holds_alternative<FieldTmpNodeCradle>(cur->as<Tmp>()->m_data)) {
          static_assert(std::tuple_size_v<FieldTmpNodeCradle> == 2);
          FieldTmpNodeCradle &data = std::get<FieldTmpNodeCradle>(cur->as<Tmp>()->m_data);
          if (std::get<0>(data) != nullptr) {
            MIXIN_STRING(std::get<0>(data)->getUniqueUUID());
          }
          MIXIN_STRING(std::get<1>(data));
        } else if (std::holds_alternative<std::string_view>(cur->as<Tmp>()->m_data)) {
          std::string_view &data = std::get<std::string_view>(cur->as<Tmp>()->m_data);
          MIXIN_STRING(data);
        } else {
          qcore_panic("Unknown TmpNodeCradle inner type");
        }
        break;
      }
    }

    return IterOp::Proceed;
  });

  if (EVP_DigestFinal_ex(ctx, hash.data(), nullptr) != 1) {
    qcore_panic("Failed to finalize EVP_MD_CTX");
  }

  EVP_MD_CTX_free(ctx);

  boost::uuids::uuid uuid;
  std::memcpy(uuid.data, hash.data(), uuid.size());
  boost::uuids::name_generator gen(uuid);
  return gen("qxir");
}

CPP_EXPORT std::string qxir::Expr::getUniqueUUID() noexcept {
  return boost::uuids::to_string(hash());
}

CPP_EXPORT qmodule_t *qxir::Expr::getModule() const noexcept { return ::getModule(m_module_idx); }

CPP_EXPORT void qxir::Expr::setModule(qmodule_t *module) noexcept {
  m_module_idx = module->getModuleId();
}

CPP_EXPORT uint64_t Expr::getUniqId() const {
  static thread_local std::unordered_map<const Expr *, uint64_t> id_map;
  static thread_local uint64_t last = 0;

  if (id_map.contains(this)) {
    return id_map.at(this);
  }

  for (auto &[key, value] : id_map) {
    if (key->cmp_eq(this)) {
      return value;
    }
  }

  id_map[this] = last;

  return last++;
}

CPP_EXPORT bool Type::is_primitive() const {
  switch (getKind()) {
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
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_array() const { return getKind() == QIR_NODE_ARRAY_TY; }

CPP_EXPORT bool Type::is_pointer() const { return getKind() == QIR_NODE_PTR_TY; }

CPP_EXPORT bool Type::is_function() const { return getKind() == QIR_NODE_FN_TY; }

CPP_EXPORT bool Type::is_composite() const {
  switch (getKind()) {
    case QIR_NODE_STRUCT_TY:
    case QIR_NODE_UNION_TY:
    case QIR_NODE_ARRAY_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_union() const { return getKind() == QIR_NODE_UNION_TY; }

CPP_EXPORT bool Type::is_numeric() const {
  switch (getKind()) {
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
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_integral() const {
  switch (getKind()) {
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
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_floating_point() const {
  switch (getKind()) {
    case QIR_NODE_F16_TY:
    case QIR_NODE_F32_TY:
    case QIR_NODE_F64_TY:
    case QIR_NODE_F128_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_signed() const {
  switch (getKind()) {
    case QIR_NODE_I8_TY:
    case QIR_NODE_I16_TY:
    case QIR_NODE_I32_TY:
    case QIR_NODE_I64_TY:
    case QIR_NODE_I128_TY:
    case QIR_NODE_F16_TY:
    case QIR_NODE_F32_TY:
    case QIR_NODE_F64_TY:
    case QIR_NODE_F128_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_unsigned() const {
  switch (getKind()) {
    case QIR_NODE_U1_TY:
    case QIR_NODE_U8_TY:
    case QIR_NODE_U16_TY:
    case QIR_NODE_U32_TY:
    case QIR_NODE_U64_TY:
    case QIR_NODE_U128_TY:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT bool Type::is_void() const { return getKind() == QIR_NODE_VOID_TY; }

CPP_EXPORT bool Type::is_bool() const { return getKind() == QIR_NODE_U1_TY; }

CPP_EXPORT bool Type::is_ptr_to(const Type *type) const {
  if (is_pointer()) {
    return as<PtrTy>()->m_pointee->cmp_eq(type);
  }

  return false;
}
