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

  if (let tok = next_if(qPuncSemi)) {
    let void_return = ReturnStmt::get(std::nullopt);
    void_return->set_end_pos(tok->end);

    return void_return;
  }

  let expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

  if (next_if(qPuncSemi)) {
    let return_stmt = ReturnStmt::get(expr);
    return_stmt->set_end_pos(current().end);

    return return_stmt;
  } else {
    diagnostic << current() << "Expected ';' after the return statement.";
  }

  return mock_stmt(QAST_NODE_RETURN);
}

Stmt *npar::recurse_retif(npar_t &S, qlex_t &rd) {
  /**
   * Syntax examples:
   *   `retif cond(), 1;`, `retif failed, -1;`
   */

  let condition = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncComa)});

  if (next_if(qPuncComa)) {
    let return_val = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

    if (next_if(qPuncSemi)) {
      let retif_stmt = ReturnIfStmt::get(condition, return_val);
      retif_stmt->set_end_pos(current().end);

      return retif_stmt;
    } else {
      diagnostic << current() << "Expected ';' after the retif value.";
    }
  } else {
    diagnostic << current() << "Expected ',' after the retif condition.";
  }

  return mock_stmt(QAST_NODE_RETIF);
}
