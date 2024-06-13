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

#ifndef __QUIXCC_PARSE_NODES_ENUM_H__
#define __QUIXCC_PARSE_NODES_ENUM_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <llvm/LLVMWrapper.h>
#include <parsetree/nodes/BasicNodes.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
class EnumTypeNode : public TypeNode {
  static std::map<std::pair<std::string, TypeNode *>, EnumTypeNode *>
      m_instances;
  EnumTypeNode(const std::string &name, TypeNode *member_type)
      : m_name(name), m_member_type(member_type) {
    ntype = NodeType::EnumTypeNode;
  }

 public:
  static EnumTypeNode *create(const std::string &name, TypeNode *member_type) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    auto key = std::make_pair(name, member_type);
    if (m_instances.find(key) == m_instances.end())
      m_instances[key] = new EnumTypeNode(name, member_type);
    return m_instances[key];
  }

  std::string m_name;
  TypeNode *m_member_type;
};

class EnumFieldNode : public ParseNode {
 public:
  EnumFieldNode() { ntype = NodeType::EnumFieldNode; }
  EnumFieldNode(const std::string &name,
                const std::shared_ptr<ConstExprNode> &value)
      : m_name(name), m_value(value) {
    ntype = NodeType::EnumFieldNode;
  }

  std::string m_name;
  std::shared_ptr<ConstExprNode> m_value;
};

class EnumDefNode : public DefNode {
 public:
  EnumDefNode() { ntype = NodeType::EnumDefNode; }
  EnumDefNode(EnumTypeNode *type, bool scoped,
              const std::vector<std::shared_ptr<EnumFieldNode>> &fields = {})
      : m_type(type), m_fields(fields), m_scoped(scoped) {
    ntype = NodeType::EnumDefNode;
  }
  virtual TypeNode *get_type() const { return m_type; }

  EnumTypeNode *m_type;
  std::vector<std::shared_ptr<EnumFieldNode>> m_fields;
  bool m_scoped = false;
};
}  // namespace libquixcc

#endif  // __QUIXCC_PARSE_NODES_ENUM_H__