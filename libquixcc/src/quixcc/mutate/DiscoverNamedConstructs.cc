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

#include <quixcc/Library.h>
#include <quixcc/core/Logger.h>
#include <quixcc/mutate/Routine.h>

#include <algorithm>
#include <iostream>
#include <mutex>
#include <set>

using namespace libquixcc;

static std::string ConstructName(const std::vector<std::string> &prefix,
                                 const std::string &name) {
  if (prefix.empty())
    return name;
  else {
    std::string tmp;
    for (auto &p : prefix) tmp += p + "::";
    return tmp + name;
  }
}

static std::map<libquixcc::NodeType, libquixcc::Msg> error_message_index = {
    {NodeType::VarDeclNode, VAR_NAME_DUPLICATE},
    {NodeType::LetDeclNode, LET_NAME_DUPLICATE},
    {NodeType::ConstDeclNode, CONST_NAME_DUPLICATE},
    {NodeType::StructDefNode, STRUCT_NAME_DUPLICATE},
    {NodeType::StructFieldNode, STRUCT_FIELD_DUPLICATE},
    {NodeType::RegionDefNode, REGION_NAME_DUPLICATE},
    {NodeType::RegionFieldNode, REGION_FIELD_DUPLICATE},
    {NodeType::GroupDefNode, GROUP_NAME_DUPLICATE},
    {NodeType::GroupFieldNode, GROUP_FIELD_DUPLICATE},
    {NodeType::UnionDefNode, UNION_NAME_DUPLICATE},
    {NodeType::UnionFieldNode, UNION_FIELD_DUPLICATE},
    {NodeType::EnumDefNode, ENUM_NAME_DUPLICATE},
    {NodeType::EnumFieldNode, ENUM_FIELD_DUPLICATE},
    {NodeType::FunctionParamNode, PARAM_NAME_DUPLICATE},
    {NodeType::TypedefNode, TYPEDEF_NAME_DUPLICATE},
};

void libquixcc::mutate::DiscoverNamedConstructs(
    quixcc_cc_job_t *job, std::shared_ptr<libquixcc::BlockNode> ast) {
  std::map<std::pair<NodeType, std::string>,
           std::shared_ptr<libquixcc::ParseNode>>
      named_construct_map;
  std::map<std::string, std::shared_ptr<libquixcc::ParseNode>> named_types_map;

  ast->dfs_preorder([&named_construct_map, &named_types_map](
                        const std::vector<std::string> &_namespace,
                        const std::vector<std::string> &_scope,
                        libquixcc::ParseNode *parent,
                        traversal::TraversePtr node) {
    if (node.first != traversal::TraversePtrType::Smart) return;
    auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    std::string tmp;
    bool is_type = false;

    switch ((ptr)->ntype) {
      case NodeType::VarDeclNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::VarDeclNode>(ptr)->m_name);
        break;
      case NodeType::LetDeclNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::LetDeclNode>(ptr)->m_name);
        break;
      case NodeType::ConstDeclNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::ConstDeclNode>(ptr)->m_name);
        break;
      case NodeType::FunctionDeclNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::FunctionDeclNode>(ptr)->m_name);
        break;
      case NodeType::StructDefNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::StructDefNode>(ptr)->m_name);
        is_type = true;
        break;
      case NodeType::StructFieldNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::StructFieldNode>(ptr)->m_name);
        break;
      case NodeType::RegionDefNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::RegionDefNode>(ptr)->m_name);
        is_type = true;
        break;
      case NodeType::RegionFieldNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::RegionFieldNode>(ptr)->m_name);
        break;
      case NodeType::GroupDefNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::GroupDefNode>(ptr)->m_name);
        is_type = true;
        break;
      case NodeType::GroupFieldNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::GroupFieldNode>(ptr)->m_name);
        break;
      case NodeType::UnionDefNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::UnionDefNode>(ptr)->m_name);
        is_type = true;
        break;
      case NodeType::UnionFieldNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::UnionFieldNode>(ptr)->m_name);
        break;
      case NodeType::EnumDefNode:
        tmp = ConstructName(
            _scope, std::static_pointer_cast<libquixcc::EnumDefNode>(ptr)
                        ->m_type->m_name);
        is_type = true;
        break;
      case NodeType::EnumFieldNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::EnumFieldNode>(ptr)->m_name);
        break;
      case NodeType::FunctionParamNode:
        tmp = ConstructName(
            _scope, std::static_pointer_cast<libquixcc::FunctionParamNode>(ptr)
                        ->m_name);
        break;
      case NodeType::TypedefNode:
        tmp = ConstructName(
            _scope,
            std::static_pointer_cast<libquixcc::TypedefNode>(ptr)->m_name);
        is_type = true;
        break;

      default:
        return;
    }

    auto key = std::make_pair((ptr)->ntype, tmp);
    if (named_construct_map.contains(key) && !(ptr)->is<FunctionDeclNode>()) {
      // LOG(ERROR) << core::feedback[error_message_index[(ptr)->ntype]] << tmp
      //            << std::endl;
      return;
    }

    named_construct_map[key] = ptr;

    if (is_type) named_types_map[tmp] = ptr;
  });

  for (auto &pair : named_construct_map)
    LOG(DEBUG) << log::raw << "Found named construct: " << pair.first.second
               << std::endl;

  job->m_inner.m_named_construsts = named_construct_map;
  job->m_inner.m_named_types = named_types_map;
}