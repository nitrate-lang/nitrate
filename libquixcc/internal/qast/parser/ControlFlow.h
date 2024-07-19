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

#ifndef __QUIXCC_PARSE_NODES_CONTROL_FLOW_H__
#define __QUIXCC_PARSE_NODES_CONTROL_FLOW_H__

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
  class ReturnStmtNode : public StmtNode {
public:
    ReturnStmtNode(const std::shared_ptr<ExprNode> &expr) : m_expr(expr) {
      ntype = NodeType::ReturnStmtNode;
    }

    std::shared_ptr<ExprNode> m_expr;
  };

  class RetifStmtNode : public StmtNode {
public:
    RetifStmtNode(const std::shared_ptr<ExprNode> &cond,
                  const std::shared_ptr<ExprNode> &return_val)
        : m_cond(cond), m_return(return_val) {
      ntype = NodeType::RetifStmtNode;
    }

    std::shared_ptr<ExprNode> m_cond;
    std::shared_ptr<ExprNode> m_return;
  };

  class RetzStmtNode : public StmtNode {
public:
    RetzStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<ExprNode> &return_val)
        : m_cond(cond), m_return(return_val) {
      ntype = NodeType::RetzStmtNode;
    }

    std::shared_ptr<ExprNode> m_cond;
    std::shared_ptr<ExprNode> m_return;
  };

  class RetvStmtNode : public StmtNode {
public:
    RetvStmtNode(const std::shared_ptr<ExprNode> &cond) : m_cond(cond) {
      ntype = NodeType::RetvStmtNode;
    }

    std::shared_ptr<ExprNode> m_cond;
  };

  class BreakStmtNode : public StmtNode {
public:
    BreakStmtNode() { ntype = NodeType::BreakStmtNode; }
  };

  class ContinueStmtNode : public StmtNode {
public:
    ContinueStmtNode() { ntype = NodeType::ContinueStmtNode; }
  };

  class IfStmtNode : public StmtNode {
public:
    IfStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<StmtNode> &then,
               const std::shared_ptr<StmtNode> &els)
        : m_cond(cond), m_then(then), m_else(els) {
      ntype = NodeType::IfStmtNode;
    }

    std::shared_ptr<ExprNode> m_cond;
    std::shared_ptr<StmtNode> m_then;
    std::shared_ptr<StmtNode> m_else;
  };

  class WhileStmtNode : public StmtNode {
public:
    WhileStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<StmtNode> &body)
        : m_cond(cond), m_stmt(body) {
      ntype = NodeType::WhileStmtNode;
    }

    std::shared_ptr<ExprNode> m_cond;
    std::shared_ptr<StmtNode> m_stmt;
  };

  class ForStmtNode : public StmtNode {
public:
    ForStmtNode(const std::shared_ptr<StmtNode> &init, const std::shared_ptr<ExprNode> &cond,
                const std::shared_ptr<ExprNode> &step, const std::shared_ptr<StmtNode> &body)
        : m_init(init), m_cond(cond), m_step(step), m_stmt(body) {
      ntype = NodeType::ForStmtNode;
    }

    std::shared_ptr<StmtNode> m_init;
    std::shared_ptr<ExprNode> m_cond;
    std::shared_ptr<ExprNode> m_step;
    std::shared_ptr<StmtNode> m_stmt;
  };

  class FormStmtNode : public StmtNode {
public:
    FormStmtNode(const std::string &var, const std::shared_ptr<ExprNode> &range,
                 const std::shared_ptr<BlockNode> &block)
        : m_var(var), m_range(range), m_block(block) {
      ntype = NodeType::FormStmtNode;
    }

    std::string m_var;
    std::shared_ptr<ExprNode> m_range;
    std::shared_ptr<BlockNode> m_block;
  };

  class ForeachStmtNode : public StmtNode {
public:
    ForeachStmtNode(const std::string &var, const std::shared_ptr<ExprNode> &range,
                    const std::shared_ptr<BlockNode> &block)
        : m_var(var), m_range(range), m_block(block) {
      ntype = NodeType::ForeachStmtNode;
    }

    std::string m_var;
    std::shared_ptr<ExprNode> m_range;
    std::shared_ptr<BlockNode> m_block;
  };

  class CaseStmtNode : public StmtNode {
public:
    CaseStmtNode(const std::shared_ptr<ExprNode> &expr, const std::shared_ptr<BlockNode> &block)
        : m_expr(expr), m_block(block) {
      ntype = NodeType::CaseStmtNode;
    }

    std::shared_ptr<ExprNode> m_expr;
    std::shared_ptr<BlockNode> m_block;
  };

  class SwitchStmtNode : public StmtNode {
public:
    SwitchStmtNode(const std::shared_ptr<ExprNode> &expr,
                   const std::vector<std::shared_ptr<CaseStmtNode>> &cases,
                   const std::shared_ptr<BlockNode> &def)
        : m_expr(expr), m_cases(cases), m_default(def) {
      ntype = NodeType::SwitchStmtNode;
    }

    std::shared_ptr<ExprNode> m_expr;
    std::vector<std::shared_ptr<CaseStmtNode>> m_cases;
    std::shared_ptr<BlockNode> m_default;
  };
} // namespace libquixcc

#endif // __QUIXCC_PARSE_NODES_CONTROL_FLOW_H__