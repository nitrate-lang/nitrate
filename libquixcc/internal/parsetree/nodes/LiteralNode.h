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

#ifndef __QUIXCC_PARSE_NODES_LITERAL_H__
#define __QUIXCC_PARSE_NODES_LITERAL_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <llvm/LLVMWrapper.h>
#include <parsetree/nodes/BasicNodes.h>
#include <parsetree/nodes/BasicTypeNode.h>

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace libquixcc {
class LiteralNode : public ConstExprNode {
 public:
  LiteralNode() { ntype = NodeType::LiteralNode; }
};

class UndefLiteralNode : public LiteralNode {
 protected:
  static std::shared_ptr<UndefLiteralNode> m_instance;
  UndefLiteralNode() { ntype = NodeType::UndefLiteralNode; }

 public:
  static const std::shared_ptr<UndefLiteralNode> create() { return m_instance; }
};

class IntegerNode : public LiteralNode {
 protected:
  IntegerNode(const std::string &val);

 public:
  static const std::shared_ptr<IntegerNode> create(const std::string &val) {
    return std::shared_ptr<IntegerNode>(new IntegerNode(val));
  }

  std::string m_val;
  std::shared_ptr<TypeNode> m_val_type;
};

class FloatLiteralNode : public LiteralNode {
 protected:
  FloatLiteralNode(const std::string &val);

 public:
  static const std::shared_ptr<FloatLiteralNode> create(
      const std::string &val) {
    return std::shared_ptr<FloatLiteralNode>(new FloatLiteralNode(val));
  }

  std::string m_val;
  std::shared_ptr<TypeNode> m_val_type;
  double m_value;
};

class StringNode : public LiteralNode {
 protected:
  StringNode(const std::string &val) : m_val(val) {
    ntype = NodeType::StringNode;
  }

 public:
  static const std::shared_ptr<StringNode> create(const std::string &val) {
    return std::shared_ptr<StringNode>(new StringNode(val));
  }

  std::string m_val;
};

class CharNode : public LiteralNode {
 protected:
  CharNode(const std::string &val) : m_val(val) { ntype = NodeType::CharNode; }

 public:
  static const std::shared_ptr<CharNode> create(const std::string &val) {
    return std::shared_ptr<CharNode>(new CharNode(val));
  }

  std::string m_val;
};

class BoolLiteralNode : public LiteralNode {
 protected:
  static std::shared_ptr<BoolLiteralNode> m_true_instance;
  static std::shared_ptr<BoolLiteralNode> m_false_instance;

  BoolLiteralNode(bool val) : m_val(val) { ntype = NodeType::BoolLiteralNode; }

 public:
  static const std::shared_ptr<BoolLiteralNode> create(bool val) {
    if (val)
      return m_true_instance;
    else
      return m_false_instance;
  }

  bool m_val;
};

class NullLiteralNode : public LiteralNode {
 protected:
  static std::shared_ptr<NullLiteralNode> m_instance;
  NullLiteralNode() { ntype = NodeType::NullLiteralNode; }

 public:
  static const std::shared_ptr<NullLiteralNode> create() { return m_instance; }
};
}  // namespace libquixcc

#endif  // __QUIXCC_PARSE_NODES_LITERAL_H__