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

#ifndef __QUIXCC_LLVM_CTX_H__
#define __QUIXCC_LLVM_CTX_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <memory>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <parse/NodeType.h>
#include <map>
#include <stack>

namespace libquixcc
{
    enum class ExportLangType
    {
        Default,
        C,
        CXX,
        DLang,
    };

    class LLVMContext
    {
        LLVMContext(const LLVMContext &) = delete;
        LLVMContext &operator=(const LLVMContext &) = delete;

    public:
        std::unique_ptr<llvm::LLVMContext> m_ctx;
        std::unique_ptr<llvm::Module> m_module;
        std::unique_ptr<llvm::IRBuilder<>> m_builder;
        std::map<std::pair<NodeType, std::string>, std::shared_ptr<libquixcc::ParseNode>> m_named_construsts;
        std::map<std::string, std::shared_ptr<libquixcc::ParseNode>> m_named_types;
        std::map<std::string, std::pair<llvm::AllocaInst *, llvm::Type *>> m_named_stack_vars;
        std::map<std::string, std::pair<llvm::Argument *, llvm::Type *>> m_named_params;
        std::map<std::string, llvm::GlobalVariable *> m_named_global_vars;
        std::map<std::string, llvm::BasicBlock *> m_named_blocks;
        std::vector<std::pair<llvm::Function *, const FunctionDeclNode *>> m_named_functions;
        std::map<std::string, llvm::StructType *> m_named_structs;
        std::string prefix;
        bool m_pub = false;
        size_t m_skipbr = 0;
        ExportLangType m_lang = ExportLangType::Default;

        LLVMContext() = default;

        void setup(const std::string &filename)
        {
            m_ctx = std::make_unique<llvm::LLVMContext>();
            m_module = std::make_unique<llvm::Module>(filename, *m_ctx);
            m_builder = std::make_unique<llvm::IRBuilder<>>(*m_ctx);
        }
    };

};

#endif // __QUIXCC_LLVM_CTX_H__