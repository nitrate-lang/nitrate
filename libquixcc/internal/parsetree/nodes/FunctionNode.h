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

#include <llvm/LLVMWrapper.h>
#include <parsetree/nodes/BasicNodes.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
class FunctionTypeNode : public TypeNode {
  FunctionTypeNode(TypeNode *return_type,
                   std::vector<std::pair<std::string, TypeNode *>> params,
                   bool variadic, bool pure, bool thread_safe, bool foreign,
                   bool _noexcept)
      : m_return_type(return_type),
        m_params(params),
        m_variadic(variadic),
        m_pure(pure),
        m_thread_safe(thread_safe),
        m_foreign(foreign),
        m_noexcept(_noexcept) {
    ntype = NodeType::FunctionTypeNode;
  }

  struct Inner {
    TypeNode *return_type;
    std::vector<std::pair<std::string, TypeNode *>> params;
    bool variadic;
    bool pure;
    bool thread_safe;
    bool foreign;
    bool _noexcept;

    Inner(TypeNode *return_type,
          std::vector<std::pair<std::string, TypeNode *>> params, bool variadic,
          bool pure, bool thread_safe, bool foreign, bool _noexcept)
        : return_type(return_type),
          params(params),
          variadic(variadic),
          pure(pure),
          thread_safe(thread_safe),
          foreign(foreign),
          _noexcept(_noexcept) {}

    bool operator<(const Inner &other) const {
      if (return_type != other.return_type)
        return return_type < other.return_type;
      if (params != other.params) return params < other.params;
      if (variadic != other.variadic) return variadic < other.variadic;
      if (pure != other.pure) return pure < other.pure;
      if (thread_safe != other.thread_safe)
        return thread_safe < other.thread_safe;
      if (foreign != other.foreign) return foreign < other.foreign;
      if (_noexcept != other._noexcept) return _noexcept < other._noexcept;
      return false;
    }
  };

  static std::map<Inner, FunctionTypeNode *> s_instances;

 public:
  static FunctionTypeNode *create(
      TypeNode *return_type,
      std::vector<std::pair<std::string, TypeNode *>> params, bool variadic,
      bool pure, bool thread_safe, bool foreign, bool _noexcept) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    Inner inner(return_type, params, variadic, pure, thread_safe, foreign,
                _noexcept);
    if (s_instances.find(inner) == s_instances.end())
      s_instances[inner] = new FunctionTypeNode(
          return_type, params, variadic, pure, thread_safe, foreign, _noexcept);
    return s_instances[inner];
  }

  TypeNode *m_return_type;
  std::vector<std::pair<std::string, TypeNode *>> m_params;
  bool m_variadic;
  bool m_pure;
  bool m_thread_safe;
  bool m_foreign;
  bool m_noexcept;
};

class FunctionParamNode : public DeclNode {
 public:
  FunctionParamNode() : m_type(nullptr) { ntype = NodeType::FunctionParamNode; }
  FunctionParamNode(const std::string &name, TypeNode *type,
                    std::shared_ptr<ExprNode> value)
      : m_name(name), m_type(type), m_value(value) {
    ntype = NodeType::FunctionParamNode;
  }

  std::string m_name;
  TypeNode *m_type;
  std::shared_ptr<ExprNode> m_value;
};

class FunctionDeclNode : public DeclNode {
 public:
  FunctionDeclNode() : m_type(nullptr) { ntype = NodeType::FunctionDeclNode; }

  std::string m_name;
  std::vector<std::shared_ptr<FunctionParamNode>> m_params;
  FunctionTypeNode *m_type;
};

class FunctionDefNode : public DefNode {
 public:
  FunctionDefNode() { ntype = NodeType::FunctionDefNode; }
  FunctionDefNode(std::shared_ptr<FunctionDeclNode> decl,
                  std::shared_ptr<BlockNode> body)
      : m_decl(decl), m_body(body) {
    ntype = NodeType::FunctionDefNode;
  }

  std::shared_ptr<FunctionDeclNode> m_decl;
  std::shared_ptr<BlockNode> m_body;
};
}  // namespace libquixcc

#endif  // __QUIXCC_PARSE_NODES_FUNCTION_H__