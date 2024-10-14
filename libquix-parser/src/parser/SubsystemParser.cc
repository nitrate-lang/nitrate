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

#include <parser/Parse.h>

using namespace qparse::parser;
using namespace qparse::diag;

bool qparse::parser::parse_subsystem(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is(qName)) {
    syntax(tok, "Expected subsystem name");
  }

  std::string name = tok.as_string(rd);
  SubsystemDeps deps;

  tok = qlex_peek(rd);
  if (tok.is<qPuncColn>()) {
    qlex_next(rd);

    tok = qlex_next(rd);
    if (!tok.is<qPuncLBrk>()) {
      syntax(tok, "Expected '[' after subsystem dependencies");
    }

    while (true) {
      tok = qlex_next(rd);
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

      deps.insert(tok.as_string(rd));

      tok = qlex_peek(rd);
      if (tok.is<qPuncComa>()) {
        qlex_next(rd);
      }
    }
  }

  Block *block = nullptr;
  if (!parse(job, rd, &block, true)) {
    syntax(qlex_peek(rd), "Expected block in subsystem definition");
  }

  std::set<std::string> implements;
  tok = qlex_peek(rd);
  if (tok.is<qKWith>()) {
    qlex_next(rd);
    tok = qlex_next(rd);
    if (!tok.is<qPuncLBrk>()) {
      syntax(tok, "Expected '[' after 'impl' keyword");
    }

    while (true) {
      tok = qlex_next(rd);
      if (tok.is(qEofF)) {
        syntax(tok, "Unexpected end of file in 'impl' list");
        break;
      }

      if (tok.is<qPuncRBrk>()) break;

      if (!tok.is(qName)) {
        syntax(tok, "Expected tag name in 'impl' list");
      }

      implements.insert(tok.as_string(rd));

      tok = qlex_peek(rd);
      if (tok.is<qPuncComa>()) {
        qlex_next(rd);
      }
    }

    tok = qlex_peek(rd);
  }

  SubsystemDecl *sub = SubsystemDecl::get(name, block);
  sub->set_end_pos(block->get_end_pos());
  sub->add_deps(deps);
  sub->add_tags(std::move(implements));

  *node = sub;

  return true;
}