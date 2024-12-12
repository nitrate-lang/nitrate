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

#include <nitrate-lexer/Token.h>
#include <nitrate-parser/Node.h>

#include <descent/Recurse.hh>

using namespace npar;

std::optional<std::pair<std::string_view, std::string_view>>
recurse_foreach_names(qlex_t &rd) {
  if (let ident1 = next_if(qName)) {
    let ident1_value = ident1->as_string(&rd);

    if (next_if(qPuncComa)) {
      if (let ident2 = next_if(qName)) {
        let ident2_value = ident2->as_string(&rd);
        return std::make_pair(ident1_value, ident2_value);
      } else {
        diagnostic << current() << "Expected identifier in foreach statement";
      }
    } else {
      return std::make_pair("", ident1_value);
    }
  } else {
    diagnostic << current() << "Expected identifier in foreach statement";
  }

  return std::nullopt;
}

static Expr *recurse_foreach_expr(npar_t &S, qlex_t &rd, bool has_paren) {
  if (has_paren) {
    return recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRPar)});
  } else {
    return recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qOper, qOpArrow)});
  }
}

static Stmt *recurse_foreach_body(npar_t &S, qlex_t &rd) {
  if (next_if(qOpArrow)) {
    return recurse_block(S, rd, false, true);
  } else {
    return recurse_block(S, rd, true, false);
  }
}

npar::Stmt *npar::recurse_foreach(npar_t &S, qlex_t &rd) {
  /**
   * Syntax examples:
   *   `foreach (i, v in arr) { }`, `foreach (v in arr) { }`
   *   `foreach (i, v in arr) => v.put(i);`, `foreach (v in arr) => v.put();`
   */

  bool has_paren = next_if(qPuncLPar).has_value();

  if (let ident_pair_opt = recurse_foreach_names(rd)) {
    let[index_name, value_name] = ident_pair_opt.value();

    if (next_if(qOpIn)) {
      let iter_expr = recurse_foreach_expr(S, rd, has_paren);
      if (has_paren) {
        if (!next_if(qPuncRPar)) {
          diagnostic << current() << "Expected ')' in foreach statement";
        }
      }

      let body = recurse_foreach_body(S, rd);

      return ForeachStmt::get(index_name, value_name, iter_expr, body);
    } else {
      diagnostic << current() << "Expected 'in' keyword in foreach statement";
    }
  } else {
    diagnostic << current() << "Expected identifier pair in foreach statement";
  }

  return mock_stmt(QAST_NODE_FOREACH);
}
