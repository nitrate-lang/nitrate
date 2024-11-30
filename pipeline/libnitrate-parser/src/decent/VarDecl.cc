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
using namespace qparse;

static bool recurse_decl(qparse_t &S, qlex_tok_t tok, qlex_t &rd,
                         std::pair<std::string, Type *> &decl) {
  if (!tok.is(qName)) {
    syntax(tok, "Expected a name in var declaration");
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

  if (!recurse_type(S, rd, &type)) {
    syntax(tok, "Failed to parse type in declaration");
  }

  decl = std::make_pair(name, type);
  return true;
}

bool qparse::recurse_var(qparse_t &S, qlex_t &rd, std::vector<Stmt *> &nodes) {
  qlex_tok_t tok = next();

  std::vector<std::pair<std::string, Type *>> decls;
  if (tok.is(qName)) {
    std::pair<std::string, Type *> decl;
    if (!recurse_decl(S, tok, rd, decl)) {
      return false;
    }

    decls.push_back(decl);
  } else {
    syntax(tok, "Expected a name or '[' in var declaration");
    return false;
  }

  if (decls.empty()) {
    syntax(tok, "Empty list of var declarations");
    return false;
  }

  tok = next();
  if (tok.is<qPuncSemi>()) {
    for (auto &decl : decls) {
      VarDecl *var_decl = VarDecl::get(decl.first, decl.second, nullptr);
      var_decl->set_end_pos(tok.end);
      nodes.push_back(var_decl);
    }
  } else if (tok.is<qOpSet>()) {
    Expr *init = nullptr;
    if (!recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &init) || !init) {
      syntax(tok, "Failed to parse initializer in var declaration");
    }

    tok = next();
    if (!tok.is<qPuncSemi>()) {
      syntax(tok, "Expected a ';' after the initializer in var declaration");
    }

    VarDecl *var_decl = VarDecl::get(decls[0].first, decls[0].second, init);
    var_decl->set_end_pos(tok.end);
    nodes.push_back(var_decl);
  } else {
    syntax(tok, "Expected a ';' or '=' after the var declaration");
  }

  return true;
}