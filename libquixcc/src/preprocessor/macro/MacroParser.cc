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

#include <LibMacro.h>
#include <core/Logger.h>
#include <preprocessor/Preprocessor.h>

#include <map>
#include <stdexcept>

static std::string trim(const std::string &str) {
  return str.substr(str.find_first_not_of(" \t\n\r\f\v"),
                    str.find_last_not_of(" \t\n\r\f\v") + 1);
}

bool libquixcc::PrepEngine::parse_macro(const libquixcc::Token &macro) {
  typedef bool (libquixcc::PrepEngine::*Routine)(
      const libquixcc::Token &, const std::string &, const std::string &);

  const static std::map<std::string, Routine> routines = {
      {"define", &PrepEngine::ParseDefine},
      {"pragma", &PrepEngine::ParsePragma},
      {"print", &PrepEngine::ParsePrint},
      {"readstdin", &PrepEngine::ParseReadstdin},
      {"encoding", &PrepEngine::ParseEncoding},
      {"lang", &PrepEngine::ParseLang},
      {"language", &PrepEngine::ParseLang},
      {"copyright", &PrepEngine::ParseAuthor},
      {"license", &PrepEngine::ParseLicense},
      {"use", &PrepEngine::ParseUse},
      {"description", &PrepEngine::ParseDescription},
      {"qsys", &PrepEngine::ParseQSys},
      {"fn", &PrepEngine::ParseFn},
  };

  std::string content = trim(macro.as<std::string>());

  if (macro.type == TT::MacroSingleLine) {
    std::string directive, parameter;
    size_t start = content.find('('), end = 0;

    if (start != std::string::npos) {
      /*==================== PARSE MACRO DIRECTIVE ====================*/
      end = content.find(')', start);
      if (end == std::string::npos) {
        LOG(ERROR) << "Invalid macro directive: {}" << content << macro
                   << std::endl;
        return false;
      }

      directive = trim(content.substr(0, start));
      if (start + 1 < content.size())
        parameter = content.substr(start + 1, end - start - 1);

      /*==================== EXECUTE BUILTIN MACRO DIRECTIVE
       * ====================*/
      if (routines.contains(directive)) {
        if (!(this->*routines.at(directive))(macro, directive, parameter)) {
          LOG(ERROR) << "Failed to process macro directive: {}" << directive
                     << macro << std::endl;
        }
        return true;
      }

      /*==================== EXECUTE USER DEFINED MACRO DIRECTIVE
       * ====================*/
      if (job->m_macros.contains(directive)) {
        auto fn = job->m_macros.at(directive);
        std::vector<std::string> args_lifetime;
        std::vector<const char *> args_c;

        /*==================== PARSE MACRO ARGUMENTS ====================*/
        size_t pos = 0;
        while (pos < parameter.size()) {
          size_t next = parameter.find(',', pos);
          if (next == std::string::npos) {
            args_lifetime.push_back(trim(parameter.substr(pos)));
            break;
          }

          args_lifetime.push_back(trim(parameter.substr(pos, next - pos)));
          pos = next + 1;
        }

        for (auto &arg : args_lifetime) {
          args_c.push_back(arg.c_str());
        }

        /*==================== INVOKE DYNAMIC SYMBOL ====================*/
        char *result = fn(args_c.size(), args_c.data());
        if (result) {
          std::string result_str = result;
          free(result);

          /*========== RECURSIVELY EXPAND MACRO TOKEN OUTPUT ============*/
          auto lex = clone();
          lex->set_source(result_str, job->m_filename.top());

          Token t;
          while ((t = lex->next()).type != TT::Eof) {
            emit(t);
          }
        }

        return true;
      }

      LOG(ERROR) << "Unknown macro directive: {}" << directive << macro
                 << std::endl;

      return false;
    } else {
      for (auto &routine : routines) {
        if (content.starts_with(routine.first)) {
          if (!(this->*routine.second)(macro, routine.first,
                                       content.substr(routine.first.size()))) {
            LOG(ERROR) << "Failed to process macro directive: {}"
                       << routine.first << macro << std::endl;
          }

          return true;
        }
      }

      LOG(WARN) << "Ignoring unknown macro directive: {}" << content << macro
                << std::endl;
      return true;
    }
  } else if (macro.type == TT::MacroBlock) {
    std::string directive;
    std::string body;

    size_t start = content.find(' ');
    if (start == std::string::npos) {
      directive = content;
    } else {
      directive = content.substr(0, start);
      body = content.substr(start + 1);
    }

    if (!routines.contains(directive)) {
      LOG(ERROR) << "Unknown macro directive: {}" << directive << macro
                 << std::endl;
    }

    if (!(this->*routines.at(directive))(macro, directive, body)) {
      LOG(ERROR) << "Failed to process macro directive: {}" << directive
                 << macro << std::endl;
    }

    return true;
  } else {
    throw std::runtime_error("Invalid macro type");
  }

  return false;
}