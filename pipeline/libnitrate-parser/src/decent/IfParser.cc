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

#include <decent/Parse.h>

using namespace qparse::parser;

bool qparse::parser::parse_if(qparse_t &job, qlex_t *rd, Stmt **node) {
  Expr *cond = nullptr;
  if (!parse_expr(job, rd,
                  {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)},
                  &cond)) {
    return false;
  }

  Block *then_block = nullptr;
  if (qlex_peek(rd).is<qOpArrow>()) {
    qlex_next(rd);
    if (!parse(job, rd, &then_block, false, true)) return false;
  } else {
    if (!parse(job, rd, &then_block, true)) return false;
  }

  qlex_tok_t tok = qlex_peek(rd);
  if (tok.is<qKElse>()) {
    qlex_next(rd);
    Block *else_block = nullptr;

    if (qlex_peek(rd).is<qOpArrow>()) {
      qlex_next(rd);

      if (!parse(job, rd, &else_block, false, true)) {
        return false;
      }
    } else {
      if (qlex_peek(rd).is<qKIf>()) {
        qlex_next(rd);
        if (!parse_if(job, rd, reinterpret_cast<Stmt **>(&else_block))) {
          return false;
        }
      } else {
        if (!parse(job, rd, &else_block, true, false)) {
          return false;
        }
      }
    }

    uint32_t loc_end = else_block->get_end_pos();
    *node = IfStmt::get(cond, then_block, else_block);
    (*node)->set_end_pos(loc_end);

  } else {
    uint32_t loc_end = then_block->get_end_pos();
    *node = IfStmt::get(cond, then_block, nullptr);
    (*node)->set_end_pos(loc_end);
  }

  return true;
}
