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
///     License along with the GNU C Library; if not, see                        ///
///     <https://www.gnu.org/licenses/>.                                         ///
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
#include <variant>

#include <llvm/LLVMWrapper.h>
#include <parse/nodes/BasicNodes.h>
#include <parse/nodes/BasicTypeNode.h>

namespace libquixcc
{
    class LiteralNode : public ConstExprNode
    {
    public:
        LiteralNode() { ntype = NodeType::LiteralNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual bool is_negative() const = 0;
        virtual TypeNode *type() const = 0;
    };

    typedef std::variant<int64_t, uint64_t, int32_t, uint32_t> NumbericLiteralNode;

    class IntegerLiteralNode : public LiteralNode
    {
        static std::unordered_map<std::string, std::shared_ptr<IntegerLiteralNode>> m_instances;

    public:
        IntegerLiteralNode(const std::string &val);

        static std::shared_ptr<IntegerLiteralNode> create(const std::string &val)
        {
            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<IntegerLiteralNode>(new IntegerLiteralNode(val));
            return m_instances[val];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual TypeNode *type() const override { return m_val_type; }
        virtual bool is_negative() const override { return m_val.front() == '-'; }
        virtual std::unique_ptr<LiteralNode> reduce() const override { return std::unique_ptr<IntegerLiteralNode>(new IntegerLiteralNode(m_val)); }

        virtual int64_t GetInt64() const override { return std::stoll(m_val); }

        std::string m_val;
        TypeNode *m_val_type;
        NumbericLiteralNode m_value;
    };

    class FloatLiteralNode : public LiteralNode
    {
        static std::unordered_map<std::string, std::shared_ptr<FloatLiteralNode>> m_instances;

    public:
        FloatLiteralNode(const std::string &val);

        static std::shared_ptr<FloatLiteralNode> create(const std::string &val)
        {
            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<FloatLiteralNode>(new FloatLiteralNode(val));
            return m_instances[val];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual TypeNode *type() const override { return m_val_type; }
        virtual bool is_negative() const override { return true; }
        virtual std::unique_ptr<LiteralNode> reduce() const override { return std::unique_ptr<FloatLiteralNode>(new FloatLiteralNode(m_val)); }

        std::string m_val;
        TypeNode *m_val_type;
        double m_value;
    };

    class StringLiteralNode : public LiteralNode
    {
        static std::unordered_map<std::string, std::shared_ptr<StringLiteralNode>> m_instances;

    public:
        StringLiteralNode(const std::string &val) : m_val(val) { ntype = NodeType::StringLiteralNode; }

        static std::shared_ptr<StringLiteralNode> create(const std::string &val)
        {
            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<StringLiteralNode>(new StringLiteralNode(val));
            return m_instances[val];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual TypeNode *type() const override;
        virtual bool is_negative() const override { return false; }
        virtual std::unique_ptr<LiteralNode> reduce() const override { return std::unique_ptr<StringLiteralNode>(new StringLiteralNode(m_val)); }

        std::string m_val;
    };

    class CharLiteralNode : public LiteralNode
    {
        static std::unordered_map<std::string, std::shared_ptr<CharLiteralNode>> m_instances;

    public:
        CharLiteralNode(const std::string &val) : m_val(val) { ntype = NodeType::CharLiteralNode; }

        static std::shared_ptr<CharLiteralNode> create(const std::string &val)
        {
            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<CharLiteralNode>(new CharLiteralNode(val));
            return m_instances[val];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual TypeNode *type() const override;
        virtual bool is_negative() const override { return false; }
        virtual std::unique_ptr<LiteralNode> reduce() const override { return std::unique_ptr<CharLiteralNode>(new CharLiteralNode(m_val)); }

        std::string m_val;
    };

    class BoolLiteralNode : public LiteralNode
    {
        static std::shared_ptr<BoolLiteralNode> m_true_instance;
        static std::shared_ptr<BoolLiteralNode> m_false_instance;

    public:
        BoolLiteralNode(bool val) : m_val(val) { ntype = NodeType::BoolLiteralNode; }

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
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual TypeNode *type() const override;
        virtual bool is_negative() const override { return false; }
        virtual std::unique_ptr<LiteralNode> reduce() const override { return std::unique_ptr<BoolLiteralNode>(new BoolLiteralNode(m_val)); }

        bool m_val;
    };

    class NullLiteralNode : public LiteralNode
    {
        static std::shared_ptr<NullLiteralNode> m_instance;

    public:
        NullLiteralNode() { ntype = NodeType::NullLiteralNode; }

        static std::shared_ptr<NullLiteralNode> create()
        {
            if (m_instance == nullptr)
                m_instance = std::shared_ptr<NullLiteralNode>(new NullLiteralNode());
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Constant *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual TypeNode *type() const override;
        virtual bool is_negative() const override { return false; }
        virtual std::unique_ptr<LiteralNode> reduce() const override { return std::unique_ptr<NullLiteralNode>(new NullLiteralNode()); }
    };
}

#endif // __QUIXCC_PARSE_NODES_LITERAL_H__