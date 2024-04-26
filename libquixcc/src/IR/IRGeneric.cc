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

#include <IR/IRModule.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <map>

enum class IR
{
    Alpha,
    Beta,
    Gamma,
    Delta,
};

enum class NodeType
{
    Generic,
    Group,
    Node,
};

using namespace libquixcc::ir;

class GroupNode : public libquixcc::ir::Value<NodeType::Group>
{
protected:
    Result<bool> print_text_impl(std::ostream &os, bool debug) const override
    {
        os << "GroupNode(" << m_name << ", [";
        for (auto it = cbegin(); it != cend(); ++it)
        {
            if (it != cbegin())
                os << ", ";
            it->print<PrintMode::Text>(os);
        }

        os << "])";
        return true;
    }

    Result<bool> deserialize_text_impl(std::istream &is) override
    {
        return true;
    }

    boost::uuids::uuid graph_hash_impl() const override
    {
        return boost::uuids::nil_uuid();
    }

    bool verify_impl() const override
    {
        return true;
    };

public:
    std::string m_name;
};

class NodeNode : public libquixcc::ir::Value<NodeType::Node>
{
protected:
    Result<bool> print_text_impl(std::ostream &os, bool debug) const override
    {
        os << "NodeNode(" << m_name << ")";
        return true;
    }

    Result<bool> deserialize_text_impl(std::istream &is) override
    {
        return true;
    }

    boost::uuids::uuid graph_hash_impl() const override
    {
        return boost::uuids::nil_uuid();
    }

    bool verify_impl() const override
    {
        return true;
    };

public:
    NodeNode() : Value<NodeType::Node>()
    {
    }

    std::string m_name;
};

class IRAlpha : public libquixcc::ir::IRModule<IR::Alpha, NodeType::Group>
{
protected:
    Result<bool> print_text_impl(std::ostream &os, bool debug) const override
    {
        if (!m_root)
        {
            os << "IRAlpha_1_0\n\n";
            return true;
        }

        os << "IRAlpha_1_0\n\n";
        if (debug)
            return m_root->print<PrintMode::Debug>(os);
        else
            return m_root->print<PrintMode::Text>(os);
    }

    Result<bool> deserialize_text_impl(std::istream &is) override
    {
        char buf[13];
        if (is.readsome(buf, 13) != 13)
            return false;

        if (memcmp(buf, "IRAlpha_1_0\n\n", 13) != 0)
            return false;

        m_root = std::make_shared<GroupNode>();
        return m_root->deserialize<DeserializeMode::Text>(is);
    }

    size_t node_count_impl() const override
    {
        if (!m_root)
            return 0;

        return m_root->node_count();
    }

    boost::uuids::uuid graph_hash_impl() const override
    {
        if (!m_root)
            return boost::uuids::nil_uuid();

        return m_root->hash();
    }

    std::string_view graph_hash_algorithm_name_impl() const override
    {
        return "node-recurse-sha1-trunc96";
    }

    std::string_view ir_dialect_name_impl() const override
    {
        return "QIR-Alpha";
    }

    unsigned ir_dialect_version_impl() const override
    {
        return 1;
    }

    std::string_view ir_dialect_family_impl() const override
    {
        return "QIR";
    }

    std::string_view ir_dialect_description_impl() const override
    {
        return "Quix Alpha Intermediate Representation (QIR-Alpha-V1.0) is a high-level intermediate representation for the Quix language. It contains high level information such as control flow, lambda expressions, coroutines, heap allocations, and other high-level constructs.";
    }

    bool verify_impl() const override
    {
        if (!m_root)
            return false;

        return m_root->verify();
    };

public:
    IRAlpha() : IRModule<IR::Alpha, NodeType::Group>()
    {
    }

    ~IRAlpha() = default;
};

static void setup_graph(std::shared_ptr<GroupNode> node)
{
    std::shared_ptr<NodeNode> node1 = std::make_shared<NodeNode>();
    std::shared_ptr<NodeNode> node2 = std::make_shared<NodeNode>();
    std::shared_ptr<NodeNode> node3 = std::make_shared<NodeNode>();
    std::shared_ptr<NodeNode> node4 = std::make_shared<NodeNode>();

    node1->m_name = "Node1";
    node2->m_name = "Node2";
    node3->m_name = "Node3";

    node4->m_name = "Node4";

    node3->add_child(node4);

    node->m_name = "GroupNode";

    node->add_child(qir_cast<GroupNode>(node1));
    node->add_child(qir_cast<GroupNode>(node2));
    node->add_child(qir_cast<GroupNode>(node3));
}

void x()
{
    std::shared_ptr<GroupNode> node = std::make_shared<GroupNode>();

    setup_graph(node);

    std::unique_ptr<libquixcc::ir::IRModule<IR::Alpha, NodeType::Group>> ir(new IRAlpha());

    ir->assign(node);

    std::cout << "Depth First Preorder Traversal" << std::endl;
    ir->getRoot()->dfs_preorder(
        [](Value<NodeType::Group> *node)
        {
            node->print<PrintMode::Text>(std::cout);
            std::cout << std::endl;
        });

    std::cout << "Depth First Postorder Traversal" << std::endl;
    ir->getRoot()->dfs_postorder(
        [](Value<NodeType::Group> *node)
        {
            node->print<PrintMode::Text>(std::cout);
            std::cout << std::endl;
        });

    std::cout << "Breadth First Preorder Traversal" << std::endl;
    ir->getRoot()->bfs_preorder(
        [](Value<NodeType::Group> *node)
        {
            node->print<PrintMode::Text>(std::cout);
            std::cout << std::endl;
        });

    std::cout << "Breadth First Postorder Traversal" << std::endl;
    ir->getRoot()->bfs_postorder(
        [](Value<NodeType::Group> *node)
        {
            node->print<PrintMode::Text>(std::cout);
            std::cout << std::endl;
        });

}