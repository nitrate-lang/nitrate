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

#define __QXIR_NODE_REFLECT_IMPL__  // Make private fields accessible

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <nitrate-ir/IR.h>
#include <openssl/evp.h>

#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdint>
#include <cstring>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <variant>

using namespace nr;

///=============================================================================
namespace nr {
  thread_local ArenaAllocatorImpl nr_arena;
}

void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;
  return qcore_arena_alloc_ex(m_arena.get(), size, alignment);
}

void ArenaAllocatorImpl::deallocate(void *ptr) noexcept { (void)ptr; }

///=============================================================================

CPP_EXPORT uint32_t Expr::getKindSize(nr_ty_t type) noexcept {
  static const std::unordered_map<nr_ty_t, uint32_t> sizes = {
      {QIR_NODE_BINEXPR, sizeof(BinExpr)},
      {QIR_NODE_UNEXPR, sizeof(UnExpr)},
      {QIR_NODE_POST_UNEXPR, sizeof(PostUnExpr)},
      {QIR_NODE_INT, sizeof(Int)},
      {QIR_NODE_FLOAT, sizeof(Float)},
      {QIR_NODE_LIST, sizeof(List)},
      {QIR_NODE_CALL, sizeof(Call)},
      {QIR_NODE_SEQ, sizeof(Seq)},
      {QIR_NODE_INDEX, sizeof(Index)},
      {QIR_NODE_IDENT, sizeof(Ident)},
      {QIR_NODE_EXTERN, sizeof(Extern)},
      {QIR_NODE_LOCAL, sizeof(Local)},
      {QIR_NODE_RET, sizeof(Ret)},
      {QIR_NODE_BRK, sizeof(Brk)},
      {QIR_NODE_CONT, sizeof(Cont)},
      {QIR_NODE_IF, sizeof(If)},
      {QIR_NODE_WHILE, sizeof(While)},
      {QIR_NODE_FOR, sizeof(For)},
      {QIR_NODE_CASE, sizeof(Case)},
      {QIR_NODE_SWITCH, sizeof(Switch)},
      {QIR_NODE_FN, sizeof(Fn)},
      {QIR_NODE_ASM, sizeof(Asm)},
      {QIR_NODE_IGN, sizeof(Expr)},
      {QIR_NODE_U1_TY, sizeof(U1Ty)},
      {QIR_NODE_U8_TY, sizeof(U8Ty)},
      {QIR_NODE_U16_TY, sizeof(U16Ty)},
      {QIR_NODE_U32_TY, sizeof(U32Ty)},
      {QIR_NODE_U64_TY, sizeof(U64Ty)},
      {QIR_NODE_U128_TY, sizeof(U128Ty)},
      {QIR_NODE_I8_TY, sizeof(I8Ty)},
      {QIR_NODE_I16_TY, sizeof(I16Ty)},
      {QIR_NODE_I32_TY, sizeof(I32Ty)},
      {QIR_NODE_I64_TY, sizeof(I64Ty)},
      {QIR_NODE_I128_TY, sizeof(I128Ty)},
      {QIR_NODE_F16_TY, sizeof(F16Ty)},
      {QIR_NODE_F32_TY, sizeof(F32Ty)},
      {QIR_NODE_F64_TY, sizeof(F64Ty)},
      {QIR_NODE_F128_TY, sizeof(F128Ty)},
      {QIR_NODE_VOID_TY, sizeof(VoidTy)},
      {QIR_NODE_PTR_TY, sizeof(PtrTy)},
      {QIR_NODE_OPAQUE_TY, sizeof(OpaqueTy)},
      {QIR_NODE_STRUCT_TY, sizeof(StructTy)},
      {QIR_NODE_UNION_TY, sizeof(UnionTy)},
      {QIR_NODE_ARRAY_TY, sizeof(ArrayTy)},
      {QIR_NODE_FN_TY, sizeof(FnTy)},
      {QIR_NODE_TMP, sizeof(Tmp)},
  };

  qcore_assert(sizes.size() == QIR_NODE_COUNT,
               "Polymorphic type size lookup table is incomplete");

  return sizes.at(type);
}

CPP_EXPORT const char *Expr::getKindName(nr_ty_t type) noexcept {
  static const std::unordered_map<nr_ty_t, const char *> names = {
      {QIR_NODE_BINEXPR, "bin_expr"},
      {QIR_NODE_UNEXPR, "unary_expr"},
      {QIR_NODE_POST_UNEXPR, "post_unary_expr"},
      {QIR_NODE_INT, "int"},
      {QIR_NODE_FLOAT, "float"},
      {QIR_NODE_LIST, "list"},
      {QIR_NODE_CALL, "call"},
      {QIR_NODE_SEQ, "seq"},
      {QIR_NODE_INDEX, "index"},
      {QIR_NODE_IDENT, "ident"},
      {QIR_NODE_EXTERN, "extern"},
      {QIR_NODE_LOCAL, "local"},
      {QIR_NODE_RET, "return"},
      {QIR_NODE_BRK, "break"},
      {QIR_NODE_CONT, "continue"},
      {QIR_NODE_IF, "if"},
      {QIR_NODE_WHILE, "while"},
      {QIR_NODE_FOR, "for"},
      {QIR_NODE_CASE, "case"},
      {QIR_NODE_SWITCH, "switch"},
      {QIR_NODE_FN, "fn"},
      {QIR_NODE_ASM, "asm"},
      {QIR_NODE_IGN, "ignore"},
      {QIR_NODE_U1_TY, "u1"},
      {QIR_NODE_U8_TY, "u8"},
      {QIR_NODE_U16_TY, "u16"},
      {QIR_NODE_U32_TY, "u32"},
      {QIR_NODE_U64_TY, "u64"},
      {QIR_NODE_U128_TY, "u128"},
      {QIR_NODE_I8_TY, "i8"},
      {QIR_NODE_I16_TY, "i16"},
      {QIR_NODE_I32_TY, "i32"},
      {QIR_NODE_I64_TY, "i64"},
      {QIR_NODE_I128_TY, "i128"},
      {QIR_NODE_F16_TY, "f16"},
      {QIR_NODE_F32_TY, "f32"},
      {QIR_NODE_F64_TY, "f64"},
      {QIR_NODE_F128_TY, "f128"},
      {QIR_NODE_VOID_TY, "void"},
      {QIR_NODE_PTR_TY, "ptr"},
      {QIR_NODE_OPAQUE_TY, "opaque"},
      {QIR_NODE_STRUCT_TY, "struct"},
      {QIR_NODE_UNION_TY, "union"},
      {QIR_NODE_ARRAY_TY, "array"},
      {QIR_NODE_FN_TY, "fn_ty"},
      {QIR_NODE_TMP, "tmp"},
  };

  qcore_assert(names.size() == QIR_NODE_COUNT,
               "Polymorphic type name lookup table is incomplete");

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
    case QIR_NODE_TMP:
      return true;
    default:
      return false;
  }
}

CPP_EXPORT std::optional<nr::Type *> nr::Expr::getType() noexcept {
  /// TODO: Communicate the ptrSizeBytes properly here
  Type *R = static_cast<Type *>(nr_infer(this, 8));

  if (R) {
    return R;
  } else {
    return std::nullopt;
  }
}

CPP_EXPORT bool nr::Expr::isSame(const nr::Expr *other) const {
  nr_ty_t kind = getKind();

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
      return a->m_lhs->isSame(b->m_lhs) && a->m_rhs->isSame(b->m_rhs);
    }
    case QIR_NODE_UNEXPR: {
      auto a = as<UnExpr>();
      auto b = other->as<UnExpr>();
      if (a->m_op != b->m_op) {
        return false;
      }
      return a->m_expr->isSame(b->m_expr);
    }
    case QIR_NODE_POST_UNEXPR: {
      auto a = as<PostUnExpr>();
      auto b = other->as<PostUnExpr>();
      if (a->m_op != b->m_op) {
        return false;
      }
      return a->m_expr->isSame(b->m_expr);
    }
    case QIR_NODE_INT: {
      return as<Int>()->getValue() == other->as<Int>()->getValue();
    }
    case QIR_NODE_FLOAT: {
      return as<Float>()->getValue() == other->as<Float>()->getValue();
    }
    case QIR_NODE_LIST: {
      auto a = as<List>();
      auto b = other->as<List>();
      if (a->m_items.size() != b->m_items.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_items.size(); i++) {
        if (!a->m_items[i]->isSame(b->m_items[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_CALL: {
      auto a = as<Call>();
      auto b = other->as<Call>();
      if (!a->m_iref->isSame(b->m_iref)) {
        return false;
      }
      if (a->m_args.size() != b->m_args.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_args.size(); i++) {
        if (!a->m_args[i]->isSame(b->m_args[i])) {
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
        if (!a->m_items[i]->isSame(b->m_items[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_INDEX: {
      auto a = as<Index>();
      auto b = other->as<Index>();
      if (!a->m_expr->isSame(b->m_expr)) {
        return false;
      }
      if (!a->m_index->isSame(b->m_index)) {
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
      return a->m_value->isSame(b->m_value);
    }
    case QIR_NODE_LOCAL: {
      auto a = as<Local>();
      auto b = other->as<Local>();
      if (a->m_name != b->m_name) {
        return false;
      }
      return a->m_value->isSame(b->m_value);
    }
    case QIR_NODE_RET: {
      return as<Ret>()->m_expr->isSame(other->as<Ret>()->m_expr);
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
      if (!a->m_cond->isSame(b->m_cond)) {
        return false;
      }
      if (!a->m_then->isSame(b->m_then)) {
        return false;
      }
      if (!a->m_else->isSame(b->m_else)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_WHILE: {
      auto a = as<While>();
      auto b = other->as<While>();
      if (!a->m_cond->isSame(b->m_cond)) {
        return false;
      }
      if (!a->m_body->isSame(b->m_body)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_FOR: {
      auto a = as<For>();
      auto b = other->as<For>();
      if (!a->m_init->isSame(b->m_init)) {
        return false;
      }
      if (!a->m_cond->isSame(b->m_cond)) {
        return false;
      }
      if (!a->m_step->isSame(b->m_step)) {
        return false;
      }
      if (!a->m_body->isSame(b->m_body)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_CASE: {
      auto a = as<Case>();
      auto b = other->as<Case>();
      if (!a->m_cond->isSame(b->m_cond)) {
        return false;
      }
      if (!a->m_body->isSame(b->m_body)) {
        return false;
      }
      return true;
    }
    case QIR_NODE_SWITCH: {
      auto a = as<Switch>();
      auto b = other->as<Switch>();
      if (!a->m_cond->isSame(b->m_cond)) {
        return false;
      }
      if (!a->m_default->isSame(b->m_default)) {
        return false;
      }
      if (a->m_cases.size() != b->m_cases.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_cases.size(); i++) {
        if (!a->m_cases[i]->isSame(b->m_cases[i])) {
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
      if (!a->m_return->isSame(b->m_return)) {
        return false;
      }
      if (a->m_params.size() != b->m_params.size()) {
        return false;
      }
      for (size_t i = 0; i < a->m_params.size(); i++) {
        if (a->m_params[i].second != b->m_params[i].second) {
          return false;
        }
        if (!a->m_params[i].first->isSame(b->m_params[i].first)) {
          return false;
        }
      }
      if (a->m_body.has_value() && b->m_body.has_value()) {
        if (!a->m_body.value()->isSame(b->m_body.value())) {
          return false;
        }
      } else if (!a->m_body.has_value() ^ b->m_body.has_value()) {
        return false;
      }
      return true;
    }
    case QIR_NODE_ASM: {
      qcore_implement();
      break;
    }
    case QIR_NODE_IGN: {
      return true;
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
      return as<PtrTy>()->m_pointee->isSame(other->as<PtrTy>()->m_pointee);
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
        if (!a->m_fields[i]->isSame(b->m_fields[i])) {
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
        if (!a->m_fields[i]->isSame(b->m_fields[i])) {
          return false;
        }
      }
      return true;
    }
    case QIR_NODE_ARRAY_TY: {
      auto a = as<ArrayTy>();
      auto b = other->as<ArrayTy>();
      if (!a->m_element->isSame(b->m_element)) {
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
        if (!a->m_params[i]->isSame(b->m_params[i])) {
          return false;
        }
      }
      if (!a->m_return->isSame(b->m_return)) {
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

      qcore_panic(
          "Expr::isSame: attempt to compare fine structure of QIR_NODE_TMP");
    }
  }

  __builtin_unreachable();
}

static bool isCyclicUtil(nr::Expr *base,
                         std::unordered_set<nr::Expr *> &visited,
                         std::unordered_set<nr::Expr *> &recStack) {
  bool has_cycle = false;

  if (!visited.contains(base)) {
    // Mark the current node as visited
    // and part of recursion stack
    visited.insert(base);
    recStack.insert(base);

    // Recur for all the vertices adjacent
    // to this vertex
    iterate<IterMode::children>(
        base, [&](nr::Expr *, nr::Expr **cur) -> IterOp {
          if (!visited.contains(*cur) && isCyclicUtil(*cur, visited, recStack))
              [[unlikely]] {
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

CPP_EXPORT bool nr::Expr::isAcyclic() const noexcept {
  std::unordered_set<Expr *> visited, recStack;
  bool has_cycle = false;

  Expr *ptr = const_cast<Expr *>(this);
  iterate<IterMode::children>(ptr, [&](Expr *, Expr **cur) -> IterOp {
    if (!visited.contains(*cur) && isCyclicUtil(*cur, visited, recStack))
        [[unlikely]] {
      has_cycle = true;
      return IterOp::Abort;
    }

    return IterOp::Proceed;
  });

  return !has_cycle;
}

CPP_EXPORT std::string_view nr::Expr::getName() const noexcept {
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

    case QIR_NODE_CASE: {
      break;
    }

    case QIR_NODE_SWITCH: {
      break;
    }

    case QIR_NODE_IGN: {
      break;
    }

    case QIR_NODE_FN: {
      R = as<Fn>()->m_name;
      break;
    }

    case QIR_NODE_ASM: {
      qcore_implement();
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

CPP_EXPORT std::pair<uint32_t, uint32_t> nr::Expr::getLoc() noexcept {
  return {m_src_offset, m_src_offset + m_span};
}

CPP_EXPORT uint32_t nr::Expr::locBeg() noexcept { return m_src_offset; }

CPP_EXPORT uint32_t nr::Expr::locEnd() noexcept {
  return m_src_offset + m_span;
}

CPP_EXPORT Type *Expr::asType() noexcept {
#ifndef NDEBUG
  if (!isType()) {
    qcore_panicf("Failed to cast a non-type node `%s` to a type node",
                 getKindName());
  }
#endif
  return static_cast<Type *>(this);
}

CPP_EXPORT bool Expr::is(nr_ty_t type) const noexcept {
  return type == getKind();
}

CPP_EXPORT void nr::Expr::dump(std::ostream &os, bool isForDebug) const {
  (void)isForDebug;

  char *cstr = nullptr;
  size_t len = 0;

  FILE *fmembuf = open_memstream(&cstr, &len);
  if (!nr_write(nullptr, this, QXIR_SERIAL_CODE, fmembuf, nullptr, 0)) {
    qcore_panic("Failed to dump expression");
  }
  fflush(fmembuf);

  os.write(cstr, len);

  fclose(fmembuf);
  free(cstr);
}

CPP_EXPORT boost::uuids::uuid nr::Expr::hash() noexcept {
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
        uint128_t x = cur->as<Int>()->getValue();
        MIXIN_PRIMITIVE(x);
        break;
      }
      case QIR_NODE_FLOAT: {
        double v = cur->as<Float>()->getValue();
        FloatSize s = cur->as<Float>()->getSize();
        MIXIN_PRIMITIVE(v);
        MIXIN_PRIMITIVE(s);
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
      case QIR_NODE_CASE: {
        break;
      }
      case QIR_NODE_SWITCH: {
        break;
      }
      case QIR_NODE_IGN: {
        break;
      }
      case QIR_NODE_FN: {
        MIXIN_STRING(cur->as<Fn>()->m_name);
        break;
      }
      case QIR_NODE_ASM: {
        qcore_implement();
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

          LetTmpNodeCradle &data =
              std::get<LetTmpNodeCradle>(cur->as<Tmp>()->m_data);
          MIXIN_STRING(std::get<0>(data));
          if (std::get<1>(data) != nullptr) {
            MIXIN_STRING(std::get<1>(data)->getStateUUID());
          }
        } else if (std::holds_alternative<CallArgsTmpNodeCradle>(
                       cur->as<Tmp>()->m_data)) {
          static_assert(std::tuple_size_v<CallArgsTmpNodeCradle> == 2);
          CallArgsTmpNodeCradle &data =
              std::get<CallArgsTmpNodeCradle>(cur->as<Tmp>()->m_data);
          if (std::get<0>(data) != nullptr) {
            MIXIN_STRING(std::get<0>(data)->getStateUUID());
          }
          for (auto &arg : std::get<1>(data)) {
            MIXIN_STRING(arg.first);
            MIXIN_STRING(arg.second->getStateUUID());
          }
        } else if (std::holds_alternative<FieldTmpNodeCradle>(
                       cur->as<Tmp>()->m_data)) {
          static_assert(std::tuple_size_v<FieldTmpNodeCradle> == 2);
          FieldTmpNodeCradle &data =
              std::get<FieldTmpNodeCradle>(cur->as<Tmp>()->m_data);
          if (std::get<0>(data) != nullptr) {
            MIXIN_STRING(std::get<0>(data)->getStateUUID());
          }
          MIXIN_STRING(std::get<1>(data));
        } else if (std::holds_alternative<std::string_view>(
                       cur->as<Tmp>()->m_data)) {
          std::string_view &data =
              std::get<std::string_view>(cur->as<Tmp>()->m_data);
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
  return gen("nr");
}

CPP_EXPORT uint64_t Expr::getUniqId() const {
  static thread_local std::unordered_map<const Expr *, uint64_t> id_map;
  static thread_local uint64_t last = 0;

  if (id_map.contains(this)) {
    return id_map.at(this);
  }

  for (auto &[key, value] : id_map) {
    if (key->isSame(this)) {
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

CPP_EXPORT bool Type::is_array() const {
  return getKind() == QIR_NODE_ARRAY_TY;
}

CPP_EXPORT bool Type::is_pointer() const {
  return getKind() == QIR_NODE_PTR_TY;
}

CPP_EXPORT bool Type::is_function() const {
  return getKind() == QIR_NODE_FN_TY;
}

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

CPP_EXPORT bool Type::is_union() const {
  return getKind() == QIR_NODE_UNION_TY;
}

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
    return as<PtrTy>()->m_pointee->isSame(type);
  }

  return false;
}

CPP_EXPORT uint128_t Int::str2u128(std::string_view s) noexcept {
  uint128_t x = 0;

  for (char c : s) {
    if (!std::isdigit(c)) {
      qcore_panicf("Failed to convert string `%s` to uint128_t", s.data());
    }

    // Check for overflow
    if (x > (std::numeric_limits<uint128_t>::max() - (c - '0')) / 10) {
      qcore_panicf("Overflow when converting string `%s` to uint128_t",
                   s.data());
    }

    x = x * 10 + (c - '0');
  }

  return x;
}

Expr *nr::createIgn() {
  return new (Arena<Expr>().allocate(1)) Expr(QIR_NODE_IGN);
}
