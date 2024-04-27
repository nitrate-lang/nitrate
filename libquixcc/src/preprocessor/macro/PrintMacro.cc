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

#define QUIXCC_INTERNAL

#include <preprocessor/macro/PrintMacro.h>
#include <core/Exception.h>
#include <lexer/Lex.h>
#include <core/Logger.h>
#include <iostream>

bool libquixcc::macro::ParsePrint(quixcc_job_t *job, const Token &tok, const std::string &directive, const std::string &parameter, std::vector<libquixcc::Token> &exp)
{
    // print [mode] "<message>"
    (void)directive;
    (void)exp;

    std::vector<libquixcc::Token> tokens;
    if (!libquixcc::StringLexer::QuickLex(parameter, tokens))
    {
        LOG(ERROR) << "Failed to lex print message" << std::endl;
        return false;
    }

    if (tokens.empty())
    {
        LOG(ERROR) << "Empty print message" << std::endl;
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
            LOG(ERROR) << "Invalid print level" << std::endl;
            return false;
        }

        if (tokens[1].type() != TokenType::Punctor || std::get<Punctor>(tokens[1].val()) != Punctor::Comma)
        {
            LOG(ERROR) << "Expected comma after print level" << std::endl;
            return false;
        }

        switch (level)
        {
        case E::DEBUG:
            LOG(DEBUG) << std::get<std::string>(tokens[2].val()) << tok << std::endl;
            break;
        case E::SUCCESS:
            LOG(SUCCESS) << std::get<std::string>(tokens[2].val()) << tok << std::endl;
            break;
        case E::INFO:
            LOG(INFO) << std::get<std::string>(tokens[2].val()) << tok << std::endl;
            break;
        case E::WARN:
            LOG(WARN) << std::get<std::string>(tokens[2].val()) << tok << std::endl;
            break;
        case E::ERROR:
            LOG(ERROR) << std::get<std::string>(tokens[2].val()) << tok << std::endl;
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
        LOG(INFO) << std::get<std::string>(t.val()) << tok << std::endl;
    }

    return true;
}