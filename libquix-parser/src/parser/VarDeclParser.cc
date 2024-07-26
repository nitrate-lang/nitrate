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

#include "LibMacro.h"
#include "parser/Parse.h"
#include <quixcc/core/Logger.h>

using namespace libquixcc;
using namespace libquixcc::qast;
using namespace libquixcc::qast::parser;

static bool parse_decl(quixcc_cc_job_t &job, Token tok, libquixcc::Scanner *scanner,
                       std::pair<std::string, Type *> &decl) {
  std::string name = tok.as_string();

  tok = scanner->peek();
  if (!tok.is<Punctor>(Colon)) {
    decl = std::make_pair(name, nullptr);
    return true;
  }

  scanner->next();

  Type *type = nullptr;

  if (!parse_type(job, scanner, &type)) {
    LOG(ERROR) << core::feedback[VAR_DECL_TYPE_ERR] << name << tok << std::endl;
    return false;
  }

  decl = std::make_pair(name, type);
  return true;
}

bool libquixcc::qast::parser::parse_var(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                        StmtListItems &nodes) {
  Token tok = scanner->next();

  std::vector<std::pair<std::string, Type *>> decls;
  bool multi_decl = false;
  if (tok.is<Punctor>(OpenBracket)) {
    multi_decl = true;

    while (true) {
      tok = scanner->next();

      std::pair<std::string, Type *> decl;
      if (!parse_decl(job, tok, scanner, decl)) return false;

      decls.push_back(decl);

      tok = scanner->next();
      if (tok.is<Punctor>(Comma))
        continue;
      else if (tok.is<Punctor>(CloseBracket))
        break;
      else {
        LOG(ERROR) << core::feedback[VAR_DECL_MISSING_PUNCTOR] << decl.first << tok << std::endl;
        return false;
      }
    }
  } else if (tok.type() == tName) {
    std::pair<std::string, Type *> decl;
    if (!parse_decl(job, tok, scanner, decl)) return false;

    decls.push_back(decl);
  } else {
    LOG(ERROR) << core::feedback[VAR_DECL_MISSING_IDENTIFIER] << tok << std::endl;
    return false;
  }

  if (decls.empty()) {
    LOG(ERROR) << core::feedback[VAR_DECL_MISSING_IDENTIFIER] << tok << std::endl;
    return false;
  }

  tok = scanner->next();
  if (tok.is<Punctor>(Semicolon)) {
    for (auto &decl : decls) nodes.push_back(VarDecl::get(decl.first, decl.second, nullptr));
  } else if (tok.is<Operator>(OpAssign)) {
    if (multi_decl)
      throw std::runtime_error("Initializer not implemented for multiple declarations");

    Expr *init = nullptr;
    if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, &init)) return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Semicolon)) {
      LOG(ERROR) << core::feedback[VAR_DECL_MISSING_PUNCTOR] << decls[0].first << tok << std::endl;
      return false;
    }

    nodes.push_back(VarDecl::get(decls[0].first, decls[0].second, init));
  } else {
    LOG(ERROR) << core::feedback[VAR_DECL_MISSING_PUNCTOR] << tok << std::endl;
    return false;
  }

  return true;
}