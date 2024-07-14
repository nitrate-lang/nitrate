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

#ifndef __QUIXCC_PARSE_NODES_STRUCT_H__
#define __QUIXCC_PARSE_NODES_STRUCT_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/llvm/LLVMWrapper.h>
#include <quixcc/parsetree/nodes/BasicNodes.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace libquixcc {
  class StructTypeNode : public TypeNode {
public:
    StructTypeNode(std::vector<std::shared_ptr<TypeNode>> fields, const std::string &name)
        : m_fields(fields), m_name(name) {
      ntype = NodeType::StructTypeNode;
    }

    std::vector<std::shared_ptr<TypeNode>> m_fields;
    std::string m_name;
  };

  class StructFieldNode : public ParseNode {
public:
    StructFieldNode() { ntype = NodeType::StructFieldNode; }
    StructFieldNode(const std::string &name, std::shared_ptr<TypeNode> type,
                    std::shared_ptr<ExprNode> value = nullptr)
        : m_name(name), m_type(type), m_value(value) {
      ntype = NodeType::StructFieldNode;
    }

    std::string m_name;
    std::shared_ptr<TypeNode> m_type;
    std::shared_ptr<ExprNode> m_value;
  };

  class StructDefNode : public DefNode {
public:
    StructDefNode() { ntype = NodeType::StructDefNode; }
    StructDefNode(const std::string &name,
                  const std::vector<std::shared_ptr<StructFieldNode>> &fields,
                  std::set<std::string> implements = {})
        : m_name(name), m_fields(fields), m_implements(implements) {
      ntype = NodeType::StructDefNode;
    }

    virtual std::shared_ptr<StructTypeNode> get_type() const {
      std::vector<std::shared_ptr<TypeNode>> fields;
      for (auto &field : m_fields)
        fields.push_back(field->m_type);
      return std::make_shared<StructTypeNode>(fields, m_name);
    }

    std::string m_name;
    std::vector<std::shared_ptr<StructFieldNode>> m_fields;
    std::vector<std::shared_ptr<StmtNode>> m_methods;
    std::vector<std::shared_ptr<StmtNode>> m_static_methods;
    std::set<std::string> m_implements;
  };
} // namespace libquixcc

#endif // __QUIXCC_PARSE_NODES_STRUCT_H__