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
#include <parsetree/Parser.h>

using namespace libquixcc;

bool libquixcc::parse_switch(quixcc_job_t &job, libquixcc::Scanner *scanner,
                             std::shared_ptr<libquixcc::StmtNode> &node) {
  std::shared_ptr<ExprNode> cond;
  if (!parse_expr(job, scanner, {Token(tPunc, Punctor::OpenBrace)},
                  cond)) {
    return false;
  }

  std::vector<std::shared_ptr<libquixcc::CaseStmtNode>> cases;
  std::shared_ptr<BlockNode> default_case;

  Token tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::OpenBrace)) {
    LOG(ERROR) << core::feedback[SWITCH_EXPECTED_LEFT_BRACE] << tok.serialize()
               << tok << std::endl;
    return false;
  }

  while (true) {
    tok = scanner->peek();
    if (tok.is<Punctor>(Punctor::CloseBrace)) {
      break;
    }

    if (tok.is<Keyword>(Keyword::Default)) {
      scanner->next();
      if (default_case) {
        LOG(ERROR) << core::feedback[SWITCH_MULTIPLE_DEFAULT] << tok.serialize()
                   << tok << std::endl;
        return false;
      }

      tok = scanner->next();
      if (!tok.is<Punctor>(Punctor::Colon)) {
        LOG(ERROR) << core::feedback[SWITCH_EXPECTED_COLON] << tok.serialize()
                   << tok << std::endl;
        return false;
      }

      if (!parse(job, scanner, default_case)) {
        return false;
      }

      continue;
    }

    if (!tok.is<Keyword>(Keyword::Case)) {
      LOG(ERROR) << core::feedback[SWITCH_EXPECTED_CASE] << tok.serialize()
                 << tok << std::endl;
      return false;
    }
    scanner->next();

    std::shared_ptr<ExprNode> case_expr;
    if (!parse_expr(job, scanner, {Token(tPunc, Punctor::Colon)},
                    case_expr)) {
      return false;
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Colon)) {
      LOG(ERROR) << core::feedback[SWITCH_EXPECTED_COLON] << tok.serialize()
                 << tok << std::endl;
      return false;
    }

    std::shared_ptr<BlockNode> case_block;
    if (!parse(job, scanner, case_block)) {
      return false;
    }

    cases.push_back(std::make_shared<CaseStmtNode>(case_expr, case_block));
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::CloseBrace)) {
    LOG(ERROR) << core::feedback[SWITCH_EXPECTED_RIGHT_BRACE] << tok.serialize()
               << tok << std::endl;
    return false;
  }

  node = std::make_shared<SwitchStmtNode>(cond, cases, default_case);

  return true;
}
