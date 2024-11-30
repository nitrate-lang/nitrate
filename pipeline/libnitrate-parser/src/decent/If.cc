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

#include <decent/Recurse.hh>

#include "nitrate-parser/Node.h"

qparse::Stmt *qparse::recurse_if(qparse_t &S, qlex_t &rd) {
  Expr *cond = nullptr;
  if (!recurse_expr(S, rd,
                    {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)},
                    &cond)) {
    return mock_stmt(QAST_NODE_IF);
  }

  Stmt *then_block = nullptr;
  if (peek().is<qOpArrow>()) {
    next();
    if (!recurse(S, rd, &then_block, false, true)) {
      return mock_stmt(QAST_NODE_IF);
    }
  } else {
    if (!recurse(S, rd, &then_block, true)) {
      return mock_stmt(QAST_NODE_IF);
    };
  }

  qlex_tok_t tok = peek();
  if (tok.is<qKElse>()) {
    next();
    Stmt *else_block = nullptr;

    if (peek().is<qOpArrow>()) {
      next();

      if (!recurse(S, rd, &else_block, false, true)) {
        return mock_stmt(QAST_NODE_IF);
      }
    } else {
      if (peek().is<qKIf>()) {
        next();
        else_block = recurse_if(S, rd);
      } else {
        if (!recurse(S, rd, &else_block, true, false)) {
          return mock_stmt(QAST_NODE_IF);
        }
      }
    }

    uint32_t loc_end = else_block->get_end_pos();
    auto R = IfStmt::get(cond, then_block, else_block);
    R->set_end_pos(loc_end);
    return R;
  } else {
    uint32_t loc_end = then_block->get_end_pos();
    auto R = IfStmt::get(cond, then_block, nullptr);
    R->set_end_pos(loc_end);
    return R;
  }
}
