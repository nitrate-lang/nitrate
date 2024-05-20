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
#include <optional>
#include <memory>
#include <string>
#include <iostream>

#include <IR/delta/DeltaIR.h>
#include <IR/delta/Type.h>
#include <IR/delta/Variable.h>
#include <IR/delta/Memory.h>
#include <IR/delta/Cast.h>
#include <IR/delta/Control.h>
#include <IR/delta/Segment.h>
#include <IR/delta/Math.h>
#include <IR/delta/Ident.h>

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
            std::stack<std::map<std::string, llvm::AllocaInst *>> locals;
            std::stack<std::map<std::string, llvm::Argument *>> params;
            std::stack<std::map<std::string, llvm::BasicBlock *>> labels;
            std::map<std::string, llvm::GlobalVariable *> globals;
            std::map<std::string, llvm::Function *> functions;
            std::map<std::string, llvm::Type *> types;
            bool terminate_early;
            bool incond;

            bool m_pub;
            bool m_deref;

            CodegenState()
            {
                name = "";
                m_pub = false;
                m_deref = true;
                incond = false;
            }
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
        llvm::Value *gen(const ir::delta::List *node);
        llvm::Value *gen(const ir::delta::Ident *node);
        llvm::Value *gen(const ir::delta::Assign *node);
        llvm::Value *gen(const ir::delta::AddressOf *node);
        llvm::Value *gen(const ir::delta::Member *node);
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
        llvm::Value *gen(const ir::delta::Block *node);
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

        llvm::Type *gent(const ir::delta::Type *node);
        llvm::Value *gen(const ir::delta::Value *node);

        LLVM14Codegen(LLVMContext &ctx) : m_ctx(&ctx) {}

    public:
        static bool codegen(const std::unique_ptr<ir::delta::IRDelta> &ir, LLVMContext &ctx);
    };

    class C11Codegen
    {
    public:
        static bool codegen(const std::unique_ptr<ir::delta::IRDelta> &ir, std::ostream &os);
    };
}

#endif // __QUIXCC_LLVM_CODEGEN_H__