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

#ifndef __QUIXCC_PARSE_NODES_CONTAINER_H__
#define __QUIXCC_PARSE_NODES_CONTAINER_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/llvm/LLVMWrapper.h>
#include <quixcc/parsetree/nodes/BasicNodes.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
class ArrayTypeNode : public TypeNode {
 public:
  ArrayTypeNode(std::shared_ptr<TypeNode> type,
                std::shared_ptr<ConstExprNode> size)
      : m_type(type), m_size(size) {
    ntype = NodeType::ArrayTypeNode;
  }

  std::shared_ptr<TypeNode> m_type;
  std::shared_ptr<ConstExprNode> m_size;
};

class VectorTypeNode : public TypeNode {
 public:
  VectorTypeNode(std::shared_ptr<TypeNode> type) : m_type(type) {
    ntype = NodeType::VectorTypeNode;
  }

  std::shared_ptr<TypeNode> m_type;
};

class MapTypeNode : public TypeNode {
 public:
  MapTypeNode(std::shared_ptr<TypeNode> key_type,
              std::shared_ptr<TypeNode> value_type)
      : m_key_type(key_type), m_value_type(value_type) {
    ntype = NodeType::MapTypeNode;
  }

  std::shared_ptr<TypeNode> m_key_type;
  std::shared_ptr<TypeNode> m_value_type;
};

class TupleTypeNode : public TypeNode {
 public:
  TupleTypeNode(std::vector<std::shared_ptr<TypeNode>> types) : m_types(types) {
    ntype = NodeType::TupleTypeNode;
  }

  std::vector<std::shared_ptr<TypeNode>> m_types;
};

class SetTypeNode : public TypeNode {
 public:
  SetTypeNode(std::shared_ptr<TypeNode> type) : m_type(type) {
    ntype = NodeType::SetTypeNode;
  }

  std::shared_ptr<TypeNode> m_type;
};
}  // namespace libquixcc

#endif  // __QUIXCC_PARSE_NODES_CONTAINER_H__
