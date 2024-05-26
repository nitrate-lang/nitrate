////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_PARSE_NODES_UNION_H__
#define __QUIXCC_PARSE_NODES_UNION_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <llvm/LLVMWrapper.h>
#include <parsetree/nodes/BasicNodes.h>

namespace libquixcc
{
    class UnionTypeNode : public TypeNode
    {
        UnionTypeNode(std::vector<TypeNode *> fields, const std::string &name) : m_fields(fields), m_name(name) { ntype = NodeType::UnionTypeNode; }
        static std::map<std::pair<std::vector<TypeNode *>, std::string>, UnionTypeNode *> m_instances;

    public:
        static UnionTypeNode *create(const std::vector<TypeNode *> &fields, const std::string &name)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            auto key = std::make_pair(fields, name);
            if (m_instances.contains(key))
                return m_instances[key];
            auto instance = new UnionTypeNode(fields, name);
            instance->m_fields = fields;
            m_instances[key] = instance;
            return instance;
        }

        std::vector<TypeNode *> m_fields;
        std::string m_name;
    };

    class UnionFieldNode : public ParseNode
    {
    public:
        UnionFieldNode() { ntype = NodeType::UnionFieldNode; }
        UnionFieldNode(const std::string &name, TypeNode *type, const std::shared_ptr<ConstExprNode> value)
            : m_name(name), m_type(type), m_value(value) { ntype = NodeType::UnionFieldNode; }


        std::string m_name;
        TypeNode *m_type;
        std::shared_ptr<ConstExprNode> m_value;
    };

    class UnionDefNode : public DefNode
    {
    public:
        UnionDefNode() { ntype = NodeType::UnionDefNode; }
        UnionDefNode(const std::string &name, const std::vector<std::shared_ptr<UnionFieldNode>> &fields)
            : m_name(name), m_fields(fields) { ntype = NodeType::UnionDefNode; }

        virtual UnionTypeNode *get_type() const
        {
            std::vector<TypeNode *> fields;
            for (auto &field : m_fields)
                fields.push_back(field->m_type);
            return UnionTypeNode::create(fields, m_name);
        }

        std::string m_name;
        std::vector<std::shared_ptr<UnionFieldNode>> m_fields;
        std::vector<std::shared_ptr<StmtNode>> m_methods;
        std::vector<std::shared_ptr<StmtNode>> m_static_methods;
    };
}

#endif // __QUIXCC_PARSE_NODES_UNION_H__