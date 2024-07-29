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

#define __QUIX_IMPL__

#include "LibMacro.h"
#include "parser/Parse.h"

bool qparse::parser::parse_subsystem(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is(qName)) {
    /// TODO: Write the ERROR message
    return false;
  }

  std::string name = tok.as_string(rd);

  if (name.find("::") != std::string::npos) {
    Stmt *sub = nullptr;

    std::string subname = name.substr(0, name.find("::"));

    qlex_tok_t newtok;
    qlex_tok_fromstr(rd, qName, name.substr(name.find("::") + 2).c_str(), 0, &newtok);
    qlex_push(rd, newtok);

    if (!parse_subsystem(job, rd, &sub)) return false;

    Block *block = Block::get();
    block->add_item(sub);

    *node = SubsystemDecl::get(subname, block);

    return true;
  }

  std::set<std::string> deps;

  tok = qlex_peek(rd);

  // check if : item1, item2, item3
  if (tok.is<qPuncColn>()) {
    qlex_next(rd);  // consume colon
    tok = qlex_next(rd);
    if (!tok.is<qPuncLBrk>()) {
      /// TODO: Write the ERROR message
      return false;
    }
    tok = qlex_next(rd);

    if (!tok.is(qName)) {
      /// TODO: Write the ERROR message
      return false;
    }
    deps.insert(tok.as_string(rd));

    tok = qlex_peek(rd);
    while (tok.is<qPuncComa>()) {
      qlex_next(rd);  // consume comma
      tok = qlex_next(rd);
      if (!tok.is(qName)) {
        /// TODO: Write the ERROR message
        return false;
      }
      deps.insert(tok.as_string(rd));
      tok = qlex_peek(rd);
    }

    tok = qlex_next(rd);
    if (!tok.is<qPuncRBrk>()) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  Block *block = nullptr;
  if (!parse(job, rd, &block, true)) return false;

  SubsystemDecl *sub = SubsystemDecl::get(name, block);
  sub->add_tags(deps);

  *node = sub;

  return true;
}