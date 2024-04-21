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

void libquixcc::mutate::MethodToFunc(quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast)
{
    std::map<std::string, std::shared_ptr<libquixcc::ParseNode>> vars;

    ast->dfs_preorder(ParseNodePreorderVisitor(
        [&](const std::vector<std::string> &_namespace, libquixcc::ParseNode *parent, std::shared_ptr<libquixcc::ParseNode> *node)
        {
            if ((*node)->ntype == NodeType::LetDeclNode)
            {
                vars[std::static_pointer_cast<LetDeclNode>(*node)->m_name] = *node;
                return;
            }
            if ((*node)->ntype == NodeType::VarDeclNode)
            {
                vars[std::static_pointer_cast<VarDeclNode>(*node)->m_name] = *node;
                return;
            }
            if ((*node)->ntype == NodeType::FunctionParamNode)
            {
                vars[std::static_pointer_cast<FunctionParamNode>(*node)->m_name] = *node;
                return;
            }

            if ((*node)->ntype != NodeType::CallExprNode)
                return;

            auto call = std::static_pointer_cast<CallExprNode>(*node);

            if (!call->m_name.contains("."))
                return;

            std::string varname = call->m_name.substr(0, call->m_name.find_last_of("."));
            std::string method = call->m_name.substr(call->m_name.find_last_of(".") + 1);

            if (!vars.contains(varname))
            {
                LOG(ERROR) << "MethodToFunc: Named item '" << varname << "' not found." << std::endl;
                return;
            }

            auto var = vars[varname];
            std::string _typename;

            switch (var->ntype)
            {
            case NodeType::LetDeclNode:
            {
                _typename = std::static_pointer_cast<LetDeclNode>(var)->m_type->name();
                break;
            }
            case NodeType::VarDeclNode:
            {
                _typename = std::static_pointer_cast<VarDeclNode>(var)->m_type->name();
                break;
            }
            case NodeType::FunctionParamNode:
            {
                _typename = std::static_pointer_cast<FunctionParamNode>(var)->m_type->name();
                break;
            }
            default:
                break;
            }

            if (_typename.empty())
            {
                LOG(FATAL) << "MethodToFunc: Named item '" << varname << "' has no typename!" << std::endl;
                return;
            }

            auto thisPtr = std::make_shared<UnaryExprNode>(Operator::BitwiseAnd, std::make_shared<IdentifierNode>(varname));

            call->m_name = _typename + "::" + method;
            call->m_positional_args.insert(call->m_positional_args.begin(), thisPtr);
        },
        {}));
}