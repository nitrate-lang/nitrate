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

#define __QUIX_IMPL__

#include "LibMacro.h"
#include "parser/Parse.h"

using namespace qparse::parser;

bool qparse::parser::parse_pub(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_peek(rd);

  ExportLang langType = ExportLang::Default;

  if (tok.is(qText)) {
    qlex_next(rd);

    std::string lang = tok.as_string();

    std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

    if (lang == "c")
      langType = ExportLang::C;
    else if (lang == "c++" || lang == "cxx")
      langType = ExportLang::CXX;
    else if (lang == "d" || lang == "dlang")
      langType = ExportLang::DLang;
    else {
      /// TODO: Write the ERROR message
      return false;
    }

    tok = qlex_peek(rd);
  }

  if (tok.is<qPuncLCur>()) {
    Block *block = nullptr;
    if (!parse(job, rd, &block, true)) return false;

    StmtList *stmts = StmtList::get(block->get_items());
    *node = ExportDecl::get(stmts, langType);
    return true;
  }

  qlex_next(rd);
  if (!tok.is(qKeyW)) {
    /// TODO: Write the ERROR message
    return false;
  }

  StmtListItems stmts;
  Stmt *stmt = nullptr;

  switch (tok.as<qlex_key_t>()) {
    case qKVar:
      if (!parse_var(job, rd, stmts)) return false;
      break;
    case qKLet:
      if (!parse_let(job, rd, stmts)) return false;
      break;
    case qKSubsystem:
      if (!parse_subsystem(job, rd, &stmt)) return false;
      break;
    case qKFn:
      if (!parse_function(job, rd, &stmt)) return false;
      break;
    default:
      /// TODO: Write the ERROR message
      return false;
  }
  if (stmt) stmts.push_back(stmt);

  *node = ExportDecl::get(StmtList::get(stmts), langType);

  return true;
}