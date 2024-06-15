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
#include <sstream>
#include <string>

namespace libquixcc {
namespace serialize {

class ParseTreeSerializer {
  std::set<const TypeNode *> visited;
  size_t indent = 0;
  std::ostringstream o;

  void ind();
  void conv(const ExprStmtNode *);
  void conv(const StmtExprNode *);
  void conv(const NopStmtNode *);
  void conv(const BlockNode *);
  void conv(const StmtGroupNode *);
  void conv(const StaticCastExprNode *);
  void conv(const BitCastExprNode *);
  void conv(const SignedUpcastExprNode *);
  void conv(const UnsignedUpcastExprNode *);
  void conv(const DowncastExprNode *);
  void conv(const PtrToIntCastExprNode *);
  void conv(const IntToPtrCastExprNode *);
  void conv(const UnaryExprNode *);
  void conv(const PostUnaryExprNode *);
  void conv(const BinaryExprNode *);
  void conv(const CallExprNode *);
  void conv(const ListExprNode *);
  void conv(const MemberAccessNode *);
  void conv(const IndexNode *);
  void conv(const FStringNode *);
  void conv(const ConstUnaryExprNode *);
  void conv(const ConstPostUnaryExprNode *);
  void conv(const ConstBinaryExprNode *);
  void conv(const IdentifierNode *);
  void conv(const MutTypeNode *);
  void conv(const U8TypeNode *);
  void conv(const U16TypeNode *);
  void conv(const U32TypeNode *);
  void conv(const U64TypeNode *);
  void conv(const U128TypeNode *);
  void conv(const I8TypeNode *);
  void conv(const I16TypeNode *);
  void conv(const I32TypeNode *);
  void conv(const I64TypeNode *);
  void conv(const I128TypeNode *);
  void conv(const F32TypeNode *);
  void conv(const F64TypeNode *);
  void conv(const BoolTypeNode *);
  void conv(const VoidTypeNode *);
  void conv(const NullTypeNode *);
  void conv(const PointerTypeNode *);
  void conv(const OpaqueTypeNode *);
  void conv(const StringTypeNode *);
  void conv(const EnumTypeNode *);
  void conv(const StructTypeNode *);
  void conv(const GroupTypeNode *);
  void conv(const RegionTypeNode *);
  void conv(const UnionTypeNode *);
  void conv(const ArrayTypeNode *);
  void conv(const VectorTypeNode *);
  void conv(const ResultTypeNode *);
  void conv(const GeneratorTypeNode *);
  void conv(const FunctionTypeNode *);
  void conv(const UserTypeNode *);
  void conv(const IntegerNode *);
  void conv(const FloatLiteralNode *);
  void conv(const StringNode *);
  void conv(const CharNode *);
  void conv(const BoolLiteralNode *);
  void conv(const NullLiteralNode *);
  void conv(const UndefLiteralNode *);
  void conv(const TypedefNode *);
  void conv(const VarDeclNode *);
  void conv(const LetDeclNode *);
  void conv(const ConstDeclNode *);
  void conv(const FunctionDeclNode *);
  void conv(const StructDefNode *);
  void conv(const StructFieldNode *);
  void conv(const RegionDefNode *);
  void conv(const RegionFieldNode *);
  void conv(const GroupDefNode *);
  void conv(const GroupFieldNode *);
  void conv(const UnionDefNode *);
  void conv(const UnionFieldNode *);
  void conv(const EnumDefNode *);
  void conv(const EnumFieldNode *);
  void conv(const FunctionDefNode *);
  void conv(const FunctionParamNode *);
  void conv(const SubsystemNode *);
  void conv(const ExportNode *);
  void conv(const InlineAsmNode *);
  void conv(const ReturnStmtNode *);
  void conv(const RetifStmtNode *);
  void conv(const RetzStmtNode *);
  void conv(const RetvStmtNode *);
  void conv(const IfStmtNode *);
  void conv(const WhileStmtNode *);
  void conv(const ForStmtNode *);
  void conv(const FormStmtNode *);
  void conv(const CaseStmtNode *);
  void conv(const SwitchStmtNode *);
  void dispatch(const ParseNode *);

  template <typename T>
  inline void next(const T node) {
    if (!node) {
      indent++;
      ind();
      o << "(nil)";
      indent--;
      return;
    }
    if constexpr (std::is_pointer<T>::value) {
      return dispatch(node);
    } else {
      return dispatch(node.get());
    }
  }

 public:
  ParseTreeSerializer() = default;

  std::string serialize(const ParseNode *node);
};
}  // namespace serialize
}  // namespace libquixcc

#endif  // __QUIXCC_PARSETREE_SERIALIZE_H__