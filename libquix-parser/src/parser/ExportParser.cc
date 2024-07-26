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

using namespace qparse::parser;

bool qparse::parser::parse_pub(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                        Stmt **node) {
  Token tok = scanner->peek();

  ExportLang langType = ExportLang::Default;

  if (tok.is(tText)) {
    scanner->next();

    std::string lang = tok.as_string();

    std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

    if (lang == "c")
      langType = ExportLang::C;
    else if (lang == "c++" || lang == "cxx")
      langType = ExportLang::CXX;
    else if (lang == "d" || lang == "dlang")
      langType = ExportLang::DLang;
    else {
      LOG(ERROR) << core::feedback[PARSER_UNKNOWN_LANGUAGE] << lang << tok << std::endl;
      return false;
    }

    tok = scanner->peek();
  }

  if (tok.is<Punctor>(OpenBrace)) {
    Block *block = nullptr;
    if (!parse(job, scanner, &block, true)) return false;

    StmtList *stmts = StmtList::get(block->get_items());
    *node = ExportDecl::get(stmts, langType);
    return true;
  }

  scanner->next();
  if (!tok.is(tKeyW)) {
    LOG(ERROR) << core::feedback[PARSER_EXPECTED_KEYWORD] << tok.serialize() << tok << std::endl;
    return false;
  }

  StmtListItems stmts;
  Stmt *stmt = nullptr;

  switch (tok.as<Keyword>()) {
    case Keyword::Var:
      if (!parse_var(job, scanner, stmts)) return false;
      break;
    case Keyword::Let:
      if (!parse_let(job, scanner, stmts)) return false;
      break;
    case Keyword::Subsystem:
      if (!parse_subsystem(job, scanner, &stmt)) return false;
      break;
    case Keyword::Fn:
      if (!parse_function(job, scanner, &stmt)) return false;
      break;
    default:
      LOG(ERROR) << core::feedback[PARSER_EXPECTED_KEYWORD] << tok.serialize() << tok << std::endl;
      return false;
  }
  if (stmt) stmts.push_back(stmt);

  *node = ExportDecl::get(StmtList::get(stmts), langType);

  return true;
}