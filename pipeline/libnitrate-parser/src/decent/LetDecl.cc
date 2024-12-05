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

#include <decent/Recurse.hh>

#include "nitrate-parser/Node.h"

using namespace qparse;
using namespace qparse;

static bool recurse_decl(qparse_t &S, qlex_tok_t tok, qlex_t &rd,
                         std::pair<std::string, Type *> &decl) {
  if (!tok.is(qName)) {
    diagnostic << tok << "Expected a name in let declaration";
    return false;
  }

  std::string name = tok.as_string(&rd);

  tok = peek();
  if (!tok.is<qPuncColn>()) {
    decl = std::make_pair(name, nullptr);
    return true;
  }

  next();

  Type *type = nullptr;

  type = recurse_type(S, rd);

  decl = std::make_pair(name, type);
  return true;
}

std::vector<Stmt *> qparse::recurse_let(qparse_t &S, qlex_t &rd) {
  qlex_tok_t tok = next();

  std::vector<std::pair<std::string, Type *>> decls;
  if (tok.is(qName)) {
    std::pair<std::string, Type *> decl;
    if (!recurse_decl(S, tok, rd, decl)) {
      return {mock_stmt(QAST_NODE_LET)};
    }

    decls.push_back(decl);
  } else {
    diagnostic << tok << "Expected a name or '[' in let declaration";
    return {mock_stmt(QAST_NODE_LET)};
  }

  if (decls.empty()) {
    diagnostic << tok << "Empty list of let declarations";
    return {mock_stmt(QAST_NODE_LET)};
  }

  std::vector<Stmt *> nodes;

  tok = next();
  if (tok.is<qPuncSemi>()) {
    for (auto &decl : decls) {
      LetDecl *let_decl = LetDecl::get(decl.first, decl.second, nullptr);
      let_decl->set_end_pos(tok.end);
      nodes.push_back(let_decl);
    }
  } else if (tok.is<qOpSet>()) {
    Expr *init = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

    tok = next();
    if (!tok.is<qPuncSemi>()) {
      diagnostic << tok
                 << "Expected a ';' after the initializer in let declaration";
    }

    LetDecl *let_decl = LetDecl::get(decls[0].first, decls[0].second, init);
    let_decl->set_end_pos(tok.end);
    nodes.push_back(let_decl);
  } else {
    diagnostic << tok << "Expected a ';' or '=' after the let declaration";
  }

  return nodes;
}