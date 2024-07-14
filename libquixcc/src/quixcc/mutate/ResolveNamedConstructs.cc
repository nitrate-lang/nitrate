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

static std::string join_ns(const std::vector<std::string> &ns) {
  std::string tmp;
  for (auto &p : ns)
    tmp += p + "::";
  return tmp;
}

static void resolve_user_type_nodes(quixcc_cc_job_t *job, std::shared_ptr<BlockNode> ast) {
  ast->dfs_preorder([job](const std::vector<std::string> &_namespace,
                          const std::vector<std::string> &_scope, ParseNode *parent,
                          traversal::TraversePtr node) {
    if (node.first != traversal::TraversePtrType::Smart)
      return;

    auto sptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    if (!(sptr)->is<UserTypeNode>())
      return;

    std::shared_ptr<ParseNode> *user_type_ptr = std::get<std::shared_ptr<ParseNode> *>(node.second);

    std::shared_ptr<UserTypeNode> user_type =
        std::static_pointer_cast<UserTypeNode>(*user_type_ptr);
    std::string name = user_type->m_name;
    std::shared_ptr<ParseNode> named_type;

    if (!job->m_inner.m_named_types.contains(user_type->m_name) && _scope.empty()) {
      LOG(ERROR) << core::feedback[UNRESOLVED_TYPE] << user_type->m_name << std::endl;
      return;
    } else if (!job->m_inner.m_named_types.contains(user_type->m_name)) {
      std::vector<std::string> tmp = _namespace;

      while (!tmp.empty()) {
        std::string abs = join_ns(tmp) + user_type->m_name;

        LOG(DEBUG) << "Searching for " << abs << std::endl;

        if (job->m_inner.m_named_types.contains(abs)) {
          named_type = job->m_inner.m_named_types[abs];
          break;
        }

        tmp.pop_back();
      }

      if (!named_type) {
        LOG(ERROR) << core::feedback[UNRESOLVED_TYPE] << user_type->m_name << std::endl;
        return;
      }
    } else {
      named_type = job->m_inner.m_named_types[user_type->m_name];
    }

    std::shared_ptr<TypeNode> type = nullptr;

    switch (named_type->ntype) {
    case NodeType::StructDefNode:
      type = std::static_pointer_cast<StructDefNode>(named_type)->get_type();
      break;
    case NodeType::RegionDefNode:
      type = std::static_pointer_cast<RegionDefNode>(named_type)->get_type();
      break;
    case NodeType::GroupDefNode:
      type = std::static_pointer_cast<GroupDefNode>(named_type)->get_type();
      break;
    case NodeType::UnionDefNode:
      type = std::static_pointer_cast<UnionDefNode>(named_type)->get_type();
      break;
    case NodeType::EnumDefNode:
      type = std::static_pointer_cast<EnumDefNode>(named_type)->get_type();
      break;
    case NodeType::TypedefNode:
      type = std::static_pointer_cast<TypedefNode>(named_type)->m_orig;
      break;
    default:
      throw std::runtime_error("Unimplemented typeid in ResolveNamedConstructs");
    }

    *user_type_ptr = std::static_pointer_cast<ParseNode>(type);

    if (job->m_debug) {
      LOG(DEBUG) << core::feedback[RESOLVED_TYPE] << name << type->to_string() << std::endl;
    }
  });
}

static void resolve_function_decls_to_calls(quixcc_cc_job_t *job, std::shared_ptr<BlockNode> ast) {
  ast->dfs_preorder([job](const std::vector<std::string> &_namespace,
                          const std::vector<std::string> &_scope, ParseNode *parent,
                          traversal::TraversePtr node) {
    if (node.first != traversal::TraversePtrType::Smart)
      return;
    auto ptr = *std::get<std::shared_ptr<ParseNode> *>(node.second);

    if (!(ptr)->is<CallExprNode>())
      return;

    auto call = std::static_pointer_cast<CallExprNode>(ptr);

    std::string callee_name;

    /// TODO: implement this

    switch (call->m_callee->ntype) {
    case NodeType::IdentifierNode:
      callee_name = std::static_pointer_cast<IdentifierNode>(call->m_callee)->m_name;
      break;
    case NodeType::MemberAccessNode:
      callee_name = std::static_pointer_cast<MemberAccessNode>(call->m_callee)->m_field;
      break;
    default:
      throw std::runtime_error(std::string("Unimplemented typeid in ResolveNamedConstructs: ") +
                               NodeTypeNames[call->m_callee->ntype].data());
    }

    std::vector<std::string> tmp = _scope;

    while (!tmp.empty()) {
      std::string abs = join_ns(tmp) + callee_name;
      if (job->m_inner.m_named_construsts.contains(
              std::make_pair(NodeType::FunctionDeclNode, abs))) {
        auto func_decl = std::static_pointer_cast<FunctionDeclNode>(
            job->m_inner.m_named_construsts[std::make_pair(NodeType::FunctionDeclNode, abs)]);
        call->m_decl = func_decl;
        return;
      }

      tmp.pop_back();
    }

    if (!job->m_inner.m_named_construsts.contains(
            std::make_pair(NodeType::FunctionDeclNode, callee_name))) {
      LOG(DEBUG) << core::feedback[UNRESOLVED_FUNCTION] << callee_name << std::endl;
      return;
    }

    auto func_decl = std::static_pointer_cast<FunctionDeclNode>(
        job->m_inner.m_named_construsts[std::make_pair(NodeType::FunctionDeclNode, callee_name)]);
    call->m_decl = func_decl;
  });
}

void mutate::ResolveNamedConstructs(quixcc_cc_job_t *job, std::shared_ptr<BlockNode> ast) {
  resolve_user_type_nodes(job, ast);
  resolve_function_decls_to_calls(job, ast);
}