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

#ifndef __QUIXCC_PARSETREE_ITERATE_H__
#define __QUIXCC_PARSETREE_ITERATE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <parsetree/NodeType.h>

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <variant>
#include <vector>

namespace libquixcc {
namespace traversal {
enum class TraversePtrType { Smart, Raw };

typedef std::pair<TraversePtrType,
                  std::variant<std::shared_ptr<libquixcc::ParseNode> *,
                               libquixcc::ParseNode **>>
    TraversePtr;

typedef std::function<void(const std::vector<std::string> &_namespace,
                           const std::vector<std::string> &_scope,
                           libquixcc::ParseNode *, TraversePtr)>
    ParseTreePreorderCallback;

class ParseTreePreorder {
  std::vector<std::string> m_ns;
  std::vector<std::string> m_scope;
  std::set<TypeNode *> m_visited;
  ParseTreePreorderCallback m_callback;

  size_t iter(ExprStmtNode *node);
  size_t iter(StmtExprNode *node);
  size_t iter(NopStmtNode *node);
  size_t iter(BlockNode *node);
  size_t iter(StmtGroupNode *node);
  size_t iter(CastExprNode *node);
  size_t iter(StaticCastExprNode *node);
  size_t iter(BitCastExprNode *node);
  size_t iter(SignedUpcastExprNode *node);
  size_t iter(UnsignedUpcastExprNode *node);
  size_t iter(DowncastExprNode *node);
  size_t iter(UnaryExprNode *node);
  size_t iter(PostUnaryExprNode *node);
  size_t iter(BinaryExprNode *node);
  size_t iter(SeqExprNode *node);
  size_t iter(CallExprNode *node);
  size_t iter(ListExprNode *node);
  size_t iter(AssocExprNode *node);
  size_t iter(MemberAccessNode *node);
  size_t iter(IndexNode *node);
  size_t iter(SliceNode *node);
  size_t iter(FStringNode *node);
  size_t iter(ConstUnaryExprNode *node);
  size_t iter(ConstPostUnaryExprNode *node);
  size_t iter(ConstBinaryExprNode *node);
  size_t iter(IdentifierNode *node);
  size_t iter(MutTypeNode *node);
  size_t iter(U8TypeNode *node);
  size_t iter(U16TypeNode *node);
  size_t iter(U32TypeNode *node);
  size_t iter(U64TypeNode *node);
  size_t iter(U128TypeNode *node);
  size_t iter(I8TypeNode *node);
  size_t iter(I16TypeNode *node);
  size_t iter(I32TypeNode *node);
  size_t iter(I64TypeNode *node);
  size_t iter(I128TypeNode *node);
  size_t iter(F32TypeNode *node);
  size_t iter(F64TypeNode *node);
  size_t iter(BoolTypeNode *node);
  size_t iter(VoidTypeNode *node);
  size_t iter(NullTypeNode *node);
  size_t iter(PointerTypeNode *node);
  size_t iter(OpaqueTypeNode *node);
  size_t iter(StringTypeNode *node);
  size_t iter(EnumTypeNode *node);
  size_t iter(StructTypeNode *node);
  size_t iter(GroupTypeNode *node);
  size_t iter(RegionTypeNode *node);
  size_t iter(UnionTypeNode *node);
  size_t iter(ArrayTypeNode *node);
  size_t iter(VectorTypeNode *node);
  size_t iter(MapTypeNode *node);
  size_t iter(ResultTypeNode *node);
  size_t iter(FunctionTypeNode *node);
  size_t iter(UserTypeNode *node);
  size_t iter(IntegerNode *node);
  size_t iter(FloatLiteralNode *node);
  size_t iter(StringNode *node);
  size_t iter(CharNode *node);
  size_t iter(BoolLiteralNode *node);
  size_t iter(NullLiteralNode *node);
  size_t iter(UndefLiteralNode *node);
  size_t iter(TypedefNode *node);
  size_t iter(VarDeclNode *node);
  size_t iter(LetDeclNode *node);
  size_t iter(ConstDeclNode *node);
  size_t iter(FunctionDeclNode *node);
  size_t iter(StructDefNode *node);
  size_t iter(StructFieldNode *node);
  size_t iter(RegionDefNode *node);
  size_t iter(RegionFieldNode *node);
  size_t iter(GroupDefNode *node);
  size_t iter(GroupFieldNode *node);
  size_t iter(UnionDefNode *node);
  size_t iter(UnionFieldNode *node);
  size_t iter(EnumDefNode *node);
  size_t iter(EnumFieldNode *node);
  size_t iter(FunctionDefNode *node);
  size_t iter(FunctionParamNode *node);
  size_t iter(SubsystemNode *node);
  size_t iter(ExportNode *node);
  size_t iter(InlineAsmNode *node);
  size_t iter(ReturnStmtNode *node);
  size_t iter(RetifStmtNode *node);
  size_t iter(RetzStmtNode *node);
  size_t iter(RetvStmtNode *node);
  size_t iter(BreakStmtNode *node);
  size_t iter(ContinueStmtNode *node);
  size_t iter(IfStmtNode *node);
  size_t iter(WhileStmtNode *node);
  size_t iter(ForStmtNode *node);
  size_t iter(FormStmtNode *node);
  size_t iter(ForeachStmtNode *node);
  size_t iter(CaseStmtNode *node);
  size_t iter(SwitchStmtNode *node);
  size_t dispatch(ParseNode *node);

 public:
  ParseTreePreorder(ParseTreePreorderCallback callback)
      : m_callback(callback) {}

  template <typename T>
  inline size_t next(T node) {
    if constexpr (std::is_pointer<T>::value) {
      return dispatch(node);
    } else {
      return dispatch(node.get());
    }
  }
};
}  // namespace traversal
}  // namespace libquixcc

#endif  // __QUIXCC_PARSETREE_ITERATE_H__