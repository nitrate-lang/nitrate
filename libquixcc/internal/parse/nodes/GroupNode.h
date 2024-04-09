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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_PARSE_NODES_GROUP_H__
#define __QUIXCC_PARSE_NODES_GROUP_H__

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
    class GroupTypeNode : public TypeNode
    {
        GroupTypeNode() { ntype = NodeType::GroupTypeNode; }
        static std::map<std::vector<TypeNode *>, GroupTypeNode *> m_instances;

    public:
        static GroupTypeNode *create(const std::vector<TypeNode *> &fields)
        {
            if (m_instances.contains(fields))
                return m_instances[fields];
            auto instance = new GroupTypeNode();
            instance->m_fields = fields;
            m_instances[fields] = instance;
            return instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { throw std::runtime_error("GroupTypeNode::codegen not implemented"); }
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
            std::string source = "group { ";
            for (auto &field : m_fields)
                source += field->to_source() + "; ";
            source += "}";
            return source;
        }

        std::vector<TypeNode *> m_fields;
    };

    class GroupFieldNode : public ParseNode
    {
    public:
        GroupFieldNode() { ntype = NodeType::GroupFieldNode; }
        GroupFieldNode(const std::string &name, TypeNode *type, std::shared_ptr<ConstExprNode> value = nullptr) : m_name(name), m_type(type), m_value(value) { ntype = NodeType::GroupFieldNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const { return visitor.visit(this); }

        std::string m_name;
        TypeNode *m_type;
        std::shared_ptr<ConstExprNode> m_value;
    };

    class GroupDefNode : public DefNode
    {
    public:
        GroupDefNode() { ntype = NodeType::GroupDefNode; }
        GroupDefNode(const std::string &name, const std::vector<std::shared_ptr<GroupFieldNode>> &fields) : m_name(name), m_fields(fields) { ntype = NodeType::GroupDefNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { throw std::runtime_error("GroupTypeNode::codegen not implemented"); }
        std::unique_ptr<StmtNode> reduce() const override;

        virtual GroupTypeNode *get_type() const
        {
            std::vector<TypeNode *> fields;
            for (auto &field : m_fields)
                fields.push_back(field->m_type);
            return GroupTypeNode::create(fields);
        }

        std::string m_name;
        std::vector<std::shared_ptr<GroupFieldNode>> m_fields;
    };
}

#endif // __QUIXCC_PARSE_NODES_GROUP_H__