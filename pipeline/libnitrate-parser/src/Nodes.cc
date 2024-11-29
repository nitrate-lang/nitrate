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
#include <unordered_map>

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

LIB_EXPORT const char *Node::type_name(qparse_ty_t type) {
#define NAMEOF_ROW(__name) \
  { QAST_NODE_##__name, "QAST_NODE_" #__name }

  static const std::unordered_map<qparse_ty_t, const char *> names = {
      NAMEOF_ROW(UNRES_TY),
      NAMEOF_ROW(U1_TY),
      NAMEOF_ROW(U8_TY),
      NAMEOF_ROW(U16_TY),
      NAMEOF_ROW(U32_TY),
      NAMEOF_ROW(U64_TY),
      NAMEOF_ROW(U128_TY),
      NAMEOF_ROW(I8_TY),
      NAMEOF_ROW(I16_TY),
      NAMEOF_ROW(I32_TY),
      NAMEOF_ROW(I64_TY),
      NAMEOF_ROW(I128_TY),
      NAMEOF_ROW(F16_TY),
      NAMEOF_ROW(F32_TY),
      NAMEOF_ROW(F64_TY),
      NAMEOF_ROW(F128_TY),
      NAMEOF_ROW(VOID_TY),
      NAMEOF_ROW(PTR_TY),
      NAMEOF_ROW(OPAQUE_TY),
      NAMEOF_ROW(TUPLE_TY),
      NAMEOF_ROW(ARRAY_TY),
      NAMEOF_ROW(REF_TY),
      NAMEOF_ROW(STRUCT_TY),
      NAMEOF_ROW(FN_TY),
      NAMEOF_ROW(UNEXPR),
      NAMEOF_ROW(BINEXPR),
      NAMEOF_ROW(POST_UNEXPR),
      NAMEOF_ROW(TEREXPR),
      NAMEOF_ROW(INT),
      NAMEOF_ROW(FLOAT),
      NAMEOF_ROW(BOOL),
      NAMEOF_ROW(STRING),
      NAMEOF_ROW(CHAR),
      NAMEOF_ROW(NULL),
      NAMEOF_ROW(UNDEF),
      NAMEOF_ROW(CALL),
      NAMEOF_ROW(TEMPL_CALL),
      NAMEOF_ROW(LIST),
      NAMEOF_ROW(ASSOC),
      NAMEOF_ROW(FIELD),
      NAMEOF_ROW(INDEX),
      NAMEOF_ROW(SLICE),
      NAMEOF_ROW(FSTRING),
      NAMEOF_ROW(IDENT),
      NAMEOF_ROW(SEQ_POINT),
      NAMEOF_ROW(STMT_EXPR),
      NAMEOF_ROW(TYPE_EXPR),
      NAMEOF_ROW(BLOCK),
      NAMEOF_ROW(VOLSTMT),
      NAMEOF_ROW(CONST),
      NAMEOF_ROW(VAR),
      NAMEOF_ROW(LET),
      NAMEOF_ROW(INLINE_ASM),
      NAMEOF_ROW(IF),
      NAMEOF_ROW(WHILE),
      NAMEOF_ROW(FOR),
      NAMEOF_ROW(FOREACH),
      NAMEOF_ROW(BREAK),
      NAMEOF_ROW(CONTINUE),
      NAMEOF_ROW(RETURN),
      NAMEOF_ROW(RETIF),
      NAMEOF_ROW(CASE),
      NAMEOF_ROW(SWITCH),
      NAMEOF_ROW(TYPEDEF),
      NAMEOF_ROW(FNDECL),
      NAMEOF_ROW(FN),
      NAMEOF_ROW(COMPOSITE_FIELD),
      NAMEOF_ROW(STRUCT),
      NAMEOF_ROW(ENUM),
      NAMEOF_ROW(SUBSYSTEM),
      NAMEOF_ROW(EXPORT),
      NAMEOF_ROW(EXPR_STMT),
  };

  qcore_assert(names.size() == QAST_NODE_COUNT,
               "Polymorphic type size lookup table is incomplete");
  qcore_assert(names.contains(type));

  return names.at(type);
}

LIB_EXPORT uint32_t Node::this_sizeof() {
#define SIZEOF_ROW(__type) \
  { typeid(__type).hash_code(), sizeof(__type) }

  static const std::unordered_map<size_t, uint32_t> sizes = {
      SIZEOF_ROW(Stmt),
      SIZEOF_ROW(Type),
      SIZEOF_ROW(Decl),
      SIZEOF_ROW(Expr),
      SIZEOF_ROW(UnresolvedType),
      SIZEOF_ROW(InferType),
      SIZEOF_ROW(TemplType),
      SIZEOF_ROW(U1),
      SIZEOF_ROW(U8),
      SIZEOF_ROW(U16),
      SIZEOF_ROW(U32),
      SIZEOF_ROW(U64),
      SIZEOF_ROW(U128),
      SIZEOF_ROW(I8),
      SIZEOF_ROW(I16),
      SIZEOF_ROW(I32),
      SIZEOF_ROW(I64),
      SIZEOF_ROW(I128),
      SIZEOF_ROW(F16),
      SIZEOF_ROW(F32),
      SIZEOF_ROW(F64),
      SIZEOF_ROW(F128),
      SIZEOF_ROW(VoidTy),
      SIZEOF_ROW(PtrTy),
      SIZEOF_ROW(OpaqueTy),
      SIZEOF_ROW(TupleTy),
      SIZEOF_ROW(ArrayTy),
      SIZEOF_ROW(RefTy),
      SIZEOF_ROW(StructTy),
      SIZEOF_ROW(FuncTy),
      SIZEOF_ROW(UnaryExpr),
      SIZEOF_ROW(BinExpr),
      SIZEOF_ROW(PostUnaryExpr),
      SIZEOF_ROW(TernaryExpr),
      SIZEOF_ROW(ConstInt),
      SIZEOF_ROW(ConstFloat),
      SIZEOF_ROW(ConstBool),
      SIZEOF_ROW(ConstString),
      SIZEOF_ROW(ConstChar),
      SIZEOF_ROW(ConstNull),
      SIZEOF_ROW(ConstUndef),
      SIZEOF_ROW(Call),
      SIZEOF_ROW(TemplCall),
      SIZEOF_ROW(List),
      SIZEOF_ROW(Assoc),
      SIZEOF_ROW(Field),
      SIZEOF_ROW(Index),
      SIZEOF_ROW(Slice),
      SIZEOF_ROW(FString),
      SIZEOF_ROW(Ident),
      SIZEOF_ROW(SeqPoint),
      SIZEOF_ROW(StmtExpr),
      SIZEOF_ROW(TypeExpr),
      SIZEOF_ROW(Block),
      SIZEOF_ROW(VolStmt),
      SIZEOF_ROW(ConstDecl),
      SIZEOF_ROW(VarDecl),
      SIZEOF_ROW(LetDecl),
      SIZEOF_ROW(InlineAsm),
      SIZEOF_ROW(IfStmt),
      SIZEOF_ROW(WhileStmt),
      SIZEOF_ROW(ForStmt),
      SIZEOF_ROW(ForeachStmt),
      SIZEOF_ROW(BreakStmt),
      SIZEOF_ROW(ContinueStmt),
      SIZEOF_ROW(ReturnStmt),
      SIZEOF_ROW(ReturnIfStmt),
      SIZEOF_ROW(CaseStmt),
      SIZEOF_ROW(SwitchStmt),
      SIZEOF_ROW(TypedefDecl),
      SIZEOF_ROW(FnDecl),
      SIZEOF_ROW(FnDef),
      SIZEOF_ROW(CompositeField),
      SIZEOF_ROW(StructDef),
      SIZEOF_ROW(EnumDef),
      SIZEOF_ROW(SubsystemDecl),
      SIZEOF_ROW(ExportDecl),
      SIZEOF_ROW(ExprStmt),
  };

  qcore_assert(sizes.size() == QAST_NODE_COUNT,
               "Polymorphic type size lookup table is incomplete");

  size_t id = typeid(*this).hash_code();
  qcore_assert(sizes.contains(id));

  return sizes.at(id);
}

LIB_EXPORT qparse_ty_t Node::this_typeid() {
#define TYPEID_ROW(__type, __name) \
  { typeid(__type).hash_code(), QAST_NODE_##__name }

  static const std::unordered_map<size_t, qparse_ty_t> typeid_map = {
      TYPEID_ROW(UnresolvedType, UNRES_TY),
      TYPEID_ROW(InferType, INFER_TY),
      TYPEID_ROW(TemplType, TEMPL_TY),
      TYPEID_ROW(U1, U1_TY),
      TYPEID_ROW(U8, U8_TY),
      TYPEID_ROW(U16, U16_TY),
      TYPEID_ROW(U32, U32_TY),
      TYPEID_ROW(U64, U64_TY),
      TYPEID_ROW(U128, U128_TY),
      TYPEID_ROW(I8, I8_TY),
      TYPEID_ROW(I16, I16_TY),
      TYPEID_ROW(I32, I32_TY),
      TYPEID_ROW(I64, I64_TY),
      TYPEID_ROW(I128, I128_TY),
      TYPEID_ROW(F16, F16_TY),
      TYPEID_ROW(F32, F32_TY),
      TYPEID_ROW(F64, F64_TY),
      TYPEID_ROW(F128, F128_TY),
      TYPEID_ROW(VoidTy, VOID_TY),
      TYPEID_ROW(PtrTy, PTR_TY),
      TYPEID_ROW(OpaqueTy, OPAQUE_TY),
      TYPEID_ROW(TupleTy, TUPLE_TY),
      TYPEID_ROW(ArrayTy, ARRAY_TY),
      TYPEID_ROW(RefTy, REF_TY),
      TYPEID_ROW(StructTy, STRUCT_TY),
      TYPEID_ROW(FuncTy, FN_TY),
      TYPEID_ROW(UnaryExpr, UNEXPR),
      TYPEID_ROW(BinExpr, BINEXPR),
      TYPEID_ROW(PostUnaryExpr, POST_UNEXPR),
      TYPEID_ROW(TernaryExpr, TEREXPR),
      TYPEID_ROW(ConstInt, INT),
      TYPEID_ROW(ConstFloat, FLOAT),
      TYPEID_ROW(ConstBool, BOOL),
      TYPEID_ROW(ConstString, STRING),
      TYPEID_ROW(ConstChar, CHAR),
      TYPEID_ROW(ConstNull, NULL),
      TYPEID_ROW(ConstUndef, UNDEF),
      TYPEID_ROW(Call, CALL),
      TYPEID_ROW(TemplCall, TEMPL_CALL),
      TYPEID_ROW(List, LIST),
      TYPEID_ROW(Assoc, ASSOC),
      TYPEID_ROW(Field, FIELD),
      TYPEID_ROW(Index, INDEX),
      TYPEID_ROW(Slice, SLICE),
      TYPEID_ROW(FString, FSTRING),
      TYPEID_ROW(Ident, IDENT),
      TYPEID_ROW(SeqPoint, SEQ_POINT),
      TYPEID_ROW(StmtExpr, STMT_EXPR),
      TYPEID_ROW(TypeExpr, TYPE_EXPR),
      TYPEID_ROW(Block, BLOCK),
      TYPEID_ROW(VolStmt, VOLSTMT),
      TYPEID_ROW(ConstDecl, CONST),
      TYPEID_ROW(VarDecl, VAR),
      TYPEID_ROW(LetDecl, LET),
      TYPEID_ROW(InlineAsm, INLINE_ASM),
      TYPEID_ROW(IfStmt, IF),
      TYPEID_ROW(WhileStmt, WHILE),
      TYPEID_ROW(ForStmt, FOR),
      TYPEID_ROW(ForeachStmt, FOREACH),
      TYPEID_ROW(BreakStmt, BREAK),
      TYPEID_ROW(ContinueStmt, CONTINUE),
      TYPEID_ROW(ReturnStmt, RETURN),
      TYPEID_ROW(ReturnIfStmt, RETIF),
      TYPEID_ROW(CaseStmt, CASE),
      TYPEID_ROW(SwitchStmt, SWITCH),
      TYPEID_ROW(TypedefDecl, TYPEDEF),
      TYPEID_ROW(FnDecl, FNDECL),
      TYPEID_ROW(FnDef, FN),
      TYPEID_ROW(CompositeField, COMPOSITE_FIELD),
      TYPEID_ROW(StructDef, STRUCT),
      TYPEID_ROW(EnumDef, ENUM),
      TYPEID_ROW(SubsystemDecl, SUBSYSTEM),
      TYPEID_ROW(ExportDecl, EXPORT),
      TYPEID_ROW(ExprStmt, EXPR_STMT),
  };

  qcore_assert(typeid_map.size() == QAST_NODE_COUNT);

  return typeid_map.at(typeid(*this).hash_code());
}

LIB_EXPORT const char *Node::this_nameof() { return type_name(this_typeid()); }

LIB_EXPORT bool Node::is_type() {
  switch (this_typeid()) {
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
  switch (this_typeid()) {
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
  switch (this_typeid()) {
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
  switch (this_typeid()) {
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

LIB_EXPORT bool Node::is(qparse_ty_t type) { return type == this_typeid(); }

LIB_EXPORT std::string Node::to_string(bool minify) {
#define INDENT_STEP 1
  size_t len = 0;
  uint8_t *outbuf = nullptr;

  outbuf = (uint8_t *)qparse_repr(this, minify, INDENT_STEP, &len);

  std::string result((char *)outbuf, len);

  free(outbuf);

  return result;
}

///=============================================================================

LIB_EXPORT bool Type::is_primitive() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_primitive();
  }

  switch (this_typeid()) {
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

  return this_typeid() == QAST_NODE_ARRAY_TY;
}

LIB_EXPORT bool Type::is_tuple() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_tuple();
  }

  return this_typeid() == QAST_NODE_TUPLE_TY;
}

LIB_EXPORT bool Type::is_pointer() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_pointer();
  }

  return this_typeid() == QAST_NODE_PTR_TY;
}

LIB_EXPORT bool Type::is_function() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_function();
  }

  return this_typeid() == QAST_NODE_FN_TY;
}

LIB_EXPORT bool Type::is_composite() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_composite();
  }

  switch (this_typeid()) {
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

  switch (this_typeid()) {
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

  switch (this_typeid()) {
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

  switch (this_typeid()) {
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

  switch (this_typeid()) {
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

  switch (this_typeid()) {
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

  return this_typeid() == QAST_NODE_VOID_TY;
}

LIB_EXPORT bool Type::is_bool() {
  if (is<RefTy>()) {
    return as<RefTy>()->get_item()->is_bool();
  }

  return this_typeid() == QAST_NODE_U1_TY;
}

LIB_EXPORT bool Type::is_ref() { return this_typeid() == QAST_NODE_REF_TY; }

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

  return item->is(type->this_typeid());
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
