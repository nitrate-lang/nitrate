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
#include <error/Logger.h>
#include <LibMacro.h>

using namespace libquixcc;

bool libquixcc::parse(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::BlockNode> &group, bool expect_braces)
{
    Token tok;

    if (expect_braces)
    {
        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::OpenBrace))
        {
            LOG(ERROR) << feedback[PARSER_EXPECTED_LEFT_BRACE] << tok.serialize() << tok << std::endl;
            return false;
        }
    }

    group = std::make_shared<BlockNode>();

    while ((tok = scanner->next()).type() != TokenType::Eof)
    {
        if (expect_braces)
        {
            if (tok.is<Punctor>(Punctor::CloseBrace))
                break;
        }

        if (tok.type() != TokenType::Keyword)
        {
            LOG(ERROR) << feedback[PARSER_EXPECTED_KEYWORD] << tok.serialize() << tok << std::endl;
            return false;
        }

        std::shared_ptr<StmtNode> node;

        switch (std::get<Keyword>(tok.val()))
        {
        case Keyword::Var:
        {
            std::vector<std::shared_ptr<StmtNode>> decls;
            if (!parse_var(job, scanner, decls))
                return false;
            for (auto &decl : decls)
                group->m_stmts.push_back(decl);
            break;
        }
        case Keyword::Let:
        {
            std::vector<std::shared_ptr<StmtNode>> decls;
            if (!parse_let(job, scanner, decls))
                return false;
            for (auto &decl : decls)
                group->m_stmts.push_back(decl);
            break;
        }
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
        case Keyword::Type:
            if (!parse_typedef(job, scanner, node))
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
        case Keyword::Return:
            if (!parse_return(job, scanner, node))
                return false;
            break;
        case Keyword::Retif:
            if (!parse_retif(job, scanner, node))
                return false;
            break;
        case Keyword::Retz:
            if (!parse_retz(job, scanner, node))
                return false;
            break;
        case Keyword::Retv:
            if (!parse_retv(job, scanner, node))
                return false;
            break;
        case Keyword::If:
            if (!parse_if(job, scanner, node))
                return false;
            break;
        case Keyword::While:
            if (!parse_while(job, scanner, node))
                return false;
            break;
        case Keyword::For:
            if (!parse_for(job, scanner, node))
                return false;
            break;
        default:
            break;
        }

        if (node)
            group->m_stmts.push_back(node);
    }

    return true;
}