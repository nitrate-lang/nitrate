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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
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

#include <llvm/IR/Value.h>
#include <llvm/LLVMWrapper.h>
#include <IR/delta/DeltaIR.h>
#include <optional>
#include <memory>
#include <string>
#include <iostream>

namespace libquixcc
{
    class CodegenException : public std::exception
    {
        std::string m_msg;

    public:
        CodegenException(const std::string &msg) : m_msg(msg) {}

        virtual const char *what() const noexcept override { return m_msg.c_str(); }
    };

    class LLVM14Codegen
    {
    public:
        static bool codegen(const std::unique_ptr<libquixcc::ir::delta::IRDelta> &ir, libquixcc::LLVMContext &ctx);
    };

    class C11Codegen
    {
    public:
        static bool codegen(const std::unique_ptr<libquixcc::ir::delta::IRDelta> &ir, std::ostream &os);
    };

    // class CodegenVisitor
    // {
    //     struct CodegenState
    //     {
    //         bool implicit_load = true;
    //         bool inside_function = false;
    //     };

    //     LLVMContext *m_ctx;
    //     CodegenState m_state;

    //     std::optional<std::pair<llvm::Value *, llvm::Type *>> create_struct_gep(llvm::Value *struct_ptr, const std::string &field_name);

    //     enum class AORLocality
    //     {
    //         Global,
    //         Local,
    //         Parameter,
    //         Function
    //     };
    //     std::optional<std::tuple<llvm::Value *, llvm::Type *, AORLocality>> address_of_identifier(const std::string &name);

    //     llvm::Value *visit(const BlockNode *node);
    //     llvm::Value *visit(const StmtGroupNode *node);
    //     llvm::Value *visit(const ExprStmtNode *node);
    //     llvm::Value *visit(const BitCastExprNode *node);
    //     llvm::Value *visit(const SignedUpcastExprNode *node);
    //     llvm::Value *visit(const UnsignedUpcastExprNode *node);
    //     llvm::Value *visit(const DowncastExprNode *node);
    //     llvm::Value *visit(const PtrToIntCastExprNode *node);
    //     llvm::Value *visit(const IntToPtrCastExprNode *node);
    //     llvm::Value *visit(const UnaryExprNode *node);
    //     llvm::Value *visit(const BinaryExprNode *node);
    //     llvm::Value *visit(const CallExprNode *node);
    //     llvm::Value *visit(const ListExprNode *node);
    //     llvm::Value *visit(const MemberAccessNode *node);
    //     llvm::Constant *visit(const ConstUnaryExprNode *node);
    //     llvm::Constant *visit(const ConstBinaryExprNode *node);
    //     llvm::Value *visit(const IdentifierNode *node);
    //     llvm::Type *visit(const MutTypeNode *node);
    //     llvm::Type *visit(const U8TypeNode *node);
    //     llvm::Type *visit(const U16TypeNode *node);
    //     llvm::Type *visit(const U32TypeNode *node);
    //     llvm::Type *visit(const U64TypeNode *node);
    //     llvm::Type *visit(const I8TypeNode *node);
    //     llvm::Type *visit(const I16TypeNode *node);
    //     llvm::Type *visit(const I32TypeNode *node);
    //     llvm::Type *visit(const I64TypeNode *node);
    //     llvm::Type *visit(const F32TypeNode *node);
    //     llvm::Type *visit(const F64TypeNode *node);
    //     llvm::Type *visit(const BoolTypeNode *node);
    //     llvm::Type *visit(const VoidTypeNode *node);
    //     llvm::Type *visit(const PointerTypeNode *node);
    //     llvm::Type *visit(const StringTypeNode *node);
    //     llvm::Type *visit(const EnumTypeNode *node);
    //     llvm::Type *visit(const StructTypeNode *node);
    //     llvm::Type *visit(const RegionTypeNode *node);
    //     llvm::Type *visit(const UnionTypeNode *node);
    //     llvm::Type *visit(const ArrayTypeNode *node);
    //     llvm::Type *visit(const FunctionTypeNode *node);
    //     llvm::Constant *visit(const IntegerNode *node);
    //     llvm::Constant *visit(const FloatLiteralNode *node);
    //     llvm::Constant *visit(const StringNode *node);
    //     llvm::Constant *visit(const CharNode *node);
    //     llvm::Constant *visit(const BoolLiteralNode *node);
    //     llvm::Constant *visit(const NullLiteralNode *node);
    //     llvm::Value *visit(const LetDeclNode *node);
    //     llvm::Function *visit(const FunctionDeclNode *node);
    //     llvm::Value *visit(const StructDefNode *node);
    //     llvm::Value *visit(const RegionDefNode *node);
    //     llvm::Value *visit(const UnionDefNode *node);
    //     llvm::Function *visit(const FunctionDefNode *node);
    //     llvm::Value *visit(const FunctionParamNode *node);
    //     llvm::Value *visit(const ExportNode *node);
    //     llvm::Value *visit(const InlineAsmNode *node);
    //     llvm::Value *visit(const ReturnStmtNode *node);
    //     llvm::Value *visit(const IfStmtNode *node);
    //     llvm::Value *visit(const WhileStmtNode *node);

    // public:
    //     CodegenVisitor(LLVMContext &llvm) : m_ctx(&llvm) {}

    //     llvm::Value *visit(const ExprNode *node);
    //     llvm::Value *visit(const StmtNode *node);
    //     llvm::Type *visit(const TypeNode *node);
    // };

    // class C11CodegenVisitor
    // {
    //     struct CodegenState
    //     {
    //         std::map<std::string, std::string> variables;
    //         std::map<std::string, bool> is_pointers;
    //         std::string subsystem;
    //         ExportLangType export_lang = ExportLangType::Default;
    //         size_t indent = 0;
    //         bool function_def = false;
    //         bool pub = false;
    //         bool mut = false;
    //     };

    //     CodegenState m_state;

    //     std::string visit(const BlockNode *node);
    //     std::string visit(const StmtGroupNode *node);
    //     std::string visit(const ExprStmtNode *node);
    //     std::string visit(const BitCastExprNode *node);
    //     std::string visit(const SignedUpcastExprNode *node);
    //     std::string visit(const UnsignedUpcastExprNode *node);
    //     std::string visit(const DowncastExprNode *node);
    //     std::string visit(const PtrToIntCastExprNode *node);
    //     std::string visit(const IntToPtrCastExprNode *node);
    //     std::string visit(const UnaryExprNode *node);
    //     std::string visit(const BinaryExprNode *node);
    //     std::string visit(const CallExprNode *node);
    //     std::string visit(const ListExprNode *node);
    //     std::string visit(const MemberAccessNode *node);
    //     std::string visit(const ConstUnaryExprNode *node);
    //     std::string visit(const ConstBinaryExprNode *node);
    //     std::string visit(const IdentifierNode *node);
    //     std::string visit(const MutTypeNode *node);
    //     std::string visit(const U8TypeNode *node);
    //     std::string visit(const U16TypeNode *node);
    //     std::string visit(const U32TypeNode *node);
    //     std::string visit(const U64TypeNode *node);
    //     std::string visit(const I8TypeNode *node);
    //     std::string visit(const I16TypeNode *node);
    //     std::string visit(const I32TypeNode *node);
    //     std::string visit(const I64TypeNode *node);
    //     std::string visit(const F32TypeNode *node);
    //     std::string visit(const F64TypeNode *node);
    //     std::string visit(const BoolTypeNode *node);
    //     std::string visit(const VoidTypeNode *node);
    //     std::string visit(const PointerTypeNode *node);
    //     std::string visit(const StringTypeNode *node);
    //     std::string visit(const EnumTypeNode *node);
    //     std::string visit(const StructTypeNode *node);
    //     std::string visit(const RegionTypeNode *node);
    //     std::string visit(const UnionTypeNode *node);
    //     std::string visit(const ArrayTypeNode *node);
    //     std::string visit(const FunctionTypeNode *node);
    //     std::string visit(const IntegerNode *node);
    //     std::string visit(const FloatLiteralNode *node);
    //     std::string visit(const StringNode *node);
    //     std::string visit(const CharNode *node);
    //     std::string visit(const BoolLiteralNode *node);
    //     std::string visit(const NullLiteralNode *node);
    //     std::string visit(const LetDeclNode *node);
    //     std::string visit(const FunctionDeclNode *node);
    //     std::string visit(const StructDefNode *node);
    //     std::string visit(const RegionDefNode *node);
    //     std::string visit(const UnionDefNode *node);
    //     std::string visit(const FunctionDefNode *node);
    //     std::string visit(const FunctionParamNode *node);
    //     std::string visit(const ExportNode *node);
    //     std::string visit(const InlineAsmNode *node);
    //     std::string visit(const ReturnStmtNode *node);
    //     std::string visit(const IfStmtNode *node);
    //     std::string visit(const WhileStmtNode *node);

    // public:
    //     C11CodegenVisitor() = default;

    //     std::string visit(const ExprNode *node);
    //     std::string visit(const StmtNode *node);
    //     std::string visit(const TypeNode *node);
    // };
};

#endif // __QUIXCC_LLVM_CODEGEN_H__