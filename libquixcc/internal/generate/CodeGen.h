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

#ifndef __QUIXCC_LLVM_CODEGEN_H__
#define __QUIXCC_LLVM_CODEGEN_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <parse/NodeType.h>
#include <llvm/IR/Value.h>
#include <llvm/LLVMWrapper.h>

namespace libquixcc
{
    class CodegenException : public std::exception
    {
        std::string m_msg;

    public:
        CodegenException(const std::string &msg) : m_msg(msg) {}

        virtual const char *what() const noexcept override { return m_msg.c_str(); }
    };

    class CodegenVisitor
    {
        LLVMContext *m_ctx;

    public:
        CodegenVisitor(LLVMContext *llvm) : m_ctx(llvm) {}

        llvm::Value *visit(const BlockNode *node) const;
        llvm::Value *visit(const NopStmtNode *node) const;
        llvm::Value *visit(const UnaryExprNode *node) const;
        llvm::Value *visit(const BinaryExprNode *node) const;
        llvm::Constant *visit(const ConstUnaryExprNode *node) const;
        llvm::Constant *visit(const ConstBinaryExprNode *node) const;
        llvm::Value *visit(const IdentifierNode *node) const;
        llvm::Type *visit(const U8TypeNode *node) const;
        llvm::Type *visit(const U16TypeNode *node) const;
        llvm::Type *visit(const U32TypeNode *node) const;
        llvm::Type *visit(const U64TypeNode *node) const;
        llvm::Type *visit(const I8TypeNode *node) const;
        llvm::Type *visit(const I16TypeNode *node) const;
        llvm::Type *visit(const I32TypeNode *node) const;
        llvm::Type *visit(const I64TypeNode *node) const;
        llvm::Type *visit(const F32TypeNode *node) const;
        llvm::Type *visit(const F64TypeNode *node) const;
        llvm::Type *visit(const BoolTypeNode *node) const;
        llvm::Type *visit(const CharTypeNode *node) const;
        llvm::Type *visit(const VoidTypeNode *node) const;
        llvm::Type *visit(const PointerTypeNode *node) const;
        llvm::Type *visit(const StringTypeNode *node) const;
        llvm::Type *visit(const EnumTypeNode *node) const;
        llvm::Type *visit(const StructTypeNode *node) const;
        llvm::Type *visit(const UnionTypeNode *node) const;
        llvm::Type *visit(const ArrayTypeNode *node) const;
        llvm::Type *visit(const FunctionTypeNode *node) const;
        llvm::Constant *visit(const IntegerLiteralNode *node) const;
        llvm::Constant *visit(const FloatLiteralNode *node) const;
        llvm::Constant *visit(const StringLiteralNode *node) const;
        llvm::Constant *visit(const CharLiteralNode *node) const;
        llvm::Constant *visit(const BoolLiteralNode *node) const;
        llvm::Constant *visit(const NullLiteralNode *node) const;
        llvm::Value *visit(const VarDeclNode *node) const;
        llvm::Value *visit(const LetDeclNode *node) const;
        llvm::Value *visit(const StructDeclNode *node) const;
        llvm::Value *visit(const UnionDeclNode *node) const;
        llvm::Value *visit(const EnumDeclNode *node) const;
        llvm::Function *visit(const FunctionDeclNode *node) const;
        llvm::Value *visit(const StructDefNode *node) const;
        llvm::Value *visit(const UnionDefNode *node) const;
        llvm::Value *visit(const UnionFieldNode *node) const;
        llvm::Value *visit(const EnumDefNode *node) const;
        llvm::Value *visit(const EnumFieldNode *node) const;
        llvm::Function *visit(const FunctionDefNode *node) const;
        llvm::Value *visit(const FunctionParamNode *node) const;
        llvm::Value *visit(const SubsystemNode *node) const;
        llvm::Value *visit(const ExportNode *node) const;
        llvm::Value *visit(const ReturnStmtNode *node) const;
        llvm::Value *visit(const RetifStmtNode *node) const;
        llvm::Value *visit(const RetzStmtNode *node) const;
        llvm::Value *visit(const RetvStmtNode *node) const;
        llvm::Value *visit(const IfStmtNode *node) const;
        llvm::Value *visit(const WhileStmtNode *node) const;
    };
};

#endif // __QUIXCC_LLVM_CODEGEN_H__