////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <core/Logger.h>
#include <mutate/Routine.h>
#include <quixcc/Quix.h>

#include <algorithm>
#include <iostream>
#include <mutex>
#include <set>

using namespace libquixcc;

static std::string getname(const TypeNode *n) {
  switch (n->ntype) {
    case NodeType::StructTypeNode:
      return static_cast<const StructTypeNode *>(n)->m_name;
    case NodeType::RegionTypeNode:
      return static_cast<const RegionTypeNode *>(n)->m_name;
    case NodeType::GroupTypeNode:
      return static_cast<const GroupTypeNode *>(n)->m_name;
    case NodeType::UnionTypeNode:
      return static_cast<const UnionTypeNode *>(n)->m_name;
    case NodeType::EnumTypeNode:
      return static_cast<const EnumTypeNode *>(n)->m_name;
    case NodeType::OpaqueTypeNode:
      return static_cast<const OpaqueTypeNode *>(n)->m_name;
    case NodeType::UserTypeNode:
      return static_cast<const UserTypeNode *>(n)->m_name;
    default:
      throw std::runtime_error("getname: Unknown type node.");
  }
}

void libquixcc::mutate::MethodToFunc(
    quixcc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast) {
  std::map<std::string, std::shared_ptr<libquixcc::ParseNode>> vars;

  ast->dfs_preorder([&](const std::vector<std::string> &_namespace,
                        const std::vector<std::string> &_scope,
                        libquixcc::ParseNode *parent,
                        traversal::TraversePtr node) {
    if (node.first != traversal::TraversePtrType::Smart) return;
    auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    if ((ptr)->is<LetDeclNode>()) {
      vars[std::static_pointer_cast<LetDeclNode>(ptr)->m_name] = ptr;
      return;
    }
    if ((ptr)->is<ConstDeclNode>()) {
      vars[std::static_pointer_cast<ConstDeclNode>(ptr)->m_name] = ptr;
      return;
    }
    if ((ptr)->is<FunctionParamNode>()) {
      vars[std::static_pointer_cast<FunctionParamNode>(ptr)->m_name] = ptr;
      return;
    }

    /// TODO: fix this

    (void)getname;

    return;

    // if (!(ptr)->is<CallExprNode>())
    //     return;

    // auto call = std::static_pointer_cast<CallExprNode>(ptr);

    // if (call->m_name.find(".") == std::string::npos)
    //     return;

    // std::string varname = call->m_name.substr(0,
    // call->m_name.find_last_of(".")); std::string method =
    // call->m_name.substr(call->m_name.find_last_of(".") + 1);

    // if (vars.find(varname) == vars.end())
    // {
    //     LOG(ERROR) << log::raw << "MethodToFunc: Named item '" << varname
    //     << "' not found." << std::endl; return;
    // }

    // auto var = vars[varname];
    // std::string _typename;

    // switch (var->ntype)
    // {
    // case NodeType::LetDeclNode:
    // {
    //     _typename =
    //     getname(std::static_pointer_cast<LetDeclNode>(var)->m_type);
    //     break;
    // }
    // case NodeType::ConstDeclNode:
    // {
    //     _typename =
    //     getname(std::static_pointer_cast<ConstDeclNode>(var)->m_type);
    //     break;
    // }
    // case NodeType::VarDeclNode:
    // {
    //     _typename =
    //     getname(std::static_pointer_cast<VarDeclNode>(var)->m_type);
    //     break;
    // }
    // case NodeType::FunctionParamNode:
    // {
    //     _typename =
    //     getname(std::static_pointer_cast<FunctionParamNode>(var)->m_type);
    //     break;
    // }
    // default:
    //     break;
    // }

    // if (_typename.empty())
    // {
    //     LOG(FATAL) << log::raw << "MethodToFunc: Named item '" << varname
    //     << "' has no typename!" << std::endl; return;
    // }

    // auto thisPtr = std::make_shared<UnaryExprNode>(BitwiseAnd,
    // std::make_shared<IdentifierNode>(varname));

    // call->m_name = _typename + "::" + method;
    // call->m_positional_args.insert(call->m_positional_args.begin(),
    // thisPtr);
  });
}