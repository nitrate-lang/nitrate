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

#include <core/SHA160.h>
#include <qast/Parser.h>
#include <quixcc/IR/Q/Function.h>
#include <quixcc/IR/Q/QIR.h>
#include <quixcc/IR/Q/Variable.h>
#include <quixcc/core/Logger.h>
#include <quixcc/lexer/Lex.h>
#include <quixcc/preprocessor/Preprocessor.h>

#include <boost/filesystem.hpp>
#include <filesystem>
#include <fstream>
#include <quixcc/Quix.hpp>

using namespace libquixcc;
using namespace std;

static std::optional<MacroParamType> parse_macro_type(Scanner *scanner) {
  static const std::map<std::string_view, MacroParamType> types = {
      {"int", MacroParamType::Int},   {"uint", MacroParamType::UInt},
      {"text", MacroParamType::Text}, {"bool", MacroParamType::Bool},
      {"real", MacroParamType::Real}, {"nil", MacroParamType::Nil}};

  Token t = scanner->next();

  if (t.is<Punctor>(OpenBracket)) {
    parse_macro_type(scanner);

    t = scanner->next();
    if (!t.is<Punctor>(CloseBracket)) {
      LOG(ERROR) << "Expected closing bracket" << t << std::endl;
      return std::nullopt;
    }

    return MacroParamType::List;
  }

  if (!t.is(tName)) {
    LOG(ERROR) << "Expected type name" << t << std::endl;
    return std::nullopt;
  }

  if (!types.contains(t.as_string())) {
    LOG(ERROR) << "Unknown type name" << t << std::endl;
    return std::nullopt;
  }

  return types.at(t.as_string());
}

static std::optional<std::string> parse_macro_default_param(Scanner *scanner) {
  Token t = scanner->next();

  if (t.is<Punctor>(OpenBracket)) {
    std::string inner;

    while (true) {
      t = scanner->peek();
      if (t.is<Punctor>(CloseBracket)) {
        scanner->next();
        break;
      }

      auto tmp = parse_macro_default_param(scanner);
      if (!tmp) {
        LOG(ERROR) << "Recursive parsing of macro parameter failed" << t << std::endl;
        return std::nullopt;
      }

      inner += tmp.value();

      t = scanner->peek();
      if (t.is<Punctor>(Comma)) {
        scanner->next();
        inner += ", ";
      }
    }

    return "{" + inner + "}";
  }

  if (t.is(tText)) {
    return "\"" + t.as_string() + "\"";
  } else if (t.is(tChar)) {
    return "'" + t.as_string() + "'";
  } else if (t.is(tName)) {
    if (t.as_string() == "nil") {
      return "nil";
    } else {
      return t.as_string();
    }
  } else if (t.is<Keyword>(Keyword::True)) {
    return "true";
  } else if (t.is<Keyword>(Keyword::False)) {
    return "false";
  } else if (t.is(tIntL) || t.is(tNumL)) {
    return t.as_string();
  } else {
    LOG(ERROR) << "Unsupported value type in default macro parameter" << t << std::endl;
    return std::nullopt;
  }
}

bool PrepEngine::ParseFn(const Token &tok, const std::string &directive,
                         const std::string &parameter) {
  (void)tok;
  (void)directive;

  std::string mname;
  size_t offset = 0;
  std::vector<MacroData::MacroArg> params;

  {
    /* Parse macro name */
    StringLexer lexer(parameter);
    Token t = lexer.next();
    if (!t.is(tName)) {
      LOG(ERROR) << "Expected macro function name" << tok << std::endl;
      return false;
    }
    mname = t.as_string();

    /* Macro open paren */
    t = lexer.next();
    if (!t.is<Punctor>(OpenParen)) {
      LOG(ERROR) << "Expected '(' after macro function name" << tok << std::endl;
      return false;
    }

    while (!t.is(tEofF)) {
      t = lexer.next();
      if (t.is<Punctor>(CloseParen)) {
        break;
      }

      if (!t.is(tName)) {
        LOG(ERROR) << "Expected macro parameter name" << tok << std::endl;
        return false;
      }

      std::string pname = t.as_string();

      t = lexer.next();
      if (!t.is<Punctor>(Colon)) {
        LOG(ERROR) << "Expected ':' after macro parameter name" << tok << std::endl;
        return false;
      }

      auto ret = parse_macro_type(&lexer);
      if (!ret) {
        LOG(ERROR) << "Expected macro parameter type" << tok << std::endl;
        return false;
      }

      t = lexer.peek();

      std::optional<std::string> def;
      if (t.is<Operator>(OpAssign)) {
        lexer.next();

        def = parse_macro_default_param(&lexer);
      }

      params.push_back({pname, ret.value(), def});

      t = lexer.peek();
      if (t.is<Punctor>(Comma)) {
        lexer.next();
      }
    }

    t = lexer.peek();
    if (t.is<Punctor>(Colon)) {
      lexer.next();

      auto ret = parse_macro_type(&lexer);
      if (!ret) {
        LOG(ERROR) << "Expected macro return type" << tok << std::endl;
        return false;
      }

      // Macro return types are unused for now
      (void)ret;
    }

    t = lexer.next();
    if (!t.is<Punctor>(OpenBrace)) {
      LOG(ERROR) << "Expected '{' after macro function parameters" << tok << std::endl;
      return false;
    }

    offset = lexer.offset() - 1;

    bool ended_in_brace = false;

    while (!(t = lexer.next()).is(tEofF)) {
      if (t.is<Punctor>(CloseBrace)) {
        ended_in_brace = true;
      } else {
        ended_in_brace = false;
      }
    }

    if (!ended_in_brace) {
      LOG(ERROR) << "Expected macro function body" << tok << std::endl;
      return false;
    }
  }

  if (m_macros->contains(mname)) {
    LOG(DEBUG) << "Overloading macro function " << mname << tok << std::endl;
  }

  std::string luacode = parameter.substr(offset, parameter.size() - offset - 1);

  (*m_macros)[mname] = MacroData(luacode, params);

  return true;
}

std::ostream &libquixcc::operator<<(std::ostream &os, libquixcc::MacroParamType type) {
  switch (type) {
  case MacroParamType::Int:
    os << "int";
    break;
  case MacroParamType::UInt:
    os << "uint";
    break;
  case MacroParamType::Text:
    os << "text";
    break;
  case MacroParamType::Bool:
    os << "bool";
    break;
  case MacroParamType::Real:
    os << "real";
    break;
  case MacroParamType::Nil:
    os << "nil";
    break;
  case MacroParamType::List:
    os << "[text]";
    break;
  }

  return os;
}