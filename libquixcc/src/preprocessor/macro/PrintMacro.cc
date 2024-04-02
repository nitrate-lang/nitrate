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

#include <preprocessor/macro/PrintMacro.h>
#include <error/Exception.h>
#include <lexer/Lex.h>
#include <error/Message.h>
#include <iostream>

bool libquixcc::macro::ParsePrint(quixcc_job_t *job, const Token &tok, const std::string &directive, const std::string &parameter, std::vector<libquixcc::Token> &exp)
{
    // print [mode] "<message>"
    (void)directive;
    (void)exp;

    std::vector<libquixcc::Token> tokens;
    if (!libquixcc::StringLexer::QuickLex(parameter, tokens))
    {
        libquixcc::Message(*job, libquixcc::E::ERROR, "Failed to lex print message");
        return false;
    }

    if (tokens.empty())
    {
        libquixcc::Message(*job, libquixcc::E::ERROR, "Empty print message");
        return false;
    }

    E level = E::INFO;

    Token t = tokens[0];
    if (t.type() == TokenType::Identifier && tokens.size() == 3)
    {
        std::string id = std::get<std::string>(t.val());
        std::transform(id.begin(), id.end(), id.begin(), ::tolower);

        if (id == "debug")
            level = E::DEBUG;
        else if (id == "ok" || id == "good" || id == "success")
            level = E::SUCCESS;
        else if (id == "info")
            level = E::INFO;
        else if (id == "warning" || id == "warn")
            level = E::WARN;
        else if (id == "error" || id == "err")
            level = E::ERROR;
        else if (id == "raw")
            level = E::RAW;
        else
        {
            libquixcc::Message(*job, libquixcc::E::ERROR, "Invalid print level");
            return false;
        }

        if (tokens[1].type() != TokenType::Punctor || std::get<Punctor>(tokens[1].val()) != Punctor::Comma)
        {
            libquixcc::Message(*job, libquixcc::E::ERROR, "Expected comma after print level");
            return false;
        }

        switch (level)
        {
        case E::DEBUG:
            PreprocessorMessage(*job, tok, libquixcc::E::DEBUG, false, std::get<std::string>(tokens[2].val()));
            break;
        case E::SUCCESS:
            PreprocessorMessage(*job, tok, libquixcc::E::SUCCESS, false, std::get<std::string>(tokens[2].val()));
            break;
        case E::INFO:
            PreprocessorMessage(*job, tok, libquixcc::E::INFO, false, std::get<std::string>(tokens[2].val()));
            break;
        case E::WARN:
            PreprocessorMessage(*job, tok, libquixcc::E::WARN, false, std::get<std::string>(tokens[2].val()));
            break;
        case E::ERROR:
            PreprocessorMessage(*job, tok, libquixcc::E::ERROR, false, std::get<std::string>(tokens[2].val()));
            throw libquixcc::ProgrammaticPreprocessorException();
        case E::RAW:
            std::cout << std::get<std::string>(tokens[2].val());
            break;
        default:
            break;
        }
    }
    else
    {
        PreprocessorMessage(*job, tok, libquixcc::E::INFO, false, std::get<std::string>(t.val()));
    }

    return true;
}