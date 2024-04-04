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

#ifndef __QUIXCC_PARSE_NODES_FUNCTION_H__
#define __QUIXCC_PARSE_NODES_FUNCTION_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <vector>
#include <memory>

#include <llvm/LLVMWrapper.h>
#include <parse/nodes/BasicNodes.h>

namespace libquixcc
{
    class FunctionTypeNode : public TypeNode
    {
        FunctionTypeNode(TypeNode *return_type, std::vector<TypeNode *> params, bool variadic = false, bool pure = false, bool thread_safe = false, bool foreign = false, bool nothrow = false)
            : m_return_type(return_type), m_params(params), m_variadic(variadic), m_pure(pure), m_thread_safe(thread_safe), m_foreign(foreign), m_nothrow(nothrow) { ntype = NodeType::FunctionTypeNode; }

        struct Inner
        {
            TypeNode *return_type;
            std::vector<TypeNode *> params;
            bool variadic;
            bool pure;
            bool thread_safe;
            bool foreign;
            bool nothrow;

            Inner(TypeNode *return_type, std::vector<TypeNode *> params, bool variadic, bool pure, bool thread_safe, bool foreign, bool nothrow)
                : return_type(return_type), params(params), variadic(variadic), pure(pure), thread_safe(thread_safe), foreign(foreign), nothrow(nothrow) {}

            bool operator<(const Inner &other) const
            {
                if (return_type != other.return_type)
                    return return_type < other.return_type;
                if (params != other.params)
                    return params < other.params;
                if (variadic != other.variadic)
                    return variadic < other.variadic;
                if (pure != other.pure)
                    return pure < other.pure;
                if (thread_safe != other.thread_safe)
                    return thread_safe < other.thread_safe;
                if (foreign != other.foreign)
                    return foreign < other.foreign;
                if (nothrow != other.nothrow)
                    return nothrow < other.nothrow;
                return false;
            }
        };

        static std::map<Inner, FunctionTypeNode *> s_instances;

    public:
        static FunctionTypeNode *create(TypeNode *return_type, std::vector<TypeNode *> params, bool variadic = false, bool pure = false, bool thread_safe = false, bool foreign = false, bool nothrow = false)
        {
            Inner inner(return_type, params, variadic, pure, thread_safe, foreign, nothrow);
            if (s_instances.find(inner) == s_instances.end())
                s_instances[inner] = new FunctionTypeNode(return_type, params, variadic, pure, thread_safe, foreign, nothrow);
            return s_instances[inner];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return ptr_size; }
        virtual std::string to_source() const override
        {
            std::string source = "fn ";
            if (m_pure)
                source += "pure ";
            if (m_thread_safe)
                source += "tsafe ";
            if (m_foreign)
                source += "foreign ";
            if (m_nothrow)
                source += "nothrow ";
            source += "(";
            for (size_t i = 0; i < m_params.size(); i++)
            {
                source += m_params[i]->to_source();
                if (i < m_params.size() - 1)
                    source += ", ";
            }
            source += "): ";
            source += m_return_type->to_source();
            return source;
        }

        TypeNode *m_return_type;
        std::vector<TypeNode *> m_params;
        bool m_variadic = false;
        bool m_pure = false;
        bool m_thread_safe = false;
        bool m_foreign = false;
        bool m_nothrow = false;
    };

    class FunctionParamNode : public ParseNode
    {
    public:
        FunctionParamNode() : m_optional(false) { ntype = NodeType::FunctionParamNode; }
        FunctionParamNode(const std::string &name, TypeNode *type, std::shared_ptr<ExprNode> value, bool optional = false)
            : m_name(name), m_type(type), m_value(value), m_optional(optional) { ntype = NodeType::FunctionParamNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const { return visitor.visit(this); }

        std::string m_name;
        TypeNode *m_type;
        std::shared_ptr<ExprNode> m_value;
        bool m_optional = false;
    };

    class FunctionDeclNode : public DeclNode
    {
    public:
        FunctionDeclNode() : m_type(nullptr) { ntype = NodeType::FunctionDeclNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Function *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::string m_name;
        std::vector<std::shared_ptr<FunctionParamNode>> m_params;
        FunctionTypeNode *m_type;
    };

    class FunctionDefNode : public DefNode
    {
    public:
        FunctionDefNode() { ntype = NodeType::FunctionDefNode; }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }

        virtual llvm::Value *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }

        std::shared_ptr<FunctionDeclNode> m_decl;
        std::shared_ptr<BlockNode> m_body;
    };
}

#endif // __QUIXCC_PARSE_NODES_FUNCTION_H__