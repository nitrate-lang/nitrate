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

#ifndef __QUIXCC_PARSE_NODES_INTEGER_H__
#define __QUIXCC_PARSE_NODES_INTEGER_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/llvm/LLVMWrapper.h>
#include <quixcc/parsetree/nodes/BasicNodes.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
class MutTypeNode : public TypeNode {
 public:
  MutTypeNode(std::shared_ptr<TypeNode> type) : m_type(type) {
    ntype = NodeType::MutTypeNode;
  }

  std::shared_ptr<TypeNode> m_type;
};

class U8TypeNode : public TypeNode {
 public:
  U8TypeNode() { ntype = NodeType::U8TypeNode; }
};

class U16TypeNode : public TypeNode {
 public:
  U16TypeNode() { ntype = NodeType::U16TypeNode; }
};

class U32TypeNode : public TypeNode {
 public:
  U32TypeNode() { ntype = NodeType::U32TypeNode; }
};

class U64TypeNode : public TypeNode {
 public:
  U64TypeNode() { ntype = NodeType::U64TypeNode; }
};

class U128TypeNode : public TypeNode {
 public:
  U128TypeNode() { ntype = NodeType::U128TypeNode; }
};

class I8TypeNode : public TypeNode {
 public:
  I8TypeNode() { ntype = NodeType::I8TypeNode; }
};

class I16TypeNode : public TypeNode {
 public:
  I16TypeNode() { ntype = NodeType::I16TypeNode; }
};

class I32TypeNode : public TypeNode {
 public:
  I32TypeNode() { ntype = NodeType::I32TypeNode; }
};

class I64TypeNode : public TypeNode {
 public:
  I64TypeNode() { ntype = NodeType::I64TypeNode; }
};

class I128TypeNode : public TypeNode {
 public:
  I128TypeNode() { ntype = NodeType::I128TypeNode; }
};

class F32TypeNode : public TypeNode {
 public:
  F32TypeNode() { ntype = NodeType::F32TypeNode; }
};

class F64TypeNode : public TypeNode {
 public:
  F64TypeNode() { ntype = NodeType::F64TypeNode; }
};

class BoolTypeNode : public TypeNode {
 public:
  BoolTypeNode() { ntype = NodeType::BoolTypeNode; }
};

class VoidTypeNode : public TypeNode {
 public:
  VoidTypeNode() { ntype = NodeType::VoidTypeNode; }
};

class NullTypeNode : public TypeNode {
 public:
  NullTypeNode() { ntype = NodeType::NullTypeNode; }
};

class PointerTypeNode : public TypeNode {
 public:
  PointerTypeNode(std::shared_ptr<TypeNode> type) : m_type(type) {
    ntype = NodeType::PointerTypeNode;
  }

  std::shared_ptr<TypeNode> m_type;
};

class OpaqueTypeNode : public TypeNode {
 public:
  OpaqueTypeNode(std::string name) : m_name(name) {
    ntype = NodeType::OpaqueTypeNode;
  }

  std::string m_name;
};
}  // namespace libquixcc

#endif  // __QUIXCC_PARSE_NODES_INTEGER_H__