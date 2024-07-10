////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <core/Exception.h>
#include <core/Logger.h>
#include <lexer/Lex.h>
#include <preprocessor/Preprocessor.h>

#include <iostream>

bool libquixcc::PrepEngine::ParsePrint(const Token &tok,
                                       const std::string &directive,
                                       const std::string &parameter) {
  (void)directive;

  LOG(WARN) << "This macro is deprecated and will be removed in the future"
            << tok << std::endl;

  std::vector<libquixcc::Token> tokens;
  if (!libquixcc::StringLexer::QuickLex(parameter, tokens)) {
    LOG(ERROR) << "Failed to lex print message" << std::endl;
    return false;
  }

  if (tokens.empty()) {
    LOG(ERROR) << "Empty print message" << std::endl;
    return false;
  }

  E level = E::INFO;

  Token t = tokens[0];
  if (t.type() == tName && tokens.size() == 3) {
    std::string id = t.as<std::string>();
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
    else {
      LOG(ERROR) << "Invalid print level" << std::endl;
      return false;
    }

    if (tokens[1].type() != tPunc ||
        (tokens[1]).as<Punctor>() != Punctor::Comma) {
      LOG(ERROR) << "Expected comma after print level" << std::endl;
      return false;
    }

    switch (level) {
      case E::DEBUG:
        LOG(DEBUG) << log::raw << tokens[2].as<std::string>() << tok
                   << std::endl;
        break;
      case E::SUCCESS:
        LOG(SUCCESS) << log::raw << tokens[2].as<std::string>() << tok
                     << std::endl;
        break;
      case E::INFO:
        LOG(INFO) << log::raw << tokens[2].as<std::string>() << tok
                  << std::endl;
        break;
      case E::WARN:
        LOG(WARN) << log::raw << tokens[2].as<std::string>() << tok
                  << std::endl;
        break;
      case E::ERROR:
        LOG(ERROR) << log::raw << tokens[2].as<std::string>() << tok
                   << std::endl;
        throw libquixcc::core::ProgrammaticPreprocessorException();
      case E::RAW:
        std::cout << tokens[2].as<std::string>();
        break;
      default:
        break;
    }
  } else {
    LOG(INFO) << log::raw << t.as<std::string>() << tok << std::endl;
  }

  return true;
}