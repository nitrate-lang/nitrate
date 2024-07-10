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

#ifndef __QUIXCC_PARSE_NODES_VARIABLE_H__
#define __QUIXCC_PARSE_NODES_VARIABLE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/llvm/LLVMWrapper.h>
#include <quixcc/parsetree/nodes/BasicNodes.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
class VarDeclNode : public DeclNode {
 public:
  VarDeclNode()
      : m_name(""),
        m_type(nullptr),
        m_init(nullptr),
        m_is_mut(false),
        m_is_thread_local(false),
        m_is_static(false),
        m_is_deprecated(false) {
    ntype = NodeType::VarDeclNode;
  }
  VarDeclNode(const std::string &name, std::shared_ptr<TypeNode> type,
              const std::shared_ptr<ExprNode> init, bool is_mut,
              bool is_thread_local, bool is_static, bool is_deprecated = false)
      : m_name(name),
        m_type(type),
        m_init(init),
        m_is_mut(is_mut),
        m_is_thread_local(is_thread_local),
        m_is_static(is_static),
        m_is_deprecated(is_deprecated) {
    ntype = NodeType::VarDeclNode;
  }

  std::string m_name;
  std::shared_ptr<TypeNode> m_type;
  std::shared_ptr<ExprNode> m_init;

  bool m_is_mut;
  bool m_is_thread_local;
  bool m_is_static;
  bool m_is_deprecated;
};

class LetDeclNode : public DeclNode {
 public:
  LetDeclNode()
      : m_name(""),
        m_type(nullptr),
        m_init(nullptr),
        m_is_mut(false),
        m_is_thread_local(false),
        m_is_static(false),
        m_is_deprecated(false) {
    ntype = NodeType::LetDeclNode;
  }
  LetDeclNode(const std::string &name, std::shared_ptr<TypeNode> type,
              const std::shared_ptr<ExprNode> init, bool is_mut = false,
              bool is_thread_local = false, bool is_static = false,
              bool is_deprecated = false)
      : m_name(name),
        m_type(type),
        m_init(init),
        m_is_mut(is_mut),
        m_is_thread_local(is_thread_local),
        m_is_static(is_static),
        m_is_deprecated(is_deprecated) {
    ntype = NodeType::LetDeclNode;
  }

  std::string m_name;
  std::shared_ptr<TypeNode> m_type;
  std::shared_ptr<ExprNode> m_init;

  bool m_is_mut;
  bool m_is_thread_local;
  bool m_is_static;
  bool m_is_deprecated;
};

class ConstDeclNode : public DeclNode {
 public:
  ConstDeclNode()
      : m_name(""), m_type(nullptr), m_init(nullptr), m_is_deprecated(false) {
    ntype = NodeType::ConstDeclNode;
  }
  ConstDeclNode(const std::string &name, std::shared_ptr<TypeNode> type,
                const std::shared_ptr<ExprNode> init,
                bool is_deprecated = false)
      : m_name(name),
        m_type(type),
        m_init(init),
        m_is_deprecated(is_deprecated) {
    ntype = NodeType::ConstDeclNode;
  }

  std::string m_name;
  std::shared_ptr<TypeNode> m_type;
  std::shared_ptr<ExprNode> m_init;

  bool m_is_deprecated;
};
}  // namespace libquixcc

#endif  // __QUIXCC_PARSE_NODES_VARIABLE_H__