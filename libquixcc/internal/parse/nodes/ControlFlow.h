////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_PARSE_NODES_CONTROL_FLOW_H__
#define __QUIXCC_PARSE_NODES_CONTROL_FLOW_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <llvm/LLVMWrapper.h>
#include <lexer/Token.h>
#include <parse/nodes/BasicNodes.h>
#include <parse/nodes/LiteralNode.h>

namespace libquixcc
{
    class ReturnStmtNode : public StmtNode
    {
    public:
        ReturnStmtNode(const std::shared_ptr<ExprNode> &expr) : m_expr(expr) { ntype = NodeType::ReturnStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_expr;
    };

    class RetifStmtNode : public StmtNode
    {
    public:
        RetifStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<ExprNode> &return_val)
            : m_cond(cond), m_return(return_val) { ntype = NodeType::RetifStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const override { throw std::runtime_error("RetifStmtNode::codegen not implemented"); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_cond;
        std::shared_ptr<ExprNode> m_return;
    };

    class RetzStmtNode : public StmtNode
    {
    public:
        RetzStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<ExprNode> &return_val)
            : m_cond(cond), m_return(return_val) { ntype = NodeType::RetzStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const override { throw std::runtime_error("RetzStmtNode::codegen not implemented"); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_cond;
        std::shared_ptr<ExprNode> m_return;
    };

    class RetvStmtNode : public StmtNode
    {
    public:
        RetvStmtNode(const std::shared_ptr<ExprNode> &cond) : m_cond(cond) { ntype = NodeType::RetvStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const override { throw std::runtime_error("RetvStmtNode::codegen not implemented"); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_cond;
    };

    class IfStmtNode : public StmtNode
    {
    public:
        IfStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<StmtNode> &then, const std::shared_ptr<StmtNode> &els)
            : m_cond(cond), m_then(then), m_else(els) { ntype = NodeType::IfStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_cond;
        std::shared_ptr<StmtNode> m_then;
        std::shared_ptr<StmtNode> m_else;
    };

    class WhileStmtNode : public StmtNode
    {
    public:
        WhileStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<StmtNode> &body)
            : m_cond(cond), m_stmt(body) { ntype = NodeType::WhileStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_cond;
        std::shared_ptr<StmtNode> m_stmt;
    };

    class ForStmtNode : public StmtNode
    {
    public:
        ForStmtNode(const std::shared_ptr<ExprNode> &init, const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<ExprNode> &step, const std::shared_ptr<StmtNode> &body)
            : m_init(init), m_cond(cond), m_step(step), m_stmt(body) { ntype = NodeType::ForStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(CodegenVisitor &visitor) const override { throw std::runtime_error("ForStmtNode::codegen not implemented"); }
        virtual std::unique_ptr<StmtNode> reduce(ReductionState &state) const override;

        std::shared_ptr<ExprNode> m_init;
        std::shared_ptr<ExprNode> m_cond;
        std::shared_ptr<ExprNode> m_step;
        std::shared_ptr<StmtNode> m_stmt;
    };
}

#endif // __QUIXCC_PARSE_NODES_CONTROL_FLOW_H__