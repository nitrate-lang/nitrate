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
#include <error/Message.h>

using namespace libquixcc;

bool libquixcc::parse_pub(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::shared_ptr<libquixcc::StmtNode> &node)
{
    Token tok = scanner->next();

    ExportLangType langType = ExportLangType::Default;

    if (tok.type() == TokenType::StringLiteral)
    {
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
            PARMSG(tok, libquixcc::Err::ERROR, feedback[PARSER_UNKNOWN_LANGUAGE], lang.c_str());
            return false;
        }

        tok = scanner->next();
    }

    if (tok.type() != TokenType::Keyword)
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[PARSER_EXPECTED_KEYWORD], tok.serialize().c_str());
        return false;
    }

    std::shared_ptr<libquixcc::StmtNode> stmt;

    switch (std::get<Keyword>(tok.val()))
    {
    case Keyword::Var:
        if (!parse_var(job, scanner, stmt))
            return false;
        break;
    case Keyword::Let:
        if (!parse_let(job, scanner, stmt))
            return false;
        break;
    case Keyword::Const:
        if (!parse_const(job, scanner, stmt))
            return false;
        break;
    case Keyword::Enum:
        if (!parse_enum(job, scanner, stmt))
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
    case Keyword::Pub:
        if (!parse_pub(job, scanner, stmt))
            return false;
        break;
    default:
        PARMSG(tok, libquixcc::Err::ERROR, feedback[PARSER_EXPECTED_KEYWORD], tok.serialize().c_str());
        return false;
    }

    node = std::make_shared<libquixcc::ExportNode>(stmt, langType);

    return true;
}