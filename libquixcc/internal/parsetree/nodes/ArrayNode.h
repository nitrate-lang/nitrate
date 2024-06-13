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

#ifndef __QUIXCC_PARSE_NODES_ARRAY_H__
#define __QUIXCC_PARSE_NODES_ARRAY_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <llvm/LLVMWrapper.h>
#include <parsetree/nodes/BasicNodes.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
class ArrayTypeNode : public TypeNode {
  ArrayTypeNode(TypeNode *type, std::shared_ptr<ConstExprNode> size)
      : m_type(type), m_size(size) {
    ntype = NodeType::ArrayTypeNode;
  }
  static thread_local std::map<
      std::pair<TypeNode *, std::shared_ptr<ConstExprNode>>, ArrayTypeNode *>
      m_instances;

 public:
  static ArrayTypeNode *create(TypeNode *type,
                               std::shared_ptr<ConstExprNode> size) {
    auto key = std::make_pair(type, size);
    if (m_instances.contains(key)) return m_instances[key];
    auto instance = new ArrayTypeNode(type, size);
    m_instances[key] = instance;
    return instance;
  }

  TypeNode *m_type;
  std::shared_ptr<ConstExprNode> m_size;
};

class VectorTypeNode : public TypeNode {
  VectorTypeNode(TypeNode *type) : m_type(type) {
    ntype = NodeType::VectorTypeNode;
  }
  static thread_local std::map<TypeNode *, VectorTypeNode *> m_instances;

 public:
  static VectorTypeNode *create(TypeNode *type) {
    if (!m_instances.contains(type))
      m_instances[type] = new VectorTypeNode(type);
    return m_instances[type];
  }

  TypeNode *m_type;
};
}  // namespace libquixcc

#endif  // __QUIXCC_PARSE_NODES_ARRAY_H__