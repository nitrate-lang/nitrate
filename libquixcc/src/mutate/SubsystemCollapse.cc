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

using namespace libquixcc;

static void collapse(const std::vector<std::string> &__namespace, libquixcc::ParseNode *parent, traversal::TraversePtr node)
{
    if (node.first != traversal::TraversePtrType::Smart)
        return;
    auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    if (!ptr->is<SubsystemNode>())
        return;
    auto sub = std::static_pointer_cast<SubsystemNode>(ptr);

    // This node is not the namespace node. So we must account for the prefix
    std::string _namespace = Symbol::join(__namespace, sub->m_name);

    std::shared_ptr<StmtGroupNode> stmts = std::make_shared<StmtGroupNode>();

    for (auto &child : sub->m_block->m_stmts)
    {
        switch (child->ntype)
        {
        case NodeType::TypedefNode:
        {
            auto def = std::static_pointer_cast<TypedefNode>(child);
            def->m_name = Symbol::join(_namespace, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::LetDeclNode:
        {
            auto decl = std::static_pointer_cast<LetDeclNode>(child);
            decl->m_name = Symbol::join(_namespace, decl->m_name);
            stmts->m_stmts.push_back(decl);
            break;
        }
        case NodeType::VarDeclNode:
        {
            auto decl = std::static_pointer_cast<VarDeclNode>(child);
            decl->m_name = Symbol::join(_namespace, decl->m_name);
            stmts->m_stmts.push_back(decl);
            break;
        }
        case NodeType::FunctionDeclNode:
        {
            auto decl = std::static_pointer_cast<FunctionDeclNode>(child);
            decl->m_name = Symbol::join(_namespace, decl->m_name);
            stmts->m_stmts.push_back(decl);
            break;
        }
        case NodeType::FunctionDefNode:
        {
            auto def = std::static_pointer_cast<FunctionDefNode>(child);
            def->m_decl->m_name = Symbol::join(_namespace, def->m_decl->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::StructDefNode:
        {
            auto def = std::static_pointer_cast<StructDefNode>(child);
            def->m_name = Symbol::join(_namespace, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::RegionDefNode:
        {
            auto def = std::static_pointer_cast<RegionDefNode>(child);
            def->m_name = Symbol::join(_namespace, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::UnionDefNode:
        {
            auto def = std::static_pointer_cast<UnionDefNode>(child);
            def->m_name = Symbol::join(_namespace, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::GroupDefNode:
        {
            auto def = std::static_pointer_cast<GroupDefNode>(child);
            def->m_name = Symbol::join(_namespace, def->m_name);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::EnumDefNode:
        {
            auto def = std::static_pointer_cast<EnumDefNode>(child);
            def->m_type = EnumTypeNode::create(Symbol::join(_namespace, def->m_type->m_name), def->m_type->m_member_type);
            stmts->m_stmts.push_back(def);
            break;
        }
        case NodeType::SubsystemNode:
        {
            auto def = std::static_pointer_cast<SubsystemNode>(child);
            std::vector<std::string> __namespace2 = __namespace;
            __namespace2.push_back(sub->m_name);
            collapse(__namespace2, parent, std::make_pair(traversal::TraversePtrType::Smart, reinterpret_cast<std::shared_ptr<ParseNode> *>(&def)));
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

static void typenode_rename(const std::vector<std::string> &__namespace, libquixcc::ParseNode *parent, traversal::TraversePtr node)
{
    if (node.first != traversal::TraversePtrType::Raw)
        return;
    auto ptr = *std::get<ParseNode **>(node.second);
    auto dobptr = std::get<ParseNode **>(node.second);

    std::vector<std::string> _namespace = __namespace;
    if (!_namespace.empty())
        _namespace.pop_back();

    switch (ptr->ntype)
    {
    case NodeType::UserTypeNode:
    {
        *dobptr = UserTypeNode::create(Symbol::join(_namespace, static_cast<UserTypeNode *>(ptr)->m_name));
        break;
    }
    // case NodeType::EnumTypeNode:
    // {
    //     _namespace.pop_back();
    //     auto def = static_cast<EnumTypeNode *>(ptr);
    //     *dobptr = EnumTypeNode::create(Symbol::join(_namespace, def->m_name), def->m_member_type);
    //     break;
    // }
    case NodeType::UnionTypeNode:
    {
        auto def = static_cast<UnionTypeNode *>(ptr);
        *dobptr = UnionTypeNode::create(def->m_fields, Symbol::join(_namespace, def->m_name));
        break;
    }
    case NodeType::StructTypeNode:
    {
        auto def = static_cast<StructTypeNode *>(ptr);
        *dobptr = StructTypeNode::create(def->m_fields, Symbol::join(_namespace, def->m_name));
        break;
    }
    case NodeType::RegionTypeNode:
    {
        auto def = static_cast<RegionTypeNode *>(ptr);
        *dobptr = RegionTypeNode::create(def->m_fields, Symbol::join(_namespace, def->m_name));
        break;
    }
    case NodeType::OpaqueTypeNode:
    {
        auto def = static_cast<OpaqueTypeNode *>(ptr);
        *dobptr = OpaqueTypeNode::create(Symbol::join(_namespace, def->m_name));
        break;
    }
    default:
        break;
    }
}

void libquixcc::mutate::SubsystemCollapse(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(traversal::ASTTraversalState(typenode_rename, {}));
    ast->dfs_preorder(traversal::ASTTraversalState(collapse, {}));
}