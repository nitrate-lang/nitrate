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

#define QUIXCC_INTERNAL

#include <parse/Parser.h>
#include <error/Message.h>
#include <LibMacro.h>

using namespace libquixcc;

bool libquixcc::parse(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::BlockNode> &group, bool expect_braces)
{
    Token tok;

    if (expect_braces)
    {
        tok = scanner->next();
        if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::OpenBrace)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[PARSER_EXPECTED_LEFT_BRACE], tok.serialize().c_str());
            return false;
        }
    }

    group = std::make_shared<BlockNode>();

    while ((tok = scanner->next()).type() != TokenType::Eof)
    {

        if (expect_braces)
        {
            if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::CloseBrace)
                break;
        }

        if (tok.type() != TokenType::Keyword)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[PARSER_EXPECTED_KEYWORD], tok.serialize().c_str());
            return false;
        }

        std::shared_ptr<StmtNode> node;

        switch (std::get<Keyword>(tok.val()))
        {
        case Keyword::Var:
            if (!parse_var(job, scanner, node))
                return false;
            break;
        case Keyword::Let:
            if (!parse_let(job, scanner, node))
                return false;
            break;
        case Keyword::Const:
            if (!parse_const(job, scanner, node))
                return false;
            break;
        case Keyword::Enum:
            if (!parse_enum(job, scanner, node))
                return false;
            break;
        case Keyword::Struct:
            if (!parse_struct(job, scanner, node))
                return false;
            break;
        case Keyword::Union:
            if (!parse_union(job, scanner, node))
                return false;
            break;
        case Keyword::Subsystem:
            if (!parse_subsystem(job, scanner, node))
                return false;
            break;
        case Keyword::Fn:
            if (!parse_function(job, scanner, node))
                return false;
            break;
        case Keyword::Pub:
            if (!parse_pub(job, scanner, node))
                return false;
            break;
        default:
            break;
        }

        if (node)
            group->m_stmts.push_back(node);
    }

    // llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_ctx), false);
    // llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", &module);

    // llvm::BasicBlock *entry = llvm::BasicBlock::Create(llvm_ctx, "entry", mainFunc);
    // builder.SetInsertPoint(entry);
    // builder.CreateRetVoid();

    return true;
}