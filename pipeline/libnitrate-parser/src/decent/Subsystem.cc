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

using namespace qparse;

bool qparse::recurse_subsystem(qparse_t &S, qlex_t &rd, Stmt **node) {
  qlex_tok_t tok = next();
  if (!tok.is(qName)) {
    syntax(tok, "Expected subsystem name");
  }

  std::string name = tok.as_string(&rd);
  SubsystemDeps deps;

  tok = peek();
  if (tok.is<qPuncColn>()) {
    next();

    tok = next();
    if (!tok.is<qPuncLBrk>()) {
      syntax(tok, "Expected '[' after subsystem dependencies");
    }

    while (true) {
      tok = next();
      if (tok.is(qEofF)) {
        syntax(tok, "Unexpected end of file in subsystem dependencies");
        break;
      }

      if (tok.is<qPuncRBrk>()) {
        break;
      }

      if (!tok.is(qName)) {
        syntax(tok, "Expected dependency name");
      }

      deps.insert(tok.as_string(&rd));

      tok = peek();
      if (tok.is<qPuncComa>()) {
        next();
      }
    }
  }

  Block *block = nullptr;
  if (!recurse(S, rd, &block, true)) {
    syntax(peek(), "Expected block in subsystem definition");
  }

  std::set<Expr *> attributes;
  tok = peek();
  if (tok.is<qKWith>()) {
    next();

    if (!recurse_attributes(S, rd, attributes)) {
      return false;
    }
  }

  SubsystemDecl *sub = SubsystemDecl::get(name, block);
  sub->set_end_pos(block->get_end_pos());
  sub->get_deps() = deps;
  sub->get_tags().insert(attributes.begin(), attributes.end());

  *node = sub;

  return true;
}