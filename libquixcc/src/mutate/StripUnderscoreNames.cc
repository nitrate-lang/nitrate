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
#include <error/Logger.h>
#include <mutex>
#include <set>
#include <quixcc.h>
#include <iostream>
#include <algorithm>

using namespace libquixcc;

void libquixcc::mutate::StripUnderscoreNames(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    ast->dfs_preorder(ParseNodePreorderVisitor(
        [job](const std::vector<std::string> &_namespace, libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
        {
            switch ((*node)->ntype)
            {
            case NodeType::LetDeclNode:
            {
                auto let = std::dynamic_pointer_cast<LetDeclNode>(*node);
                if (let->m_name == "_")
                    *node = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::VarDeclNode:
            {
                auto var = std::dynamic_pointer_cast<VarDeclNode>(*node);
                if (var->m_name == "_")
                    *node = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::FunctionDeclNode:
            {
                auto func = std::dynamic_pointer_cast<FunctionDeclNode>(*node);
                if (func->m_name == "_")
                    *node = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::StructDefNode:
            {
                auto struc = std::dynamic_pointer_cast<StructDefNode>(*node);
                if (struc->m_name == "_")
                    *node = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::EnumDefNode:
            {
                auto enm = std::dynamic_pointer_cast<EnumDefNode>(*node);
                if (enm->m_type->m_name == "_")
                    *node = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::GroupDefNode:
            {
                auto group = std::dynamic_pointer_cast<GroupDefNode>(*node);
                if (group->m_name == "_")
                    *node = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::RegionDefNode:
            {
                auto region = std::dynamic_pointer_cast<RegionDefNode>(*node);
                if (region->m_name == "_")
                    *node = std::make_shared<NopStmtNode>();
                break;
            }
            case NodeType::UnionDefNode:
            {
                auto uni = std::dynamic_pointer_cast<UnionDefNode>(*node);
                if (uni->m_name == "_")
                    *node = std::make_shared<NopStmtNode>();
                break;
            }

            default:
                /// TODO: Implement more cases.
                break;
            }
        },
        {}));
}