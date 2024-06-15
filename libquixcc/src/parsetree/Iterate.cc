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

#define QUIXCC_INTERNAL

#include <parsetree/Iterate.h>
#include <parsetree/nodes/AllNodes.h>

using namespace libquixcc;
using namespace libquixcc::traversal;

template <typename T>
TraversePtr mk_ptr(std::shared_ptr<T> *ptr) {
  return std::make_pair(TraversePtrType::Smart,
                        reinterpret_cast<std::shared_ptr<ParseNode> *>(ptr));
}

template <typename T>
TraversePtr mk_ptr(T **ptr) {
  return std::make_pair(TraversePtrType::Raw,
                        reinterpret_cast<ParseNode **>(ptr));
}

size_t ParseNode::dfs_preorder(ParseTreePreorderCallback cb) {
  ParseTreePreorder iter(cb);
  return iter.next(this);
}

size_t ParseTreePreorder::dispatch(ParseNode *n) {
#define match(type) \
  if (n->is<type>()) return iter(n->as<type>())

  match(ExprStmtNode);
  match(StmtExprNode);
  match(NopStmtNode);
  match(BlockNode);
  match(StmtGroupNode);
  match(StaticCastExprNode);
  match(BitCastExprNode);
  match(SignedUpcastExprNode);
  match(UnsignedUpcastExprNode);
  match(DowncastExprNode);
  match(PtrToIntCastExprNode);
  match(IntToPtrCastExprNode);
  match(UnaryExprNode);
  match(PostUnaryExprNode);
  match(BinaryExprNode);
  match(CallExprNode);
  match(ListExprNode);
  match(MemberAccessNode);
  match(IndexNode);
  match(FStringNode);
  match(ConstUnaryExprNode);
  match(ConstPostUnaryExprNode);
  match(ConstBinaryExprNode);
  match(IdentifierNode);
  match(MutTypeNode);
  match(U8TypeNode);
  match(U16TypeNode);
  match(U32TypeNode);
  match(U64TypeNode);
  match(U128TypeNode);
  match(I8TypeNode);
  match(I16TypeNode);
  match(I32TypeNode);
  match(I64TypeNode);
  match(I128TypeNode);
  match(F32TypeNode);
  match(F64TypeNode);
  match(BoolTypeNode);
  match(VoidTypeNode);
  match(NullTypeNode);
  match(PointerTypeNode);
  match(OpaqueTypeNode);
  match(StringTypeNode);
  match(EnumTypeNode);
  match(StructTypeNode);
  match(GroupTypeNode);
  match(RegionTypeNode);
  match(UnionTypeNode);
  match(ArrayTypeNode);
  match(VectorTypeNode);
  match(ResultTypeNode);
  match(GeneratorTypeNode);
  match(FunctionTypeNode);
  match(UserTypeNode);
  match(IntegerNode);
  match(FloatLiteralNode);
  match(StringNode);
  match(CharNode);
  match(BoolLiteralNode);
  match(NullLiteralNode);
  match(UndefLiteralNode);
  match(TypedefNode);
  match(VarDeclNode);
  match(LetDeclNode);
  match(ConstDeclNode);
  match(FunctionDeclNode);
  match(StructDefNode);
  match(StructFieldNode);
  match(RegionDefNode);
  match(RegionFieldNode);
  match(GroupDefNode);
  match(GroupFieldNode);
  match(UnionDefNode);
  match(UnionFieldNode);
  match(EnumDefNode);
  match(EnumFieldNode);
  match(FunctionDefNode);
  match(FunctionParamNode);
  match(SubsystemNode);
  match(ExportNode);
  match(InlineAsmNode);
  match(ReturnStmtNode);
  match(RetifStmtNode);
  match(RetzStmtNode);
  match(RetvStmtNode);
  match(IfStmtNode);
  match(WhileStmtNode);
  match(ForStmtNode);
  match(FormStmtNode);
  match(CaseStmtNode);
  match(SwitchStmtNode);

  LOG(FATAL) << "No conversion function for node type " << (int)n->ntype
             << " found." << std::endl;

  return 0;
}

size_t ParseTreePreorder::iter(ExprStmtNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  return next(node->m_expr) + 1;
}

size_t ParseTreePreorder::iter(StmtExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_stmt));
  return next(node->m_stmt) + 1;
}

size_t ParseTreePreorder::iter(NopStmtNode *node) { return 0; }

size_t ParseTreePreorder::iter(BlockNode *node) {
  size_t count = 0;
  for (auto &stmt : node->m_stmts) {
    m_callback(m_ns, m_scope, node, mk_ptr(&stmt));
    count += next(stmt);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(StmtGroupNode *node) {
  size_t count = 0;
  for (auto &stmt : node->m_stmts) {
    m_callback(m_ns, m_scope, node, mk_ptr(&stmt));
    count += next(stmt);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(CastExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);
  return count + 1;
}

size_t ParseTreePreorder::iter(StaticCastExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);
  return count + 1;
}

size_t ParseTreePreorder::iter(BitCastExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);
  return count + 1;
}

size_t ParseTreePreorder::iter(SignedUpcastExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);
  return count + 1;
}

size_t ParseTreePreorder::iter(UnsignedUpcastExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);
  return count + 1;
}

size_t ParseTreePreorder::iter(DowncastExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);
  return count + 1;
}

size_t ParseTreePreorder::iter(PtrToIntCastExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);
  return count + 1;
}

size_t ParseTreePreorder::iter(IntToPtrCastExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);
  return count + 1;
}

size_t ParseTreePreorder::iter(UnaryExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  return next(node->m_expr) + 1;
}

size_t ParseTreePreorder::iter(PostUnaryExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  return next(node->m_expr) + 1;
}

size_t ParseTreePreorder::iter(BinaryExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_lhs));
  size_t count = next(node->m_lhs);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_rhs));
  count += next(node->m_rhs);
  return count + 1;
}

size_t ParseTreePreorder::iter(CallExprNode *node) {
  size_t count = 0;

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_callee));
  count += next(node->m_callee);

  for (auto &arg : node->m_positional_args) {
    m_callback(m_ns, m_scope, node, mk_ptr(&arg));
    count += next(arg);
  }
  for (auto &arg : node->m_named_args) {
    m_callback(m_ns, m_scope, node, mk_ptr(&arg.second));
    count += next(arg.second);
  }

  if (node->m_decl) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_decl));
    count += next(node->m_decl);
  }

  return count + 1;
}

size_t ParseTreePreorder::iter(ListExprNode *node) {
  size_t count = 0;
  for (auto &elem : node->m_elements) {
    m_callback(m_ns, m_scope, node, mk_ptr(&elem));
    count += next(elem);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(MemberAccessNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  return next(node->m_expr) + 1;
}

size_t ParseTreePreorder::iter(IndexNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  size_t count = next(node->m_expr);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_index));
  count += next(node->m_index);
  return count + 1;
}

size_t ParseTreePreorder::iter(FStringNode *node) {
  size_t count = 0;
  for (auto &elem : node->args) {
    m_callback(m_ns, m_scope, node, mk_ptr(&elem));
    count += next(elem);
  }

  return count + 1;
}

size_t ParseTreePreorder::iter(ConstUnaryExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  return next(node->m_expr) + 1;
}

size_t ParseTreePreorder::iter(ConstPostUnaryExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  return next(node->m_expr) + 1;
}

size_t ParseTreePreorder::iter(ConstBinaryExprNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_lhs));
  size_t count = next(node->m_lhs);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_rhs));
  count += next(node->m_rhs);
  return count + 1;
}

size_t ParseTreePreorder::iter(IdentifierNode *node) { return 1; }

size_t ParseTreePreorder::iter(MutTypeNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  return next(node->m_type) + 1;
}

size_t ParseTreePreorder::iter(U8TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(U16TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(U32TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(U64TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(U128TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(I8TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(I16TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(I32TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(I64TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(I128TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(F32TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(F64TypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(BoolTypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(VoidTypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(NullTypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(PointerTypeNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  return next(node->m_type) + 1;
}

size_t ParseTreePreorder::iter(OpaqueTypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(StringTypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(EnumTypeNode *node) {
  if (!node->m_member_type) return 1;

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_member_type));
  return next(node->m_member_type) + 1;
}

size_t ParseTreePreorder::iter(StructTypeNode *node) {
  if (m_visited.contains(node)) return 1;

  m_visited.insert(node);  // Prevent infinite recursion

  size_t count = 0;
  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(GroupTypeNode *node) {
  if (m_visited.contains(node)) return 1;

  m_visited.insert(node);  // Prevent infinite recursion

  size_t count = 0;
  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(RegionTypeNode *node) {
  if (m_visited.contains(node)) return 1;

  m_visited.insert(node);  // Prevent infinite recursion

  size_t count = 0;
  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(UnionTypeNode *node) {
  if (m_visited.contains(node)) return 1;

  m_visited.insert(node);  // Prevent infinite recursion

  size_t count = 0;
  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(ArrayTypeNode *node) {
  size_t count = 0;
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  count += next(node->m_type);
  if (node->m_size) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_size));
    count += next(node->m_size);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(VectorTypeNode *node) {
  size_t count = 0;
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  count += next(node->m_type);
  return count + 1;
}

size_t ParseTreePreorder::iter(ResultTypeNode *node) {
  size_t count = 0;
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  count += next(node->m_type);
  return count + 1;
}

size_t ParseTreePreorder::iter(GeneratorTypeNode *node) {
  size_t count = 0;
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  count += next(node->m_type);
  return count + 1;
}

size_t ParseTreePreorder::iter(FunctionTypeNode *node) {
  size_t count = 0;
  for (auto &param : node->m_params) {
    m_callback(m_ns, m_scope, node, mk_ptr(&param.second));
    count += next(param.second);
  }
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_return_type));
  count += next(node->m_return_type);
  return count + 1;
}

size_t ParseTreePreorder::iter(UserTypeNode *node) { return 1; }

size_t ParseTreePreorder::iter(IntegerNode *node) { return 1; }

size_t ParseTreePreorder::iter(FloatLiteralNode *node) { return 1; }

size_t ParseTreePreorder::iter(StringNode *node) { return 1; }

size_t ParseTreePreorder::iter(CharNode *node) { return 1; }

size_t ParseTreePreorder::iter(BoolLiteralNode *node) { return 1; }

size_t ParseTreePreorder::iter(NullLiteralNode *node) { return 1; }

size_t ParseTreePreorder::iter(UndefLiteralNode *node) { return 1; }

size_t ParseTreePreorder::iter(TypedefNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_orig));
  return next(node->m_orig) + 1;
}

size_t ParseTreePreorder::iter(VarDeclNode *node) {
  m_visited.clear();

  size_t count = 0;

  if (node->m_type) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
    count += next(node->m_type);
  }

  if (node->m_init) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_init));
    count += next(node->m_init);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(LetDeclNode *node) {
  m_visited.clear();

  size_t count = 0;

  if (node->m_type) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
    count += next(node->m_type);
  }

  if (node->m_init) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_init));
    count += next(node->m_init);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(ConstDeclNode *node) {
  m_visited.clear();
  size_t count = 0;

  if (node->m_type) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
    count += next(node->m_type);
  }

  if (node->m_init) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_init));
    count += next(node->m_init);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(FunctionDeclNode *node) {
  m_visited.clear();

  size_t count = 0;
  m_scope.push_back(node->m_name);

  for (auto &param : node->m_params) {
    m_callback(m_ns, m_scope, node, mk_ptr(&param));
    count += next(param);
  }
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  count += next(node->m_type);

  m_scope.pop_back();
  return count + 1;
}

size_t ParseTreePreorder::iter(StructDefNode *node) {
  size_t count = 0;
  m_scope.push_back(node->m_name);

  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }

  for (auto &method : node->m_methods) {
    m_callback(m_ns, m_scope, node, mk_ptr(&method));
    count += next(method);
  }

  for (auto &method : node->m_static_methods) {
    m_callback(m_ns, m_scope, node, mk_ptr(&method));
    count += next(method);
  }

  m_scope.pop_back();
  return count + 1;
}

size_t ParseTreePreorder::iter(StructFieldNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  if (node->m_value) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_value));
    count += next(node->m_value);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(RegionDefNode *node) {
  size_t count = 0;
  m_scope.push_back(node->m_name);

  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }

  for (auto &method : node->m_methods) {
    m_callback(m_ns, m_scope, node, mk_ptr(&method));
    count += next(method);
  }

  for (auto &method : node->m_static_methods) {
    m_callback(m_ns, m_scope, node, mk_ptr(&method));
    count += next(method);
  }

  m_scope.pop_back();
  return count + 1;
}

size_t ParseTreePreorder::iter(RegionFieldNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  if (node->m_value) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_value));
    count += next(node->m_value);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(GroupDefNode *node) {
  size_t count = 0;
  m_scope.push_back(node->m_name);

  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }

  for (auto &method : node->m_methods) {
    m_callback(m_ns, m_scope, node, mk_ptr(&method));
    count += next(method);
  }

  for (auto &method : node->m_static_methods) {
    m_callback(m_ns, m_scope, node, mk_ptr(&method));
    count += next(method);
  }

  m_scope.pop_back();
  return count + 1;
}

size_t ParseTreePreorder::iter(GroupFieldNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  if (node->m_value) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_value));
    count += next(node->m_value);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(UnionDefNode *node) {
  size_t count = 0;
  m_scope.push_back(node->m_name);

  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }

  for (auto &method : node->m_methods) {
    m_callback(m_ns, m_scope, node, mk_ptr(&method));
    count += next(method);
  }

  for (auto &method : node->m_static_methods) {
    m_callback(m_ns, m_scope, node, mk_ptr(&method));
    count += next(method);
  }

  m_scope.pop_back();
  return count + 1;
}

size_t ParseTreePreorder::iter(UnionFieldNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  if (node->m_value) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_value));
    count += next(node->m_value);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(EnumDefNode *node) {
  if (node->m_scoped) m_scope.push_back(node->m_type->m_name);

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }

  if (node->m_scoped) m_scope.pop_back();

  return count + 1;
}

size_t ParseTreePreorder::iter(EnumFieldNode *node) {
  if (node->m_value) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_value));
    return next(node->m_value) + 1;
  }
  return 1;
}

size_t ParseTreePreorder::iter(FunctionDefNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_decl));
  size_t count = next(node->m_decl);

  m_scope.push_back(node->m_decl->m_name);

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_body));
  count += next(node->m_body);

  m_scope.pop_back();

  return count + 1;
}

size_t ParseTreePreorder::iter(FunctionParamNode *node) {
  m_visited.clear();

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  if (node->m_value) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_value));
    count += next(node->m_value);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(SubsystemNode *node) {
  m_ns.push_back(node->m_name);
  m_scope.push_back(node->m_name);

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_block));
  size_t count = next(node->m_block);

  m_scope.pop_back();
  m_ns.pop_back();

  return count + 1;
}

size_t ParseTreePreorder::iter(ExportNode *node) {
  size_t count = 0;
  for (auto &stmt : node->m_stmts) {
    m_callback(m_ns, m_scope, node, mk_ptr(&stmt));
    count += next(stmt);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(InlineAsmNode *node) {
  size_t count = 0;

  for (auto &[reg, expr] : node->m_outputs) {
    m_callback(m_ns, m_scope, node, mk_ptr(&expr));
    count += next(expr);
  }

  for (auto &[reg, expr] : node->m_outputs) {
    m_callback(m_ns, m_scope, node, mk_ptr(&expr));
    count += next(expr);
  }

  return count + 1;
}

size_t ParseTreePreorder::iter(ReturnStmtNode *node) {
  if (node->m_expr) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
    return next(node->m_expr) + 1;
  }
  return 1;
}

size_t ParseTreePreorder::iter(RetifStmtNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_cond));
  size_t count = next(node->m_cond);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_return));
  count += next(node->m_return);
  return count + 1;
}

size_t ParseTreePreorder::iter(RetzStmtNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_cond));
  size_t count = next(node->m_cond);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_return));
  count += next(node->m_return);
  return count + 1;
}

size_t ParseTreePreorder::iter(RetvStmtNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_cond));
  return next(node->m_cond) + 1;
}

size_t ParseTreePreorder::iter(IfStmtNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_cond));
  size_t count = next(node->m_cond);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_then));
  count += next(node->m_then);
  if (node->m_else) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_else));
    count += next(node->m_else);
  }
  return count + 1;
}

size_t ParseTreePreorder::iter(WhileStmtNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_cond));
  size_t count = next(node->m_cond);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_stmt));
  count += next(node->m_stmt);
  return count + 1;
}

size_t ParseTreePreorder::iter(ForStmtNode *node) {
  size_t count = 1;

  if (node->m_init) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_init));
    count += next(node->m_init);
  }

  if (node->m_cond) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_cond));
    count += next(node->m_cond);
  }

  if (node->m_step) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_step));
    count += next(node->m_step);
  }

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_stmt));
  count += next(node->m_stmt);

  return count;
}

size_t ParseTreePreorder::iter(FormStmtNode *node) {
  size_t count = 0;

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_range));
  count += next(node->m_range);

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_block));
  count += next(node->m_block);

  return count + 1;
}

size_t ParseTreePreorder::iter(CaseStmtNode *node) {
  size_t count = 0;

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_block));
  count += next(node->m_block);

  return count + 1;
}

size_t ParseTreePreorder::iter(SwitchStmtNode *node) {
  size_t count = 0;

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  count += next(node->m_expr);

  for (auto &stmt : node->m_cases) {
    m_callback(m_ns, m_scope, node, mk_ptr(&stmt));
    count += next(stmt);
  }

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_default));
  count += next(node->m_default);

  return count + 1;
}