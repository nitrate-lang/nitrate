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
        CodegenVisitor(LLVMContext &llvm) : m_ctx(&llvm) {}

        llvm::Value *visit(const BlockNode *node) const;
        llvm::Value *visit(const StmtGroupNode *node) const;
        llvm::Value *visit(const ExprStmtNode *node) const;
        llvm::Value *visit(const UnaryExprNode *node) const;
        llvm::Value *visit(const BinaryExprNode *node) const;
        llvm::Value *visit(const CallExprNode *node) const;
        llvm::Value *visit(const ListExprNode *node) const;
        llvm::Value *visit(const MemberAccessNode *node) const;
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
        llvm::Type *visit(const VoidTypeNode *node) const;
        llvm::Type *visit(const PointerTypeNode *node) const;
        llvm::Type *visit(const StringTypeNode *node) const;
        llvm::Type *visit(const EnumTypeNode *node) const;
        llvm::Type *visit(const StructTypeNode *node) const;
        llvm::Type *visit(const RegionTypeNode *node) const;
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
        llvm::Function *visit(const FunctionDeclNode *node) const;
        llvm::Value *visit(const StructDefNode *node) const;
        llvm::Value *visit(const RegionDefNode *node) const;
        llvm::Value *visit(const UnionDefNode *node) const;
        llvm::Function *visit(const FunctionDefNode *node) const;
        llvm::Value *visit(const FunctionParamNode *node) const;
        llvm::Value *visit(const SubsystemNode *node) const;
        llvm::Value *visit(const ExportNode *node) const;
        llvm::Value *visit(const InlineAsmNode *node) const;
        llvm::Value *visit(const ReturnStmtNode *node) const;
        llvm::Value *visit(const IfStmtNode *node) const;
        llvm::Value *visit(const WhileStmtNode *node) const;
    };
};

#endif // __QUIXCC_LLVM_CODEGEN_H__