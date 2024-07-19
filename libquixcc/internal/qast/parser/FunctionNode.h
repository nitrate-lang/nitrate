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

#ifndef __QUIXCC_PARSE_NODES_FUNCTION_H__
#define __QUIXCC_PARSE_NODES_FUNCTION_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/llvm/LLVMWrapper.h>
#include <qast/parser/BasicNodes.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace libquixcc {
  class FunctionTypeNode : public TypeNode {
public:
    FunctionTypeNode(std::shared_ptr<TypeNode> return_type,
                     std::vector<std::pair<std::string, std::shared_ptr<TypeNode>>> params,
                     bool variadic, bool pure, bool thread_safe, bool foreign, bool _noexcept)
        : m_return_type(return_type), m_params(params), m_variadic(variadic), m_pure(pure),
          m_thread_safe(thread_safe), m_foreign(foreign), m_noexcept(_noexcept) {
      ntype = NodeType::FunctionTypeNode;
    }

    std::shared_ptr<TypeNode> m_return_type;
    std::vector<std::pair<std::string, std::shared_ptr<TypeNode>>> m_params;
    bool m_variadic;
    bool m_pure;
    bool m_thread_safe;
    bool m_foreign;
    bool m_noexcept;
  };

  class FunctionParamNode : public DeclNode {
public:
    FunctionParamNode() : m_type(nullptr) { ntype = NodeType::FunctionParamNode; }
    FunctionParamNode(const std::string &name, std::shared_ptr<TypeNode> type,
                      std::shared_ptr<ExprNode> value)
        : m_name(name), m_type(type), m_value(value) {
      ntype = NodeType::FunctionParamNode;
    }

    std::string m_name;
    std::shared_ptr<TypeNode> m_type;
    std::shared_ptr<ExprNode> m_value;
  };

  class FunctionDeclNode : public DeclNode {
public:
    FunctionDeclNode() : m_type(nullptr) { ntype = NodeType::FunctionDeclNode; }

    std::string m_name;
    std::vector<std::shared_ptr<FunctionParamNode>> m_params;
    std::shared_ptr<FunctionTypeNode> m_type;
  };

  class FunctionDefNode : public DefNode {
public:
    FunctionDefNode() { ntype = NodeType::FunctionDefNode; }
    FunctionDefNode(std::shared_ptr<FunctionDeclNode> decl, std::shared_ptr<BlockNode> body,
                    std::shared_ptr<ExprNode> req_in = nullptr,
                    std::shared_ptr<ExprNode> req_out = nullptr, std::set<std::string> items = {})
        : m_decl(decl), m_body(body), m_req_in(req_in), m_req_out(req_out), m_items(items) {
      ntype = NodeType::FunctionDefNode;
    }

    std::shared_ptr<FunctionDeclNode> m_decl;
    std::shared_ptr<BlockNode> m_body;
    std::shared_ptr<ExprNode> m_req_in;
    std::shared_ptr<ExprNode> m_req_out;
    std::set<std::string> m_items;
  };
} // namespace libquixcc

#endif // __QUIXCC_PARSE_NODES_FUNCTION_H__