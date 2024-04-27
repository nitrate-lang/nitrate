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

#ifndef __QUIXCC_PARSE_NODES_REGION_H__
#define __QUIXCC_PARSE_NODES_REGION_H__

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
    class RegionTypeNode : public TypeNode
    {
        RegionTypeNode(std::vector<TypeNode *> fields, const std::string &name) : m_fields(fields), m_name(name) { ntype = NodeType::RegionTypeNode; }
        static std::map<std::pair<std::vector<TypeNode *>, std::string>, RegionTypeNode *> m_instances;

    public:
        static RegionTypeNode *create(const std::vector<TypeNode *> &fields, const std::string &name)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            auto key = std::make_pair(fields, name);
            if (m_instances.contains(key))
                return m_instances[key];
            auto instance = new RegionTypeNode(fields, name);
            instance->m_fields = fields;
            m_instances[key] = instance;
            return instance;
        }

        virtual size_t size(size_t ptr_size) const override
        {
            size_t size = 0;
            for (auto &field : m_fields)
                size += field->size(ptr_size);
            return size;
        }
        virtual std::string to_source() const override
        {
            std::string source = "region " + m_name + " {";
            for (auto &field : m_fields)
                source += field->to_source() + "; ";
            source += "}";
            return source;
        }
        virtual std::string name() const override { return m_name; }

        std::vector<TypeNode *> m_fields;
        std::string m_name;
    };

    class RegionFieldNode : public ParseNode
    {
    public:
        RegionFieldNode() : m_type(nullptr) { ntype = NodeType::RegionFieldNode; }
        RegionFieldNode(const std::string &name, TypeNode *type, std::shared_ptr<ConstExprNode> value = nullptr) : m_name(name), m_type(type), m_value(value) { ntype = NodeType::RegionFieldNode; }


        std::string m_name;
        TypeNode *m_type;
        std::shared_ptr<ConstExprNode> m_value;
    };

    class RegionDefNode : public DefNode
    {
    public:
        RegionDefNode() { ntype = NodeType::RegionDefNode; }
        RegionDefNode(const std::string &name, const std::vector<std::shared_ptr<RegionFieldNode>> &fields) : m_name(name), m_fields(fields) { ntype = NodeType::RegionDefNode; }

        std::unique_ptr<StmtNode> reduce(libquixcc::ReductionState &state) const override;

        virtual RegionTypeNode *get_type() const
        {
            std::vector<TypeNode *> fields;
            for (auto &field : m_fields)
                fields.push_back(field->m_type);
            return RegionTypeNode::create(fields, m_name);
        }

        std::shared_ptr<StructDefNode> to_struct_def() const;

        std::string m_name;
        std::vector<std::shared_ptr<RegionFieldNode>> m_fields;
    };
}

#endif // __QUIXCC_PARSE_NODES_REGION_H__