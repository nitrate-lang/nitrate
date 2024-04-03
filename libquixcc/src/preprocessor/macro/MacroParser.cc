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

#include <preprocessor/macro/MacroParser.h>
#include <stdexcept>
#include <map>
#include <LibMacro.h>
#include <error/Logger.h>

static std::string trim(const std::string &str)
{
    return str.substr(str.find_first_not_of(" \t\n\r\f\v"), str.find_last_not_of(" \t\n\r\f\v") + 1);
}

bool libquixcc::MacroParser::parse(const libquixcc::Token &macro, std::vector<libquixcc::Token> &exp) const
{
    std::string content = trim(std::get<std::string>(macro.val()));

    if (macro.type() == TokenType::MacroSingleLine)
    {
        std::string directive;
        std::string parameter;
        size_t start = content.find('(');
        if (start == std::string::npos)
        {
            LOG(ERROR) << "Invalid macro directive: {}" << content << macro << std::endl;
            return false;
        }
        size_t end = content.find(')', start);
        if (end == std::string::npos)
        {
            LOG(ERROR) << "Invalid macro directive: {}" << content << macro << std::endl;
            return false;
        }

        directive = trim(content.substr(0, start));
        if (start + 1 < content.size())
            parameter = content.substr(start + 1, end - start - 1);

        if (!m_routines.contains(directive))
            LOG(ERROR) << "Unknown macro directive: {}" << directive << macro << std::endl;

        if (!m_routines.at(directive)(job, macro, directive, parameter, exp))
            LOG(ERROR) << "Failed to process macro directive: {}" << directive << macro << std::endl;

        return true;
    }
    else if (macro.type() == TokenType::MacroBlock)
    {
        /// TODO: implement block macro parsing
        return parse(Token(TokenType::MacroSingleLine, content), exp);
    }
    else
    {
        throw std::runtime_error("Invalid macro type");
    }

    return false;
}