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

#define QUIXCC_INTERNAL

#include <mutate/Routine.h>
#include <mangle/Symbol.h>
#include <stack>
#include <queue>

/// TODO: this code is slow and needs to be optimized. ideally, we do this in-place while lowering the parsetree.

using namespace libquixcc;

static void expr_collapse(const std::vector<std::string> &_namespace, const std::vector<std::string> &_scope, libquixcc::ParseNode *parent, traversal::TraversePtr node)
{
    if (node.first != traversal::TraversePtrType::Smart)
        return;
    auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    if (!ptr->isof<ExprNode>())
        return;

    auto expr = std::static_pointer_cast<ExprNode>(ptr);

    switch (expr->ntype)
    {
    case NodeType::CallExprNode:
    {
        auto call = std::static_pointer_cast<CallExprNode>(expr);
        if (call->m_callee->is<IdentifierNode>())
        {
            auto id = std::static_pointer_cast<IdentifierNode>(call->m_callee);
            id->m_name = Symbol::join(_namespace, id->m_name);
        }

        break;
    }

    default:
        break;
    }
}

static void stmt_collapse(const std::vector<std::string> &_namespace, const std::vector<std::string> &_scope, libquixcc::ParseNode *parent, traversal::TraversePtr node)
{
    if (node.first != traversal::TraversePtrType::Smart)
        return;
    auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    if (!ptr->is<SubsystemNode>())
        return;
    auto sub = std::static_pointer_cast<SubsystemNode>(ptr);

    std::shared_ptr<StmtGroupNode> stmts = std::make_shared<StmtGroupNode>();

    std::vector<std::string> ns = _namespace;
    ns.push_back(sub->m_name);

    for (auto &child : sub->m_block->m_stmts)
    {
        switch (child->ntype)
        {
        case NodeType::TypedefNode:
        {
            auto def = std::static_pointer_cast<TypedefNode>(child);
            def->m_name = Symbol::join(ns, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::LetDeclNode:
        {
            auto decl = std::static_pointer_cast<LetDeclNode>(child);
            decl->m_name = Symbol::join(ns, decl->m_name);
            stmts->m_stmts.push_back(decl);
            break;
        }
        case NodeType::ConstDeclNode:
        {
            auto decl = std::static_pointer_cast<ConstDeclNode>(child);
            decl->m_name = Symbol::join(ns, decl->m_name);
            stmts->m_stmts.push_back(decl);
            break;
        }
        case NodeType::VarDeclNode:
        {
            auto decl = std::static_pointer_cast<VarDeclNode>(child);
            decl->m_name = Symbol::join(ns, decl->m_name);
            stmts->m_stmts.push_back(decl);
            break;
        }
        case NodeType::FunctionDeclNode:
        {
            auto decl = std::static_pointer_cast<FunctionDeclNode>(child);
            decl->m_name = Symbol::join(ns, decl->m_name);
            stmts->m_stmts.push_back(decl);
            break;
        }
        case NodeType::FunctionDefNode:
        {
            auto def = std::static_pointer_cast<FunctionDefNode>(child);
            def->m_decl->m_name = Symbol::join(ns, def->m_decl->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::StructDefNode:
        {
            auto def = std::static_pointer_cast<StructDefNode>(child);
            def->m_name = Symbol::join(ns, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::RegionDefNode:
        {
            auto def = std::static_pointer_cast<RegionDefNode>(child);
            def->m_name = Symbol::join(ns, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::UnionDefNode:
        {
            auto def = std::static_pointer_cast<UnionDefNode>(child);
            def->m_name = Symbol::join(ns, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::GroupDefNode:
        {
            auto def = std::static_pointer_cast<GroupDefNode>(child);
            def->m_name = Symbol::join(ns, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::EnumDefNode:
        {
            auto def = std::static_pointer_cast<EnumDefNode>(child);
            def->m_type->m_name = Symbol::join(ns, def->m_type->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::SubsystemNode:
        {
            auto def = std::static_pointer_cast<SubsystemNode>(child);
            stmt_collapse(ns, _scope, parent, std::make_pair(traversal::TraversePtrType::Smart, reinterpret_cast<std::shared_ptr<ParseNode> *>(&def)));
            stmts->m_stmts.push_back(def);
            break;
        }
        default:
            stmts->m_stmts.push_back(child);
            break;
        }
    }

    *std::get<std::shared_ptr<ParseNode> *>(node.second) = stmts;
}

void libquixcc::mutate::SubsystemCollapse(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    std::set<std::string> visited;

    ast->dfs_preorder(traversal::ParseTreeTraversalState(
        [&visited](const std::vector<std::string> &ns, const std::vector<std::string> &_scope, libquixcc::ParseNode *parent, traversal::TraversePtr node)
        {
            if (node.first != traversal::TraversePtrType::Raw)
                return;

            auto ptr = *std::get<ParseNode **>(node.second);
            auto dobptr = std::get<ParseNode **>(node.second);

            switch (ptr->ntype)
            {
            case NodeType::UserTypeNode:
            {
                auto def = static_cast<UserTypeNode *>(ptr);
                if (visited.contains(def->m_name))
                    return;

                auto n = Symbol::join(ns, def->m_name);

                *dobptr = UserTypeNode::create(n);
                visited.insert(n);
                break;
            }
            case NodeType::UnionTypeNode:
            {
                auto def = static_cast<UnionTypeNode *>(ptr);
                if (visited.contains(def->m_name))
                    return;

                auto n = Symbol::join(ns, def->m_name);

                *dobptr = UnionTypeNode::create(def->m_fields, n);
                visited.insert(n);
                break;
            }
            case NodeType::StructTypeNode:
            {
                auto def = static_cast<StructTypeNode *>(ptr);
                if (visited.contains(def->m_name))
                    return;

                auto n = Symbol::join(ns, def->m_name);

                *dobptr = StructTypeNode::create(def->m_fields, n);
                visited.insert(n);
                break;
            }
            case NodeType::RegionTypeNode:
            {
                auto def = static_cast<RegionTypeNode *>(ptr);
                if (visited.contains(def->m_name))
                    return;

                auto n = Symbol::join(ns, def->m_name);

                *dobptr = RegionTypeNode::create(def->m_fields, n);
                visited.insert(n);
                break;
            }
            case NodeType::OpaqueTypeNode:
            {
                auto def = static_cast<OpaqueTypeNode *>(ptr);
                if (visited.contains(def->m_name))
                    return;

                auto n = Symbol::join(ns, def->m_name);

                *dobptr = OpaqueTypeNode::create(n);
                visited.insert(n);
                break;
            }
            default:
                break;
            }
        },
        {}));

    ast->dfs_preorder(traversal::ParseTreeTraversalState(expr_collapse, {}));
    ast->dfs_preorder(traversal::ParseTreeTraversalState(stmt_collapse, {}));
}