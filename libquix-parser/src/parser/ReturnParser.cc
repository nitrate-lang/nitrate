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

using namespace libquixcc::qast::parser;

bool libquixcc::qast::parser::parse_return(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                           Stmt **node) {
  Token tok = scanner->peek();

  if (tok.is<Punctor>(Semicolon)) {
    scanner->next();
    *node = ReturnStmt::get();
    return true;
  }

  Expr *expr = nullptr;
  if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, &expr)) return false;
  *node = ReturnStmt::get(expr);

  tok = scanner->next();

  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[RETIF_MISSING_SEMICOLON] << tok << std::endl;
    return false;
  }

  return true;
}

bool libquixcc::qast::parser::parse_retif(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                          Stmt **node) {
  Token tok;

  Expr *return_expr = nullptr;
  if (!parse_expr(job, scanner, {Token(tPunc, Comma)}, &return_expr)) return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Comma)) {
    LOG(ERROR) << core::feedback[RETIF_MISSING_COMMA] << tok << std::endl;
    return false;
  }

  Expr *condition = nullptr;
  if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, &condition)) return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[RETIF_MISSING_SEMICOLON] << tok << std::endl;
    return false;
  }
  *node = ReturnIfStmt::get(condition, return_expr);

  return true;
}

bool libquixcc::qast::parser::parse_retz(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                         Stmt **node) {
  Token tok;

  Expr *return_expr = nullptr;
  if (!parse_expr(job, scanner, {Token(tPunc, Comma)}, &return_expr)) return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Comma)) {
    LOG(ERROR) << core::feedback[RETZ_MISSING_COMMA] << tok << std::endl;
    return false;
  }

  Expr *condition = nullptr;
  if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, &condition)) return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[RETZ_MISSING_SEMICOLON] << tok << std::endl;
    return false;
  }
  *node = RetZStmt::get(condition, return_expr);

  return true;
}

bool libquixcc::qast::parser::parse_retv(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                         Stmt **node) {
  Token tok;

  Expr *cond = nullptr;
  if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, &cond)) return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[RETV_MISSING_SEMICOLON] << tok << std::endl;
    return false;
  }

  *node = RetVStmt::get(cond);

  return true;
}