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

#include <core/Macro.h>
#include <qast/Nodes.h>
#include <quixcc/Library.h>
#include <quixcc/interface/SyntaxTreeNodes.h>

#include <cmath>
#include <cstring>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace libquixcc::qast;

#define qassert(expr, ...)                                                                         \
  (static_cast<bool>(expr)                                                                         \
       ? void(0)                                                                                   \
       : quixcc_panicf("Assertion failed: \"%s\";\nCondition: (%s);\nSource "                      \
                       "File: %s;\nSource Line: "                                                  \
                       "%d;\nFunction: %s;\n",                                                     \
                       "" #__VA_ARGS__, #expr, __FILE__, __LINE__, __PRETTY_FUNCTION__))

///=============================================================================
namespace libquixcc::qast {
  thread_local ArenaAllocatorImpl g_allocator;
}

ArenaAllocatorImpl::ArenaAllocatorImpl() { quixcc_arena_open(&m_arena); }

ArenaAllocatorImpl::~ArenaAllocatorImpl() { quixcc_arena_close(&m_arena); }

void *ArenaAllocatorImpl::allocate(std::size_t size) {
  const std::size_t alignment = 16;

  return quixcc_arena_alloc_ex(&m_arena, size, alignment);
}

void ArenaAllocatorImpl::deallocate(void *ptr) noexcept {}

///=============================================================================

const char *Node::type_name(quixcc_ast_ntype_t type) {
#define NAMEOF_ROW(__name)                                                                         \
  { QUIXCC_AST_NODE_##__name, "QUIXCC_AST_NODE_" #__name }

  static const std::unordered_map<quixcc_ast_ntype_t, const char *> names = {
      NAMEOF_ROW(STMT),       NAMEOF_ROW(TYPE),
      NAMEOF_ROW(DECL),       NAMEOF_ROW(EXPR),
      NAMEOF_ROW(CEXPR),      NAMEOF_ROW(UNRES_TY),
      NAMEOF_ROW(U1_TY),      NAMEOF_ROW(U8_TY),
      NAMEOF_ROW(U16_TY),     NAMEOF_ROW(U32_TY),
      NAMEOF_ROW(U64_TY),     NAMEOF_ROW(U128_TY),
      NAMEOF_ROW(I8_TY),      NAMEOF_ROW(I16_TY),
      NAMEOF_ROW(I32_TY),     NAMEOF_ROW(I64_TY),
      NAMEOF_ROW(I128_TY),    NAMEOF_ROW(F32_TY),
      NAMEOF_ROW(F64_TY),     NAMEOF_ROW(VOID_TY),
      NAMEOF_ROW(STRING_TY),  NAMEOF_ROW(PTR_TY),
      NAMEOF_ROW(OPAQUE_TY),  NAMEOF_ROW(VECTOR_TY),
      NAMEOF_ROW(SET_TY),     NAMEOF_ROW(MAP_TY),
      NAMEOF_ROW(TUPLE_TY),   NAMEOF_ROW(RESULT_TY),
      NAMEOF_ROW(ARRAY_TY),   NAMEOF_ROW(ENUM_TY),
      NAMEOF_ROW(MUT_TY),     NAMEOF_ROW(STRUCT_TY),
      NAMEOF_ROW(GROUP_TY),   NAMEOF_ROW(REGION_TY),
      NAMEOF_ROW(UNION_TY),   NAMEOF_ROW(FN_TY),
      NAMEOF_ROW(UNEXPR),     NAMEOF_ROW(BINEXPR),
      NAMEOF_ROW(TEREXPR),    NAMEOF_ROW(INT),
      NAMEOF_ROW(FLOAT),      NAMEOF_ROW(BOOL),
      NAMEOF_ROW(STRING),     NAMEOF_ROW(CHAR),
      NAMEOF_ROW(NULL),       NAMEOF_ROW(UNDEF),
      NAMEOF_ROW(CALL),       NAMEOF_ROW(LIST),
      NAMEOF_ROW(ASSOC),      NAMEOF_ROW(FIELD),
      NAMEOF_ROW(INDEX),      NAMEOF_ROW(SLICE),
      NAMEOF_ROW(FSTRING),    NAMEOF_ROW(IDENT),
      NAMEOF_ROW(BLOCK),      NAMEOF_ROW(CONST),
      NAMEOF_ROW(VAR),        NAMEOF_ROW(LET),
      NAMEOF_ROW(INLINE_ASM), NAMEOF_ROW(IF),
      NAMEOF_ROW(WHILE),      NAMEOF_ROW(FOR),
      NAMEOF_ROW(FORM),       NAMEOF_ROW(FOREACH),
      NAMEOF_ROW(BREAK),      NAMEOF_ROW(CONTINUE),
      NAMEOF_ROW(RETURN),     NAMEOF_ROW(RETIF),
      NAMEOF_ROW(RETZ),       NAMEOF_ROW(RETV),
      NAMEOF_ROW(CASE),       NAMEOF_ROW(SWITCH),
      NAMEOF_ROW(TYPEDEF),    NAMEOF_ROW(FNDECL),
      NAMEOF_ROW(FN),         NAMEOF_ROW(COMPOSITE_FIELD),
      NAMEOF_ROW(STRUCT),     NAMEOF_ROW(GROUP),
      NAMEOF_ROW(REGION),     NAMEOF_ROW(UNION),
      NAMEOF_ROW(ENUM),       NAMEOF_ROW(SUBSYSTEM),
      NAMEOF_ROW(EXPORT),
  };

  qassert(names.size() == QUIXCC_AST_NODE_COUNT,
          "Polymorphic type size lookup table is incomplete");
  qassert(names.contains(type));

  return names.at(type);
}

uint32_t Node::this_sizeof() const {
#define SIZEOF_ROW(__type)                                                                         \
  { typeid(__type).hash_code(), sizeof(__type) }

  static const std::unordered_map<size_t, uint32_t> sizes = {
      SIZEOF_ROW(Stmt),        SIZEOF_ROW(Type),
      SIZEOF_ROW(Decl),        SIZEOF_ROW(Expr),
      SIZEOF_ROW(ConstExpr),   SIZEOF_ROW(UnresolvedType),
      SIZEOF_ROW(U1),          SIZEOF_ROW(U8),
      SIZEOF_ROW(U16),         SIZEOF_ROW(U32),
      SIZEOF_ROW(U64),         SIZEOF_ROW(U128),
      SIZEOF_ROW(I8),          SIZEOF_ROW(I16),
      SIZEOF_ROW(I32),         SIZEOF_ROW(I64),
      SIZEOF_ROW(I128),        SIZEOF_ROW(F32),
      SIZEOF_ROW(F64),         SIZEOF_ROW(VoidTy),
      SIZEOF_ROW(StringTy),    SIZEOF_ROW(PtrTy),
      SIZEOF_ROW(OpaqueTy),    SIZEOF_ROW(VectorTy),
      SIZEOF_ROW(SetTy),       SIZEOF_ROW(MapTy),
      SIZEOF_ROW(TupleTy),     SIZEOF_ROW(OptionalTy),
      SIZEOF_ROW(ArrayTy),     SIZEOF_ROW(EnumTy),
      SIZEOF_ROW(MutTy),       SIZEOF_ROW(StructTy),
      SIZEOF_ROW(GroupTy),     SIZEOF_ROW(RegionTy),
      SIZEOF_ROW(UnionTy),     SIZEOF_ROW(FuncTy),
      SIZEOF_ROW(UnaryExpr),   SIZEOF_ROW(BinExpr),
      SIZEOF_ROW(TernaryExpr), SIZEOF_ROW(ConstInt),
      SIZEOF_ROW(ConstFloat),  SIZEOF_ROW(ConstBool),
      SIZEOF_ROW(ConstString), SIZEOF_ROW(ConstChar),
      SIZEOF_ROW(ConstNull),   SIZEOF_ROW(ConstUndef),
      SIZEOF_ROW(Call),        SIZEOF_ROW(List),
      SIZEOF_ROW(Assoc),       SIZEOF_ROW(Field),
      SIZEOF_ROW(Index),       SIZEOF_ROW(Slice),
      SIZEOF_ROW(FString),     SIZEOF_ROW(Ident),
      SIZEOF_ROW(Block),       SIZEOF_ROW(ConstDecl),
      SIZEOF_ROW(VarDecl),     SIZEOF_ROW(LetDecl),
      SIZEOF_ROW(InlineAsm),   SIZEOF_ROW(IfStmt),
      SIZEOF_ROW(WhileStmt),   SIZEOF_ROW(ForStmt),
      SIZEOF_ROW(FormStmt),    SIZEOF_ROW(ForeachStmt),
      SIZEOF_ROW(BreakStmt),   SIZEOF_ROW(ContinueStmt),
      SIZEOF_ROW(ReturnStmt),  SIZEOF_ROW(ReturnIfStmt),
      SIZEOF_ROW(RetZStmt),    SIZEOF_ROW(RetVStmt),
      SIZEOF_ROW(CaseStmt),    SIZEOF_ROW(SwitchStmt),
      SIZEOF_ROW(TypedefDecl), SIZEOF_ROW(FnDecl),
      SIZEOF_ROW(FnDef),       SIZEOF_ROW(CompositeField),
      SIZEOF_ROW(StructDef),   SIZEOF_ROW(GroupDef),
      SIZEOF_ROW(RegionDef),   SIZEOF_ROW(UnionDef),
      SIZEOF_ROW(EnumDef),     SIZEOF_ROW(SubsystemDecl),
      SIZEOF_ROW(ExportDecl),
  };

  qassert(sizes.size() == QUIXCC_AST_NODE_COUNT,
          "Polymorphic type size lookup table is incomplete");

  size_t id = typeid(*this).hash_code();
  qassert(sizes.contains(id));

  return sizes.at(id);
}

quixcc_ast_ntype_t Node::this_typeid() const {
  /// TODO: Implement this function
#define TYPEID_ROW(__type, __name)                                                                 \
  { typeid(__type).hash_code(), QUIXCC_AST_NODE_##__name }

  static const std::unordered_map<size_t, quixcc_ast_ntype_t> typeid_map = {
      TYPEID_ROW(Stmt, STMT),
      TYPEID_ROW(Type, TYPE),
      TYPEID_ROW(Decl, DECL),
      TYPEID_ROW(Expr, EXPR),
      TYPEID_ROW(ConstExpr, CEXPR),
      TYPEID_ROW(UnresolvedType, UNRES_TY),
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
      TYPEID_ROW(F32, F32_TY),
      TYPEID_ROW(F64, F64_TY),
      TYPEID_ROW(VoidTy, VOID_TY),
      TYPEID_ROW(StringTy, STRING_TY),
      TYPEID_ROW(PtrTy, PTR_TY),
      TYPEID_ROW(OpaqueTy, OPAQUE_TY),
      TYPEID_ROW(VectorTy, VECTOR_TY),
      TYPEID_ROW(SetTy, SET_TY),
      TYPEID_ROW(MapTy, MAP_TY),
      TYPEID_ROW(TupleTy, TUPLE_TY),
      TYPEID_ROW(OptionalTy, RESULT_TY),
      TYPEID_ROW(ArrayTy, ARRAY_TY),
      TYPEID_ROW(EnumTy, ENUM_TY),
      TYPEID_ROW(MutTy, MUT_TY),
      TYPEID_ROW(StructTy, STRUCT_TY),
      TYPEID_ROW(GroupTy, GROUP_TY),
      TYPEID_ROW(RegionTy, REGION_TY),
      TYPEID_ROW(UnionTy, UNION_TY),
      TYPEID_ROW(FuncTy, FN_TY),
      TYPEID_ROW(UnaryExpr, UNEXPR),
      TYPEID_ROW(BinExpr, BINEXPR),
      TYPEID_ROW(TernaryExpr, TEREXPR),
      TYPEID_ROW(ConstInt, INT),
      TYPEID_ROW(ConstFloat, FLOAT),
      TYPEID_ROW(ConstBool, BOOL),
      TYPEID_ROW(ConstString, STRING),
      TYPEID_ROW(ConstChar, CHAR),
      TYPEID_ROW(ConstNull, NULL),
      TYPEID_ROW(ConstUndef, UNDEF),
      TYPEID_ROW(Call, CALL),
      TYPEID_ROW(List, LIST),
      TYPEID_ROW(Assoc, ASSOC),
      TYPEID_ROW(Field, FIELD),
      TYPEID_ROW(Index, INDEX),
      TYPEID_ROW(Slice, SLICE),
      TYPEID_ROW(FString, FSTRING),
      TYPEID_ROW(Ident, IDENT),
      TYPEID_ROW(Block, BLOCK),
      TYPEID_ROW(ConstDecl, CONST),
      TYPEID_ROW(VarDecl, VAR),
      TYPEID_ROW(LetDecl, LET),
      TYPEID_ROW(InlineAsm, INLINE_ASM),
      TYPEID_ROW(IfStmt, IF),
      TYPEID_ROW(WhileStmt, WHILE),
      TYPEID_ROW(ForStmt, FOR),
      TYPEID_ROW(FormStmt, FORM),
      TYPEID_ROW(ForeachStmt, FOREACH),
      TYPEID_ROW(BreakStmt, BREAK),
      TYPEID_ROW(ContinueStmt, CONTINUE),
      TYPEID_ROW(ReturnStmt, RETURN),
      TYPEID_ROW(ReturnIfStmt, RETIF),
      TYPEID_ROW(RetZStmt, RETZ),
      TYPEID_ROW(RetVStmt, RETV),
      TYPEID_ROW(CaseStmt, CASE),
      TYPEID_ROW(SwitchStmt, SWITCH),
      TYPEID_ROW(TypedefDecl, TYPEDEF),
      TYPEID_ROW(FnDecl, FNDECL),
      TYPEID_ROW(FnDef, FN),
      TYPEID_ROW(CompositeField, COMPOSITE_FIELD),
      TYPEID_ROW(StructDef, STRUCT),
      TYPEID_ROW(GroupDef, GROUP),
      TYPEID_ROW(RegionDef, REGION),
      TYPEID_ROW(UnionDef, UNION),
      TYPEID_ROW(EnumDef, ENUM),
      TYPEID_ROW(SubsystemDecl, SUBSYSTEM),
      TYPEID_ROW(ExportDecl, EXPORT),
  };

  qassert(typeid_map.size() == QUIXCC_AST_NODE_COUNT);

  return typeid_map.at(typeid(*this).hash_code());
}

const char *Node::this_nameof() const { return type_name(this_typeid()); }

size_t Node::children_count(bool recursive) const {
  /// TODO: Implement this function
  quixcc_panic("Node::children_count() is not implemented");
}

bool Node::inherits_from(quixcc_ast_ntype_t type) const {
  /// TODO: Implement this function
  quixcc_panic("Node::inherits_from() is not implemented");
}

bool Node::is_type() const { return inherits_from(QUIXCC_AST_NODE_TYPE); }
bool Node::is_stmt() const { return inherits_from(QUIXCC_AST_NODE_STMT); }
bool Node::is_decl() const { return inherits_from(QUIXCC_AST_NODE_DECL); }
bool Node::is_expr() const { return inherits_from(QUIXCC_AST_NODE_EXPR); }
bool Node::is_const_expr() const { return is_expr() && as<Expr>()->is_const(); }
bool Node::is(quixcc_ast_ntype_t type) const { return type == this_typeid(); }
bool Node::verify(std::ostream &os) const { return verify_impl(os); }
void Node::canonicalize() { canonicalize_impl(); }

///=============================================================================

uint64_t Type::infer_size_bits() const { return infer_size_bits_impl(); }
uint64_t Type::infer_size() const { return std::ceil(infer_size_bits() / 8.0); }

bool Type::is_primitive() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_primitive();
  }

  switch (this_typeid()) {
  case QUIXCC_AST_NODE_U1_TY:
  case QUIXCC_AST_NODE_U8_TY:
  case QUIXCC_AST_NODE_U16_TY:
  case QUIXCC_AST_NODE_U32_TY:
  case QUIXCC_AST_NODE_U64_TY:
  case QUIXCC_AST_NODE_U128_TY:
  case QUIXCC_AST_NODE_I8_TY:
  case QUIXCC_AST_NODE_I16_TY:
  case QUIXCC_AST_NODE_I32_TY:
  case QUIXCC_AST_NODE_I64_TY:
  case QUIXCC_AST_NODE_I128_TY:
  case QUIXCC_AST_NODE_F32_TY:
  case QUIXCC_AST_NODE_F64_TY:
  case QUIXCC_AST_NODE_VOID_TY:
    return true;
  default:
    return false;
  }
}

bool Type::is_array() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_array();
  }

  return this_typeid() == QUIXCC_AST_NODE_ARRAY_TY;
}

bool Type::is_vector() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_vector();
  }

  return this_typeid() == QUIXCC_AST_NODE_VECTOR_TY;
}

bool Type::is_tuple() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_tuple();
  }

  return this_typeid() == QUIXCC_AST_NODE_TUPLE_TY;
}

bool Type::is_set() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_set();
  }

  return this_typeid() == QUIXCC_AST_NODE_SET_TY;
}

bool Type::is_map() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_map();
  }

  return this_typeid() == QUIXCC_AST_NODE_MAP_TY;
}

bool Type::is_pointer() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_pointer();
  }

  return this_typeid() == QUIXCC_AST_NODE_PTR_TY;
}

bool Type::is_function() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_function();
  }

  return this_typeid() == QUIXCC_AST_NODE_FN_TY;
}

bool Type::is_composite() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_composite();
  }

  switch (this_typeid()) {
  case QUIXCC_AST_NODE_ARRAY_TY:
  case QUIXCC_AST_NODE_VECTOR_TY:
  case QUIXCC_AST_NODE_TUPLE_TY:
  case QUIXCC_AST_NODE_SET_TY:
  case QUIXCC_AST_NODE_MAP_TY:
  case QUIXCC_AST_NODE_STRUCT_TY:
  case QUIXCC_AST_NODE_GROUP_TY:
  case QUIXCC_AST_NODE_REGION_TY:
  case QUIXCC_AST_NODE_RESULT_TY:
    return true;
  default:
    return false;
  }
}

bool Type::is_union() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_union();
  }

  return this_typeid() == QUIXCC_AST_NODE_UNION_TY;
}

bool Type::is_numeric() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_numeric();
  }

  switch (this_typeid()) {
  case QUIXCC_AST_NODE_U1_TY:
  case QUIXCC_AST_NODE_U8_TY:
  case QUIXCC_AST_NODE_U16_TY:
  case QUIXCC_AST_NODE_U32_TY:
  case QUIXCC_AST_NODE_U64_TY:
  case QUIXCC_AST_NODE_U128_TY:
  case QUIXCC_AST_NODE_I8_TY:
  case QUIXCC_AST_NODE_I16_TY:
  case QUIXCC_AST_NODE_I32_TY:
  case QUIXCC_AST_NODE_I64_TY:
  case QUIXCC_AST_NODE_I128_TY:
  case QUIXCC_AST_NODE_F32_TY:
  case QUIXCC_AST_NODE_F64_TY:
    return true;
  default:
    return false;
  }
}

bool Type::is_integral() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_integral();
  }

  switch (this_typeid()) {
  case QUIXCC_AST_NODE_U1_TY:
  case QUIXCC_AST_NODE_U8_TY:
  case QUIXCC_AST_NODE_U16_TY:
  case QUIXCC_AST_NODE_U32_TY:
  case QUIXCC_AST_NODE_U64_TY:
  case QUIXCC_AST_NODE_U128_TY:
  case QUIXCC_AST_NODE_I8_TY:
  case QUIXCC_AST_NODE_I16_TY:
  case QUIXCC_AST_NODE_I32_TY:
  case QUIXCC_AST_NODE_I64_TY:
  case QUIXCC_AST_NODE_I128_TY:
    return true;
  default:
    return false;
  }
}

bool Type::is_floating_point() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_floating_point();
  }

  switch (this_typeid()) {
  case QUIXCC_AST_NODE_F32_TY:
  case QUIXCC_AST_NODE_F64_TY:
    return true;
  default:
    return false;
  }
}

bool Type::is_signed() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_signed();
  }

  switch (this_typeid()) {
  case QUIXCC_AST_NODE_I8_TY:
  case QUIXCC_AST_NODE_I16_TY:
  case QUIXCC_AST_NODE_I32_TY:
  case QUIXCC_AST_NODE_I64_TY:
  case QUIXCC_AST_NODE_I128_TY:
  case QUIXCC_AST_NODE_F32_TY:
  case QUIXCC_AST_NODE_F64_TY:
    return true;
  default:
    return false;
  }
}

bool Type::is_unsigned() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_unsigned();
  }

  switch (this_typeid()) {
  case QUIXCC_AST_NODE_U1_TY:
  case QUIXCC_AST_NODE_U8_TY:
  case QUIXCC_AST_NODE_U16_TY:
  case QUIXCC_AST_NODE_U32_TY:
  case QUIXCC_AST_NODE_U64_TY:
  case QUIXCC_AST_NODE_U128_TY:
    return true;
  default:
    return false;
  }
}

bool Type::is_void() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_void();
  }

  return this_typeid() == QUIXCC_AST_NODE_VOID_TY;
}

bool Type::is_bool() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_bool();
  }

  return this_typeid() == QUIXCC_AST_NODE_U1_TY;
}

bool Type::is_mutable() const { return this_typeid() == QUIXCC_AST_NODE_MUT_TY; }

bool Type::is_const() const { return !is_mutable(); }

bool Type::is_volatile() const {
  /// TODO: Implement this function
  quixcc_panic("Type::is_volatile() is not implemented");
}

bool Type::is_ptr_to(const Type *type) const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_ptr_to(type);
  }

  if (!is_pointer()) {
    return false;
  }

  Type *item = as<PtrTy>()->get_item();
  while (item->is<MutTy>()) {
    item = item->as<MutTy>()->get_item();
  }

  return item->is(type->this_typeid());
}

bool Type::is_mut_ptr_to(const Type *type) const {
  if (!is<MutTy>()) {
    return false;
  }

  return as<MutTy>()->get_item()->is_ptr_to(type);
}

bool Type::is_const_ptr_to(const Type *type) const {
  if (is<MutTy>()) {
    return false;
  }

  return is_ptr_to(type);
}

bool Type::is_ptr_to_mut(const Type *type) const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_ptr_to_mut(type);
  }

  if (!is_pointer()) {
    return false;
  }

  Type *item = as<PtrTy>()->get_item();
  if (!item->is<MutTy>()) {
    return false;
  }

  while (item->is<MutTy>()) {
    item = item->as<MutTy>()->get_item();
  }

  return item->is(type->this_typeid());
}

bool Type::is_ptr_to_const(const Type *type) const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_ptr_to_const(type);
  }

  if (!is_pointer()) {
    return false;
  }

  return as<PtrTy>()->get_item()->is(type->this_typeid());
}

bool Type::is_mut_ptr_to_mut(const Type *type) const {
  if (!is<MutTy>()) {
    return false;
  }

  return as<MutTy>()->get_item()->is_ptr_to_mut(type);
}

bool Type::is_mut_ptr_to_const(const Type *type) const {
  if (!is<MutTy>()) {
    return false;
  }

  return as<MutTy>()->get_item()->is_ptr_to_const(type);
}

bool Type::is_const_ptr_to_mut(const Type *type) const {
  if (is<MutTy>()) {
    return false;
  }

  return is_ptr_to_mut(type);
}

bool Type::is_const_ptr_to_const(const Type *type) const {
  if (is<MutTy>()) {
    return false;
  }

  return is_ptr_to_const(type);
}

bool Type::is_string() const {
  if (is<MutTy>()) {
    return as<MutTy>()->get_item()->is_string();
  }

  return this_typeid() == QUIXCC_AST_NODE_STRING_TY;
}

///=============================================================================

std::string_view Decl::get_name() const { return m_name; }
void Decl::set_name(std::string_view name) { m_name = name; }

Type *Decl::get_type() const { return m_type; }
void Decl::set_type(Type *type) { m_type = type; }

const DeclTags &Decl::get_tags() const { return m_tags; }
void Decl::add_tag(std::string_view tag) { m_tags.insert(tag); }
void Decl::add_tags(std::initializer_list<std::string_view> tags) {
  m_tags.insert(tags.begin(), tags.end());
}
void Decl::clear_tags() { m_tags.clear(); }
void Decl::remove_tag(std::string_view tag) { m_tags.erase(tag); }

Type *Decl::infer_type() const { return infer_type_impl(); }

///=============================================================================

Type *Expr::infer_type() const { return infer_type_impl(); }
bool Expr::is_const() const { return is_const_impl(); }
bool Expr::is_stochastic() const { return is_stochastic_impl(); }

bool Expr::is_binexpr() const {
  if (is<BinExpr>()) {
    return true;
  }

  if (!is<ConstExpr>()) {
    return false;
  }

  return as<ConstExpr>()->get_value()->is_binexpr();
}

bool Expr::is_unaryexpr() const {
  if (is<UnaryExpr>()) {
    return true;
  }

  if (!is<ConstExpr>()) {
    return false;
  }

  return as<ConstExpr>()->get_value()->is_unaryexpr();
}

bool Expr::is_ternaryexpr() const {
  if (is<TernaryExpr>()) {
    return true;
  }

  if (!is<ConstExpr>()) {
    return false;
  }

  return as<ConstExpr>()->get_value()->is_ternaryexpr();
}

///=============================================================================

Type *ConstExpr::infer_type_impl() const { return get_value()->infer_type(); }
bool ConstExpr::is_const_impl() const { return true; }
bool ConstExpr::is_stochastic_impl() const { return false; }

Expr *ConstExpr::get_value() const { return m_value; }
void ConstExpr::set_value(Expr *value) { m_value = value; }

bool ConstExpr::verify_impl(std::ostream &os) const {
  quixcc_panic("ConstExpr::verify_impl() is not implemented");
}

void ConstExpr::canonicalize_impl() {
  quixcc_panic("ConstExpr::canonicalize_impl() is not implemented");
}

void ConstExpr::print_impl(std::ostream &os, bool debug) const {
  quixcc_panic("ConstExpr::print_impl() is not implemented");
}

ConstExpr *ConstExpr::clone_impl() const {
  quixcc_panic("ConstExpr::clone_impl() is not implemented");
}

///=============================================================================

bool UnresolvedType::verify_impl(std::ostream &os) const {
  if (m_name.empty()) {
    os << "UnresolvedType: name is empty\n";
    return false;
  }

  return true;
}

void UnresolvedType::canonicalize_impl() {}

void UnresolvedType::print_impl(std::ostream &os, bool debug) const { os << m_name; }

UnresolvedType *UnresolvedType::clone_impl() const { return UnresolvedType::get(m_name); }

uint64_t UnresolvedType::infer_size_bits_impl() const {
  throw std::runtime_error("UnresolvedType::infer_size_bits_impl() is not implemented");
}

std::string_view UnresolvedType::get_name() const { return m_name; }
void UnresolvedType::set_name(std::string_view name) { m_name = name; }

#define TRIVIAL_TYPE_IMPL(__typename, __dumpstr, __bits)                                           \
  bool __typename::verify_impl(std::ostream &os) const { return true; }                            \
  void __typename::canonicalize_impl() {}                                                          \
  void __typename::print_impl(std::ostream &os, bool debug) const { os << __dumpstr; }             \
  __typename *__typename::clone_impl() const { return __typename::get(); }                         \
  uint64_t __typename::infer_size_bits_impl() const { return __bits; }

TRIVIAL_TYPE_IMPL(U1, "u1", 1)
TRIVIAL_TYPE_IMPL(U8, "u8", 8)
TRIVIAL_TYPE_IMPL(U16, "u16", 16)
TRIVIAL_TYPE_IMPL(U32, "u32", 32)
TRIVIAL_TYPE_IMPL(U64, "u64", 64)
TRIVIAL_TYPE_IMPL(U128, "u128", 128)
TRIVIAL_TYPE_IMPL(I8, "i8", 8)
TRIVIAL_TYPE_IMPL(I16, "i16", 16)
TRIVIAL_TYPE_IMPL(I32, "i32", 32)
TRIVIAL_TYPE_IMPL(I64, "i64", 64)
TRIVIAL_TYPE_IMPL(I128, "i128", 128)
TRIVIAL_TYPE_IMPL(F32, "f32", 32)
TRIVIAL_TYPE_IMPL(F64, "f64", 64)
TRIVIAL_TYPE_IMPL(VoidTy, "void", 0)
TRIVIAL_TYPE_IMPL(StringTy, "string", 8)

bool PtrTy::verify_impl(std::ostream &os) const {
  if (!m_item) {
    os << "PtrTy: item type is NULL\n";
    return false;
  }

  if (!m_item->verify(os)) {
    os << "PtrTy: item type is invalid\n";
    return false;
  }

  return true;
}

void PtrTy::canonicalize_impl() {
  if (m_item) {
    m_item->canonicalize();
  }
}

void PtrTy::print_impl(std::ostream &os, bool debug) const {
  if (m_is_volatile) {
    os << "volatile<ptr<";

    if (m_item) {
      m_item->print(os, debug);
    } else {
      os << "?";
    }

    os << ">>";
  } else {
    os << "ptr<";

    if (m_item) {
      m_item->print(os, debug);
    } else {
      os << "?";
    }

    os << ">";
  }
}

PtrTy *PtrTy::clone_impl() const {
  Type *item = m_item ? m_item->clone() : nullptr;
  return PtrTy::get(item, m_is_volatile);
}

uint64_t PtrTy::infer_size_bits_impl() const {
  /// TODO: Implement this function
  quixcc_panic("PtrTy::infer_size_bits_impl() is not implemented");
}

Type *PtrTy::get_item() const { return m_item; }
void PtrTy::set_item(Type *item) { m_item = item; }
bool PtrTy::is_volatile() const { return m_is_volatile; }
void PtrTy::set_volatile(bool is_volatile) { m_is_volatile = is_volatile; }

bool OpaqueTy::verify_impl(std::ostream &os) const {
  if (m_name.empty()) {
    os << "OpaqueTy: name is empty\n";
    return false;
  }

  return true;
}

void OpaqueTy::canonicalize_impl() {}
void OpaqueTy::print_impl(std::ostream &os, bool debug) const { os << "opaque<" << m_name << ">"; }
OpaqueTy *OpaqueTy::clone_impl() const { return OpaqueTy::get(m_name); }

uint64_t OpaqueTy::infer_size_bits_impl() const {
  throw std::runtime_error("OpaqueTy::infer_size_bits_impl() is not valid");
}

std::string_view OpaqueTy::get_name() const { return m_name; }
void OpaqueTy::set_name(std::string_view name) { m_name = name; }

bool VectorTy::verify_impl(std::ostream &os) const {
  if (!m_item) {
    os << "VectorTy: item type is NULL\n";
    return false;
  }

  if (!m_item->verify(os)) {
    os << "VectorTy: item type is invalid\n";
    return false;
  }

  return true;
}

void VectorTy::canonicalize_impl() {
  if (m_item) {
    m_item->canonicalize();
  }
}

void VectorTy::print_impl(std::ostream &os, bool debug) const {
  if (!m_item) {
    os << "vec<?>";
    return;
  }

  os << "vec<";
  m_item->print(os, debug);
  os << ">";
}

VectorTy *VectorTy::clone_impl() const {
  Type *item = m_item ? m_item->clone() : nullptr;
  return VectorTy::get(item);
}

uint64_t VectorTy::infer_size_bits_impl() const {
  throw std::runtime_error("VectorTy::infer_size_bits_impl() is not valid");
}

bool SetTy::verify_impl(std::ostream &os) const {
  if (!m_item) {
    os << "SetTy: item type is NULL\n";
    return false;
  }

  if (!m_item->verify(os)) {
    os << "SetTy: item type is invalid\n";
    return false;
  }

  return true;
}

void SetTy::canonicalize_impl() {
  if (m_item) {
    m_item->canonicalize();
  }
}

void SetTy::print_impl(std::ostream &os, bool debug) const {
  if (!m_item) {
    os << "set<?>";
    return;
  }

  os << "set<";
  m_item->print(os, debug);
  os << ">";
}

SetTy *SetTy::clone_impl() const {
  Type *item = m_item ? m_item->clone() : nullptr;
  return SetTy::get(item);
}

uint64_t SetTy::infer_size_bits_impl() const {
  throw std::runtime_error("SetTy::infer_size_bits_impl() is not valid");
}

bool MapTy::verify_impl(std::ostream &os) const {
  if (!m_key) {
    os << "MapTy: key type is NULL\n";
    return false;
  }

  if (!m_value) {
    os << "MapTy: value type is NULL\n";
    return false;
  }

  if (!m_key->verify(os)) {
    os << "MapTy: key type is invalid\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "MapTy: value type is invalid\n";
    return false;
  }

  return true;
}

void MapTy::canonicalize_impl() {
  if (m_key) {
    m_key->canonicalize();
  }

  if (m_value) {
    m_value->canonicalize();
  }
}

void MapTy::print_impl(std::ostream &os, bool debug) const {
  if (!m_key) {
    os << "map<?, ";
  } else {
    os << "map<";
    m_key->print(os, debug);
    os << ", ";
  }

  if (!m_value) {
    os << "?>";
  } else {
    m_value->print(os, debug);
    os << ">";
  }
}

MapTy *MapTy::clone_impl() const {
  Type *key = m_key ? m_key->clone() : nullptr;
  Type *value = m_value ? m_value->clone() : nullptr;

  return MapTy::get(key, value);
}

uint64_t MapTy::infer_size_bits_impl() const {
  throw std::runtime_error("MapTy::infer_size_bits_impl() is not valid");
}

bool TupleTy::verify_impl(std::ostream &os) const {
  for (size_t i = 0; i < m_items.size(); i++) {
    if (!m_items[i]) {
      os << "TupleTy: item " << i << " is NULL\n";
      return false;
    }

    if (!m_items[i]->verify(os)) {
      os << "TupleTy: item " << i << " is invalid\n";
      return false;
    }
  }

  return true;
}

void TupleTy::canonicalize_impl() {
  for (auto item : m_items) {
    item->canonicalize();
  }
}

void TupleTy::print_impl(std::ostream &os, bool debug) const {
  os << "tuple<";

  for (size_t i = 0; i < m_items.size(); i++) {
    if (m_items[i]) {
      m_items[i]->print(os, debug);
    } else {
      os << "?";
    }

    if (i + 1 < m_items.size()) {
      os << ", ";
    }
  }

  os << ">";
}

TupleTy *TupleTy::clone_impl() const {
  std::vector<Type *, Arena<Type *>> items;
  for (auto item : m_items) {
    if (item) {
      items.push_back(item->clone());
    } else {
      items.push_back(nullptr);
    }
  }

  return TupleTy::get(items);
}

uint64_t TupleTy::infer_size_bits_impl() const { return GroupTy::get(m_items)->infer_size_bits(); }

bool OptionalTy::verify_impl(std::ostream &os) const {
  if (!m_item) {
    os << "OptionalTy: item type is NULL\n";
    return false;
  }

  if (!m_item->verify(os)) {
    os << "OptionalTy: item type is invalid\n";
    return false;
  }

  return true;
}

void OptionalTy::canonicalize_impl() {
  if (m_item) {
    m_item->canonicalize();
  }
}

void OptionalTy::print_impl(std::ostream &os, bool debug) const {
  if (!m_item) {
    os << "opt<?>";
    return;
  }

  os << "opt<";
  m_item->print(os, debug);
  os << ">";
}

OptionalTy *OptionalTy::clone_impl() const {
  Type *item = m_item ? m_item->clone() : nullptr;
  return OptionalTy::get(item);
}

uint64_t OptionalTy::infer_size_bits_impl() const {
  throw std::runtime_error("OptionalTy::infer_size_bits_impl() is not valid");
}

bool ArrayTy::verify_impl(std::ostream &os) const {
  if (!m_item) {
    os << "ArrayTy: item type is NULL\n";
    return false;
  }

  if (!m_size) {
    os << "ArrayTy: size is NULL\n";
    return false;
  }

  if (!m_item->verify(os)) {
    os << "ArrayTy: item type is invalid\n";
    return false;
  }

  if (!m_size->verify(os)) {
    os << "ArrayTy: size is invalid\n";
    return false;
  }

  if (!m_size->infer_type()->is_unsigned()) {
    os << "ArrayTy: size is not an unsigned integral type\n";
    return false;
  }

  return true;
}

void ArrayTy::canonicalize_impl() {
  if (m_item) {
    m_item->canonicalize();
  }

  if (m_size) {
    m_size->canonicalize();
  }
}

void ArrayTy::print_impl(std::ostream &os, bool debug) const {
  os << "array<";

  if (m_item) {
    m_item->print(os, debug);
  } else {
    os << "?";
  }

  os << ", ";

  if (m_size) {
    m_size->print(os, debug);
  } else {
    os << "?";
  }

  os << ">";
}

ArrayTy *ArrayTy::clone_impl() const {
  Type *item = m_item ? m_item->clone() : nullptr;
  ConstExpr *size = m_size ? m_size->clone() : nullptr;

  return ArrayTy::get(item, size);
}

uint64_t ArrayTy::infer_size_bits_impl() const {
  qassert(m_item);
  qassert(m_size);

  return m_item->infer_size_bits() * m_size->eval_as<uint64_t>();
}

Type *ArrayTy::get_item() const { return m_item; }
void ArrayTy::set_item(Type *item) { m_item = item; }
ConstExpr *ArrayTy::get_size() const { return m_size; }
void ArrayTy::set_size(ConstExpr *size) { m_size = size; }

bool EnumTy::verify_impl(std::ostream &os) const {
  if (!m_memtype) {
    os << "EnumTy: member item type is NULL\n";
    return false;
  }

  if (!m_memtype->verify(os)) {
    os << "EnumTy: member item type is invalid\n";
    return false;
  }

  return true;
}

void EnumTy::canonicalize_impl() {
  if (m_memtype) {
    m_memtype->canonicalize();
  }
}

void EnumTy::print_impl(std::ostream &os, bool debug) const {
  os << "enum<" << m_name << ", ";

  if (m_memtype) {
    m_memtype->print(os, debug);
  } else {
    os << "?";
  }

  os << ">";
}

EnumTy *EnumTy::clone_impl() const {
  Type *memtype = m_memtype ? m_memtype->clone() : nullptr;
  return EnumTy::get(m_name, memtype);
}

uint64_t EnumTy::infer_size_bits_impl() const {
  qassert(m_memtype);

  return m_memtype->infer_size_bits();
}

std::string_view EnumTy::get_name() const { return m_name; }
void EnumTy::set_name(std::string_view name) { m_name = name; }
Type *EnumTy::get_memtype() const { return m_memtype; }
void EnumTy::set_memtype(Type *memtype) { m_memtype = memtype; }

bool MutTy::verify_impl(std::ostream &os) const {
  if (!m_item) {
    os << "MutTy: item type is NULL\n";
    return false;
  }

  if (!m_item->verify(os)) {
    os << "MutTy: item type is invalid\n";
    return false;
  }

  return true;
}

void MutTy::canonicalize_impl() {
  if (m_item) {
    m_item->canonicalize();
  }
}

void MutTy::print_impl(std::ostream &os, bool debug) const {
  if (!m_item) {
    os << "mut<?>";
    return;
  }

  os << "mut<";
  m_item->print(os, debug);
  os << ">";
}

MutTy *MutTy::clone_impl() const {
  Type *item = m_item ? m_item->clone() : nullptr;
  return MutTy::get(item);
}

uint64_t MutTy::infer_size_bits_impl() const {
  qassert(m_item);

  return m_item->infer_size_bits();
}

Type *MutTy::get_item() const { return m_item; }
void MutTy::set_item(Type *item) { m_item = item; }

bool StructTy::verify_impl(std::ostream &os) const {
  std::unordered_set<std::string_view> names;

  for (size_t i = 0; i < m_items.size(); i++) {
    if (names.contains(m_items[i].first)) {
      os << "StructTy: duplicate field name '" << m_items[i].first << "'\n";
      return false;
    }

    if (!m_items[i].second) {
      os << "StructTy: item " << i << " is NULL\n";
      return false;
    }

    if (!m_items[i].second->verify(os)) {
      os << "StructTy: item " << i << " is invalid\n";
      return false;
    }

    names.insert(m_items[i].first);
  }

  return true;
}

void StructTy::canonicalize_impl() {
  for (auto &[name, item] : m_items) {
    if (item) {
      item->canonicalize();
    }
  }
}

void StructTy::print_impl(std::ostream &os, bool debug) const {
  os << "struct<";

  for (size_t i = 0; i < m_items.size(); i++) {
    os << m_items[i].first << ": ";

    if (m_items[i].second) {
      m_items[i].second->print(os, debug);
    } else {
      os << "?";
    }

    if (i + 1 < m_items.size()) {
      os << ", ";
    }
  }

  os << ">";
}

StructTy *StructTy::clone_impl() const {
  std::vector<StructItem, Arena<StructItem>> fields;
  for (auto &[name, item] : m_items) {
    if (item) {
      fields.push_back({name, item->clone()});
    } else {
      fields.push_back({name, nullptr});
    }
  }

  return StructTy::get(fields);
}

uint64_t StructTy::infer_size_bits_impl() const {
  /// TODO: Implement this function
  quixcc_panic("StructTy::infer_size_bits_impl() is not implemented");
}

const std::vector<StructItem, Arena<StructItem>> &StructTy::get_items() const { return m_items; }

void StructTy::add_item(std::string_view name, Type *item) { m_items.push_back({name, item}); }

void StructTy::add_items(std::initializer_list<StructItem> fields) {
  for (auto [name, item] : fields) {
    add_item(name, item);
  }
}

void StructTy::clear_items() { m_items.clear(); }

void StructTy::remove_item(std::string_view name) {
  std::erase_if(m_items, [name](auto &field) { return field.first == name; });
}

bool GroupTy::verify_impl(std::ostream &os) const {
  for (size_t i = 0; i < m_items.size(); i++) {
    if (!m_items[i]) {
      os << "GroupTy: item " << i << " is NULL\n";
      return false;
    }

    if (!m_items[i]->verify(os)) {
      os << "GroupTy: item " << i << " is invalid\n";
      return false;
    }
  }

  return true;
}

void GroupTy::canonicalize_impl() {
  for (auto item : m_items) {
    if (item) {
      item->canonicalize();
    }
  }
}

void GroupTy::print_impl(std::ostream &os, bool debug) const {
  os << "group<";

  for (size_t i = 0; i < m_items.size(); i++) {
    if (m_items[i]) {
      m_items[i]->print(os, debug);
    } else {
      os << "?";
    }

    if (i + 1 < m_items.size()) {
      os << ", ";
    }
  }

  os << ">";
}

GroupTy *GroupTy::clone_impl() const {
  std::vector<Type *, Arena<Type *>> fields;
  for (auto item : m_items) {
    if (item) {
      fields.push_back(item->clone());
    } else {
      fields.push_back(nullptr);
    }
  }

  return GroupTy::get(fields);
}

uint64_t GroupTy::infer_size_bits_impl() const {
  /// TODO: Implement this function
  quixcc_panic("GroupTy::infer_size_bits_impl() is not implemented");
}

const std::vector<Type *, Arena<Type *>> &GroupTy::get_items() const { return m_items; }

void GroupTy::add_item(Type *item) { m_items.push_back(item); }

void GroupTy::add_items(std::initializer_list<Type *> fields) {
  for (auto field : fields) {
    add_item(field);
  }
}

void GroupTy::clear_items() { m_items.clear(); }

void GroupTy::remove_item(Type *item) {
  std::erase_if(m_items, [item](auto &field) { return field == item; });
}

bool RegionTy::verify_impl(std::ostream &os) const {
  for (size_t i = 0; i < m_items.size(); i++) {
    if (!m_items[i]) {
      os << "RegionTy: item " << i << " is NULL\n";
      return false;
    }

    if (!m_items[i]->verify(os)) {
      os << "RegionTy: item " << i << " is invalid\n";
      return false;
    }
  }

  return true;
}

void RegionTy::canonicalize_impl() {
  for (auto item : m_items) {
    if (item) {
      item->canonicalize();
    }
  }
}

void RegionTy::print_impl(std::ostream &os, bool debug) const {
  os << "region<";

  for (size_t i = 0; i < m_items.size(); i++) {
    if (m_items[i]) {
      m_items[i]->print(os, debug);
    } else {
      os << "?";
    }

    if (i + 1 < m_items.size()) {
      os << ", ";
    }
  }

  os << ">";
}

RegionTy *RegionTy::clone_impl() const {
  std::vector<Type *, Arena<Type *>> fields;
  for (auto item : m_items) {
    if (item) {
      fields.push_back(item->clone());
    } else {
      fields.push_back(nullptr);
    }
  }

  return RegionTy::get(fields);
}

uint64_t RegionTy::infer_size_bits_impl() const {
  size_t size = 0;
  for (auto item : m_items) {
    qassert(item);
    size += item->infer_size_bits();
  }

  return size;
}

const std::vector<Type *, Arena<Type *>> &RegionTy::get_items() const { return m_items; }

void RegionTy::add_item(Type *item) { m_items.push_back(item); }

void RegionTy::add_items(std::initializer_list<Type *> fields) {
  for (auto field : fields) {
    add_item(field);
  }
}

void RegionTy::clear_items() { m_items.clear(); }

void RegionTy::remove_item(Type *item) {
  std::erase_if(m_items, [item](auto &field) { return field == item; });
}

bool UnionTy::verify_impl(std::ostream &os) const {
  for (size_t i = 0; i < m_items.size(); i++) {
    if (!m_items[i]) {
      os << "UnionTy: item " << i << " is NULL\n";
      return false;
    }

    if (!m_items[i]->verify(os)) {
      os << "UnionTy: item " << i << " is invalid\n";
      return false;
    }
  }

  return true;
}

void UnionTy::canonicalize_impl() {
  for (auto item : m_items) {
    if (item) {
      item->canonicalize();
    }
  }
}

void UnionTy::print_impl(std::ostream &os, bool debug) const {
  os << "union<";

  for (size_t i = 0; i < m_items.size(); i++) {
    if (m_items[i]) {
      m_items[i]->print(os, debug);
    } else {
      os << "?";
    }

    if (i + 1 < m_items.size()) {
      os << ", ";
    }
  }

  os << ">";
}

UnionTy *UnionTy::clone_impl() const {
  std::vector<Type *, Arena<Type *>> fields;
  for (auto item : m_items) {
    if (item) {
      fields.push_back(item->clone());
    } else {
      fields.push_back(nullptr);
    }
  }

  return UnionTy::get(fields);
}

uint64_t UnionTy::infer_size_bits_impl() const {
  size_t size = 0;
  for (auto item : m_items) {
    size = std::max(size, item->infer_size_bits());
  }

  return size;
}

const std::vector<Type *, Arena<Type *>> &UnionTy::get_items() const { return m_items; }

void UnionTy::add_item(Type *item) { m_items.push_back(item); }

void UnionTy::add_items(std::initializer_list<Type *> fields) {
  for (auto field : fields) {
    add_item(field);
  }
}

void UnionTy::clear_items() { m_items.clear(); }

void UnionTy::remove_item(Type *item) {
  std::erase_if(m_items, [item](auto &field) { return field == item; });
}

bool FuncTy::verify_impl(std::ostream &os) const {
  if (!m_return) {
    os << "FuncTy: return type is NULL\n";
    return false;
  }

  std::unordered_set<std::string_view> names;
  for (size_t i = 0; i < m_params.size(); i++) {
    if (names.contains(std::get<0>(m_params[i]))) {
      os << "FuncTy: duplicate param name '" << std::get<0>(m_params[i]) << "'\n";
      return false;
    }

    if (!std::get<1>(m_params[i])) {
      os << "FuncTy: param " << i << " is NULL\n";
      return false;
    }

    if (!std::get<1>(m_params[i])->verify(os)) {
      os << "FuncTy: param " << i << " is invalid\n";
      return false;
    }

    if (std::get<2>(m_params[i])) {
      if (!std::get<2>(m_params[i])->verify(os)) {
        os << "FuncTy: param default " << i << " is invalid\n";
        return false;
      }
    }

    names.insert(std::get<0>(m_params[i]));
  }

  if (!m_return->verify(os)) {
    os << "FuncTy: return type is invalid\n";
    return false;
  }

  if (m_noreturn) {
    if (!m_return->is_void()) {
      os << "FuncTy: noreturn function does not return void\n";
      return false;
    }
  }

  return true;
}

void FuncTy::canonicalize_impl() {
  for (auto &[name, param, default_val] : m_params) {
    if (param) {
      param->canonicalize();
    }

    if (default_val) {
      default_val->canonicalize();
    }
  }

  if (m_return) {
    m_return->canonicalize();
  }
}

void FuncTy::print_impl(std::ostream &os, bool debug) const {
  os << "func<(";

  for (size_t i = 0; i < m_params.size(); i++) {
    os << std::get<0>(m_params[i]) << ": ";

    if (std::get<1>(m_params[i])) {
      std::get<1>(m_params[i])->print(os, debug);
    } else {
      os << "?";
    }

    if (std::get<2>(m_params[i])) {
      os << " = ";
      std::get<2>(m_params[i])->print(os, debug);
    }

    if (i + 1 < m_params.size()) {
      os << ", ";
    }
  }

  if (m_variadic) {
    os << ", ...";
  }

  os << ") ";

  if (m_is_foreign) {
    os << "foreign ";
  }

  if (m_crashpoint) {
    os << "crashpoint ";
  }

  if (m_noexcept) {
    os << "noexcept ";
  }

  if (m_noreturn) {
    os << "noreturn ";
  }

  switch (m_purity) {
  case FuncPurity::IMPURE_THREAD_UNSAFE:
    os << "impure ";
    break;
  case FuncPurity::IMPURE_THREAD_SAFE:
    os << "impure tsafe ";
    break;
  case FuncPurity::PURE:
    os << "pure tsafe ";
    break;
  case FuncPurity::QUASIPURE:
    os << "quasipure tsafe ";
    break;
  case FuncPurity::RETROPURE:
    os << "retropure tsafe ";
    break;
  }

  os << "-> ";

  if (m_return) {
    m_return->print(os, debug);
  } else {
    os << "?";
  }

  os << ">";
}

FuncTy *FuncTy::clone_impl() const {
  std::vector<FuncParam, Arena<FuncParam>> params;
  for (auto [name, param, default_val] : m_params) {
    Type *p = param ? param->clone() : nullptr;
    Expr *d = default_val ? default_val->clone() : nullptr;

    params.push_back({name, p, d});
  }

  Type *ret = m_return ? m_return->clone() : nullptr;

  return FuncTy::get(ret, params, m_variadic, m_purity, m_is_foreign, m_crashpoint, m_noexcept,
                     m_noreturn);
}

uint64_t FuncTy::infer_size_bits_impl() const {
  return PtrTy::get(VoidTy::get())->infer_size_bits();
}

bool FuncTy::is_noreturn() const { return m_noreturn; }
void FuncTy::set_noreturn(bool noreturn) { m_noreturn = noreturn; }
Type *FuncTy::get_return_ty() const { return m_return; }
void FuncTy::set_return_ty(Type *ty) { m_return = ty; }
const std::vector<FuncParam, Arena<FuncParam>> &FuncTy::get_params() const { return m_params; }
void FuncTy::add_param(std::string_view name, Type *ty, Expr *default_val) {
  m_params.push_back({name, ty, default_val});
}
void FuncTy::add_params(std::initializer_list<FuncParam> params) {
  for (auto [name, ty, expr] : params) {
    add_param(name, ty, expr);
  }
}
void FuncTy::clear_params() { m_params.clear(); }
void FuncTy::remove_param(std::string_view name) {
  std::erase_if(m_params, [name](auto &param) { return std::get<0>(param) == name; });
}

FuncPurity FuncTy::get_purity() const { return m_purity; }
void FuncTy::set_purity(FuncPurity purity) { m_purity = purity; }

bool FuncTy::is_variadic() const { return m_variadic; }
void FuncTy::set_variadic(bool variadic) { m_variadic = variadic; }

bool FuncTy::is_foreign() const { return m_is_foreign; }
void FuncTy::set_foreign(bool is_foreign) { m_is_foreign = is_foreign; }

bool FuncTy::is_crashpoint() const { return m_crashpoint; }
void FuncTy::set_crashpoint(bool crashpoint) { m_crashpoint = crashpoint; }

bool FuncTy::is_noexcept() const { return m_noexcept; }
void FuncTy::set_noexcept(bool _noexcept) { m_noexcept = _noexcept; }

///=============================================================================

bool UnaryExpr::verify_impl(std::ostream &os) const {
  if (!m_rhs) {
    os << "UnaryExpr: rhs is NULL\n";
    return false;
  }

  if (m_op == UnaryOp::UNKNOWN) {
    os << "UnaryExpr: unknown operator\n";
    return false;
  }

  if (!m_rhs->verify(os)) {
    os << "UnaryExpr: rhs is invalid\n";
    return false;
  }

  return true;
}

void UnaryExpr::canonicalize_impl() {
  if (m_rhs) {
    m_rhs->canonicalize();
  }
}

void UnaryExpr::print_impl(std::ostream &os, bool debug) const {
  os << "(";

  switch (m_op) {
  case UnaryOp::UNKNOWN:
    os << "unknown";
    break;
  }

  if (m_rhs) {
    m_rhs->print(os, debug);
  } else {
    os << "?";
  }
}

UnaryExpr *UnaryExpr::clone_impl() const {
  Expr *rhs = m_rhs ? m_rhs->clone() : nullptr;
  return UnaryExpr::get(m_op, rhs);
}

Type *UnaryExpr::infer_type_impl() const {
  /// TODO: Implement this function
  quixcc_panic("UnaryExpr::infer_type_impl() is not implemented");
}

bool UnaryExpr::is_const_impl() const {
  qassert(m_rhs);
  return m_rhs->is_const();
}

bool UnaryExpr::is_stochastic_impl() const {
  qassert(m_rhs);
  return m_rhs->is_stochastic();
}

UnaryOp UnaryExpr::get_op() const { return m_op; }
void UnaryExpr::set_op(UnaryOp op) { m_op = op; }

Expr *UnaryExpr::get_rhs() const { return m_rhs; }
void UnaryExpr::set_rhs(Expr *rhs) { m_rhs = rhs; }

bool BinExpr::verify_impl(std::ostream &os) const {
  if (!m_lhs) {
    os << "BinExpr: lhs is NULL\n";
    return false;
  }

  if (!m_rhs) {
    os << "BinExpr: rhs is NULL\n";
    return false;
  }

  if (m_op == BinOp::UNKNOWN) {
    os << "BinExpr: unknown operator\n";
    return false;
  }

  if (!m_lhs->verify(os)) {
    os << "BinExpr: lhs is invalid\n";
    return false;
  }

  if (!m_rhs->verify(os)) {
    os << "BinExpr: rhs is invalid\n";
    return false;
  }

  return true;
}

void BinExpr::canonicalize_impl() {
  if (m_lhs) {
    m_lhs->canonicalize();
  }

  if (m_rhs) {
    m_rhs->canonicalize();
  }
}

void BinExpr::print_impl(std::ostream &os, bool debug) const {
  os << "(";

  if (m_lhs) {
    m_lhs->print(os, debug);
  } else {
    os << "?";
  }

  switch (m_op) {
  case BinOp::UNKNOWN:
    os << " unknown ";
    break;
  }

  if (m_rhs) {
    m_rhs->print(os, debug);
  } else {
    os << "?";
  }

  os << ")";
}

BinExpr *BinExpr::clone_impl() const {
  Expr *lhs = m_lhs ? m_lhs->clone() : nullptr;
  Expr *rhs = m_rhs ? m_rhs->clone() : nullptr;

  return BinExpr::get(lhs, m_op, rhs);
}

Type *BinExpr::infer_type_impl() const {
  /// TODO: Implement this function
  quixcc_panic("BinExpr::infer_type_impl() is not implemented");
}

bool BinExpr::is_const_impl() const {
  qassert(m_lhs);
  qassert(m_rhs);
  return m_lhs->is_const() && m_rhs->is_const();
}

bool BinExpr::is_stochastic_impl() const {
  qassert(m_lhs);
  qassert(m_rhs);
  return m_lhs->is_stochastic() || m_rhs->is_stochastic();
}

BinOp BinExpr::get_op() const { return m_op; }
void BinExpr::set_op(BinOp op) { m_op = op; }

Expr *BinExpr::get_lhs() const { return m_lhs; }
void BinExpr::set_lhs(Expr *lhs) { m_lhs = lhs; }

Expr *BinExpr::get_rhs() const { return m_rhs; }
void BinExpr::set_rhs(Expr *rhs) { m_rhs = rhs; }

bool TernaryExpr::verify_impl(std::ostream &os) const {
  if (!m_cond) {
    os << "TernaryExpr: cond is NULL\n";
    return false;
  }

  if (!m_lhs) {
    os << "TernaryExpr: lhs is NULL\n";
    return false;
  }

  if (!m_rhs) {
    os << "TernaryExpr: rhs is NULL\n";
    return false;
  }

  if (!m_cond->verify(os)) {
    os << "TernaryExpr: cond is invalid\n";
    return false;
  }

  if (!m_lhs->verify(os)) {
    os << "TernaryExpr: lhs is invalid\n";
    return false;
  }

  if (!m_rhs->verify(os)) {
    os << "TernaryExpr: rhs is invalid\n";
    return false;
  }

  return true;
}

void TernaryExpr::canonicalize_impl() {
  if (m_cond) {
    m_cond->canonicalize();
  }

  if (m_lhs) {
    m_lhs->canonicalize();
  }

  if (m_rhs) {
    m_rhs->canonicalize();
  }
}

void TernaryExpr::print_impl(std::ostream &os, bool debug) const {
  os << "(";

  if (m_cond) {
    m_cond->print(os, debug);
  } else {
    os << "?";
  }

  os << " ? (";

  if (m_lhs) {
    m_lhs->print(os, debug);
  } else {
    os << "?";
  }

  os << ") : (";

  if (m_rhs) {
    m_rhs->print(os, debug);
  } else {
    os << "?";
  }

  os << "))";
}

TernaryExpr *TernaryExpr::clone_impl() const {
  Expr *cond = m_cond ? m_cond->clone() : nullptr;
  Expr *lhs = m_lhs ? m_lhs->clone() : nullptr;
  Expr *rhs = m_rhs ? m_rhs->clone() : nullptr;

  return TernaryExpr::get(cond, lhs, rhs);
}

Type *TernaryExpr::infer_type_impl() const {
  /// TODO: Implement this function
  quixcc_panic("TernaryExpr::infer_type_impl() is not implemented");
}

bool TernaryExpr::is_const_impl() const {
  qassert(m_cond);
  qassert(m_lhs);
  qassert(m_rhs);
  return m_cond->is_const() && m_lhs->is_const() && m_rhs->is_const();
}

bool TernaryExpr::is_stochastic_impl() const {
  qassert(m_cond);
  qassert(m_lhs);
  qassert(m_rhs);
  return m_cond->is_stochastic() || m_lhs->is_stochastic() || m_rhs->is_stochastic();
}

Expr *TernaryExpr::get_cond() const { return m_cond; }
void TernaryExpr::set_cond(Expr *cond) { m_cond = cond; }

Expr *TernaryExpr::get_lhs() const { return m_lhs; }
void TernaryExpr::set_lhs(Expr *lhs) { m_lhs = lhs; }

Expr *TernaryExpr::get_rhs() const { return m_rhs; }
void TernaryExpr::set_rhs(Expr *rhs) { m_rhs = rhs; }

///=============================================================================

bool ConstInt::verify_impl(std::ostream &os) const {
  if (m_value.empty()) {
    os << "ConstInt: value is empty\n";
    return false;
  }

  for (char c : m_value) {
    if (!std::isdigit(c)) {
      os << "ConstInt: value is not a number\n";
      return false;
    }
  }

  return true;
}

void ConstInt::canonicalize_impl() {}

void ConstInt::print_impl(std::ostream &os, bool debug) const { os << m_value; }

ConstInt *ConstInt::clone_impl() const { return ConstInt::get(m_value); }

std::string_view ConstInt::get_value() const { return m_value; }

bool ConstFloat::verify_impl(std::ostream &os) const {
  if (m_value.empty()) {
    os << "ConstFloat: value is empty\n";
    return false;
  }

  bool found_dot = false;

  for (char c : m_value) {
    if (!std::isdigit(c) && c != '.') {
      os << "ConstFloat: value is not a number\n";
      return false;
    }

    if (c == '.') {
      if (found_dot) {
        os << "ConstFloat: value has multiple dots\n";
        return false;
      }

      found_dot = true;
    }
  }

  return true;
}

void ConstFloat::canonicalize_impl() {}

void ConstFloat::print_impl(std::ostream &os, bool debug) const { os << m_value; }

ConstFloat *ConstFloat::clone_impl() const { return ConstFloat::get(m_value); }

std::string_view ConstFloat::get_value() const { return m_value; }

bool ConstBool::verify_impl(std::ostream &os) const { return true; }

void ConstBool::canonicalize_impl() {}

void ConstBool::print_impl(std::ostream &os, bool debug) const {
  os << (m_value ? "true" : "false");
}

ConstBool *ConstBool::clone_impl() const { return ConstBool::get(m_value); }

bool ConstBool::get_value() const { return m_value; }

bool ConstChar::verify_impl(std::ostream &os) const {
  if (m_value.empty()) {
    os << "ConstChar: value is empty\n";
    return false;
  }

  return true;
}

void ConstChar::canonicalize_impl() {}

void ConstChar::print_impl(std::ostream &os, bool debug) const { os << "'" << m_value << "'"; }

ConstChar *ConstChar::clone_impl() const { return ConstChar::get(m_value); }

std::string_view ConstChar::get_value() const { return m_value; }

bool ConstString::verify_impl(std::ostream &os) const {
  if (m_value.empty()) {
    os << "ConstString: value is empty\n";
    return false;
  }

  return true;
}

void ConstString::canonicalize_impl() {}

void ConstString::print_impl(std::ostream &os, bool debug) const { os << "\"" << m_value << "\""; }

ConstString *ConstString::clone_impl() const { return ConstString::get(m_value); }

std::string_view ConstString::get_value() const { return m_value; }

bool ConstNull::verify_impl(std::ostream &os) const { return true; }
void ConstNull::canonicalize_impl() {}
void ConstNull::print_impl(std::ostream &os, bool debug) const { os << "null"; }
ConstNull *ConstNull::clone_impl() const { return ConstNull::get(); }

bool ConstUndef::verify_impl(std::ostream &os) const { return true; }
void ConstUndef::canonicalize_impl() {}
void ConstUndef::print_impl(std::ostream &os, bool debug) const { os << "undef"; }
ConstUndef *ConstUndef::clone_impl() const { return ConstUndef::get(); }

///=============================================================================

bool Call::verify_impl(std::ostream &os) const {
  if (!m_func) {
    os << "CallExpr: function is NULL\n";
    return false;
  }

  for (const auto &[name, expr] : m_args) {
    if (!expr) {
      os << "CallExpr: arg '" << name << "' is NULL\n";
      return false;
    }

    if (!expr->verify(os)) {
      os << "CallExpr: arg '" << name << "' is invalid\n";
      return false;
    }
  }

  if (!m_func->verify(os)) {
    os << "CallExpr: function is invalid\n";
    return false;
  }

  return true;
}

void Call::canonicalize_impl() {
  if (m_func) {
    m_func->canonicalize();
  }

  for (auto &[name, expr] : m_args) {
    if (expr) {
      expr->canonicalize();
    }
  }
}

void Call::print_impl(std::ostream &os, bool debug) const {
  if (m_func) {
    m_func->print(os, debug);
  } else {
    os << "?";
  }

  os << "(";

  for (auto it = m_args.begin(); it != m_args.end(); ++it) {
    os << it->first << ": ";
    if (it->second) {
      it->second->print(os, debug);
    } else {
      os << "?";
    }
    if (std::next(it) != m_args.end()) {
      os << ", ";
    }
  }

  os << ")";
}

Call *Call::clone_impl() const {
  Expr *func = m_func ? m_func->clone() : nullptr;
  CallArgs args;

  for (const auto &[name, expr] : m_args) {
    Expr *e = expr ? expr->clone() : nullptr;
    args[name] = e;
  }

  return Call::get(func, args);
}

Type *Call::infer_type_impl() const {
  qassert(m_func);
  return m_func->infer_type()->as<FuncTy>()->get_return_ty();
}

bool Call::is_const_impl() const {
  qassert(m_func);
  return m_func->is_const();
}

bool Call::is_stochastic_impl() const {
  qassert(m_func);

  if (m_func->is_stochastic()) {
    return true;
  }

  for (const auto &[name, expr] : m_args) {
    if (expr->is_stochastic()) {
      return true;
    }
  }

  return false;
}

Expr *Call::get_func() const { return m_func; }
void Call::set_func(Expr *func) { m_func = func; }

const CallArgs &Call::get_args() const { return m_args; }
void Call::add_arg(std::string_view name, Expr *arg) { m_args[name] = arg; }
void Call::clear_args() { m_args.clear(); }
void Call::remove_arg(std::string_view name) { m_args.erase(name); }

bool List::verify_impl(std::ostream &os) const {
  for (size_t i = 0; i < m_items.size(); i++) {
    if (!m_items[i]) {
      os << "List: item " << i << " is NULL\n";
      return false;
    }

    if (!m_items[i]->verify(os)) {
      os << "List: item " << i << " is invalid\n";
      return false;
    }
  }

  return true;
}

void List::canonicalize_impl() {
  for (auto item : m_items) {
    if (item) {
      item->canonicalize();
    }
  }
}

void List::print_impl(std::ostream &os, bool debug) const {
  os << "[";

  for (size_t i = 0; i < m_items.size(); i++) {
    if (m_items[i]) {
      m_items[i]->print(os, debug);
    } else {
      os << "?";
    }

    if (i + 1 < m_items.size()) {
      os << ", ";
    }
  }

  os << "]";
}

List *List::clone_impl() const {
  ListData items;
  for (auto item : m_items) {
    if (item) {
      items.push_back(item->clone());
    } else {
      items.push_back(nullptr);
    }
  }

  return List::get(items);
}

Type *List::infer_type_impl() const {
  /// TODO: Implement this function
  quixcc_panic("List::infer_type_impl() is not implemented");
}

bool List::is_const_impl() const {
  for (auto item : m_items) {
    if (!item->is_const()) {
      return false;
    }
  }

  return true;
}

bool List::is_stochastic_impl() const {
  for (auto item : m_items) {
    if (item->is_stochastic()) {
      return true;
    }
  }

  return false;
}

const ListData &List::get_items() const { return m_items; }
void List::add_item(Expr *item) { m_items.push_back(item); }
void List::clear_items() { m_items.clear(); }
void List::remove_item(Expr *item) {
  std::erase_if(m_items, [item](auto &field) { return field == item; });
}

bool Assoc::verify_impl(std::ostream &os) const {
  if (!m_key) {
    os << "Assoc: key is NULL\n";
    return false;
  }

  if (!m_value) {
    os << "Assoc: value is NULL\n";
    return false;
  }

  if (!m_key->verify(os)) {
    os << "Assoc: key is invalid\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "Assoc: value is invalid\n";
    return false;
  }

  return true;
}

void Assoc::canonicalize_impl() {
  if (m_key) {
    m_key->canonicalize();
  }

  if (m_value) {
    m_value->canonicalize();
  }
}

void Assoc::print_impl(std::ostream &os, bool debug) const {
  if (m_key) {
    m_key->print(os, debug);
  } else {
    os << "?";
  }

  os << ": ";

  if (m_value) {
    m_value->print(os, debug);
  } else {
    os << "?";
  }
}

Assoc *Assoc::clone_impl() const {
  Expr *key = m_key ? m_key->clone() : nullptr;
  Expr *value = m_value ? m_value->clone() : nullptr;

  return Assoc::get(key, value);
}

Type *Assoc::infer_type_impl() const {
  /// TODO: Implement this function
  quixcc_panic("Assoc::infer_type_impl() is not implemented");
}

bool Assoc::is_const_impl() const {
  qassert(m_key);
  qassert(m_value);
  return m_key->is_const() && m_value->is_const();
}

bool Assoc::is_stochastic_impl() const {
  qassert(m_key);
  qassert(m_value);
  return m_key->is_stochastic() || m_value->is_stochastic();
}

Expr *Assoc::get_key() const { return m_key; }
void Assoc::set_key(Expr *key) { m_key = key; }

Expr *Assoc::get_value() const { return m_value; }
void Assoc::set_value(Expr *value) { m_value = value; }

bool Field::verify_impl(std::ostream &os) const {
  if (!m_base) {
    os << "Field: base is NULL\n";
    return false;
  }

  if (m_field.empty()) {
    os << "Field: field is empty\n";
    return false;
  }

  if (!m_base->verify(os)) {
    os << "Field: base is invalid\n";
    return false;
  }

  return true;
}

void Field::canonicalize_impl() {
  if (m_base) {
    m_base->canonicalize();
  }
}

void Field::print_impl(std::ostream &os, bool debug) const {
  if (m_base) {
    m_base->print(os, debug);
  } else {
    os << "?";
  }

  os << "." << m_field;
}

Field *Field::clone_impl() const {
  Expr *base = m_base ? m_base->clone() : nullptr;
  return Field::get(base, m_field);
}

Type *Field::infer_type_impl() const {
  /// TODO: Implement this function
  quixcc_panic("Field::infer_type_impl() is not implemented");
}

bool Field::is_const_impl() const {
  qassert(m_base);
  return m_base->is_const();
}

bool Field::is_stochastic_impl() const {
  qassert(m_base);
  return m_base->is_stochastic();
}

Expr *Field::get_base() const { return m_base; }
void Field::set_base(Expr *base) { m_base = base; }

std::string_view Field::get_field() const { return m_field; }
void Field::set_field(std::string_view field) { m_field = field; }

bool Index::verify_impl(std::ostream &os) const {
  if (!m_base) {
    os << "Index: base is NULL\n";
    return false;
  }

  if (!m_index) {
    os << "Index: index is NULL\n";
    return false;
  }

  if (!m_base->verify(os)) {
    os << "Index: base is invalid\n";
    return false;
  }

  if (!m_index->verify(os)) {
    os << "Index: index is invalid\n";
    return false;
  }

  return true;
}

void Index::canonicalize_impl() {
  if (m_base) {
    m_base->canonicalize();
  }

  if (m_index) {
    m_index->canonicalize();
  }
}

void Index::print_impl(std::ostream &os, bool debug) const {
  if (m_base) {
    m_base->print(os, debug);
  } else {
    os << "?";
  }

  os << "[";

  if (m_index) {
    m_index->print(os, debug);
  } else {
    os << "?";
  }

  os << "]";
}

Index *Index::clone_impl() const {
  Expr *base = m_base ? m_base->clone() : nullptr;
  Expr *index = m_index ? m_index->clone() : nullptr;

  return Index::get(base, index);
}

Type *Index::infer_type_impl() const {
  quixcc_panic("Index::infer_type_impl() is not implemented");
  qassert(m_base);

  Type *bt = m_base->infer_type();
  qassert(bt != nullptr);

  if (bt->is_array()) {
    return bt->as<ArrayTy>()->get_item();
  }

  if (bt->is_pointer()) {
    return bt->as<PtrTy>()->get_item();
  }

  if (bt->is_vector()) {
    return bt->as<VectorTy>()->get_item();
  }

  if (bt->is_string()) {
    /// TODO: Decide whether to return I8 or I32. Should strings be native UTF-8?
    return I32::get();
  }

  if (bt->is_map()) {
    return bt->as<MapTy>()->get_value();
  }

  if (bt->is_tuple()) {
    qassert(m_index->is_const());
    qassert(m_index->infer_type()->is_integral());

    TupleTy *tt = bt->as<TupleTy>();
    uint64_t idx = m_index->as<ConstExpr>()->eval_as<uint64_t>();
    return tt->get_items().at(idx);
  }

  throw std::runtime_error("Indexing non-indexable type");
}

bool Index::is_const_impl() const {
  qassert(m_base);
  qassert(m_index);
  return m_base->is_const() && m_index->is_const();
}

bool Index::is_stochastic_impl() const {
  qassert(m_base);
  qassert(m_index);
  return m_base->is_stochastic() || m_index->is_stochastic();
}

Expr *Index::get_base() const { return m_base; }
void Index::set_base(Expr *base) { m_base = base; }

Expr *Index::get_index() const { return m_index; }
void Index::set_index(Expr *index) { m_index = index; }

bool Slice::verify_impl(std::ostream &os) const {
  if (!m_base) {
    os << "Slice: base is NULL\n";
    return false;
  }

  if (!m_start) {
    os << "Slice: start is NULL\n";
    return false;
  }

  if (!m_end) {
    os << "Slice: end is NULL\n";
    return false;
  }

  if (!m_base->verify(os)) {
    os << "Slice: base is invalid\n";
    return false;
  }

  if (!m_start->verify(os)) {
    os << "Slice: start is invalid\n";
    return false;
  }

  if (!m_end->verify(os)) {
    os << "Slice: end is invalid\n";
    return false;
  }

  return true;
}

void Slice::canonicalize_impl() {
  if (m_base) {
    m_base->canonicalize();
  }

  if (m_start) {
    m_start->canonicalize();
  }

  if (m_end) {
    m_end->canonicalize();
  }
}

void Slice::print_impl(std::ostream &os, bool debug) const {
  if (m_base) {
    m_base->print(os, debug);
  } else {
    os << "?";
  }

  os << "[";

  if (m_start) {
    m_start->print(os, debug);
  } else {
    os << "?";
  }

  os << ":";

  if (m_end) {
    m_end->print(os, debug);
  } else {
    os << "?";
  }

  os << "]";
}

Slice *Slice::clone_impl() const {
  Expr *base = m_base ? m_base->clone() : nullptr;
  Expr *start = m_start ? m_start->clone() : nullptr;
  Expr *end = m_end ? m_end->clone() : nullptr;

  return Slice::get(base, start, end);
}

Type *Slice::infer_type_impl() const {
  /// TODO: Implement this function
  quixcc_panic("Slice::infer_type_impl() is not implemented");

  qassert(m_base);
  qassert(m_start);
  qassert(m_end);

  Type *bt = m_base->infer_type();
  qassert(bt != nullptr);

  bool bounds_const = m_start->is_const() && m_end->is_const();
  bool base_size_const = bt->is_array() || bt->is_tuple();

  if (bounds_const) {
    int64_t start = m_start->as<ConstExpr>()->eval_as<int64_t>();
    int64_t end = m_end->as<ConstExpr>()->eval_as<int64_t>();

    if ((start < 0 || end < 0) && !base_size_const) {
      throw std::runtime_error("Negative slice index on compound type of indeterminate size");
    } else if (start < 0 || end < 0) {
      size_t base_size = 0;

      if (bt->is_array()) {
        base_size = bt->as<ArrayTy>()->get_size()->eval_as<size_t>();
      } else if (bt->is_tuple()) {
        base_size = bt->as<TupleTy>()->get_items().size();
      }

      if (start < 0) {
        start += base_size;
      }

      if (end < 0) {
        end += base_size;
      }

      if (start < 0 || end < 0) {
        throw std::runtime_error("Negative slice index after resolving negative index");
      }
    }

    if (start > end) {
      throw std::runtime_error("Slice start index is greater than end index");
    }

    if (bt->is_array()) {
      return ArrayTy::get(bt->as<ArrayTy>()->get_item(), ConstInt::get(end - start + 1));
    }

    if (bt->is_tuple()) {
      TupleTy *tt = bt->as<TupleTy>();
      TupleTyItems items;

      for (size_t i = start; i <= (size_t)end; i++) {
        items.push_back(tt->get_items().at(i));
      }

      return TupleTy::get(items);
    }

    throw std::runtime_error("Slicing non-sliceable type");
  } else {
    if (bt->is_array()) {
      return VectorTy::get(bt->as<ArrayTy>()->get_item());
    }

    throw std::runtime_error("Slicing non-sliceable type");
  }
}

bool Slice::is_const_impl() const {
  qassert(m_base);
  qassert(m_start);
  qassert(m_end);
  return m_base->is_const() && m_start->is_const() && m_end->is_const();
}

bool Slice::is_stochastic_impl() const {
  qassert(m_base);
  qassert(m_start);
  qassert(m_end);
  return m_base->is_stochastic() || m_start->is_stochastic() || m_end->is_stochastic();
}

Expr *Slice::get_base() const { return m_base; }
void Slice::set_base(Expr *base) { m_base = base; }

Expr *Slice::get_start() const { return m_start; }
void Slice::set_start(Expr *start) { m_start = start; }

Expr *Slice::get_end() const { return m_end; }
void Slice::set_end(Expr *end) { m_end = end; }

bool FString::verify_impl(std::ostream &os) const {
  for (auto item : m_items) {
    if (!item) {
      os << "FString: item is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "FString: item is invalid\n";
      return false;
    }
  }

  return true;
}

void FString::canonicalize_impl() {
  for (auto item : m_items) {
    if (item) {
      item->canonicalize();
    }
  }
}

void FString::print_impl(std::ostream &os, bool debug) const {
  /// TODO: Implement this function
  quixcc_panic("FString::print_impl() is not implemented");
}

FString *FString::clone_impl() const {
  FStringArgs items;
  for (auto item : m_items) {
    if (item) {
      items.push_back(item->clone());
    } else {
      items.push_back(nullptr);
    }
  }

  return FString::get(m_value, items);
}

Type *FString::infer_type_impl() const { return StringTy::get(); }

bool FString::is_const_impl() const {
  for (auto item : m_items) {
    if (!item->is_const()) {
      return false;
    }
  }

  return true;
}

bool FString::is_stochastic_impl() const {
  for (auto item : m_items) {
    if (item->is_stochastic()) {
      return true;
    }
  }

  return false;
}

std::string_view FString::get_value() const { return m_value; }
void FString::set_value(std::string_view value) { m_value = value; }

const FStringArgs &FString::get_items() const { return m_items; }
void FString::add_item(Expr *item) { m_items.push_back(item); }
void FString::clear_items() { m_items.clear(); }
void FString::remove_item(Expr *item) {
  std::erase_if(m_items, [item](auto &field) { return field == item; });
}

bool Ident::verify_impl(std::ostream &os) const {
  if (m_name.empty()) {
    os << "Ident: name is empty\n";
    return false;
  }

  return true;
}

void Ident::canonicalize_impl() {}

void Ident::print_impl(std::ostream &os, bool debug) const { os << m_name; }

Ident *Ident::clone_impl() const { return Ident::get(m_name); }

Type *Ident::infer_type_impl() const {
  /// TODO: Implement this function
  quixcc_panic("Ident::infer_type_impl() is not implemented");
}

bool Ident::is_const_impl() const {
  /// TODO: Implement this function
  quixcc_panic("Ident::is_const_impl() is not implemented");
}

bool Ident::is_stochastic_impl() const {
  /// TODO: Implement this function
  quixcc_panic("Ident::is_stochastic_impl() is not implemented");
}

std::string_view Ident::get_name() const { return m_name; }
void Ident::set_name(std::string_view name) { m_name = name; }

///=============================================================================

bool Block::verify_impl(std::ostream &os) const {
  for (auto item : m_items) {
    if (!item) {
      os << "Block: item is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "Block: item is invalid\n";
      return false;
    }
  }

  return true;
}

void Block::canonicalize_impl() {
  for (auto item : m_items) {
    if (item) {
      item->canonicalize();
    }
  }
}

void Block::print_impl(std::ostream &os, bool debug) const {
  os << "{\n";

  for (auto item : m_items) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "}";
}

Block *Block::clone_impl() const {
  BlockItems items;
  for (auto item : m_items) {
    if (item) {
      items.push_back(item->clone());
    } else {
      items.push_back(nullptr);
    }
  }

  return Block::get(items);
}

const BlockItems &Block::get_items() const { return m_items; }

void Block::add_item(Stmt *item) { m_items.push_back(item); }
void Block::clear_items() { m_items.clear(); }
void Block::remove_item(Stmt *item) {
  std::erase_if(m_items, [item](auto &field) { return field == item; });
}

bool ConstDecl::verify_impl(std::ostream &os) const {
  if (!m_value) {
    os << "ConstDecl: value is NULL\n";
    return false;
  }

  if (!m_type) {
    os << "ConstDecl: type is NULL\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "ConstDecl: value is invalid\n";
    return false;
  }

  if (!m_type->verify(os)) {
    os << "ConstDecl: type is invalid\n";
    return false;
  }

  return true;
}

void ConstDecl::canonicalize_impl() {
  if (m_value) {
    m_value->canonicalize();
  }
}

void ConstDecl::print_impl(std::ostream &os, bool debug) const {
  os << "const " << m_name;

  if (m_type) {
    os << ": ";
    m_type->print(os, debug);
  }

  if (m_value) {
    os << " = ";
    m_value->print(os, debug);
  }

  os << ";";
}

ConstDecl *ConstDecl::clone_impl() const {
  Expr *value = m_value ? m_value->clone() : nullptr;
  Type *type = m_type ? m_type->clone() : nullptr;
  return ConstDecl::get(m_name, type, value);
}

Type *ConstDecl::infer_type_impl() const { return m_type; }

Expr *ConstDecl::get_value() const { return m_value; }
void ConstDecl::set_value(Expr *value) { m_value = value; }

bool VarDecl::verify_impl(std::ostream &os) const {
  if (!m_value) {
    os << "VarDecl: value is NULL\n";
    return false;
  }

  if (!m_type) {
    os << "VarDecl: type is NULL\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "VarDecl: value is invalid\n";
    return false;
  }

  if (!m_type->verify(os)) {
    os << "VarDecl: type is invalid\n";
    return false;
  }

  return true;
}

void VarDecl::canonicalize_impl() {
  if (m_value) {
    m_value->canonicalize();
  }
}

void VarDecl::print_impl(std::ostream &os, bool debug) const {
  os << "var " << m_name;

  if (m_type) {
    os << ": ";
    m_type->print(os, debug);
  }

  if (m_value) {
    os << " = ";
    m_value->print(os, debug);
  }

  os << ";";
}

VarDecl *VarDecl::clone_impl() const {
  Expr *value = m_value ? m_value->clone() : nullptr;
  Type *type = m_type ? m_type->clone() : nullptr;
  return VarDecl::get(m_name, type, value);
}

Type *VarDecl::infer_type_impl() const { return m_type; }

Expr *VarDecl::get_value() const { return m_value; }
void VarDecl::set_value(Expr *value) { m_value = value; }

bool LetDecl::verify_impl(std::ostream &os) const {
  if (!m_value) {
    os << "LetDecl: value is NULL\n";
    return false;
  }

  if (!m_type) {
    os << "LetDecl: type is NULL\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "LetDecl: value is invalid\n";
    return false;
  }

  if (!m_type->verify(os)) {
    os << "LetDecl: type is invalid\n";
    return false;
  }

  return true;
}

void LetDecl::canonicalize_impl() {
  if (m_value) {
    m_value->canonicalize();
  }
}

void LetDecl::print_impl(std::ostream &os, bool debug) const {
  os << "let " << m_name;

  if (m_type) {
    os << ": ";
    m_type->print(os, debug);
  }

  if (m_value) {
    os << " = ";
    m_value->print(os, debug);
  }

  os << ";";
}

LetDecl *LetDecl::clone_impl() const {
  Expr *value = m_value ? m_value->clone() : nullptr;
  Type *type = m_type ? m_type->clone() : nullptr;
  return LetDecl::get(m_name, type, value);
}

Type *LetDecl::infer_type_impl() const { return m_type; }

Expr *LetDecl::get_value() const { return m_value; }
void LetDecl::set_value(Expr *value) { m_value = value; }

bool InlineAsm::verify_impl(std::ostream &os) const {
  for (auto item : m_args) {
    if (!item) {
      os << "InlineAsm: item is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "InlineAsm: item is invalid\n";
      return false;
    }
  }

  /// TODO: Implement ASM verification logic
  return true;
}

void InlineAsm::canonicalize_impl() {
  for (auto item : m_args) {
    if (item) {
      item->canonicalize();
    }
  }

  /// TODO: Implement ASM canonicalization logic
}

void InlineAsm::print_impl(std::ostream &os, bool debug) const {
  os << "asm(\"" << m_code << "\"";

  for (auto item : m_args) {
    os << ", ";
    item->print(os, debug);
  }

  os << ");";
}

InlineAsm *InlineAsm::clone_impl() const {
  InlineAsmArgs args;
  for (auto item : m_args) {
    if (item) {
      args.push_back(item->clone());
    } else {
      args.push_back(nullptr);
    }
  }

  return InlineAsm::get(m_code, args);
}

std::string_view InlineAsm::get_code() const { return m_code; }
void InlineAsm::set_code(std::string_view code) { m_code = code; }

const InlineAsmArgs &InlineAsm::get_args() const { return m_args; }
void InlineAsm::add_arg(Expr *arg) { m_args.push_back(arg); }
void InlineAsm::clear_args() { m_args.clear(); }
void InlineAsm::remove_arg(Expr *arg) {
  std::erase_if(m_args, [arg](auto &field) { return field == arg; });
}

bool IfStmt::verify_impl(std::ostream &os) const {
  if (!m_cond) {
    os << "IfStmt: cond is NULL\n";
    return false;
  }

  if (!m_then) {
    os << "IfStmt: then is NULL\n";
    return false;
  }

  if (!m_else) {
    os << "IfStmt: else is NULL\n";
    return false;
  }

  if (!m_cond->verify(os)) {
    os << "IfStmt: cond is invalid\n";
    return false;
  }

  if (!m_then->verify(os)) {
    os << "IfStmt: then is invalid\n";
    return false;
  }

  if (!m_else->verify(os)) {
    os << "IfStmt: else is invalid\n";
    return false;
  }

  return true;
}

void IfStmt::canonicalize_impl() {
  if (m_cond) {
    m_cond->canonicalize();
  }

  if (m_then) {
    m_then->canonicalize();
  }

  if (m_else) {
    m_else->canonicalize();
  }
}

void IfStmt::print_impl(std::ostream &os, bool debug) const {
  os << "if (";
  m_cond->print(os, debug);
  os << ") ";

  if (m_then) {
    m_then->print(os, debug);
  } else {
    os << "?";
  }

  os << " else ";

  if (m_else) {
    m_else->print(os, debug);
  } else {
    os << "?";
  }
}

IfStmt *IfStmt::clone_impl() const {
  Expr *cond = m_cond ? m_cond->clone() : nullptr;
  Stmt *then = m_then ? m_then->clone() : nullptr;
  Stmt *els = m_else ? m_else->clone() : nullptr;

  return IfStmt::get(cond, then, els);
}

Expr *IfStmt::get_cond() const { return m_cond; }
void IfStmt::set_cond(Expr *cond) { m_cond = cond; }

Stmt *IfStmt::get_then() const { return m_then; }
void IfStmt::set_then(Stmt *then) { m_then = then; }

Stmt *IfStmt::get_else() const { return m_else; }
void IfStmt::set_else(Stmt *els) { m_else = els; }

bool WhileStmt::verify_impl(std::ostream &os) const {
  if (!m_cond) {
    os << "WhileStmt: cond is NULL\n";
    return false;
  }

  if (!m_body) {
    os << "WhileStmt: body is NULL\n";
    return false;
  }

  if (!m_cond->verify(os)) {
    os << "WhileStmt: cond is invalid\n";
    return false;
  }

  if (!m_body->verify(os)) {
    os << "WhileStmt: body is invalid\n";
    return false;
  }

  return true;
}

void WhileStmt::canonicalize_impl() {
  if (m_cond) {
    m_cond->canonicalize();
  }

  if (m_body) {
    m_body->canonicalize();
  }
}

void WhileStmt::print_impl(std::ostream &os, bool debug) const {
  os << "while (";
  m_cond->print(os, debug);
  os << ") ";

  if (m_body) {
    m_body->print(os, debug);
  } else {
    os << "?";
  }
}

WhileStmt *WhileStmt::clone_impl() const {
  Expr *cond = m_cond ? m_cond->clone() : nullptr;
  Stmt *body = m_body ? m_body->clone() : nullptr;

  return WhileStmt::get(cond, body);
}

Expr *WhileStmt::get_cond() const { return m_cond; }
void WhileStmt::set_cond(Expr *cond) { m_cond = cond; }

Stmt *WhileStmt::get_body() const { return m_body; }
void WhileStmt::set_body(Stmt *body) { m_body = body; }

bool ForStmt::verify_impl(std::ostream &os) const {
  if (!m_init) {
    os << "ForStmt: init is NULL\n";
    return false;
  }

  if (!m_cond) {
    os << "ForStmt: cond is NULL\n";
    return false;
  }

  if (!m_step) {
    os << "ForStmt: incr is NULL\n";
    return false;
  }

  if (!m_body) {
    os << "ForStmt: body is NULL\n";
    return false;
  }

  if (!m_init->verify(os)) {
    os << "ForStmt: init is invalid\n";
    return false;
  }

  if (!m_cond->verify(os)) {
    os << "ForStmt: cond is invalid\n";
    return false;
  }

  if (!m_step->verify(os)) {
    os << "ForStmt: incr is invalid\n";
    return false;
  }

  if (!m_body->verify(os)) {
    os << "ForStmt: body is invalid\n";
    return false;
  }

  return true;
}

void ForStmt::canonicalize_impl() {
  if (m_init) {
    m_init->canonicalize();
  }

  if (m_cond) {
    m_cond->canonicalize();
  }

  if (m_step) {
    m_step->canonicalize();
  }

  if (m_body) {
    m_body->canonicalize();
  }
}

void ForStmt::print_impl(std::ostream &os, bool debug) const {
  os << "for (";

  if (m_init) {
    m_init->print(os, debug);
  } else {
    os << "?";
  }

  os << "; ";

  if (m_cond) {
    m_cond->print(os, debug);
  } else {
    os << "?";
  }

  os << "; ";

  if (m_step) {
    m_step->print(os, debug);
  } else {
    os << "?";
  }

  os << ") ";

  if (m_body) {
    m_body->print(os, debug);
  } else {
    os << "?";
  }
}

ForStmt *ForStmt::clone_impl() const {
  Stmt *init = m_init ? m_init->clone() : nullptr;
  Expr *cond = m_cond ? m_cond->clone() : nullptr;
  Stmt *step = m_step ? m_step->clone() : nullptr;
  Stmt *body = m_body ? m_body->clone() : nullptr;

  return ForStmt::get(init, cond, step, body);
}

Stmt *ForStmt::get_init() const { return m_init; }
void ForStmt::set_init(Stmt *init) { m_init = init; }

Expr *ForStmt::get_cond() const { return m_cond; }
void ForStmt::set_cond(Expr *cond) { m_cond = cond; }

Stmt *ForStmt::get_step() const { return m_step; }
void ForStmt::set_step(Stmt *step) { m_step = step; }

Stmt *ForStmt::get_body() const { return m_body; }
void ForStmt::set_body(Stmt *body) { m_body = body; }

bool FormStmt::verify_impl(std::ostream &os) const {
  if (!m_init) {
    os << "FormStmt: init is NULL\n";
    return false;
  }

  if (!m_generator) {
    os << "FormStmt: generator is NULL\n";
    return false;
  }

  if (!m_body) {
    os << "FormStmt: body is NULL\n";
    return false;
  }

  if (!m_init->verify(os)) {
    os << "FormStmt: init is invalid\n";
    return false;
  }

  if (!m_generator->verify(os)) {
    os << "FormStmt: generator is invalid\n";
    return false;
  }

  if (!m_body->verify(os)) {
    os << "FormStmt: body is invalid\n";
    return false;
  }

  return true;
}

void FormStmt::canonicalize_impl() {
  if (m_init) {
    m_init->canonicalize();
  }

  if (m_generator) {
    m_generator->canonicalize();
  }

  if (m_body) {
    m_body->canonicalize();
  }
}

void FormStmt::print_impl(std::ostream &os, bool debug) const {
  os << "form (";

  if (m_init) {
    m_init->print(os, debug);
  } else {
    os << "?";
  }

  os << " in ";

  if (m_generator) {
    m_generator->print(os, debug);
  } else {
    os << "?";
  }

  os << ") ";

  if (m_body) {
    m_body->print(os, debug);
  } else {
    os << "?";
  }
}

FormStmt *FormStmt::clone_impl() const {
  Expr *init = m_init ? m_init->clone() : nullptr;
  Expr *generator = m_generator ? m_generator->clone() : nullptr;
  Stmt *body = m_body ? m_body->clone() : nullptr;

  return FormStmt::get(init, generator, body);
}

Expr *FormStmt::get_init() const { return m_init; }
void FormStmt::set_init(Expr *init) { m_init = init; }

Expr *FormStmt::get_generator() const { return m_generator; }
void FormStmt::set_generator(Expr *generator) { m_generator = generator; }

Stmt *FormStmt::get_body() const { return m_body; }
void FormStmt::set_body(Stmt *body) { m_body = body; }

bool ForeachStmt::verify_impl(std::ostream &os) const {
  quixcc_panic("ForeachStmt::verify_impl() is not implemented");
}

void ForeachStmt::canonicalize_impl() {
  quixcc_panic("ForeachStmt::canonicalize_impl() is not implemented");
}

void ForeachStmt::print_impl(std::ostream &os, bool debug) const {
  quixcc_panic("ForeachStmt::print_impl() is not implemented");
}

ForeachStmt *ForeachStmt::clone_impl() const {
  quixcc_panic("ForeachStmt::clone_impl() is not implemented");
}

Expr *ForeachStmt::get_iter() const { return m_iter; }
void ForeachStmt::set_iter(Expr *iter) { m_iter = iter; }

Stmt *ForeachStmt::get_body() const { return m_body; }
void ForeachStmt::set_body(Stmt *body) { m_body = body; }

bool BreakStmt::verify_impl(std::ostream &os) const { return true; }
void BreakStmt::canonicalize_impl() {}
void BreakStmt::print_impl(std::ostream &os, bool debug) const { os << "break;"; }
BreakStmt *BreakStmt::clone_impl() const { return BreakStmt::get(); }

bool ContinueStmt::verify_impl(std::ostream &os) const { return true; }
void ContinueStmt::canonicalize_impl() {}
void ContinueStmt::print_impl(std::ostream &os, bool debug) const { os << "continue;"; }
ContinueStmt *ContinueStmt::clone_impl() const { return ContinueStmt::get(); }

bool ReturnStmt::verify_impl(std::ostream &os) const {
  if (!m_value) {
    os << "ReturnStmt: value is NULL\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "ReturnStmt: value is invalid\n";
    return false;
  }

  return true;
}

void ReturnStmt::canonicalize_impl() {
  if (m_value) {
    m_value->canonicalize();
  }
}

void ReturnStmt::print_impl(std::ostream &os, bool debug) const {
  os << "return ";

  if (m_value) {
    m_value->print(os, debug);
  } else {
    os << "?";
  }

  os << ";";
}

ReturnStmt *ReturnStmt::clone_impl() const {
  Expr *value = m_value ? m_value->clone() : nullptr;
  return ReturnStmt::get(value);
}

Expr *ReturnStmt::get_value() const { return m_value; }
void ReturnStmt::set_value(Expr *value) { m_value = value; }

bool ReturnIfStmt::verify_impl(std::ostream &os) const {
  if (!m_cond) {
    os << "ReturnIfStmt: cond is NULL\n";
    return false;
  }

  if (!m_value) {
    os << "ReturnIfStmt: value is NULL\n";
    return false;
  }

  if (!m_cond->verify(os)) {
    os << "ReturnIfStmt: cond is invalid\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "ReturnIfStmt: value is invalid\n";
    return false;
  }

  return true;
}

void ReturnIfStmt::canonicalize_impl() {
  if (m_cond) {
    m_cond->canonicalize();
  }

  if (m_value) {
    m_value->canonicalize();
  }
}

void ReturnIfStmt::print_impl(std::ostream &os, bool debug) const {
  os << "retif (";

  if (m_cond) {
    m_cond->print(os, debug);
  } else {
    os << "?";
  }

  os << ") ";

  if (m_value) {
    m_value->print(os, debug);
  } else {
    os << "?";
  }

  os << ";";
}

ReturnIfStmt *ReturnIfStmt::clone_impl() const {
  Expr *cond = m_cond ? m_cond->clone() : nullptr;
  Expr *value = m_value ? m_value->clone() : nullptr;

  return ReturnIfStmt::get(cond, value);
}

Expr *ReturnIfStmt::get_cond() const { return m_cond; }
void ReturnIfStmt::set_cond(Expr *cond) { m_cond = cond; }

Expr *ReturnIfStmt::get_value() const { return m_value; }
void ReturnIfStmt::set_value(Expr *value) { m_value = value; }

bool RetZStmt::verify_impl(std::ostream &os) const {
  if (!m_cond) {
    os << "RetZStmt: cond is NULL\n";
    return false;
  }

  if (!m_value) {
    os << "RetZStmt: value is NULL\n";
    return false;
  }

  if (!m_cond->verify(os)) {
    os << "RetZStmt: cond is invalid\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "RetZStmt: value is invalid\n";
    return false;
  }

  return true;
}

void RetZStmt::canonicalize_impl() {
  if (m_cond) {
    m_cond->canonicalize();
  }

  if (m_value) {
    m_value->canonicalize();
  }
}

void RetZStmt::print_impl(std::ostream &os, bool debug) const {
  os << "retz (";

  if (m_cond) {
    m_cond->print(os, debug);
  } else {
    os << "?";
  }

  os << ") ";

  if (m_value) {
    m_value->print(os, debug);
  } else {
    os << "?";
  }

  os << ";";
}

RetZStmt *RetZStmt::clone_impl() const {
  Expr *cond = m_cond ? m_cond->clone() : nullptr;
  Expr *value = m_value ? m_value->clone() : nullptr;

  return RetZStmt::get(cond, value);
}

Expr *RetZStmt::get_cond() const { return m_cond; }
void RetZStmt::set_cond(Expr *cond) { m_cond = cond; }

Expr *RetZStmt::get_value() const { return m_value; }
void RetZStmt::set_value(Expr *value) { m_value = value; }

bool RetVStmt::verify_impl(std::ostream &os) const {
  if (!m_cond) {
    os << "RetVStmt: cond is NULL\n";
    return false;
  }

  return true;
}

void RetVStmt::canonicalize_impl() {
  if (m_cond) {
    m_cond->canonicalize();
  }
}

void RetVStmt::print_impl(std::ostream &os, bool debug) const {
  os << "retv (";

  if (m_cond) {
    m_cond->print(os, debug);
  } else {
    os << "?";
  }

  os << ")";
}

RetVStmt *RetVStmt::clone_impl() const {
  Expr *cond = m_cond ? m_cond->clone() : nullptr;

  return RetVStmt::get(cond);
}

Expr *RetVStmt::get_cond() const { return m_cond; }
void RetVStmt::set_cond(Expr *cond) { m_cond = cond; }

bool CaseStmt::verify_impl(std::ostream &os) const {
  if (!m_cond) {
    os << "CaseStmt: cond is NULL\n";
    return false;
  }

  if (!m_body) {
    os << "CaseStmt: body is NULL\n";
    return false;
  }

  if (!m_cond->verify(os)) {
    os << "CaseStmt: cond is invalid\n";
    return false;
  }

  if (!m_body->verify(os)) {
    os << "CaseStmt: body is invalid\n";
    return false;
  }

  return true;
}

void CaseStmt::canonicalize_impl() {
  if (m_cond) {
    m_cond->canonicalize();
  }

  if (m_body) {
    m_body->canonicalize();
  }
}

void CaseStmt::print_impl(std::ostream &os, bool debug) const {
  os << "case ";

  if (m_cond) {
    m_cond->print(os, debug);
  } else {
    os << "?";
  }

  os << ":\n";

  if (m_body) {
    m_body->print(os, debug);
  } else {
    os << "?";
  }
}

CaseStmt *CaseStmt::clone_impl() const {
  Expr *cond = m_cond ? m_cond->clone() : nullptr;
  Stmt *body = m_body ? m_body->clone() : nullptr;

  return CaseStmt::get(cond, body);
}

Expr *CaseStmt::get_cond() const { return m_cond; }
void CaseStmt::set_cond(Expr *cond) { m_cond = cond; }

Stmt *CaseStmt::get_body() const { return m_body; }
void CaseStmt::set_body(Stmt *body) { m_body = body; }

bool SwitchStmt::verify_impl(std::ostream &os) const {
  if (!m_cond) {
    os << "SwitchStmt: cond is NULL\n";
    return false;
  }

  if (!m_cond->verify(os)) {
    os << "SwitchStmt: cond is invalid\n";
    return false;
  }

  for (auto item : m_cases) {
    if (!item) {
      os << "SwitchStmt: case is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "SwitchStmt: case is invalid\n";
      return false;
    }
  }

  return true;
}

void SwitchStmt::canonicalize_impl() {
  if (m_cond) {
    m_cond->canonicalize();
  }

  for (auto item : m_cases) {
    if (item) {
      item->canonicalize();
    }
  }
}

void SwitchStmt::print_impl(std::ostream &os, bool debug) const {
  os << "switch (";

  if (m_cond) {
    m_cond->print(os, debug);
  } else {
    os << "?";
  }

  os << ") {\n";

  for (auto item : m_cases) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "}";
}

SwitchStmt *SwitchStmt::clone_impl() const {
  Expr *cond = m_cond ? m_cond->clone() : nullptr;
  SwitchCases cases;
  for (auto item : m_cases) {
    if (item) {
      cases.push_back(item->clone());
    } else {
      cases.push_back(nullptr);
    }
  }

  return SwitchStmt::get(cond, m_cases);
}

Expr *SwitchStmt::get_cond() const { return m_cond; }
void SwitchStmt::set_cond(Expr *cond) { m_cond = cond; }

const SwitchCases &SwitchStmt::get_cases() const { return m_cases; }
void SwitchStmt::add_case(CaseStmt *item) { m_cases.push_back(item); }
void SwitchStmt::clear_cases() { m_cases.clear(); }
void SwitchStmt::remove_case(CaseStmt *item) {
  std::erase_if(m_cases, [item](auto &field) { return field == item; });
}

bool TypedefDecl::verify_impl(std::ostream &os) const {
  if (!m_type) {
    os << "TypedefDecl: type is NULL\n";
    return false;
  }

  if (!m_type->verify(os)) {
    os << "TypedefDecl: type is invalid\n";
    return false;
  }

  return true;
}

void TypedefDecl::canonicalize_impl() {
  if (m_type) {
    m_type->canonicalize();
  }
}

void TypedefDecl::print_impl(std::ostream &os, bool debug) const {
  os << "typedef ";

  if (m_type) {
    m_type->print(os, debug);
  } else {
    os << "?";
  }

  os << " " << m_name << ";";
}

TypedefDecl *TypedefDecl::clone_impl() const {
  Type *type = m_type ? m_type->clone() : nullptr;
  return TypedefDecl::get(m_name, type);
}

Type *TypedefDecl::infer_type_impl() const { return m_type; }

bool FnDecl::verify_impl(std::ostream &os) const {
  if (!m_type) {
    os << "FnDecl: type is NULL\n";
    return false;
  }

  if (!m_type->verify(os)) {
    os << "FnDecl: type is invalid\n";
    return false;
  }

  return true;
}

void FnDecl::canonicalize_impl() {
  if (m_type) {
    m_type->canonicalize();
  }
}

void FnDecl::print_impl(std::ostream &os, bool debug) const {
  /// TODO: Re-implement this function
  os << "fn " << m_name << "(";

  if (m_type) {
    m_type->print(os, debug);
  } else {
    os << "?";
  }

  os << ")";
}

FnDecl *FnDecl::clone_impl() const {
  if (m_type) {
    return FnDecl::get(m_name, m_type->clone()->as<FuncTy>());
  } else {
    return FnDecl::get(m_name, nullptr);
  }
}

Type *FnDecl::infer_type_impl() const { return m_type; }

FuncTy *FnDecl::get_type() const {
  if (!m_type) {
    return nullptr;
  }

  return m_type->as<FuncTy>();
}

bool FnDef::verify_impl(std::ostream &os) const {
  if (!m_type) {
    os << "FnDef: type is NULL\n";
    return false;
  }

  if (!m_type->verify(os)) {
    os << "FnDef: type is invalid\n";
    return false;
  }

  if (!m_body) {
    os << "FnDef: body is NULL\n";
    return false;
  }

  if (!m_body->verify(os)) {
    os << "FnDef: body is invalid\n";
    return false;
  }

  return true;
}

void FnDef::canonicalize_impl() {
  if (m_type) {
    m_type->canonicalize();
  }

  if (m_body) {
    m_body->canonicalize();
  }
}

void FnDef::print_impl(std::ostream &os, bool debug) const {
  os << "fn " << m_name << "(";

  if (m_type) {
    m_type->print(os, debug);
  } else {
    os << "?";
  }

  os << ") ";

  if (m_body) {
    m_body->print(os, debug);
  } else {
    os << "?";
  }
}

FnDef *FnDef::clone_impl() const {
  Stmt *body = m_body ? m_body->clone() : nullptr;

  if (m_type) {
    return FnDef::get(m_name, m_type->clone()->as<FuncTy>(), body);
  } else {
    return FnDef::get(m_name, nullptr, body);
  }
}

bool CompositeField::verify_impl(std::ostream &os) const {
  if (!m_type) {
    os << "CompositeField: type is NULL\n";
    return false;
  }

  if (!m_type->verify(os)) {
    os << "CompositeField: type is invalid\n";
    return false;
  }

  if (!m_value) {
    os << "CompositeField: value is NULL\n";
    return false;
  }

  if (!m_value->verify(os)) {
    os << "CompositeField: value is invalid\n";
    return false;
  }

  return true;
}

void CompositeField::canonicalize_impl() {
  if (m_type) {
    m_type->canonicalize();
  }

  if (m_value) {
    m_value->canonicalize();
  }
}

void CompositeField::print_impl(std::ostream &os, bool debug) const {
  os << m_name;

  if (m_type) {
    os << ": ";
    m_type->print(os, debug);
  }

  if (m_value) {
    os << " = ";
    m_value->print(os, debug);
  }
}

CompositeField *CompositeField::clone_impl() const {
  Type *type = m_type ? m_type->clone() : nullptr;
  Expr *value = m_value ? m_value->clone() : nullptr;

  return CompositeField::get(m_name, type, value);
}

Type *CompositeField::infer_type_impl() const { return m_type; }

Expr *CompositeField::get_value() const { return m_value; }
void CompositeField::set_value(Expr *value) { m_value = value; }

bool StructDef::verify_impl(std::ostream &os) const {
  for (auto item : m_methods) {
    if (!item) {
      os << "StructDef: method is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "StructDef: method is invalid\n";
      return false;
    }
  }

  for (auto item : m_static_methods) {
    if (!item) {
      os << "StructDef: static method is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "StructDef: static method is invalid\n";
      return false;
    }
  }

  for (auto item : m_fields) {
    if (!item) {
      os << "StructDef: field is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "StructDef: field is invalid\n";
      return false;
    }
  }

  return true;
}

void StructDef::canonicalize_impl() {
  for (auto item : m_methods) {
    if (item) {
      item->canonicalize();
    }
  }

  for (auto item : m_static_methods) {
    if (item) {
      item->canonicalize();
    }
  }

  for (auto item : m_fields) {
    if (item) {
      item->canonicalize();
    }
  }
}

void StructDef::print_impl(std::ostream &os, bool debug) const {
  os << "struct " << m_name << " {\n";

  for (auto item : m_fields) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "\n";

  for (auto item : m_methods) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "\n";

  for (auto item : m_static_methods) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "}";
}

StructDef *StructDef::clone_impl() const {
  auto type = m_type ? m_type->clone() : nullptr;

  StructDefFields fields;
  for (auto item : m_fields) {
    if (item) {
      fields.push_back(item->clone());
    } else {
      fields.push_back(nullptr);
    }
  }

  StructDefMethods methods;
  for (auto item : m_methods) {
    if (item) {
      methods.push_back(item->clone());
    } else {
      methods.push_back(nullptr);
    }
  }

  StructDefStaticMethods static_methods;
  for (auto item : m_static_methods) {
    if (item) {
      static_methods.push_back(item->clone());
    } else {
      static_methods.push_back(nullptr);
    }
  }

  return StructDef::get(m_name, static_cast<StructTy *>(type), fields, methods, static_methods);
}

StructTy *StructDef::get_type() const { return static_cast<StructTy *>(m_type); }

const StructDefMethods &StructDef::get_methods() const { return m_methods; }
void StructDef::add_method(FnDecl *item) { m_methods.push_back(item); }
void StructDef::add_methods(std::initializer_list<FnDecl *> items) {
  m_methods.insert(m_methods.end(), items.begin(), items.end());
}
void StructDef::clear_methods() { m_methods.clear(); }
void StructDef::remove_method(FnDecl *item) {
  std::erase_if(m_methods, [item](auto &field) { return field == item; });
}

const StructDefStaticMethods &StructDef::get_static_methods() const { return m_static_methods; }
void StructDef::add_static_method(FnDecl *item) { m_static_methods.push_back(item); }
void StructDef::add_static_methods(std::initializer_list<FnDecl *> items) {
  m_static_methods.insert(m_static_methods.end(), items.begin(), items.end());
}
void StructDef::clear_static_methods() { m_static_methods.clear(); }
void StructDef::remove_static_method(FnDecl *item) {
  std::erase_if(m_static_methods, [item](auto &field) { return field == item; });
}

const StructDefFields &StructDef::get_fields() const { return m_fields; }
void StructDef::add_field(CompositeField *item) { m_fields.push_back(item); }
void StructDef::add_fields(std::initializer_list<CompositeField *> items) {
  m_fields.insert(m_fields.end(), items.begin(), items.end());
}
void StructDef::clear_fields() { m_fields.clear(); }
void StructDef::remove_field(CompositeField *item) {
  std::erase_if(m_fields, [item](auto &field) { return field == item; });
}

Type *StructDef::infer_type_impl() const { return m_type; }

bool GroupDef::verify_impl(std::ostream &os) const {
  for (auto item : m_methods) {
    if (!item) {
      os << "GroupDef: method is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "GroupDef: method is invalid\n";
      return false;
    }
  }

  for (auto item : m_static_methods) {
    if (!item) {
      os << "GroupDef: static method is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "GroupDef: static method is invalid\n";
      return false;
    }
  }

  for (auto item : m_fields) {
    if (!item) {
      os << "GroupDef: field is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "GroupDef: field is invalid\n";
      return false;
    }
  }

  return true;
}

void GroupDef::canonicalize_impl() {
  for (auto item : m_methods) {
    if (item) {
      item->canonicalize();
    }
  }

  for (auto item : m_static_methods) {
    if (item) {
      item->canonicalize();
    }
  }

  for (auto item : m_fields) {
    if (item) {
      item->canonicalize();
    }
  }
}

void GroupDef::print_impl(std::ostream &os, bool debug) const {
  os << "group " << m_name << " {\n";

  for (auto item : m_fields) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "\n";

  for (auto item : m_methods) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "\n";

  for (auto item : m_static_methods) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "}";
}

GroupDef *GroupDef::clone_impl() const {
  Type *type = m_type ? m_type->clone() : nullptr;

  GroupDefFields fields;
  for (auto item : m_fields) {
    if (item) {
      fields.push_back(item->clone());
    } else {
      fields.push_back(nullptr);
    }
  }

  GroupDefMethods methods;
  for (auto item : m_methods) {
    if (item) {
      methods.push_back(item->clone());
    } else {
      methods.push_back(nullptr);
    }
  }

  GroupDefStaticMethods static_methods;
  for (auto item : m_static_methods) {
    if (item) {
      static_methods.push_back(item->clone());
    } else {
      static_methods.push_back(nullptr);
    }
  }

  return GroupDef::get(m_name, static_cast<GroupTy *>(type), fields, methods, static_methods);
}

GroupTy *GroupDef::get_type() const { return static_cast<GroupTy *>(m_type); }

const GroupDefMethods &GroupDef::get_methods() const { return m_methods; }
void GroupDef::add_method(FnDecl *item) { m_methods.push_back(item); }
void GroupDef::add_methods(std::initializer_list<FnDecl *> items) {
  m_methods.insert(m_methods.end(), items.begin(), items.end());
}
void GroupDef::clear_methods() { m_methods.clear(); }
void GroupDef::remove_method(FnDecl *item) {
  std::erase_if(m_methods, [item](auto &field) { return field == item; });
}

const GroupDefStaticMethods &GroupDef::get_static_methods() const { return m_static_methods; }
void GroupDef::add_static_method(FnDecl *item) { m_static_methods.push_back(item); }
void GroupDef::add_static_methods(std::initializer_list<FnDecl *> items) {
  m_static_methods.insert(m_static_methods.end(), items.begin(), items.end());
}
void GroupDef::clear_static_methods() { m_static_methods.clear(); }
void GroupDef::remove_static_method(FnDecl *item) {
  std::erase_if(m_static_methods, [item](auto &field) { return field == item; });
}

const GroupDefFields &GroupDef::get_fields() const { return m_fields; }
void GroupDef::add_field(CompositeField *item) { m_fields.push_back(item); }
void GroupDef::add_fields(std::initializer_list<CompositeField *> items) {
  m_fields.insert(m_fields.end(), items.begin(), items.end());
}
void GroupDef::clear_fields() { m_fields.clear(); }
void GroupDef::remove_field(CompositeField *item) {
  std::erase_if(m_fields, [item](auto &field) { return field == item; });
}

Type *GroupDef::infer_type_impl() const { return m_type; }

bool RegionDef::verify_impl(std::ostream &os) const {
  for (auto item : m_methods) {
    if (!item) {
      os << "RegionDef: method is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "RegionDef: method is invalid\n";
      return false;
    }
  }

  for (auto item : m_static_methods) {
    if (!item) {
      os << "RegionDef: static method is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "RegionDef: static method is invalid\n";
      return false;
    }
  }

  for (auto item : m_fields) {
    if (!item) {
      os << "RegionDef: field is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "RegionDef: field is invalid\n";
      return false;
    }
  }

  return true;
}

void RegionDef::canonicalize_impl() {
  for (auto item : m_methods) {
    if (item) {
      item->canonicalize();
    }
  }

  for (auto item : m_static_methods) {
    if (item) {
      item->canonicalize();
    }
  }

  for (auto item : m_fields) {
    if (item) {
      item->canonicalize();
    }
  }
}

void RegionDef::print_impl(std::ostream &os, bool debug) const {
  os << "region " << m_name << " {\n";

  for (auto item : m_fields) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "\n";

  for (auto item : m_methods) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "\n";

  for (auto item : m_static_methods) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "}";
}

RegionDef *RegionDef::clone_impl() const {
  Type *type = m_type ? m_type->clone() : nullptr;

  RegionDefFields fields;
  for (auto item : m_fields) {
    if (item) {
      fields.push_back(item->clone());
    } else {
      fields.push_back(nullptr);
    }
  }

  RegionDefMethods methods;
  for (auto item : m_methods) {
    if (item) {
      methods.push_back(item->clone());
    } else {
      methods.push_back(nullptr);
    }
  }

  RegionDefStaticMethods static_methods;
  for (auto item : m_static_methods) {
    if (item) {
      static_methods.push_back(item->clone());
    } else {
      static_methods.push_back(nullptr);
    }
  }

  return RegionDef::get(m_name, static_cast<RegionTy *>(type), fields, methods, static_methods);
}

RegionTy *RegionDef::get_type() const { return static_cast<RegionTy *>(m_type); }

const RegionDefMethods &RegionDef::get_methods() const { return m_methods; }
void RegionDef::add_method(FnDecl *item) { m_methods.push_back(item); }
void RegionDef::add_methods(std::initializer_list<FnDecl *> items) {
  m_methods.insert(m_methods.end(), items.begin(), items.end());
}
void RegionDef::clear_methods() { m_methods.clear(); }
void RegionDef::remove_method(FnDecl *item) {
  std::erase_if(m_methods, [item](auto &field) { return field == item; });
}

const RegionDefStaticMethods &RegionDef::get_static_methods() const { return m_static_methods; }
void RegionDef::add_static_method(FnDecl *item) { m_static_methods.push_back(item); }
void RegionDef::add_static_methods(std::initializer_list<FnDecl *> items) {
  m_static_methods.insert(m_static_methods.end(), items.begin(), items.end());
}
void RegionDef::clear_static_methods() { m_static_methods.clear(); }
void RegionDef::remove_static_method(FnDecl *item) {
  std::erase_if(m_static_methods, [item](auto &field) { return field == item; });
}

const RegionDefFields &RegionDef::get_fields() const { return m_fields; }
void RegionDef::add_field(CompositeField *item) { m_fields.push_back(item); }
void RegionDef::add_fields(std::initializer_list<CompositeField *> items) {
  m_fields.insert(m_fields.end(), items.begin(), items.end());
}
void RegionDef::clear_fields() { m_fields.clear(); }
void RegionDef::remove_field(CompositeField *item) {
  std::erase_if(m_fields, [item](auto &field) { return field == item; });
}

Type *RegionDef::infer_type_impl() const { return m_type; }

bool UnionDef::verify_impl(std::ostream &os) const {
  for (auto item : m_methods) {
    if (!item) {
      os << "UnionDef: method is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "UnionDef: method is invalid\n";
      return false;
    }
  }

  for (auto item : m_static_methods) {
    if (!item) {
      os << "UnionDef: static method is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "UnionDef: static method is invalid\n";
      return false;
    }
  }

  for (auto item : m_fields) {
    if (!item) {
      os << "UnionDef: field is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "UnionDef: field is invalid\n";
      return false;
    }
  }

  return true;
}

void UnionDef::canonicalize_impl() {
  for (auto item : m_methods) {
    if (item) {
      item->canonicalize();
    }
  }

  for (auto item : m_static_methods) {
    if (item) {
      item->canonicalize();
    }
  }

  for (auto item : m_fields) {
    if (item) {
      item->canonicalize();
    }
  }
}

void UnionDef::print_impl(std::ostream &os, bool debug) const {
  os << "union " << m_name << " {\n";

  for (auto item : m_fields) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "\n";

  for (auto item : m_methods) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "\n";

  for (auto item : m_static_methods) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "}";
}

UnionDef *UnionDef::clone_impl() const {
  Type *type = m_type ? m_type->clone() : nullptr;

  UnionDefFields fields;
  for (auto item : m_fields) {
    if (item) {
      fields.push_back(item->clone());
    } else {
      fields.push_back(nullptr);
    }
  }

  UnionDefMethods methods;
  for (auto item : m_methods) {
    if (item) {
      methods.push_back(item->clone());
    } else {
      methods.push_back(nullptr);
    }
  }

  UnionDefStaticMethods static_methods;
  for (auto item : m_static_methods) {
    if (item) {
      static_methods.push_back(item->clone());
    } else {
      static_methods.push_back(nullptr);
    }
  }

  return UnionDef::get(m_name, static_cast<UnionTy *>(type), fields, methods, static_methods);
}

UnionTy *UnionDef::get_type() const { return static_cast<UnionTy *>(m_type); }

const UnionDefMethods &UnionDef::get_methods() const { return m_methods; }
void UnionDef::add_method(FnDecl *item) { m_methods.push_back(item); }
void UnionDef::add_methods(std::initializer_list<FnDecl *> items) {
  m_methods.insert(m_methods.end(), items.begin(), items.end());
}
void UnionDef::clear_methods() { m_methods.clear(); }
void UnionDef::remove_method(FnDecl *item) {
  std::erase_if(m_methods, [item](auto &field) { return field == item; });
}

const UnionDefStaticMethods &UnionDef::get_static_methods() const { return m_static_methods; }
void UnionDef::add_static_method(FnDecl *item) { m_static_methods.push_back(item); }
void UnionDef::add_static_methods(std::initializer_list<FnDecl *> items) {
  m_static_methods.insert(m_static_methods.end(), items.begin(), items.end());
}
void UnionDef::clear_static_methods() { m_static_methods.clear(); }
void UnionDef::remove_static_method(FnDecl *item) {
  std::erase_if(m_static_methods, [item](auto &field) { return field == item; });
}

const UnionDefFields &UnionDef::get_fields() const { return m_fields; }
void UnionDef::add_field(CompositeField *item) { m_fields.push_back(item); }
void UnionDef::add_fields(std::initializer_list<CompositeField *> items) {
  m_fields.insert(m_fields.end(), items.begin(), items.end());
}
void UnionDef::clear_fields() { m_fields.clear(); }
void UnionDef::remove_field(CompositeField *item) {
  std::erase_if(m_fields, [item](auto &field) { return field == item; });
}

Type *UnionDef::infer_type_impl() const { return m_type; }

bool EnumDef::verify_impl(std::ostream &os) const {
  for (auto item : m_items) {
    if (!item.second) {
      os << "EnumDef: item is NULL\n";
      return false;
    }

    if (!item.second->verify(os)) {
      os << "EnumDef: item is invalid\n";
      return false;
    }
  }

  return true;
}

void EnumDef::canonicalize_impl() {
  for (auto item : m_items) {
    if (item.second) {
      item.second->canonicalize();
    }
  }
}

void EnumDef::print_impl(std::ostream &os, bool debug) const {
  os << "enum " << m_name << " {\n";

  for (auto item : m_items) {
    os << item.first;

    if (item.second) {
      os << " = ";
      item.second->print(os, debug);
    }

    os << ",\n";
  }

  os << "}";
}

EnumTy *EnumDef::get_type() const { return static_cast<EnumTy *>(m_type); }

EnumDef *EnumDef::clone_impl() const {
  Type *type = m_type ? m_type->clone() : nullptr;

  EnumDefItems items;
  for (auto item : m_items) {
    if (item.second) {
      items.push_back({item.first, item.second->clone()});
    } else {
      items.push_back({item.first, nullptr});
    }
  }

  return EnumDef::get(m_name, static_cast<EnumTy *>(type), items);
}

const EnumDefItems &EnumDef::get_items() const { return m_items; }
void EnumDef::add_item(EnumItem item) { m_items.push_back(item); }
void EnumDef::add_items(std::initializer_list<EnumItem> items) {
  m_items.insert(m_items.end(), items.begin(), items.end());
}
void EnumDef::clear_items() { m_items.clear(); }
void EnumDef::remove_item(EnumItem item) {
  std::erase_if(m_items, [item](auto &field) { return field == item; });
}

Type *EnumDef::infer_type_impl() const { return m_type; }

bool SubsystemDecl::verify_impl(std::ostream &os) const {
  if (m_type) {
    os << "SubsystemDecl: type must be NULL\n";
    return false;
  }

  for (auto item : m_body) {
    if (!item) {
      os << "SubsystemDecl: item is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "SubsystemDecl: item is invalid\n";
      return false;
    }
  }

  return true;
}

void SubsystemDecl::canonicalize_impl() {
  for (auto item : m_body) {
    if (item) {
      item->canonicalize();
    }
  }
}

void SubsystemDecl::print_impl(std::ostream &os, bool debug) const {
  os << "subsystem " << m_name << " {\n";

  for (auto item : m_body) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "}";
}

SubsystemDecl *SubsystemDecl::clone_impl() const {
  SubsystemDeclBody body;
  for (auto item : m_body) {
    if (item) {
      body.push_back(item->clone());
    } else {
      body.push_back(nullptr);
    }
  }

  return SubsystemDecl::get(m_name, body);
}

const SubsystemDeclBody &SubsystemDecl::get_item() const { return m_body; }
void SubsystemDecl::add_item(Decl *item) { m_body.push_back(item); }
void SubsystemDecl::add_items(std::initializer_list<Decl *> items) {
  m_body.insert(m_body.end(), items.begin(), items.end());
}
void SubsystemDecl::clear_items() { m_body.clear(); }
void SubsystemDecl::remove_item(Decl *item) {
  std::erase_if(m_body, [item](auto &field) { return field == item; });
}

Type *SubsystemDecl::infer_type_impl() const { return nullptr; }

bool ExportDecl::verify_impl(std::ostream &os) const {
  if (m_type) {
    os << "ExportDecl: type must be NULL\n";
    return false;
  }

  for (auto item : m_body) {
    if (!item) {
      os << "ExportDecl: item is NULL\n";
      return false;
    }

    if (!item->verify(os)) {
      os << "ExportDecl: item is invalid\n";
      return false;
    }
  }

  return true;
}

void ExportDecl::canonicalize_impl() {
  for (auto item : m_body) {
    if (item) {
      item->canonicalize();
    }
  }
}

void ExportDecl::print_impl(std::ostream &os, bool debug) const {
  os << "export " << m_name << " {\n";

  for (auto item : m_body) {
    if (item) {
      item->print(os, debug);
    } else {
      os << "?";
    }

    os << "\n";
  }

  os << "}";
}

ExportDecl *ExportDecl::clone_impl() const {
  ExportDeclBody body;
  for (auto item : m_body) {
    if (item) {
      body.push_back(item->clone());
    } else {
      body.push_back(nullptr);
    }
  }

  return ExportDecl::get(m_name, body);
}

const ExportDeclBody &ExportDecl::get_item() const { return m_body; }
void ExportDecl::add_item(Decl *item) { m_body.push_back(item); }
void ExportDecl::add_items(std::initializer_list<Decl *> items) {
  m_body.insert(m_body.end(), items.begin(), items.end());
}
void ExportDecl::clear_items() { m_body.clear(); }
void ExportDecl::remove_item(Decl *item) {
  std::erase_if(m_body, [item](auto &field) { return field == item; });
}

Type *ExportDecl::infer_type_impl() const { return nullptr; }

///=============================================================================

LIB_EXPORT quixcc_ast_node_t *quixcc_ast_alloc(quixcc_ast_ntype_t type, quixcc_arena_t *arena) {
  Node *node = nullptr;

  g_allocator.swap(*arena);

  switch (type) {
  case QUIXCC_AST_NODE_STMT:
    quixcc_panic("quixcc_ast_alloc(): QUIXCC_AST_NODE_STMT is not valid here");
  case QUIXCC_AST_NODE_TYPE:
    quixcc_panic("quixcc_ast_alloc(): QUIXCC_AST_NODE_TYPE is not valid here");
  case QUIXCC_AST_NODE_DECL:
    quixcc_panic("quixcc_ast_alloc(): QUIXCC_AST_NODE_DECL is not valid here");
  case QUIXCC_AST_NODE_EXPR:
    quixcc_panic("quixcc_ast_alloc(): QUIXCC_AST_NODE_EXPR is not valid here");
  case QUIXCC_AST_NODE_CEXPR:
    node = ConstExpr::get();
    break;
  case QUIXCC_AST_NODE_UNRES_TY:
    node = UnresolvedType::get();
    break;
  case QUIXCC_AST_NODE_U1_TY:
    node = U1::get();
    break;
  case QUIXCC_AST_NODE_U8_TY:
    node = U8::get();
    break;
  case QUIXCC_AST_NODE_U16_TY:
    node = U16::get();
    break;
  case QUIXCC_AST_NODE_U32_TY:
    node = U32::get();
    break;
  case QUIXCC_AST_NODE_U64_TY:
    node = U64::get();
    break;
  case QUIXCC_AST_NODE_U128_TY:
    node = U128::get();
    break;
  case QUIXCC_AST_NODE_I8_TY:
    node = I8::get();
    break;
  case QUIXCC_AST_NODE_I16_TY:
    node = I16::get();
    break;
  case QUIXCC_AST_NODE_I32_TY:
    node = I32::get();
    break;
  case QUIXCC_AST_NODE_I64_TY:
    node = I64::get();
    break;
  case QUIXCC_AST_NODE_I128_TY:
    node = I128::get();
    break;
  case QUIXCC_AST_NODE_F32_TY:
    node = F32::get();
    break;
  case QUIXCC_AST_NODE_F64_TY:
    node = F64::get();
    break;
  case QUIXCC_AST_NODE_VOID_TY:
    node = VoidTy::get();
    break;
  case QUIXCC_AST_NODE_STRING_TY:
    node = StringTy::get();
    break;
  case QUIXCC_AST_NODE_PTR_TY:
    node = PtrTy::get();
    break;
  case QUIXCC_AST_NODE_OPAQUE_TY:
    node = OpaqueTy::get();
    break;
  case QUIXCC_AST_NODE_VECTOR_TY:
    node = VectorTy::get();
    break;
  case QUIXCC_AST_NODE_SET_TY:
    node = SetTy::get();
    break;
  case QUIXCC_AST_NODE_MAP_TY:
    node = MapTy::get();
    break;
  case QUIXCC_AST_NODE_TUPLE_TY:
    node = TupleTy::get();
    break;
  case QUIXCC_AST_NODE_RESULT_TY:
    node = OptionalTy::get();
    break;
  case QUIXCC_AST_NODE_ARRAY_TY:
    node = ArrayTy::get();
    break;
  case QUIXCC_AST_NODE_ENUM_TY:
    node = EnumTy::get();
    break;
  case QUIXCC_AST_NODE_MUT_TY:
    node = MutTy::get();
    break;
  case QUIXCC_AST_NODE_STRUCT_TY:
    node = StructTy::get();
    break;
  case QUIXCC_AST_NODE_GROUP_TY:
    node = GroupTy::get();
    break;
  case QUIXCC_AST_NODE_REGION_TY:
    node = RegionTy::get();
    break;
  case QUIXCC_AST_NODE_UNION_TY:
    node = UnionTy::get();
    break;
  case QUIXCC_AST_NODE_FN_TY:
    node = FuncTy::get();
    break;
  case QUIXCC_AST_NODE_UNEXPR:
    node = UnaryExpr::get();
    break;
  case QUIXCC_AST_NODE_BINEXPR:
    node = BinExpr::get();
    break;
  case QUIXCC_AST_NODE_TEREXPR:
    node = TernaryExpr::get();
    break;
  case QUIXCC_AST_NODE_INT:
    node = ConstInt::get();
    break;
  case QUIXCC_AST_NODE_FLOAT:
    node = ConstFloat::get();
    break;
  case QUIXCC_AST_NODE_BOOL:
    node = ConstBool::get();
    break;
  case QUIXCC_AST_NODE_STRING:
    node = ConstString::get();
    break;
  case QUIXCC_AST_NODE_CHAR:
    node = ConstChar::get();
    break;
  case QUIXCC_AST_NODE_NULL:
    node = ConstNull::get();
    break;
  case QUIXCC_AST_NODE_UNDEF:
    node = ConstUndef::get();
    break;
  case QUIXCC_AST_NODE_CALL:
    node = Call::get();
    break;
  case QUIXCC_AST_NODE_LIST:
    node = List::get();
    break;
  case QUIXCC_AST_NODE_ASSOC:
    node = Assoc::get();
    break;
  case QUIXCC_AST_NODE_FIELD:
    node = Field::get();
    break;
  case QUIXCC_AST_NODE_INDEX:
    node = Index::get();
    break;
  case QUIXCC_AST_NODE_SLICE:
    node = Slice::get();
    break;
  case QUIXCC_AST_NODE_FSTRING:
    node = FString::get();
    break;
  case QUIXCC_AST_NODE_IDENT:
    node = Ident::get();
    break;
  case QUIXCC_AST_NODE_BLOCK:
    node = Block::get();
    break;
  case QUIXCC_AST_NODE_CONST:
    node = ConstDecl::get();
    break;
  case QUIXCC_AST_NODE_VAR:
    node = VarDecl::get();
    break;
  case QUIXCC_AST_NODE_LET:
    node = LetDecl::get();
    break;
  case QUIXCC_AST_NODE_INLINE_ASM:
    node = InlineAsm::get();
    break;
  case QUIXCC_AST_NODE_IF:
    node = IfStmt::get();
    break;
  case QUIXCC_AST_NODE_WHILE:
    node = WhileStmt::get();
    break;
  case QUIXCC_AST_NODE_FOR:
    node = ForStmt::get();
    break;
  case QUIXCC_AST_NODE_FORM:
    node = FormStmt::get();
    break;
  case QUIXCC_AST_NODE_FOREACH:
    node = ForeachStmt::get();
    break;
  case QUIXCC_AST_NODE_BREAK:
    node = BreakStmt::get();
    break;
  case QUIXCC_AST_NODE_CONTINUE:
    node = ContinueStmt::get();
    break;
  case QUIXCC_AST_NODE_RETURN:
    node = ReturnStmt::get();
    break;
  case QUIXCC_AST_NODE_RETIF:
    node = ReturnIfStmt::get();
    break;
  case QUIXCC_AST_NODE_RETZ:
    node = RetZStmt::get();
    break;
  case QUIXCC_AST_NODE_RETV:
    node = RetVStmt::get();
    break;
  case QUIXCC_AST_NODE_CASE:
    node = CaseStmt::get();
    break;
  case QUIXCC_AST_NODE_SWITCH:
    node = SwitchStmt::get();
    break;
  case QUIXCC_AST_NODE_TYPEDEF:
    node = TypedefDecl::get();
    break;
  case QUIXCC_AST_NODE_FNDECL:
    node = FnDecl::get();
    break;
  case QUIXCC_AST_NODE_FN:
    node = FnDef::get();
    break;
  case QUIXCC_AST_NODE_COMPOSITE_FIELD:
    node = CompositeField::get();
    break;
  case QUIXCC_AST_NODE_STRUCT:
    node = StructDef::get();
    break;
  case QUIXCC_AST_NODE_GROUP:
    node = GroupDef::get();
    break;
  case QUIXCC_AST_NODE_REGION:
    node = RegionDef::get();
    break;
  case QUIXCC_AST_NODE_UNION:
    node = UnionDef::get();
    break;
  case QUIXCC_AST_NODE_ENUM:
    node = EnumDef::get();
    break;
  case QUIXCC_AST_NODE_SUBSYSTEM:
    node = SubsystemDecl::get();
    break;
  case QUIXCC_AST_NODE_EXPORT:
    node = ExportDecl::get();
    break;
  }

  g_allocator.swap(*arena);

  return node;
}

LIB_EXPORT void quixcc_ast_done(quixcc_ast_node_t *node) { node->~quixcc_ast_node_t(); }

///=============================================================================
///=============================================================================

LIB_EXPORT void quixcc_test_hook() {}