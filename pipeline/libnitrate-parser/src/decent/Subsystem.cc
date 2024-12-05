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

/// TODO: Cleanup this code; it's a mess from refactoring.

#include <nitrate-parser/Node.h>

#include <decent/Recurse.hh>

qparse::Stmt *qparse::recurse_subsystem(qparse_t &S, qlex_t &rd) {
  qlex_tok_t tok = next();
  if (!tok.is(qName)) {
    diagnostic << tok << "Expected subsystem name";
    return mock_stmt(QAST_NODE_SUBSYSTEM);
  }

  std::string name = tok.as_string(&rd);
  SubsystemDeps deps;

  tok = peek();
  if (tok.is<qPuncColn>()) {
    next();

    tok = next();
    if (!tok.is<qPuncLBrk>()) {
      diagnostic << tok << "Expected '[' after subsystem dependencies";
      return mock_stmt(QAST_NODE_SUBSYSTEM);
    }

    while (true) {
      tok = next();
      if (tok.is(qEofF)) {
        diagnostic << tok << "Unexpected end of file in subsystem dependencies";
        return mock_stmt(QAST_NODE_SUBSYSTEM);
        break;
      }

      if (tok.is<qPuncRBrk>()) {
        return mock_stmt(QAST_NODE_SUBSYSTEM);
        break;
      }

      if (!tok.is(qName)) {
        diagnostic << tok << "Expected dependency name";
        return mock_stmt(QAST_NODE_SUBSYSTEM);
      }

      deps.insert(tok.as_string(&rd));

      tok = peek();
      if (tok.is<qPuncComa>()) {
        next();
      }
    }
  }

  Stmt *block = recurse(S, rd, true);

  std::set<Expr *> attributes;
  tok = peek();
  if (tok.is<qKWith>()) {
    next();

    if (!recurse_attributes(S, rd, attributes)) {
      return mock_stmt(QAST_NODE_SUBSYSTEM);
    }
  }

  SubsystemDecl *sub = SubsystemDecl::get(name, block);
  sub->set_end_pos(block->get_end_pos());
  sub->get_deps() = deps;
  sub->get_tags().insert(attributes.begin(), attributes.end());

  return sub;
}
