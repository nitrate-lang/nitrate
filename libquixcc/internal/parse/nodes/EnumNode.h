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

#ifndef __QUIXCC_PARSE_NODES_ENUM_H__
#define __QUIXCC_PARSE_NODES_ENUM_H__

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
    class EnumTypeNode : public TypeNode
    {
        static std::map<std::pair<std::string, TypeNode *>, EnumTypeNode *> m_instances;
        EnumTypeNode(const std::string &name, TypeNode *member_type) : m_name(name), m_member_type(member_type) { ntype = NodeType::EnumTypeNode; }

    public:
        static EnumTypeNode *create(const std::string &name, TypeNode *member_type)
        {
            auto key = std::make_pair(name, member_type);
            if (m_instances.find(key) == m_instances.end())
                m_instances[key] = new EnumTypeNode(name, member_type);
            return m_instances[key];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return m_member_type->size(ptr_size); }
        virtual std::string to_source() const override
        {
            return "enum {" + m_member_type->to_source() + "}";
        }

        std::string m_name;
        TypeNode *m_member_type;
    };

    class EnumDeclNode : public DeclNode
    {
    public:
        EnumDeclNode() { ntype = NodeType::EnumDeclNode; }
        EnumDeclNode(EnumTypeNode *type) : m_type(type) { ntype = NodeType::EnumDeclNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { throw CodegenException("EnumDeclNode is not codegenable"); }

        EnumTypeNode *m_type;
    };

    class EnumFieldNode : public ParseNode
    {
    public:
        EnumFieldNode() { ntype = NodeType::EnumFieldNode; }
        EnumFieldNode(const std::string &name, const std::shared_ptr<ConstExprNode> &value) : m_name(name), m_value(value) { ntype = NodeType::EnumFieldNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const { throw CodegenException("EnumDeclNode is not codegenable"); }

        std::string m_name;
        std::shared_ptr<ConstExprNode> m_value;
    };

    class EnumDefNode : public DefNode
    {
    public:
        EnumDefNode() { ntype = NodeType::EnumDefNode; }
        EnumDefNode(std::shared_ptr<EnumDeclNode> decl, bool scoped, const std::vector<std::shared_ptr<EnumFieldNode>> &fields = {}) : m_decl(decl), m_fields(fields), m_scoped(scoped) { ntype = NodeType::EnumDefNode; }
        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { throw CodegenException("EnumDeclNode is not codegenable"); }

        virtual TypeNode *get_type() const { return m_decl->m_type; }

        std::shared_ptr<EnumDeclNode> m_decl;
        std::vector<std::shared_ptr<EnumFieldNode>> m_fields;
        bool m_scoped = false;
    };
}

#endif // __QUIXCC_PARSE_NODES_ENUM_H__