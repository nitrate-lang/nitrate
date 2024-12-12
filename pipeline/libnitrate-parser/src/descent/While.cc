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

static Expr *recurse_while_cond(npar_t &S, qlex_t &rd) {
  let cur = peek();

  if (cur.is<qOpArrow>() || cur.is<qPuncLCur>()) {
    return make<ConstBool>(true);
  } else {
    return recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)});
  }
}

static Stmt *recurse_while_body(npar_t &S, qlex_t &rd) {
  if (next_if(qOpArrow)) {
    return recurse_block(S, rd, false, true);
  } else {
    return recurse_block(S, rd, true);
  }
}

npar::Stmt *npar::recurse_while(npar_t &S, qlex_t &rd) {
  /**
   * Example syntax:
   *  `while {}`,                 `while => call();`
   *  `while (cond) => call();`,  `while cond => call();`
   *  `while (cond) { call(); }`, `while cond { call(); }`
   */

  /* The condition expression is optional */
  let cond = recurse_while_cond(S, rd);

  /* Support for single statement implicit block */
  let body = recurse_while_body(S, rd);

  return make<WhileStmt>(cond, body);
}
