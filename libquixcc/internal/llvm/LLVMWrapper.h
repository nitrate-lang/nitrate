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
        std::map<std::string, llvm::GlobalVariable *> m_named_global_vars;
        std::map<std::string, llvm::BasicBlock *> m_named_blocks;
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