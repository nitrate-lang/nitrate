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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
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
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

        std::shared_ptr<ExprNode> m_expr;
    };

    class RetifStmtNode : public StmtNode
    {
    public:
        RetifStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<ExprNode> &return_val)
            : m_cond(cond), m_return(return_val) { ntype = NodeType::RetifStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

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
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

        std::shared_ptr<ExprNode> m_cond;
        std::shared_ptr<ExprNode> m_return;
    };

    class RetvStmtNode : public StmtNode
    {
    public:
        RetvStmtNode(const std::shared_ptr<ExprNode> &cond) : m_cond(cond) { ntype = NodeType::RetvStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

        std::shared_ptr<ExprNode> m_cond;
    };

    class IfStmtNode : public StmtNode
    {
    public:
        IfStmtNode(const std::shared_ptr<ExprNode> &cond, const std::shared_ptr<StmtNode> &then, const std::shared_ptr<StmtNode> &els)
            : m_cond(cond), m_then(then), m_else(els) { ntype = NodeType::IfStmtNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

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
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

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
        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual std::unique_ptr<StmtNode> reduce() const override;

        std::shared_ptr<ExprNode> m_init;
        std::shared_ptr<ExprNode> m_cond;
        std::shared_ptr<ExprNode> m_step;
        std::shared_ptr<StmtNode> m_stmt;
    };
}

#endif // __QUIXCC_PARSE_NODES_CONTROL_FLOW_H__