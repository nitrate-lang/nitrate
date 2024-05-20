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
    };

    typedef std::variant<int64_t, uint64_t> NumbericLiteralNode;

    class IntegerNode : public LiteralNode
    {
    protected:
        static std::unordered_map<std::string, std::shared_ptr<IntegerNode>> m_instances;
        IntegerNode(const std::string &val);

    public:
        static const std::shared_ptr<IntegerNode> create(const std::string &val)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<IntegerNode>(new IntegerNode(val));
            return m_instances[val];
        }

        std::string m_val;
        TypeNode *m_val_type;
    };

    class FloatLiteralNode : public LiteralNode
    {
    protected:
        static std::unordered_map<std::string, std::shared_ptr<FloatLiteralNode>> m_instances;
        FloatLiteralNode(const std::string &val);

    public:
        static const std::shared_ptr<FloatLiteralNode> create(const std::string &val)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<FloatLiteralNode>(new FloatLiteralNode(val));
            return m_instances[val];
        }

        std::string m_val;
        TypeNode *m_val_type;
        double m_value;
    };

    class StringNode : public LiteralNode
    {
    protected:
        static std::unordered_map<std::string, std::shared_ptr<StringNode>> m_instances;
        StringNode(const std::string &val) : m_val(val) { ntype = NodeType::StringNode; }

    public:
        static const std::shared_ptr<StringNode> create(const std::string &val)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<StringNode>(new StringNode(val));
            return m_instances[val];
        }

        std::string m_val;
    };

    class CharNode : public LiteralNode
    {
    protected:
        static std::unordered_map<std::string, std::shared_ptr<CharNode>> m_instances;
        CharNode(const std::string &val) : m_val(val) { ntype = NodeType::CharNode; }

    public:
        static const std::shared_ptr<CharNode> create(const std::string &val)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instances.find(val) != m_instances.end())
                return m_instances[val];

            m_instances[val] = std::shared_ptr<CharNode>(new CharNode(val));
            return m_instances[val];
        }

        std::string m_val;
    };

    class BoolLiteralNode : public LiteralNode
    {
    protected:
        static std::shared_ptr<BoolLiteralNode> m_true_instance;
        static std::shared_ptr<BoolLiteralNode> m_false_instance;
        BoolLiteralNode(bool val) : m_val(val) { ntype = NodeType::BoolLiteralNode; }

    public:
        static const std::shared_ptr<BoolLiteralNode> create(bool val)
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

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

        bool m_val;
    };

    class NullLiteralNode : public LiteralNode
    {
    protected:
        static std::shared_ptr<NullLiteralNode> m_instance;
        NullLiteralNode() { ntype = NodeType::NullLiteralNode; }

    public:
        static const std::shared_ptr<NullLiteralNode> create()
        {
            static std::mutex mutex;
            std::lock_guard<std::mutex> lock(mutex);

            if (m_instance == nullptr)
                m_instance = std::shared_ptr<NullLiteralNode>(new NullLiteralNode());
            return m_instance;
        }
    };
}

#endif // __QUIXCC_PARSE_NODES_LITERAL_H__