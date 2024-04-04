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

#ifndef __QUIXCC_PARSE_NODES_INTEGER_H__
#define __QUIXCC_PARSE_NODES_INTEGER_H__

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
    class U8TypeNode : public TypeNode
    {
        U8TypeNode() { ntype = NodeType::U8TypeNode; }
        static U8TypeNode *m_instance;

    public:
        static U8TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new U8TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 1; }
        virtual std::string to_source() const override { return "u8"; }
    };

    class U16TypeNode : public TypeNode
    {
        U16TypeNode() { ntype = NodeType::U16TypeNode; }
        static U16TypeNode *m_instance;

    public:
        static U16TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new U16TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 2; }
        virtual std::string to_source() const override { return "u16"; }
    };

    class U32TypeNode : public TypeNode
    {
        U32TypeNode() { ntype = NodeType::U32TypeNode; }
        static U32TypeNode *m_instance;

    public:
        static U32TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new U32TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 4; }
        virtual std::string to_source() const override { return "u32"; }
    };

    class U64TypeNode : public TypeNode
    {
        U64TypeNode() { ntype = NodeType::U64TypeNode; }
        static U64TypeNode *m_instance;

    public:
        static U64TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new U64TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 8; }
        virtual std::string to_source() const override { return "u64"; }
    };

    class I8TypeNode : public TypeNode
    {
        I8TypeNode() { ntype = NodeType::I8TypeNode; }
        static I8TypeNode *m_instance;

    public:
        static I8TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new I8TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 1; }
        virtual std::string to_source() const override { return "i8"; }
    };

    class I16TypeNode : public TypeNode
    {
        I16TypeNode() { ntype = NodeType::I16TypeNode; }
        static I16TypeNode *m_instance;

    public:
        static I16TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new I16TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 2; }
        virtual std::string to_source() const override { return "i16"; }
    };

    class I32TypeNode : public TypeNode
    {
        I32TypeNode() { ntype = NodeType::I32TypeNode; }
        static I32TypeNode *m_instance;

    public:
        static I32TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new I32TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 4; }
        virtual std::string to_source() const override { return "i32"; }
    };

    class I64TypeNode : public TypeNode
    {
        I64TypeNode() { ntype = NodeType::I64TypeNode; }
        static I64TypeNode *m_instance;

    public:
        static I64TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new I64TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 8; }
        virtual std::string to_source() const override { return "i64"; }
    };

    class F32TypeNode : public TypeNode
    {
        F32TypeNode() { ntype = NodeType::F32TypeNode; }
        static F32TypeNode *m_instance;

    public:
        static F32TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new F32TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 4; }
        virtual std::string to_source() const override { return "f32"; }
    };

    class F64TypeNode : public TypeNode
    {
        F64TypeNode() { ntype = NodeType::F64TypeNode; }
        static F64TypeNode *m_instance;

    public:
        static F64TypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new F64TypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 8; }
        virtual std::string to_source() const override { return "f64"; }
    };

    class BoolTypeNode : public TypeNode
    {
        BoolTypeNode() { ntype = NodeType::BoolTypeNode; }
        static BoolTypeNode *m_instance;

    public:
        static BoolTypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new BoolTypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 1; }
        virtual std::string to_source() const override { return "bool"; }
    };

    class VoidTypeNode : public TypeNode
    {
        VoidTypeNode() { ntype = NodeType::VoidTypeNode; }
        static VoidTypeNode *m_instance;

    public:
        static VoidTypeNode *create()
        {
            if (m_instance == nullptr)
                m_instance = new VoidTypeNode();
            return m_instance;
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return 0; }
        virtual std::string to_source() const override { return "void"; }
    };

    class PointerTypeNode : public TypeNode
    {
        PointerTypeNode(TypeNode *type) : m_type(type) { ntype = NodeType::PointerTypeNode; }
        static std::map<TypeNode *, PointerTypeNode *> m_instances;

    public:
        static PointerTypeNode *create(TypeNode *type)
        {
            if (!m_instances.contains(type))
                m_instances[type] = new PointerTypeNode(type);
            return m_instances[type];
        }

        virtual size_t dfs_preorder(ParseNodePreorderVisitor visitor) override { return visitor.visit(this); }
        virtual std::string to_json(ParseNodeJsonSerializerVisitor visitor) const override { return visitor.visit(this); }
        virtual llvm::Type *codegen(const CodegenVisitor &visitor) const override { return visitor.visit(this); }
        virtual bool is_composite() const override { return false; }
        virtual size_t size(size_t ptr_size) const override { return ptr_size; }
        virtual std::string to_source() const override { return "*" + m_type->to_source(); }

        TypeNode *m_type;
    };
}

#endif // __QUIXCC_PARSE_NODES_INTEGER_H__