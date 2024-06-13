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

#ifndef __QUIXCC_PARSETREE_SERIALIZE_H__
#define __QUIXCC_PARSETREE_SERIALIZE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <parsetree/NodeType.h>

#include <memory>
#include <set>
#include <string>

namespace libquixcc {
namespace serialize {
struct ParseTreeSerializerState {
  std::set<const TypeNode *> m_visited;
};

class ParseTreeSerializer {
  static std::string ExprStmtNode_conv(ParseTreeSerializerState &state,
                                       const ExprStmtNode *node);
  static std::string NopStmtNode_conv(ParseTreeSerializerState &state,
                                      const NopStmtNode *node);
  static std::string BlockNode_conv(ParseTreeSerializerState &state,
                                    const BlockNode *node);
  static std::string StmtGroupNode_conv(ParseTreeSerializerState &state,
                                        const StmtGroupNode *node);
  static std::string StaticCastExprNode_conv(ParseTreeSerializerState &state,
                                             const StaticCastExprNode *node);
  static std::string BitCastExprNode_conv(ParseTreeSerializerState &state,
                                          const BitCastExprNode *node);
  static std::string SignedUpcastExprNode_conv(
      ParseTreeSerializerState &state, const SignedUpcastExprNode *node);
  static std::string UnsignedUpcastExprNode_conv(
      ParseTreeSerializerState &state, const UnsignedUpcastExprNode *node);
  static std::string DowncastExprNode_conv(ParseTreeSerializerState &state,
                                           const DowncastExprNode *node);
  static std::string PtrToIntCastExprNode_conv(
      ParseTreeSerializerState &state, const PtrToIntCastExprNode *node);
  static std::string IntToPtrCastExprNode_conv(
      ParseTreeSerializerState &state, const IntToPtrCastExprNode *node);
  static std::string UnaryExprNode_conv(ParseTreeSerializerState &state,
                                        const UnaryExprNode *node);
  static std::string BinaryExprNode_conv(ParseTreeSerializerState &state,
                                         const BinaryExprNode *node);
  static std::string CallExprNode_conv(ParseTreeSerializerState &state,
                                       const CallExprNode *node);
  static std::string ListExprNode_conv(ParseTreeSerializerState &state,
                                       const ListExprNode *node);
  static std::string MemberAccessNode_conv(ParseTreeSerializerState &state,
                                           const MemberAccessNode *node);
  static std::string IndexNode_conv(ParseTreeSerializerState &state,
                                    const IndexNode *node);
  static std::string ConstUnaryExprNode_conv(ParseTreeSerializerState &state,
                                             const ConstUnaryExprNode *node);
  static std::string ConstBinaryExprNode_conv(ParseTreeSerializerState &state,
                                              const ConstBinaryExprNode *node);
  static std::string IdentifierNode_conv(ParseTreeSerializerState &state,
                                         const IdentifierNode *node);
  static std::string MutTypeNode_conv(ParseTreeSerializerState &state,
                                      const MutTypeNode *node);
  static std::string U8TypeNode_conv(ParseTreeSerializerState &state,
                                     const U8TypeNode *node);
  static std::string U16TypeNode_conv(ParseTreeSerializerState &state,
                                      const U16TypeNode *node);
  static std::string U32TypeNode_conv(ParseTreeSerializerState &state,
                                      const U32TypeNode *node);
  static std::string U64TypeNode_conv(ParseTreeSerializerState &state,
                                      const U64TypeNode *node);
  static std::string U128TypeNode_conv(ParseTreeSerializerState &state,
                                       const U128TypeNode *node);
  static std::string I8TypeNode_conv(ParseTreeSerializerState &state,
                                     const I8TypeNode *node);
  static std::string I16TypeNode_conv(ParseTreeSerializerState &state,
                                      const I16TypeNode *node);
  static std::string I32TypeNode_conv(ParseTreeSerializerState &state,
                                      const I32TypeNode *node);
  static std::string I64TypeNode_conv(ParseTreeSerializerState &state,
                                      const I64TypeNode *node);
  static std::string I128TypeNode_conv(ParseTreeSerializerState &state,
                                       const I128TypeNode *node);
  static std::string F32TypeNode_conv(ParseTreeSerializerState &state,
                                      const F32TypeNode *node);
  static std::string F64TypeNode_conv(ParseTreeSerializerState &state,
                                      const F64TypeNode *node);
  static std::string BoolTypeNode_conv(ParseTreeSerializerState &state,
                                       const BoolTypeNode *node);
  static std::string VoidTypeNode_conv(ParseTreeSerializerState &state,
                                       const VoidTypeNode *node);
  static std::string NullTypeNode_conv(ParseTreeSerializerState &state,
                                       const NullTypeNode *node);
  static std::string PointerTypeNode_conv(ParseTreeSerializerState &state,
                                          const PointerTypeNode *node);
  static std::string OpaqueTypeNode_conv(ParseTreeSerializerState &state,
                                         const OpaqueTypeNode *node);
  static std::string StringTypeNode_conv(ParseTreeSerializerState &state,
                                         const StringTypeNode *node);
  static std::string EnumTypeNode_conv(ParseTreeSerializerState &state,
                                       const EnumTypeNode *node);
  static std::string StructTypeNode_conv(ParseTreeSerializerState &state,
                                         const StructTypeNode *node);
  static std::string GroupTypeNode_conv(ParseTreeSerializerState &state,
                                        const GroupTypeNode *node);
  static std::string RegionTypeNode_conv(ParseTreeSerializerState &state,
                                         const RegionTypeNode *node);
  static std::string UnionTypeNode_conv(ParseTreeSerializerState &state,
                                        const UnionTypeNode *node);
  static std::string ArrayTypeNode_conv(ParseTreeSerializerState &state,
                                        const ArrayTypeNode *node);
  static std::string VectorTypeNode_conv(ParseTreeSerializerState &state,
                                         const VectorTypeNode *node);
  static std::string ResultTypeNode_conv(ParseTreeSerializerState &state,
                                         const ResultTypeNode *node);
  static std::string GeneratorTypeNode_conv(ParseTreeSerializerState &state,
                                            const GeneratorTypeNode *node);
  static std::string FunctionTypeNode_conv(ParseTreeSerializerState &state,
                                           const FunctionTypeNode *node);
  static std::string UserTypeNode_conv(ParseTreeSerializerState &state,
                                       const UserTypeNode *node);
  static std::string IntegerNode_conv(ParseTreeSerializerState &state,
                                      const IntegerNode *node);
  static std::string FloatLiteralNode_conv(ParseTreeSerializerState &state,
                                           const FloatLiteralNode *node);
  static std::string StringNode_conv(ParseTreeSerializerState &state,
                                     const StringNode *node);
  static std::string CharNode_conv(ParseTreeSerializerState &state,
                                   const CharNode *node);
  static std::string BoolLiteralNode_conv(ParseTreeSerializerState &state,
                                          const BoolLiteralNode *node);
  static std::string NullLiteralNode_conv(ParseTreeSerializerState &state,
                                          const NullLiteralNode *node);
  static std::string UndefLiteralNode_conv(ParseTreeSerializerState &state,
                                           const UndefLiteralNode *node);
  static std::string TypedefNode_conv(ParseTreeSerializerState &state,
                                      const TypedefNode *node);
  static std::string VarDeclNode_conv(ParseTreeSerializerState &state,
                                      const VarDeclNode *node);
  static std::string LetDeclNode_conv(ParseTreeSerializerState &state,
                                      const LetDeclNode *node);
  static std::string ConstDeclNode_conv(ParseTreeSerializerState &state,
                                        const ConstDeclNode *node);
  static std::string FunctionDeclNode_conv(ParseTreeSerializerState &state,
                                           const FunctionDeclNode *node);
  static std::string StructDefNode_conv(ParseTreeSerializerState &state,
                                        const StructDefNode *node);
  static std::string StructFieldNode_conv(ParseTreeSerializerState &state,
                                          const StructFieldNode *node);
  static std::string RegionDefNode_conv(ParseTreeSerializerState &state,
                                        const RegionDefNode *node);
  static std::string RegionFieldNode_conv(ParseTreeSerializerState &state,
                                          const RegionFieldNode *node);
  static std::string GroupDefNode_conv(ParseTreeSerializerState &state,
                                       const GroupDefNode *node);
  static std::string GroupFieldNode_conv(ParseTreeSerializerState &state,
                                         const GroupFieldNode *node);
  static std::string UnionDefNode_conv(ParseTreeSerializerState &state,
                                       const UnionDefNode *node);
  static std::string UnionFieldNode_conv(ParseTreeSerializerState &state,
                                         const UnionFieldNode *node);
  static std::string EnumDefNode_conv(ParseTreeSerializerState &state,
                                      const EnumDefNode *node);
  static std::string EnumFieldNode_conv(ParseTreeSerializerState &state,
                                        const EnumFieldNode *node);
  static std::string FunctionDefNode_conv(ParseTreeSerializerState &state,
                                          const FunctionDefNode *node);
  static std::string FunctionParamNode_conv(ParseTreeSerializerState &state,
                                            const FunctionParamNode *node);
  static std::string SubsystemNode_conv(ParseTreeSerializerState &state,
                                        const SubsystemNode *node);
  static std::string ExportNode_conv(ParseTreeSerializerState &state,
                                     const ExportNode *node);
  static std::string InlineAsmNode_conv(ParseTreeSerializerState &state,
                                        const InlineAsmNode *node);
  static std::string ReturnStmtNode_conv(ParseTreeSerializerState &state,
                                         const ReturnStmtNode *node);
  static std::string RetifStmtNode_conv(ParseTreeSerializerState &state,
                                        const RetifStmtNode *node);
  static std::string RetzStmtNode_conv(ParseTreeSerializerState &state,
                                       const RetzStmtNode *node);
  static std::string RetvStmtNode_conv(ParseTreeSerializerState &state,
                                       const RetvStmtNode *node);
  static std::string IfStmtNode_conv(ParseTreeSerializerState &state,
                                     const IfStmtNode *node);
  static std::string WhileStmtNode_conv(ParseTreeSerializerState &state,
                                        const WhileStmtNode *node);
  static std::string ForStmtNode_conv(ParseTreeSerializerState &state,
                                      const ForStmtNode *node);

  static std::string dispatch(ParseTreeSerializerState &state,
                              const ParseNode *node);

 public:
  template <typename T>
  static std::string next(ParseTreeSerializerState &state, const T node) {
    if constexpr (std::is_pointer<T>::value) {
      return dispatch(state, node);
    } else {
      return dispatch(state, node.get());
    }
  }
};
}  // namespace serialize
}  // namespace libquixcc

#endif  // __QUIXCC_PARSETREE_SERIALIZE_H__