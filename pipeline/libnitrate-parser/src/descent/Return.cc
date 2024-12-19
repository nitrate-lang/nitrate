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

#include <descent/Recurse.hh>

using namespace ncc::lex;
using namespace ncc::parse;

Stmt *Parser::recurse_return() {
  /**
   * Syntax examples:
   *   `ret 0;`, `ret;`, `ret 0, 1;`, `ret call();`
   */

  if (let tok = next_if(qPuncSemi)) {
    return make<ReturnStmt>(std::nullopt);
  }

  let expr = recurse_expr({NCCToken(qPunc, qPuncSemi)});

  if (next_if(qPuncSemi)) {
    return make<ReturnStmt>(expr);
  } else {
    diagnostic << current() << "Expected ';' after the return statement.";
  }

  return mock_stmt(QAST_RETURN);
}

Stmt *Parser::recurse_retif() {
  /**
   * Syntax examples:
   *   `retif cond(), 1;`, `retif failed, -1;`
   */

  let condition = recurse_expr({NCCToken(qPunc, qPuncComa)});

  if (next_if(qPuncComa)) {
    let return_val = recurse_expr({NCCToken(qPunc, qPuncSemi)});

    if (next_if(qPuncSemi)) {
      return make<ReturnIfStmt>(condition, return_val);
    } else {
      diagnostic << current() << "Expected ';' after the retif value.";
    }
  } else {
    diagnostic << current() << "Expected ',' after the retif condition.";
  }

  return mock_stmt(QAST_RETIF);
}
