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

bool libquixcc::parse_for(quixcc_job_t &job, libquixcc::Scanner *scanner,
                          std::shared_ptr<libquixcc::StmtNode> &node) {
  std::shared_ptr<StmtNode> x0;
  std::shared_ptr<ExprNode> x1, x2;

  Token tok = scanner->peek();
  if (tok.is<Punctor>(OpenParen)) {
    tok = scanner->next();
    tok = scanner->peek();

    if (tok.is<Keyword>(Keyword::Let)) {
      scanner->next();
      std::vector<std::shared_ptr<libquixcc::StmtNode>> let_node;
      if (!parse_let(job, scanner, let_node)) {
        LOG(ERROR) << "Failed to parse let statement in for loop" << tok
                   << std::endl;
        return false;
      }

      x0 = std::make_shared<StmtGroupNode>(let_node);
    } else {
      std::shared_ptr<ExprNode> x0_tmp;
      if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)},
                      x0_tmp)) {
        return false;
      }
      x0 = std::make_shared<ExprStmtNode>(x0_tmp);

      tok = scanner->next();
      if (!tok.is<Punctor>(Semicolon)) {
        LOG(ERROR) << core::feedback[FOR_EXPECTED_SEMICOLON] << tok
                   << std::endl;
        return false;
      }
    }

    if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, x1))
      return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Semicolon)) {
      LOG(ERROR) << core::feedback[FOR_EXPECTED_SEMICOLON] << tok << std::endl;
      return false;
    }

    if (!parse_expr(job, scanner, {Token(tPunc, CloseParen)},
                    x2))
      return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(CloseParen)) {
      LOG(ERROR) << core::feedback[FOR_EXPECTED_CLOSING_PARANTHESIS] << tok
                 << std::endl;
      return false;
    }

    std::shared_ptr<BlockNode> then_block;

    if (scanner->peek().is<Operator>(Arrow)) {
      tok = scanner->next();
      if (!parse(job, scanner, then_block, false, true)) return false;
    } else {
      if (!parse(job, scanner, then_block, true)) return false;
    }

    node = std::make_shared<ForStmtNode>(x0, x1, x2, then_block);

    return true;
  } else {
    tok = scanner->peek();

    if (tok.is<Keyword>(Keyword::Let)) {
      scanner->next();
      std::vector<std::shared_ptr<libquixcc::StmtNode>> let_node;
      if (!parse_let(job, scanner, let_node)) {
        LOG(ERROR) << "Failed to parse let statement in for loop" << tok
                   << std::endl;
        return false;
      }

      x0 = std::make_shared<StmtGroupNode>(let_node);
    } else {
      std::shared_ptr<ExprNode> x0_tmp;
      if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)},
                      x0_tmp)) {
        return false;
      }
      x0 = std::make_shared<ExprStmtNode>(x0_tmp);

      tok = scanner->next();
      if (!tok.is<Punctor>(Semicolon)) {
        LOG(ERROR) << core::feedback[FOR_EXPECTED_SEMICOLON] << tok
                   << std::endl;
        return false;
      }
    }

    if (!parse_expr(job, scanner, {Token(tPunc, Semicolon)}, x1))
      return false;

    tok = scanner->next();
    if (!tok.is<Punctor>(Semicolon)) {
      LOG(ERROR) << core::feedback[FOR_EXPECTED_SEMICOLON] << tok << std::endl;
      return false;
    }

    if (!parse_expr(job, scanner,
                    {Token(tPunc, OpenBrace),
                     Token(tOper, Arrow)},
                    x2))
      return false;

    std::shared_ptr<BlockNode> then_block;

    if (scanner->peek().is<Operator>(Arrow)) {
      tok = scanner->next();
      if (!parse(job, scanner, then_block, false, true)) return false;
    } else {
      if (!parse(job, scanner, then_block, true)) return false;
    }

    node = std::make_shared<ForStmtNode>(x0, x1, x2, then_block);

    return true;
  }
}
