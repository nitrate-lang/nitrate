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
        StructTypeNode() { ntype = NodeType::StructTypeNode; }
        static std::map<std::vector<TypeNode *>, StructTypeNode *> m_instances;

    public:
        static StructTypeNode *create(const std::vector<TypeNode *> &fields)
        {
            if (m_instances.contains(fields))
                return m_instances[fields];
            auto instance = new StructTypeNode();
            instance->m_fields = fields;
            m_instances[fields] = instance;
            return instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
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
            std::string source = "struct { ";
            for (auto &field : m_fields)
                source += field->to_source() + "; ";
            source += "}";
            return source;
        }

        std::vector<TypeNode *> m_fields;
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
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        std::unique_ptr<StmtNode> reduce() const override;

        virtual StructTypeNode *get_type() const
        {
            std::vector<TypeNode *> fields;
            for (auto &field : m_fields)
                fields.push_back(field->m_type);
            return StructTypeNode::create(fields);
        }

        std::string m_name;
        std::vector<std::shared_ptr<StructFieldNode>> m_fields;
    };
}

#endif // __QUIXCC_PARSE_NODES_STRUCT_H__