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
#include <LibMacro.h>
#include <error/Logger.h>

using namespace libquixcc;

bool libquixcc::parse_pub(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->peek();

    ExportLangType langType = ExportLangType::Default;

    if (tok.type() == TokenType::StringLiteral)
    {
        scanner->next();

        std::string lang = std::get<std::string>(tok.val());

        std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

        if (lang == "c")
            langType = ExportLangType::C;
        else if (lang == "c++" || lang == "cxx")
            langType = ExportLangType::CXX;
        else if (lang == "d" || lang == "dlang")
            langType = ExportLangType::DLang;
        else
        {
            LOG(ERROR) << feedback[PARSER_UNKNOWN_LANGUAGE] << lang << tok << std::endl;
            return false;
        }

        tok = scanner->peek();
    }

    if (tok.is<Punctor>(Punctor::OpenBrace))
    {
        std::shared_ptr<libquixcc::BlockNode> block;
        if (!parse(job, scanner, block, true))
            return false;

        node = std::make_shared<libquixcc::ExportNode>(block->m_stmts, langType);
        return true;
    }

    scanner->next();
    if (tok.type() != TokenType::Keyword)
    {
        LOG(ERROR) << feedback[PARSER_EXPECTED_KEYWORD] << tok.serialize() << tok << std::endl;
        return false;
    }

    std::vector<std::shared_ptr<libquixcc::StmtNode>> stmts;
    std::shared_ptr<libquixcc::StmtNode> stmt;

    switch (std::get<Keyword>(tok.val()))
    {
    case Keyword::Var:
        if (!parse_var(job, scanner, stmts))
            return false;
        break;
    case Keyword::Let:
        if (!parse_let(job, scanner, stmts))
            return false;
        break;
    case Keyword::Subsystem:
        if (!parse_subsystem(job, scanner, stmt))
            return false;
        break;
    case Keyword::Fn:
        if (!parse_function(job, scanner, stmt))
            return false;
        break;
    default:
        LOG(ERROR) << feedback[PARSER_EXPECTED_KEYWORD] << tok.serialize() << tok << std::endl;
        return false;
    }
    if (stmt)
        stmts.push_back(stmt);

    node = std::make_shared<libquixcc::ExportNode>(stmts, langType);

    return true;
}