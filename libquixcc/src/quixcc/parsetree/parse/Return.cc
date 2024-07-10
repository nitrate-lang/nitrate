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
#include <quixcc/core/Logger.h>
#include <quixcc/parsetree/Parser.h>

using namespace libquixcc;

bool libquixcc::parse_return(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                             std::shared_ptr<libquixcc::StmtNode> &node) {
  Token tok = scanner->peek();

  if (tok.is<Punctor>(Semicolon)) {
    scanner->next();
    node = std::make_shared<ReturnStmtNode>(nullptr);  // void
    return true;
  }

  std::shared_ptr<ExprNode> expr;
  if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, expr)) return false;
  node = std::make_shared<ReturnStmtNode>(expr);

  tok = scanner->next();

  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[RETIF_MISSING_SEMICOLON] << tok << std::endl;
    return false;
  }

  return true;
}

bool libquixcc::parse_retif(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                            std::shared_ptr<libquixcc::StmtNode> &node) {
  /*
      Syntax:
          "retif" <return_expr>, <condition>;

      Translates to:
      if (<condition>)
      {
          return <return_expr>;
      }
  */

  Token tok;

  std::shared_ptr<ExprNode> return_expr;
  if (!parse_expr(job, scanner, {Token(tPunc, Comma)}, return_expr))
    return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Comma)) {
    LOG(ERROR) << core::feedback[RETIF_MISSING_COMMA] << tok << std::endl;
    return false;
  }

  std::shared_ptr<ExprNode> condition;
  if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, condition))
    return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[RETIF_MISSING_SEMICOLON] << tok << std::endl;
    return false;
  }
  node = std::make_shared<RetifStmtNode>(condition, return_expr);

  return true;
}

bool libquixcc::parse_retz(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                           std::shared_ptr<libquixcc::StmtNode> &node) {
  /*
      Syntax:
          "retz" <return_expr>, <condition>;

      Translates to:
      if (<condition>)
      {
          return <return_expr>;
      }
  */

  Token tok;

  std::shared_ptr<ExprNode> return_expr;
  if (!parse_expr(job, scanner, {Token(tPunc, Comma)}, return_expr))
    return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Comma)) {
    LOG(ERROR) << core::feedback[RETZ_MISSING_COMMA] << tok << std::endl;
    return false;
  }

  std::shared_ptr<ExprNode> condition;
  if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, condition))
    return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[RETZ_MISSING_SEMICOLON] << tok << std::endl;
    return false;
  }
  node = std::make_shared<RetzStmtNode>(condition, return_expr);

  return true;
}

bool libquixcc::parse_retv(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                           std::shared_ptr<libquixcc::StmtNode> &node) {
  /*
      Syntax:
          "retv" <condition>;

      Translates to:
      if (<condition>)
      {
          return;
      }
  */

  Token tok;

  std::shared_ptr<ExprNode> cond;
  if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, cond)) return false;

  tok = scanner->next();
  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[RETV_MISSING_SEMICOLON] << tok << std::endl;
    return false;
  }

  node = std::make_shared<RetvStmtNode>(cond);

  return true;
}