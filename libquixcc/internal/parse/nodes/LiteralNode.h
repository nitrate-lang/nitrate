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

#ifndef __QUIXCC_PARSE_NODES_LITERAL_H__
#define __QUIXCC_PARSE_NODES_LITERAL_H__

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
    class LiteralNode : public ConstExprNode
    {
    public:
        LiteralNode() { ntype = NodeType::LiteralNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
    };

    class IntegerLiteralNode : public LiteralNode
    {
        IntegerLiteralNode(const std::string &val) : m_val(val) { ntype = NodeType::IntegerLiteralNode; }

        static std::unordered_map<std::string, std::shared_ptr<IntegerLiteralNode>> m_instances;

    public:
        static std::shared_ptr<IntegerLiteralNode> create(const std::string &val)
        {
            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<IntegerLiteralNode>(new IntegerLiteralNode(val));
            return m_instances[val];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::string m_val;
    };

    class FloatLiteralNode : public LiteralNode
    {
        FloatLiteralNode(const std::string &val) : m_val(val) { ntype = NodeType::FloatLiteralNode; }

        static std::unordered_map<std::string, std::shared_ptr<FloatLiteralNode>> m_instances;

    public:
        static std::shared_ptr<FloatLiteralNode> create(const std::string &val)
        {
            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<FloatLiteralNode>(new FloatLiteralNode(val));
            return m_instances[val];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::string m_val;
    };

    class StringLiteralNode : public LiteralNode
    {
        StringLiteralNode(const std::string &val) : m_val(val) { ntype = NodeType::StringLiteralNode; }

        static std::unordered_map<std::string, std::shared_ptr<StringLiteralNode>> m_instances;

    public:
        static std::shared_ptr<StringLiteralNode> create(const std::string &val)
        {
            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<StringLiteralNode>(new StringLiteralNode(val));
            return m_instances[val];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::string m_val;
    };

    class CharLiteralNode : public LiteralNode
    {
        CharLiteralNode(const std::string &val) : m_val(val) { ntype = NodeType::CharLiteralNode; }

        static std::unordered_map<std::string, std::shared_ptr<CharLiteralNode>> m_instances;

    public:
        static std::shared_ptr<CharLiteralNode> create(const std::string &val)
        {
            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<CharLiteralNode>(new CharLiteralNode(val));
            return m_instances[val];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::string m_val;
    };

    class BoolLiteralNode : public LiteralNode
    {
        BoolLiteralNode(bool val) : m_val(val) { ntype = NodeType::BoolLiteralNode; }

        static std::shared_ptr<BoolLiteralNode> m_true_instance;
        static std::shared_ptr<BoolLiteralNode> m_false_instance;

    public:
        static std::shared_ptr<BoolLiteralNode> create(bool val)
        {
            if (val)
            {
                if (m_true_instance == nullptr)
                    m_true_instance = std::shared_ptr<BoolLiteralNode>(new BoolLiteralNode(true));
                return m_true_instance;
            }
            else
            {
                if (m_false_instance == nullptr)
                    m_false_instance = std::shared_ptr<BoolLiteralNode>(new BoolLiteralNode(false));
                return m_false_instance;
            }
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual size_t dfs_postorder(ParseNodePostorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        bool m_val;
    };
}

#endif // __QUIXCC_PARSE_NODES_LITERAL_H__