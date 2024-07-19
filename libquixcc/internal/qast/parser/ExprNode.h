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

#ifndef __QUIXCC_PARSE_NODES_EXPR_H__
#define __QUIXCC_PARSE_NODES_EXPR_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/lexer/Token.h>
#include <quixcc/llvm/LLVMWrapper.h>
#include <qast/parser/BasicNodes.h>
#include <qast/parser/LiteralNode.h>

#include <memory>
#include <string>
#include <vector>

namespace libquixcc {
  class UnaryExprNode : public ExprNode {
public:
    UnaryExprNode(Operator op, const std::shared_ptr<ExprNode> &expr) : m_op(op), m_expr(expr) {
      ntype = NodeType::UnaryExprNode;
    }

    Operator m_op;
    std::shared_ptr<ExprNode> m_expr;
  };

  class PostUnaryExprNode : public ExprNode {
public:
    PostUnaryExprNode(Operator op, const std::shared_ptr<ExprNode> &expr) : m_op(op), m_expr(expr) {
      ntype = NodeType::PostUnaryExprNode;
    }

    Operator m_op;
    std::shared_ptr<ExprNode> m_expr;
  };

  class BinaryExprNode : public ExprNode {
public:
    BinaryExprNode(Operator op, const std::shared_ptr<ExprNode> &lhs,
                   const std::shared_ptr<ExprNode> &rhs)
        : m_op(op), m_lhs(lhs), m_rhs(rhs) {
      ntype = NodeType::BinaryExprNode;
    }

    Operator m_op;
    std::shared_ptr<ExprNode> m_lhs;
    std::shared_ptr<ExprNode> m_rhs;
  };

  class SeqExprNode : public ExprNode {
public:
    SeqExprNode(const std::shared_ptr<ExprNode> &lhs, const std::shared_ptr<ExprNode> &rhs)
        : m_lhs(lhs), m_rhs(rhs) {
      ntype = NodeType::SeqExprNode;
    }

    std::shared_ptr<ExprNode> m_lhs;
    std::shared_ptr<ExprNode> m_rhs;
  };

  class FunctionDeclNode;

  class CallExprNode : public ExprNode {
public:
    CallExprNode() { ntype = NodeType::CallExprNode; }

    std::shared_ptr<ExprNode> m_callee;
    std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> m_named_args;
    std::vector<std::shared_ptr<ExprNode>> m_positional_args;
    std::shared_ptr<FunctionDeclNode> m_decl;
  };

  class ListExprNode : public ExprNode {
public:
    ListExprNode(std::vector<std::shared_ptr<ExprNode>> elements) : m_elements(elements) {
      ntype = NodeType::ListExprNode;
    }

    std::vector<std::shared_ptr<ExprNode>> m_elements;
  };

  class AssocExprNode : public ExprNode {
public:
    AssocExprNode(
        std::vector<std::pair<std::shared_ptr<ExprNode>, std::shared_ptr<ExprNode>>> elements)
        : m_elements(elements) {
      ntype = NodeType::AssocExprNode;
    }

    std::vector<std::pair<std::shared_ptr<ExprNode>, std::shared_ptr<ExprNode>>> m_elements;
  };

  class MemberAccessNode : public ExprNode {
public:
    MemberAccessNode(const std::shared_ptr<ExprNode> &expr, const std::string &field)
        : m_expr(expr), m_field(field) {
      ntype = NodeType::MemberAccessNode;
    }

    std::shared_ptr<ExprNode> m_expr;
    std::string m_field;
  };

  class IndexNode : public ExprNode {
public:
    IndexNode(const std::shared_ptr<ExprNode> &expr, const std::shared_ptr<ExprNode> &index)
        : m_expr(expr), m_index(index) {
      ntype = NodeType::IndexNode;
    }

    std::shared_ptr<ExprNode> m_expr;
    std::shared_ptr<ExprNode> m_index;
  };

  class SliceNode : public ExprNode {
public:
    SliceNode(const std::shared_ptr<ExprNode> &expr, const std::shared_ptr<ExprNode> &start,
              const std::shared_ptr<ExprNode> &end)
        : m_expr(expr), m_start(start), m_end(end) {
      ntype = NodeType::SliceNode;
    }

    std::shared_ptr<ExprNode> m_expr;
    std::shared_ptr<ExprNode> m_start;
    std::shared_ptr<ExprNode> m_end;
  };

  class CastExprNode : public ExprNode {
public:
    CastExprNode(const std::shared_ptr<ExprNode> &expr, std::shared_ptr<TypeNode> type)
        : m_expr(expr), m_type(type) {
      ntype = NodeType::CastExprNode;
    }

    std::shared_ptr<ExprNode> m_expr;
    std::shared_ptr<TypeNode> m_type;
  };

  class StaticCastExprNode : public CastExprNode {
public:
    StaticCastExprNode(const std::shared_ptr<ExprNode> &expr, std::shared_ptr<TypeNode> type)
        : CastExprNode(expr, type) {
      ntype = NodeType::StaticCastExprNode;
    }
  };

  class BitCastExprNode : public CastExprNode {
public:
    BitCastExprNode(const std::shared_ptr<ExprNode> &expr, std::shared_ptr<TypeNode> type)
        : CastExprNode(expr, type) {
      ntype = NodeType::BitCastExprNode;
    }
  };

  class SignedUpcastExprNode : public CastExprNode {
public:
    SignedUpcastExprNode(const std::shared_ptr<ExprNode> &expr, std::shared_ptr<TypeNode> type)
        : CastExprNode(expr, type) {
      ntype = NodeType::SignedUpcastExprNode;
    }
  };

  class UnsignedUpcastExprNode : public CastExprNode {
public:
    UnsignedUpcastExprNode(const std::shared_ptr<ExprNode> &expr, std::shared_ptr<TypeNode> type)
        : CastExprNode(expr, type) {
      ntype = NodeType::UnsignedUpcastExprNode;
    }
  };

  class DowncastExprNode : public CastExprNode {
public:
    DowncastExprNode(const std::shared_ptr<ExprNode> &expr, std::shared_ptr<TypeNode> type)
        : CastExprNode(expr, type) {
      ntype = NodeType::DowncastExprNode;
    }
  };

} // namespace libquixcc

#endif // __QUIXCC_PARSE_NODES_EXPR_H__