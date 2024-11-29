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
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <cstring>

#include "LibMacro.h"

using namespace qparse;

///=============================================================================
namespace qparse {
  void ArenaAllocatorImpl::swap(qcore_arena_t &arena) {
    std::swap(*m_arena.get(), arena);
  }

  CPP_EXPORT thread_local ArenaAllocatorImpl qparse_arena;
}  // namespace qparse

LIB_EXPORT void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;
  return qcore_arena_alloc_ex(m_arena.get(), size, alignment);
}

LIB_EXPORT void ArenaAllocatorImpl::deallocate(void *ptr) noexcept {
  (void)ptr;
}

///=============================================================================

LIB_EXPORT bool Node::is_type() {
  switch (getKind()) {
    case QAST_NODE_REF_TY:
    case QAST_NODE_U1_TY:
    case QAST_NODE_U8_TY:
    case QAST_NODE_U16_TY:
    case QAST_NODE_U32_TY:
    case QAST_NODE_U64_TY:
    case QAST_NODE_U128_TY:
    case QAST_NODE_I8_TY:
    case QAST_NODE_I16_TY:
    case QAST_NODE_I32_TY:
    case QAST_NODE_I64_TY:
    case QAST_NODE_I128_TY:
    case QAST_NODE_F16_TY:
    case QAST_NODE_F32_TY:
    case QAST_NODE_F64_TY:
    case QAST_NODE_F128_TY:
    case QAST_NODE_VOID_TY:
    case QAST_NODE_PTR_TY:
    case QAST_NODE_OPAQUE_TY:
    case QAST_NODE_STRUCT_TY:
    case QAST_NODE_ARRAY_TY:
    case QAST_NODE_TUPLE_TY:
    case QAST_NODE_FN_TY:
    case QAST_NODE_UNRES_TY:
    case QAST_NODE_INFER_TY:
    case QAST_NODE_TEMPL_TY:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Node::is_stmt() {
  switch (getKind()) {
    case QAST_NODE_TYPEDEF:
    case QAST_NODE_FNDECL:
    case QAST_NODE_STRUCT:
    case QAST_NODE_ENUM:
    case QAST_NODE_FN:
    case QAST_NODE_SUBSYSTEM:
    case QAST_NODE_EXPORT:
    case QAST_NODE_COMPOSITE_FIELD:
    case QAST_NODE_BLOCK:
    case QAST_NODE_VOLSTMT:
    case QAST_NODE_CONST:
    case QAST_NODE_VAR:
    case QAST_NODE_LET:
    case QAST_NODE_INLINE_ASM:
    case QAST_NODE_RETURN:
    case QAST_NODE_RETIF:
    case QAST_NODE_BREAK:
    case QAST_NODE_CONTINUE:
    case QAST_NODE_IF:
    case QAST_NODE_WHILE:
    case QAST_NODE_FOR:
    case QAST_NODE_FOREACH:
    case QAST_NODE_CASE:
    case QAST_NODE_SWITCH:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Node::is_decl() {
  switch (getKind()) {
    case QAST_NODE_CONST:
    case QAST_NODE_VAR:
    case QAST_NODE_LET:
    case QAST_NODE_TYPEDEF:
    case QAST_NODE_FNDECL:
    case QAST_NODE_FN:
    case QAST_NODE_COMPOSITE_FIELD:
    case QAST_NODE_STRUCT:
    case QAST_NODE_ENUM:
    case QAST_NODE_SUBSYSTEM:
    case QAST_NODE_EXPORT:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Node::is_expr() {
  switch (getKind()) {
    case QAST_NODE_BINEXPR:
    case QAST_NODE_UNEXPR:
    case QAST_NODE_TEREXPR:
    case QAST_NODE_INT:
    case QAST_NODE_FLOAT:
    case QAST_NODE_STRING:
    case QAST_NODE_CHAR:
    case QAST_NODE_BOOL:
    case QAST_NODE_NULL:
    case QAST_NODE_UNDEF:
    case QAST_NODE_CALL:
    case QAST_NODE_TEMPL_CALL:
    case QAST_NODE_LIST:
    case QAST_NODE_ASSOC:
    case QAST_NODE_FIELD:
    case QAST_NODE_INDEX:
    case QAST_NODE_SLICE:
    case QAST_NODE_FSTRING:
    case QAST_NODE_IDENT:
      return true;
    default:
      return false;
  }
}

///=============================================================================

LIB_EXPORT bool Type::is_primitive() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_primitive();
  }

  switch (getKind()) {
    case QAST_NODE_U1_TY:
    case QAST_NODE_U8_TY:
    case QAST_NODE_U16_TY:
    case QAST_NODE_U32_TY:
    case QAST_NODE_U64_TY:
    case QAST_NODE_U128_TY:
    case QAST_NODE_I8_TY:
    case QAST_NODE_I16_TY:
    case QAST_NODE_I32_TY:
    case QAST_NODE_I64_TY:
    case QAST_NODE_I128_TY:
    case QAST_NODE_F16_TY:
    case QAST_NODE_F32_TY:
    case QAST_NODE_F64_TY:
    case QAST_NODE_F128_TY:
    case QAST_NODE_VOID_TY:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Type::is_array() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_array();
  }

  return getKind() == QAST_NODE_ARRAY_TY;
}

LIB_EXPORT bool Type::is_tuple() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_tuple();
  }

  return getKind() == QAST_NODE_TUPLE_TY;
}

LIB_EXPORT bool Type::is_pointer() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_pointer();
  }

  return getKind() == QAST_NODE_PTR_TY;
}

LIB_EXPORT bool Type::is_function() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_function();
  }

  return getKind() == QAST_NODE_FN_TY;
}

LIB_EXPORT bool Type::is_composite() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_composite();
  }

  switch (getKind()) {
    case QAST_NODE_ARRAY_TY:
    case QAST_NODE_TUPLE_TY:
    case QAST_NODE_STRUCT_TY:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Type::is_numeric() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_numeric();
  }

  switch (getKind()) {
    case QAST_NODE_U1_TY:
    case QAST_NODE_U8_TY:
    case QAST_NODE_U16_TY:
    case QAST_NODE_U32_TY:
    case QAST_NODE_U64_TY:
    case QAST_NODE_U128_TY:
    case QAST_NODE_I8_TY:
    case QAST_NODE_I16_TY:
    case QAST_NODE_I32_TY:
    case QAST_NODE_I64_TY:
    case QAST_NODE_I128_TY:
    case QAST_NODE_F16_TY:
    case QAST_NODE_F32_TY:
    case QAST_NODE_F64_TY:
    case QAST_NODE_F128_TY:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Type::is_integral() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_integral();
  }

  switch (getKind()) {
    case QAST_NODE_U1_TY:
    case QAST_NODE_U8_TY:
    case QAST_NODE_U16_TY:
    case QAST_NODE_U32_TY:
    case QAST_NODE_U64_TY:
    case QAST_NODE_U128_TY:
    case QAST_NODE_I8_TY:
    case QAST_NODE_I16_TY:
    case QAST_NODE_I32_TY:
    case QAST_NODE_I64_TY:
    case QAST_NODE_I128_TY:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Type::is_floating_point() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_floating_point();
  }

  switch (getKind()) {
    case QAST_NODE_F16_TY:
    case QAST_NODE_F32_TY:
    case QAST_NODE_F64_TY:
    case QAST_NODE_F128_TY:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Type::is_signed() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_signed();
  }

  switch (getKind()) {
    case QAST_NODE_I8_TY:
    case QAST_NODE_I16_TY:
    case QAST_NODE_I32_TY:
    case QAST_NODE_I64_TY:
    case QAST_NODE_I128_TY:
    case QAST_NODE_F16_TY:
    case QAST_NODE_F32_TY:
    case QAST_NODE_F64_TY:
    case QAST_NODE_F128_TY:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Type::is_unsigned() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_unsigned();
  }

  switch (getKind()) {
    case QAST_NODE_U1_TY:
    case QAST_NODE_U8_TY:
    case QAST_NODE_U16_TY:
    case QAST_NODE_U32_TY:
    case QAST_NODE_U64_TY:
    case QAST_NODE_U128_TY:
      return true;
    default:
      return false;
  }
}

LIB_EXPORT bool Type::is_void() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_void();
  }

  return getKind() == QAST_NODE_VOID_TY;
}

LIB_EXPORT bool Type::is_bool() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_bool();
  }

  return getKind() == QAST_NODE_U1_TY;
}

LIB_EXPORT bool Type::is_ref() { return getKind() == QAST_NODE_REF_TY; }

LIB_EXPORT bool Type::is_volatile() { return m_volatile; }

LIB_EXPORT bool Type::is_ptr_to(Type *type) {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_ptr_to(type);
  }

  if (!is_pointer()) {
    return false;
  }

  Type *item = as<PtrTy>()->get_item();
  while (item->is<RefTy>()) {
    item = item->as<RefTy>()->get_item();
  }

  return item->is(type->getKind());
}

///=============================================================================

LIB_EXPORT bool Expr::is_binexpr() { return is<BinExpr>(); }

LIB_EXPORT bool Expr::is_unaryexpr() { return is<UnaryExpr>(); }

LIB_EXPORT bool Expr::is_ternaryexpr() { return is<TernaryExpr>(); }

///=============================================================================

LIB_EXPORT bool FuncTy::is_noreturn() { return m_noreturn; }

///=============================================================================

LIB_EXPORT qparse_node_t *qparse_alloc(qparse_ty_t type, qcore_arena_t *arena) {
  if (!arena) {
    arena = &qparse_arena.get();
  }

  Node *node = nullptr;

  qparse_arena.swap(*arena);

  switch (type) {
    case QAST_NODE_NODE:
      node = Node::get(QAST_NODE_NODE);
      break;
    case QAST_NODE_UNRES_TY:
      node = UnresolvedType::get();
      break;
    case QAST_NODE_INFER_TY:
      node = InferType::get();
      break;
    case QAST_NODE_TEMPL_TY:
      node = TemplType::get();
      break;
    case QAST_NODE_U1_TY:
      node = U1::get();
      break;
    case QAST_NODE_U8_TY:
      node = U8::get();
      break;
    case QAST_NODE_U16_TY:
      node = U16::get();
      break;
    case QAST_NODE_U32_TY:
      node = U32::get();
      break;
    case QAST_NODE_U64_TY:
      node = U64::get();
      break;
    case QAST_NODE_U128_TY:
      node = U128::get();
      break;
    case QAST_NODE_I8_TY:
      node = I8::get();
      break;
    case QAST_NODE_I16_TY:
      node = I16::get();
      break;
    case QAST_NODE_I32_TY:
      node = I32::get();
      break;
    case QAST_NODE_I64_TY:
      node = I64::get();
      break;
    case QAST_NODE_I128_TY:
      node = I128::get();
      break;
    case QAST_NODE_F16_TY:
      node = F16::get();
      break;
    case QAST_NODE_F32_TY:
      node = F32::get();
      break;
    case QAST_NODE_F64_TY:
      node = F64::get();
      break;
    case QAST_NODE_F128_TY:
      node = F128::get();
      break;
    case QAST_NODE_VOID_TY:
      node = VoidTy::get();
      break;
    case QAST_NODE_PTR_TY:
      node = PtrTy::get();
      break;
    case QAST_NODE_OPAQUE_TY:
      node = OpaqueTy::get();
      break;
    case QAST_NODE_TUPLE_TY:
      node = TupleTy::get();
      break;
    case QAST_NODE_ARRAY_TY:
      node = ArrayTy::get();
      break;
    case QAST_NODE_REF_TY:
      node = RefTy::get();
      break;
    case QAST_NODE_STRUCT_TY:
      node = StructTy::get();
      break;
    case QAST_NODE_FN_TY:
      node = FuncTy::get();
      break;
    case QAST_NODE_UNEXPR:
      node = UnaryExpr::get();
      break;
    case QAST_NODE_BINEXPR:
      node = BinExpr::get();
      break;
    case QAST_NODE_POST_UNEXPR:
      node = PostUnaryExpr::get();
      break;
    case QAST_NODE_TEREXPR:
      node = TernaryExpr::get();
      break;
    case QAST_NODE_INT:
      node = ConstInt::get();
      break;
    case QAST_NODE_FLOAT:
      node = ConstFloat::get();
      break;
    case QAST_NODE_BOOL:
      node = ConstBool::get();
      break;
    case QAST_NODE_STRING:
      node = ConstString::get();
      break;
    case QAST_NODE_CHAR:
      node = ConstChar::get();
      break;
    case QAST_NODE_NULL:
      node = ConstNull::get();
      break;
    case QAST_NODE_UNDEF:
      node = ConstUndef::get();
      break;
    case QAST_NODE_CALL:
      node = Call::get();
      break;
    case QAST_NODE_LIST:
      node = List::get();
      break;
    case QAST_NODE_ASSOC:
      node = Assoc::get();
      break;
    case QAST_NODE_FIELD:
      node = Field::get();
      break;
    case QAST_NODE_INDEX:
      node = Index::get();
      break;
    case QAST_NODE_SLICE:
      node = Slice::get();
      break;
    case QAST_NODE_FSTRING:
      node = FString::get();
      break;
    case QAST_NODE_IDENT:
      node = Ident::get();
      break;
    case QAST_NODE_SEQ_POINT:
      node = SeqPoint::get();
      break;
    case QAST_NODE_STMT_EXPR:
      node = StmtExpr::get();
      break;
    case QAST_NODE_TYPE_EXPR:
      node = TypeExpr::get();
      break;
    case QAST_NODE_BLOCK:
      node = Block::get();
      break;
    case QAST_NODE_VOLSTMT:
      node = VolStmt::get();
      break;
    case QAST_NODE_CONST:
      node = ConstDecl::get();
      break;
    case QAST_NODE_VAR:
      node = VarDecl::get();
      break;
    case QAST_NODE_LET:
      node = LetDecl::get();
      break;
    case QAST_NODE_INLINE_ASM:
      node = InlineAsm::get();
      break;
    case QAST_NODE_IF:
      node = IfStmt::get();
      break;
    case QAST_NODE_WHILE:
      node = WhileStmt::get();
      break;
    case QAST_NODE_FOR:
      node = ForStmt::get();
      break;
    case QAST_NODE_FOREACH:
      node = ForeachStmt::get();
      break;
    case QAST_NODE_BREAK:
      node = BreakStmt::get();
      break;
    case QAST_NODE_CONTINUE:
      node = ContinueStmt::get();
      break;
    case QAST_NODE_RETURN:
      node = ReturnStmt::get();
      break;
    case QAST_NODE_RETIF:
      node = ReturnIfStmt::get();
      break;
    case QAST_NODE_CASE:
      node = CaseStmt::get();
      break;
    case QAST_NODE_TEMPL_CALL:
      node = TemplCall::get();
      break;
    case QAST_NODE_SWITCH:
      node = SwitchStmt::get();
      break;
    case QAST_NODE_TYPEDEF:
      node = TypedefDecl::get();
      break;
    case QAST_NODE_FNDECL:
      node = FnDecl::get();
      break;
    case QAST_NODE_FN:
      node = FnDef::get();
      break;
    case QAST_NODE_COMPOSITE_FIELD:
      node = CompositeField::get();
      break;
    case QAST_NODE_STRUCT:
      node = StructDef::get();
      break;
    case QAST_NODE_ENUM:
      node = EnumDef::get();
      break;
    case QAST_NODE_SUBSYSTEM:
      node = SubsystemDecl::get();
      break;
    case QAST_NODE_EXPORT:
      node = ExportDecl::get();
      break;
    case QAST_NODE_EXPR_STMT:
      node = ExprStmt::get();
      break;
  }

  qparse_arena.swap(*arena);

  return node;
}
