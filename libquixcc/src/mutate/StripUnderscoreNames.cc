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

#define QUIXCC_INTERNAL

#include <mutate/Routine.h>
#include <core/Logger.h>
#include <mutex>
#include <set>
#include <quixcc.h>
#include <iostream>
#include <algorithm>

using namespace libquixcc;

void libquixcc::mutate::StripUnderscoreNames(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(traversal::ASTTraversalState(
        [job](const std::vector<std::string> &_namespace, libquixcc::ParseNode *parent, traversal::TraversePtr node)
        {
            if (node.first != traversal::TraversePtrType::Smart)
                return;
            auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

            switch ((ptr)->ntype)
            {
            case NodeType::LetDeclNode:
            {
                auto let = std::static_pointer_cast<LetDeclNode>(ptr);
                if (let->m_name == "_")
                    ptr = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::VarDeclNode:
            {
                auto var = std::static_pointer_cast<VarDeclNode>(ptr);
                if (var->m_name == "_")
                    ptr = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::FunctionDeclNode:
            {
                auto func = std::static_pointer_cast<FunctionDeclNode>(ptr);
                if (func->m_name == "_")
                    ptr = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::StructDefNode:
            {
                auto struc = std::static_pointer_cast<StructDefNode>(ptr);
                if (struc->m_name == "_")
                    ptr = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::EnumDefNode:
            {
                auto enm = std::static_pointer_cast<EnumDefNode>(ptr);
                if (enm->m_type->m_name == "_")
                    ptr = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::GroupDefNode:
            {
                auto group = std::static_pointer_cast<GroupDefNode>(ptr);
                if (group->m_name == "_")
                    ptr = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::RegionDefNode:
            {
                auto region = std::static_pointer_cast<RegionDefNode>(ptr);
                if (region->m_name == "_")
                    ptr = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::UnionDefNode:
            {
                auto uni = std::static_pointer_cast<UnionDefNode>(ptr);
                if (uni->m_name == "_")
                    ptr = std::make_shared<NopStmtNode>();
                break;
            }

            default:
                /// TODO: Implement more cases.
                break;
            }
        },
        {}));
}