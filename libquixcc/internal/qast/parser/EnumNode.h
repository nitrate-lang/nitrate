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

#include <quixcc/llvm/LLVMWrapper.h>
#include <qast/parser/BasicNodes.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
  class EnumTypeNode : public TypeNode {
public:
    EnumTypeNode(const std::string &name, std::shared_ptr<TypeNode> member_type)
        : m_name(name), m_member_type(member_type) {
      ntype = NodeType::EnumTypeNode;
    }

    std::string m_name;
    std::shared_ptr<TypeNode> m_member_type;
  };

  class EnumFieldNode : public ParseNode {
public:
    EnumFieldNode() { ntype = NodeType::EnumFieldNode; }
    EnumFieldNode(const std::string &name, const std::shared_ptr<ConstExprNode> &value)
        : m_name(name), m_value(value) {
      ntype = NodeType::EnumFieldNode;
    }

    std::string m_name;
    std::shared_ptr<ConstExprNode> m_value;
  };

  class EnumDefNode : public DefNode {
public:
    EnumDefNode() { ntype = NodeType::EnumDefNode; }
    EnumDefNode(std::shared_ptr<EnumTypeNode> type,
                const std::vector<std::shared_ptr<EnumFieldNode>> &fields = {})
        : m_type(type), m_fields(fields) {
      ntype = NodeType::EnumDefNode;
    }
    virtual std::shared_ptr<TypeNode> get_type() const { return m_type; }

    std::shared_ptr<EnumTypeNode> m_type;
    std::vector<std::shared_ptr<EnumFieldNode>> m_fields;
  };
} // namespace libquixcc

#endif // __QUIXCC_PARSE_NODES_ENUM_H__