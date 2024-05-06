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

#include <IR/delta/nodes/Type.h>
#include <IR/delta/nodes/Variable.h>
#include <IR/delta/nodes/Memory.h>
#include <IR/delta/nodes/Cast.h>
#include <IR/delta/nodes/Control.h>
#include <IR/delta/nodes/Segment.h>
#include <IR/delta/nodes/Math.h>

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
        struct CodegenState
        {
            std::string name;
        };

        LLVMContext *m_ctx;
        CodegenState m_state;

        llvm::Type *gen(const ir::delta::I1 *node);
        llvm::Type *gen(const ir::delta::I8 *node);
        llvm::Type *gen(const ir::delta::I16 *node);
        llvm::Type *gen(const ir::delta::I32 *node);
        llvm::Type *gen(const ir::delta::I64 *node);
        llvm::Type *gen(const ir::delta::I128 *node);
        llvm::Type *gen(const ir::delta::U8 *node);
        llvm::Type *gen(const ir::delta::U16 *node);
        llvm::Type *gen(const ir::delta::U32 *node);
        llvm::Type *gen(const ir::delta::U64 *node);
        llvm::Type *gen(const ir::delta::U128 *node);
        llvm::Type *gen(const ir::delta::F32 *node);
        llvm::Type *gen(const ir::delta::F64 *node);
        llvm::Type *gen(const ir::delta::Void *node);
        llvm::PointerType *gen(const ir::delta::Ptr *node);
        llvm::StructType *gen(const ir::delta::Packet *node);
        llvm::ArrayType *gen(const ir::delta::Array *node);
        llvm::FunctionType *gen(const ir::delta::FType *node);
        llvm::Value *gen(const ir::delta::Local *node);
        llvm::Value *gen(const ir::delta::Global *node);
        llvm::Constant *gen(const ir::delta::Number *node);
        llvm::Constant *gen(const ir::delta::String *node);
        llvm::Value *gen(const ir::delta::Assign *node);
        llvm::Value *gen(const ir::delta::Load *node);
        llvm::Value *gen(const ir::delta::Index *node);
        llvm::Value *gen(const ir::delta::SCast *node);
        llvm::Value *gen(const ir::delta::UCast *node);
        llvm::Value *gen(const ir::delta::PtrICast *node);
        llvm::Value *gen(const ir::delta::IPtrCast *node);
        llvm::Value *gen(const ir::delta::Bitcast *node);
        llvm::Value *gen(const ir::delta::IfElse *node);
        llvm::Value *gen(const ir::delta::While *node);
        llvm::Value *gen(const ir::delta::Jmp *node);
        llvm::Value *gen(const ir::delta::Label *node);
        llvm::Value *gen(const ir::delta::Ret *node);
        llvm::Value *gen(const ir::delta::Call *node);
        llvm::Value *gen(const ir::delta::PtrCall *node);
        llvm::Value *gen(const ir::delta::Halt *node);
        llvm::Function *gen(const ir::delta::Segment *node);
        llvm::Value *gen(const ir::delta::Add *node);
        llvm::Value *gen(const ir::delta::Sub *node);
        llvm::Value *gen(const ir::delta::Mul *node);
        llvm::Value *gen(const ir::delta::Div *node);
        llvm::Value *gen(const ir::delta::Mod *node);
        llvm::Value *gen(const ir::delta::BitAnd *node);
        llvm::Value *gen(const ir::delta::BitOr *node);
        llvm::Value *gen(const ir::delta::BitXor *node);
        llvm::Value *gen(const ir::delta::BitNot *node);
        llvm::Value *gen(const ir::delta::Shl *node);
        llvm::Value *gen(const ir::delta::Shr *node);
        llvm::Value *gen(const ir::delta::Rotl *node);
        llvm::Value *gen(const ir::delta::Rotr *node);
        llvm::Value *gen(const ir::delta::Eq *node);
        llvm::Value *gen(const ir::delta::Ne *node);
        llvm::Value *gen(const ir::delta::Lt *node);
        llvm::Value *gen(const ir::delta::Gt *node);
        llvm::Value *gen(const ir::delta::Le *node);
        llvm::Value *gen(const ir::delta::Ge *node);
        llvm::Value *gen(const ir::delta::And *node);
        llvm::Value *gen(const ir::delta::Or *node);
        llvm::Value *gen(const ir::delta::Not *node);
        llvm::Value *gen(const ir::delta::Xor *node);
        llvm::Value *gen(const ir::delta::RootNode *node);

        llvm::Type *gent(const ir::Value<libquixcc::ir::Delta> *node);
        llvm::Value *gen(const ir::Value<libquixcc::ir::Delta> *node);

        LLVM14Codegen(LLVMContext &ctx) : m_ctx(&ctx) {}

    public:
        static bool codegen(const std::unique_ptr<ir::delta::IRDelta> &ir, LLVMContext &ctx);
    };

    class C11Codegen
    {
        // std::string gen(const ir::delta::I1 *node);
        // std::string gen(const ir::delta::I8 *node);
        // std::string gen(const ir::delta::I16 *node);
        // std::string gen(const ir::delta::I32 *node);
        // std::string gen(const ir::delta::I64 *node);
        // std::string gen(const ir::delta::I128 *node);
        // std::string gen(const ir::delta::U8 *node);
        // std::string gen(const ir::delta::U16 *node);
        // std::string gen(const ir::delta::U32 *node);
        // std::string gen(const ir::delta::U64 *node);
        // std::string gen(const ir::delta::U128 *node);
        // std::string gen(const ir::delta::F32 *node);
        // std::string gen(const ir::delta::F64 *node);
        // std::string gen(const ir::delta::Void *node);
        // std::string gen(const ir::delta::Ptr *node);
        // std::string gen(const ir::delta::Packet *node);
        // std::string gen(const ir::delta::Array *node);
        // std::string gen(const ir::delta::FType *node);
        // std::string gen(const ir::delta::Local *node);
        // std::string gen(const ir::delta::Global *node);
        // std::string gen(const ir::delta::Number *node);
        // std::string gen(const ir::delta::String *node);
        // std::string gen(const ir::delta::Assign *node);
        // std::string gen(const ir::delta::Load *node);
        // std::string gen(const ir::delta::Index *node);
        // std::string gen(const ir::delta::SCast *node);
        // std::string gen(const ir::delta::UCast *node);
        // std::string gen(const ir::delta::PtrICast *node);
        // std::string gen(const ir::delta::IPtrCast *node);
        // std::string gen(const ir::delta::Bitcast *node);
        // std::string gen(const ir::delta::IfElse *node);
        // std::string gen(const ir::delta::While *node);
        // std::string gen(const ir::delta::Jmp *node);
        // std::string gen(const ir::delta::Label *node);
        // std::string gen(const ir::delta::Ret *node);
        // std::string gen(const ir::delta::Call *node);
        // std::string gen(const ir::delta::PtrCall *node);
        // std::string gen(const ir::delta::Halt *node);
        // std::string gen(const ir::delta::Segment *node);
        // std::string gen(const ir::delta::Add *node);
        // std::string gen(const ir::delta::Sub *node);
        // std::string gen(const ir::delta::Mul *node);
        // std::string gen(const ir::delta::Div *node);
        // std::string gen(const ir::delta::Mod *node);
        // std::string gen(const ir::delta::BitAnd *node);
        // std::string gen(const ir::delta::BitOr *node);
        // std::string gen(const ir::delta::BitXor *node);
        // std::string gen(const ir::delta::BitNot *node);
        // std::string gen(const ir::delta::Shl *node);
        // std::string gen(const ir::delta::Shr *node);
        // std::string gen(const ir::delta::Rotl *node);
        // std::string gen(const ir::delta::Rotr *node);
        // std::string gen(const ir::delta::Eq *node);
        // std::string gen(const ir::delta::Ne *node);
        // std::string gen(const ir::delta::Lt *node);
        // std::string gen(const ir::delta::Gt *node);
        // std::string gen(const ir::delta::Le *node);
        // std::string gen(const ir::delta::Ge *node);
        // std::string gen(const ir::delta::And *node);
        // std::string gen(const ir::delta::Or *node);
        // std::string gen(const ir::delta::Not *node);
        // std::string gen(const ir::delta::Xor *node);

        // llvm::Value *gen(const ir::delta::RootNode *node);

    public:
        static bool codegen(const std::unique_ptr<ir::delta::IRDelta> &ir, std::ostream &os);
    };
}

#endif // __QUIXCC_LLVM_CODEGEN_H__