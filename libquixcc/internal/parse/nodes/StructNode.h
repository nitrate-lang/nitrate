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
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
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

#ifndef __QUIXCC_PARSE_NODES_STRUCT_H__
#define __QUIXCC_PARSE_NODES_STRUCT_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <llvm/LLVMWrapper.h>
#include <parse/nodes/BasicNodes.h>

namespace libquixcc
{
    class StructTypeNode : public TypeNode
    {
        StructTypeNode(std::vector<TypeNode *> fields, const std::string &name) : m_fields(fields), m_name(name) { ntype = NodeType::StructTypeNode; }
        static std::map<std::pair<std::vector<TypeNode *>, std::string>, StructTypeNode *> m_instances;

    public:
        static StructTypeNode *create(const std::vector<TypeNode *> &fields, const std::string &name = "")
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            auto key = std::make_pair(fields, name);
            if (m_instances.contains(key))
                return m_instances[key];
            auto instance = new StructTypeNode(fields, name);
            instance->m_fields = fields;
            m_instances[key] = instance;
            return instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return true; }
        virtual size_t size(size_t ptr_size) const override
        {
            /// TODO: adjust for padding
            size_t size = 0;
            for (auto &field : m_fields)
                size += field->size(ptr_size);
            return size;
        }
        virtual std::string to_source() const override
        {
            std::string source = "struct " + m_name + " { ";
            for (auto &field : m_fields)
                source += field->to_source() + "; ";
            source += "}";
            return source;
        }
        virtual std::string name() const override { return m_name; }

        std::vector<TypeNode *> m_fields;
        std::string m_name;
    };

    class StructFieldNode : public ParseNode
    {
    public:
        StructFieldNode() { ntype = NodeType::StructFieldNode; }
        StructFieldNode(const std::string &name, TypeNode *type, std::shared_ptr<ConstExprNode> value = nullptr) : m_name(name), m_type(type), m_value(value) { ntype = NodeType::StructFieldNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const { return visitor.visit(this); }

        std::string m_name;
        TypeNode *m_type;
        std::shared_ptr<ConstExprNode> m_value;
    };

    class StructDefNode : public DefNode
    {
    public:
        StructDefNode() { ntype = NodeType::StructDefNode; }
        StructDefNode(const std::string &name, const std::vector<std::shared_ptr<StructFieldNode>> &fields) : m_name(name), m_fields(fields) { ntype = NodeType::StructDefNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        std::unique_ptr<StmtNode> reduce(libquixcc::ReductionState &state) const override;

        virtual StructTypeNode *get_type() const
        {
            std::vector<TypeNode *> fields;
            for (auto &field : m_fields)
                fields.push_back(field->m_type);
            return StructTypeNode::create(fields, m_name);
        }

        std::string m_name;
        std::vector<std::shared_ptr<StructFieldNode>> m_fields;
        std::vector<std::shared_ptr<StmtNode>> m_methods;
        std::vector<std::shared_ptr<StmtNode>> m_static_methods;
    };
}

#endif // __QUIXCC_PARSE_NODES_STRUCT_H__