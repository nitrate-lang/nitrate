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

#ifndef __QUIXCC_PARSE_NODES_GROUP_H__
#define __QUIXCC_PARSE_NODES_GROUP_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <llvm/LLVMWrapper.h>
#include <parsetree/nodes/BasicNodes.h>
#include <parsetree/nodes/StructNode.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
class GroupTypeNode : public TypeNode {
 public:
  GroupTypeNode(std::vector<std::shared_ptr<TypeNode>> fields,
                const std::string &name)
      : m_fields(fields), m_name(name) {
    ntype = NodeType::GroupTypeNode;
  }

  std::vector<std::shared_ptr<TypeNode>> m_fields;
  std::string m_name;
};

class GroupFieldNode : public ParseNode {
 public:
  GroupFieldNode() { ntype = NodeType::GroupFieldNode; }
  GroupFieldNode(const std::string &name, std::shared_ptr<TypeNode> type,
                 std::shared_ptr<ExprNode> value = nullptr)
      : m_name(name), m_type(type), m_value(value) {
    ntype = NodeType::GroupFieldNode;
  }

  std::string m_name;
  std::shared_ptr<TypeNode> m_type;
  std::shared_ptr<ExprNode> m_value;
};

class GroupDefNode : public DefNode {
 public:
  GroupDefNode() { ntype = NodeType::GroupDefNode; }
  GroupDefNode(const std::string &name,
               const std::vector<std::shared_ptr<GroupFieldNode>> &fields,
               std::vector<std::string> implements = {})
      : m_name(name), m_fields(fields), m_implements(implements) {
    ntype = NodeType::GroupDefNode;
  }

  virtual std::shared_ptr<GroupTypeNode> get_type() const {
    std::vector<std::shared_ptr<TypeNode>> types;
    for (auto &field : m_fields) types.push_back(field->m_type);
    return std::make_shared<GroupTypeNode>(types, m_name);
  }

  std::string m_name;
  std::vector<std::shared_ptr<GroupFieldNode>> m_fields;
  std::vector<std::shared_ptr<StmtNode>> m_methods;
  std::vector<std::shared_ptr<StmtNode>> m_static_methods;
  std::vector<std::string> m_implements;
};
}  // namespace libquixcc

#endif  // __QUIXCC_PARSE_NODES_GROUP_H__