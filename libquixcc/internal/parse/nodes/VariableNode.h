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

#ifndef __QUIXCC_PARSE_NODES_VARIABLE_H__
#define __QUIXCC_PARSE_NODES_VARIABLE_H__

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
    class VarDeclNode : public DeclNode
    {
    public:
        VarDeclNode() { ntype = NodeType::VarDeclNode; }
        VarDeclNode(const std::string &name, TypeNode *type, const std::shared_ptr<ExprNode> init)
            : m_name(name), m_type(type), m_init(init) { ntype = NodeType::VarDeclNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::string m_name;
        TypeNode *m_type;
        std::shared_ptr<ExprNode> m_init;

        bool m_is_mut = false;
        bool m_is_thread_local = false;
        bool m_is_static = false;
        bool m_is_deprecated = false;
    };

    class LetDeclNode : public DeclNode
    {
    public:
        LetDeclNode() { ntype = NodeType::LetDeclNode; }
        LetDeclNode(const std::string &name, TypeNode *type, const std::shared_ptr<ExprNode> init)
            : m_name(name), m_type(type), m_init(init) { ntype = NodeType::LetDeclNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::string m_name;
        TypeNode *m_type;
        std::shared_ptr<ExprNode> m_init;

        bool m_is_mut = false;
        bool m_is_thread_local = false;
        bool m_is_static = false;
        bool m_is_deprecated = false;
    };

#define LETDECLSIZE sizeof(LetDeclNode)

    class ConstDeclNode : public DeclNode
    {
    public:
        ConstDeclNode() { ntype = NodeType::ConstDeclNode; }
        ConstDeclNode(const std::string &name, TypeNode *type, const std::shared_ptr<ExprNode> init)
            : m_name(name), m_type(type), m_init(init) { ntype = NodeType::ConstDeclNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::string m_name;
        TypeNode *m_type;
        std::shared_ptr<ExprNode> m_init;

        bool m_is_deprecated = false;
    };

}

#endif // __QUIXCC_PARSE_NODES_VARIABLE_H__