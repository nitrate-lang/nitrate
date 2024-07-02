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

#include <core/Exception.h>
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
  switch (n->ntype) {
    case libquixcc::NodeType::ExprStmtNode:
      return iter(n->as<ExprStmtNode>());
    case libquixcc::NodeType::StmtExprNode:
      return iter(n->as<StmtExprNode>());
    case libquixcc::NodeType::NopStmtNode:
      return iter(n->as<NopStmtNode>());
    case libquixcc::NodeType::BlockNode:
      return iter(n->as<BlockNode>());
    case libquixcc::NodeType::StmtGroupNode:
      return iter(n->as<StmtGroupNode>());
    case libquixcc::NodeType::StaticCastExprNode:
      return iter(n->as<StaticCastExprNode>());
    case libquixcc::NodeType::BitCastExprNode:
      return iter(n->as<BitCastExprNode>());
    case libquixcc::NodeType::SignedUpcastExprNode:
      return iter(n->as<SignedUpcastExprNode>());
    case libquixcc::NodeType::UnsignedUpcastExprNode:
      return iter(n->as<UnsignedUpcastExprNode>());
    case libquixcc::NodeType::DowncastExprNode:
      return iter(n->as<DowncastExprNode>());
    case libquixcc::NodeType::UnaryExprNode:
      return iter(n->as<UnaryExprNode>());
    case libquixcc::NodeType::PostUnaryExprNode:
      return iter(n->as<PostUnaryExprNode>());
    case libquixcc::NodeType::BinaryExprNode:
      return iter(n->as<BinaryExprNode>());
    case libquixcc::NodeType::SeqExprNode:
      return iter(n->as<SeqExprNode>());
    case libquixcc::NodeType::CallExprNode:
      return iter(n->as<CallExprNode>());
    case libquixcc::NodeType::ListExprNode:
      return iter(n->as<ListExprNode>());
    case libquixcc::NodeType::MemberAccessNode:
      return iter(n->as<MemberAccessNode>());
    case libquixcc::NodeType::IndexNode:
      return iter(n->as<IndexNode>());
    case libquixcc::NodeType::SliceNode:
      return iter(n->as<SliceNode>());
    case libquixcc::NodeType::FStringNode:
      return iter(n->as<FStringNode>());
    case libquixcc::NodeType::ConstUnaryExprNode:
      return iter(n->as<ConstUnaryExprNode>());
    case libquixcc::NodeType::ConstPostUnaryExprNode:
      return iter(n->as<ConstPostUnaryExprNode>());
    case libquixcc::NodeType::ConstBinaryExprNode:
      return iter(n->as<ConstBinaryExprNode>());
    case libquixcc::NodeType::IdentifierNode:
      return iter(n->as<IdentifierNode>());
    case libquixcc::NodeType::MutTypeNode:
      return iter(n->as<MutTypeNode>());
    case libquixcc::NodeType::U8TypeNode:
      return iter(n->as<U8TypeNode>());
    case libquixcc::NodeType::U16TypeNode:
      return iter(n->as<U16TypeNode>());
    case libquixcc::NodeType::U32TypeNode:
      return iter(n->as<U32TypeNode>());
    case libquixcc::NodeType::U64TypeNode:
      return iter(n->as<U64TypeNode>());
    case libquixcc::NodeType::U128TypeNode:
      return iter(n->as<U128TypeNode>());
    case libquixcc::NodeType::I8TypeNode:
      return iter(n->as<I8TypeNode>());
    case libquixcc::NodeType::I16TypeNode:
      return iter(n->as<I16TypeNode>());
    case libquixcc::NodeType::I32TypeNode:
      return iter(n->as<I32TypeNode>());
    case libquixcc::NodeType::I64TypeNode:
      return iter(n->as<I64TypeNode>());
    case libquixcc::NodeType::I128TypeNode:
      return iter(n->as<I128TypeNode>());
    case libquixcc::NodeType::F32TypeNode:
      return iter(n->as<F32TypeNode>());
    case libquixcc::NodeType::F64TypeNode:
      return iter(n->as<F64TypeNode>());
    case libquixcc::NodeType::BoolTypeNode:
      return iter(n->as<BoolTypeNode>());
    case libquixcc::NodeType::VoidTypeNode:
      return iter(n->as<VoidTypeNode>());
    case libquixcc::NodeType::NullTypeNode:
      return iter(n->as<NullTypeNode>());
    case libquixcc::NodeType::PointerTypeNode:
      return iter(n->as<PointerTypeNode>());
    case libquixcc::NodeType::OpaqueTypeNode:
      return iter(n->as<OpaqueTypeNode>());
    case libquixcc::NodeType::StringTypeNode:
      return iter(n->as<StringTypeNode>());
    case libquixcc::NodeType::EnumTypeNode:
      return iter(n->as<EnumTypeNode>());
    case libquixcc::NodeType::StructTypeNode:
      return iter(n->as<StructTypeNode>());
    case libquixcc::NodeType::GroupTypeNode:
      return iter(n->as<GroupTypeNode>());
    case libquixcc::NodeType::RegionTypeNode:
      return iter(n->as<RegionTypeNode>());
    case libquixcc::NodeType::UnionTypeNode:
      return iter(n->as<UnionTypeNode>());
    case libquixcc::NodeType::ArrayTypeNode:
      return iter(n->as<ArrayTypeNode>());
    case libquixcc::NodeType::VectorTypeNode:
      return iter(n->as<VectorTypeNode>());
    case libquixcc::NodeType::ResultTypeNode:
      return iter(n->as<ResultTypeNode>());
    case libquixcc::NodeType::GeneratorTypeNode:
      return iter(n->as<GeneratorTypeNode>());
    case libquixcc::NodeType::FunctionTypeNode:
      return iter(n->as<FunctionTypeNode>());
    case libquixcc::NodeType::UserTypeNode:
      return iter(n->as<UserTypeNode>());
    case libquixcc::NodeType::IntegerNode:
      return iter(n->as<IntegerNode>());
    case libquixcc::NodeType::FloatLiteralNode:
      return iter(n->as<FloatLiteralNode>());
    case libquixcc::NodeType::StringNode:
      return iter(n->as<StringNode>());
    case libquixcc::NodeType::CharNode:
      return iter(n->as<CharNode>());
    case libquixcc::NodeType::BoolLiteralNode:
      return iter(n->as<BoolLiteralNode>());
    case libquixcc::NodeType::NullLiteralNode:
      return iter(n->as<NullLiteralNode>());
    case libquixcc::NodeType::UndefLiteralNode:
      return iter(n->as<UndefLiteralNode>());
    case libquixcc::NodeType::TypedefNode:
      return iter(n->as<TypedefNode>());
    case libquixcc::NodeType::VarDeclNode:
      return iter(n->as<VarDeclNode>());
    case libquixcc::NodeType::LetDeclNode:
      return iter(n->as<LetDeclNode>());
    case libquixcc::NodeType::ConstDeclNode:
      return iter(n->as<ConstDeclNode>());
    case libquixcc::NodeType::FunctionDeclNode:
      return iter(n->as<FunctionDeclNode>());
    case libquixcc::NodeType::StructDefNode:
      return iter(n->as<StructDefNode>());
    case libquixcc::NodeType::StructFieldNode:
      return iter(n->as<StructFieldNode>());
    case libquixcc::NodeType::RegionDefNode:
      return iter(n->as<RegionDefNode>());
    case libquixcc::NodeType::RegionFieldNode:
      return iter(n->as<RegionFieldNode>());
    case libquixcc::NodeType::GroupDefNode:
      return iter(n->as<GroupDefNode>());
    case libquixcc::NodeType::GroupFieldNode:
      return iter(n->as<GroupFieldNode>());
    case libquixcc::NodeType::UnionDefNode:
      return iter(n->as<UnionDefNode>());
    case libquixcc::NodeType::UnionFieldNode:
      return iter(n->as<UnionFieldNode>());
    case libquixcc::NodeType::EnumDefNode:
      return iter(n->as<EnumDefNode>());
    case libquixcc::NodeType::EnumFieldNode:
      return iter(n->as<EnumFieldNode>());
    case libquixcc::NodeType::FunctionDefNode:
      return iter(n->as<FunctionDefNode>());
    case libquixcc::NodeType::FunctionParamNode:
      return iter(n->as<FunctionParamNode>());
    case libquixcc::NodeType::SubsystemNode:
      return iter(n->as<SubsystemNode>());
    case libquixcc::NodeType::ExportNode:
      return iter(n->as<ExportNode>());
    case libquixcc::NodeType::InlineAsmNode:
      return iter(n->as<InlineAsmNode>());
    case libquixcc::NodeType::ReturnStmtNode:
      return iter(n->as<ReturnStmtNode>());
    case libquixcc::NodeType::RetifStmtNode:
      return iter(n->as<RetifStmtNode>());
    case libquixcc::NodeType::RetzStmtNode:
      return iter(n->as<RetzStmtNode>());
    case libquixcc::NodeType::RetvStmtNode:
      return iter(n->as<RetvStmtNode>());
    case libquixcc::NodeType::BreakStmtNode:
      return iter(n->as<BreakStmtNode>());
    case libquixcc::NodeType::ContinueStmtNode:
      return iter(n->as<ContinueStmtNode>());
    case libquixcc::NodeType::IfStmtNode:
      return iter(n->as<IfStmtNode>());
    case libquixcc::NodeType::WhileStmtNode:
      return iter(n->as<WhileStmtNode>());
    case libquixcc::NodeType::ForStmtNode:
      return iter(n->as<ForStmtNode>());
    case libquixcc::NodeType::FormStmtNode:
      return iter(n->as<FormStmtNode>());
    case libquixcc::NodeType::ForeachStmtNode:
      return iter(n->as<ForeachStmtNode>());
    case libquixcc::NodeType::CaseStmtNode:
      return iter(n->as<CaseStmtNode>());
    case libquixcc::NodeType::SwitchStmtNode:
      return iter(n->as<SwitchStmtNode>());

    default:
      LOG(FATAL) << "No conversion function for node type " << (int)n->ntype
                 << " found." << std::endl;
      throw core::Exception();
  }
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

size_t ParseTreePreorder::iter(SeqExprNode *node) {
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

size_t ParseTreePreorder::iter(SliceNode *node) {
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_expr));
  size_t count = next(node->m_expr);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_start));
  count += next(node->m_start);
  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_end));
  count += next(node->m_end);
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
  bool scoped = !node->m_type->m_name.empty();

  if (scoped) m_scope.push_back(node->m_type->m_name);

  m_callback(m_ns, m_scope, node, mk_ptr(&node->m_type));
  size_t count = next(node->m_type);
  for (auto &field : node->m_fields) {
    m_callback(m_ns, m_scope, node, mk_ptr(&field));
    count += next(field);
  }

  if (scoped) m_scope.pop_back();

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

size_t ParseTreePreorder::iter(BreakStmtNode *node) { return 1; }

size_t ParseTreePreorder::iter(ContinueStmtNode *node) { return 1; }

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
  size_t count = 1;
  if (node->m_cond) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_cond));
    count += next(node->m_cond);
  }
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

size_t ParseTreePreorder::iter(ForeachStmtNode *node) {
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

  if (node->m_default) {
    m_callback(m_ns, m_scope, node, mk_ptr(&node->m_default));
    count += next(node->m_default);
  }

  return count + 1;
}