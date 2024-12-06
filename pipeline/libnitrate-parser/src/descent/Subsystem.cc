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

#include <nitrate-parser/Node.h>

#include <decent/Recurse.hh>

using namespace npar;

static std::optional<ScopeDeps> recurse_scope_deps(qlex_t &rd) {
  ScopeDeps dependencies;

  if (!peek().is<qPuncColn>()) {
    return dependencies;
  }

  if (auto tok = (next(), next()); tok.is<qPuncLBrk>()) {
    while (true) {
      tok = next();

      if (tok.is<qPuncRBrk>()) {
        return dependencies;
      }

      if (tok.is(qName)) {
        let dependency_name = tok.as_string(&rd);

        dependencies.insert(dependency_name);

        if (peek().is<qPuncComa>()) {
          next();
        }
      } else {
        diagnostic << tok << "Expected dependency name";
        break;
      }
    }
  } else {
    diagnostic << tok << "Expected '[' at start of scope dependencies";
  }

  return std::nullopt;
}

static Stmt *recurse_scope_block(npar_t &S, qlex_t &rd) {
  let tok = peek();

  if (tok.is<qPuncLCur>()) {
    return recurse_block(S, rd, true, false);
  } else if (tok.is<qOpArrow>()) {
    return (next(), recurse_block(S, rd, false, true));
  } else {
    return Block::get();
  }
}

npar::Stmt *npar::recurse_scope(npar_t &S, qlex_t &rd) {
  if (let tok = peek(); tok.is(qName)) {
    let scope_name = (next(), tok.as_string(&rd));

    if (let implicit_dependencies = recurse_scope_deps(rd)) {
      let scope_block = recurse_scope_block(S, rd);

      let stmt = ScopeDecl::get(scope_name, scope_block,
                                std::move(implicit_dependencies.value()));
      stmt->set_end_pos(scope_block->get_end_pos());

      return stmt;
    } else {
      diagnostic << tok << "Expected scope dependencies";
    }
  } else {
    diagnostic << tok << "Expected name for scope";
  }

  return mock_stmt(QAST_NODE_SUBSYSTEM);
}
