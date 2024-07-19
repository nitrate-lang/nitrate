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

#include <core/Macro.h>
#include <qast/Parser.h>
#include <quixcc/core/Logger.h>

using namespace libquixcc::qast::parser;

bool libquixcc::qast::parser::parse_switch(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                           Stmt **node) {
  Expr *cond = nullptr;
  if (!parse_expr(job, scanner, {Token(tPunc, OpenBrace)}, &cond)) {
    return false;
  }

  SwitchCases cases;
  Block *default_case = nullptr;

  Token tok = scanner->next();
  if (!tok.is<Punctor>(OpenBrace)) {
    LOG(ERROR) << core::feedback[SWITCH_EXPECTED_LEFT_BRACE] << tok.serialize() << tok << std::endl;
    return false;
  }

  while (true) {
    tok = scanner->peek();
    if (tok.is<Punctor>(CloseBrace)) {
      break;
    }

    if (tok.is<Keyword>(Keyword::Default)) {
      scanner->next();
      if (default_case) {
        LOG(ERROR) << core::feedback[SWITCH_MULTIPLE_DEFAULT] << tok.serialize() << tok
                   << std::endl;
        return false;
      }

      tok = scanner->next();
      if (!tok.is<Punctor>(Colon)) {
        LOG(ERROR) << core::feedback[SWITCH_EXPECTED_COLON] << tok.serialize() << tok << std::endl;
        return false;
      }

      if (!parse(job, scanner, &default_case)) {
        return false;
      }

      continue;
    }

    if (!tok.is<Keyword>(Keyword::Case)) {
      LOG(ERROR) << core::feedback[SWITCH_EXPECTED_CASE] << tok.serialize() << tok << std::endl;
      return false;
    }
    scanner->next();

    Expr *case_expr = nullptr;
    if (!parse_expr(job, scanner, {Token(tPunc, Colon)}, &case_expr)) {
      return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Colon)) {
      LOG(ERROR) << core::feedback[SWITCH_EXPECTED_COLON] << tok.serialize() << tok << std::endl;
      return false;
    }

    Block *case_block = nullptr;
    if (!parse(job, scanner, &case_block)) {
      return false;
    }

    cases.push_back(CaseStmt::get(case_expr, case_block));
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(CloseBrace)) {
    LOG(ERROR) << core::feedback[SWITCH_EXPECTED_RIGHT_BRACE] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  *node = SwitchStmt::get(cond, cases, default_case);

  return true;
}
