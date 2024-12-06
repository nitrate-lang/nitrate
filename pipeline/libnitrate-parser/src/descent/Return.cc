////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <nitrate-parser/Node.h>

#include <descent/Recurse.hh>

using namespace npar;

Stmt *npar::recurse_return(npar_t &S, qlex_t &rd) {
  /**
   * Syntax examples:
   *   `ret 0;`, `ret;`, `ret 0, 1;`, `ret call();`
   */

  /* Return void */
  if (let tok = peek(); tok.is<qPuncSemi>()) {
    let stmt = (next(), ReturnStmt::get(std::nullopt));
    stmt->set_end_pos(tok.end);

    return stmt;
  }

  let expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

  if (let tok = peek(); tok.is<qPuncSemi>()) {
    let R = (next(), ReturnStmt::get(expr));
    R->set_end_pos(current().end);

    return R;
  } else {
    diagnostic << tok << "Expected ';' after the return statement.";
  }

  return mock_stmt(QAST_NODE_RETURN);
}

Stmt *npar::recurse_retif(npar_t &S, qlex_t &rd) {
  /**
   * Syntax examples:
   *   `retif cond(), 1;`, `retif failed, -1;`
   */

  let condition = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncComa)});

  if (let tok = peek(); tok.is<qPuncComa>()) {
    let return_expr =
        (next(), recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}));

    if (let tok = peek(); tok.is<qPuncSemi>()) {
      let R = (next(), ReturnIfStmt::get(condition, return_expr));
      R->set_end_pos(current().end);

      return R;
    } else {
      diagnostic << tok << "Expected ';' after the retif value.";
    }
  } else {
    diagnostic << tok << "Expected ',' after the retif condition.";
  }

  return mock_stmt(QAST_NODE_RETIF);
}
