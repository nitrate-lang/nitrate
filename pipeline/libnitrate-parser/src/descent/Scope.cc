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

using namespace ncc::parse;

std::string_view Parser::recurse_scope_name() {
  if (let tok = next_if(qName)) {
    return tok->as_string(&rd);
  } else {
    return "";
  }
}

std::optional<ScopeDeps> Parser::recurse_scope_deps() {
  ScopeDeps dependencies;

  if (!next_if(qPuncColn)) {
    return dependencies;
  }

  if (next_if(qPuncLBrk)) {
    while (true) {
      let tok = next();

      if (tok.is<qPuncRBrk>()) {
        return dependencies;
      }

      if (tok.is(qName)) {
        let dependency_name = tok.as_string(&rd);

        dependencies.insert(SaveString(dependency_name));

        next_if(qPuncComa);
      } else {
        diagnostic << tok << "Expected dependency name";
        break;
      }
    }
  } else {
    diagnostic << current() << "Expected '[' at start of scope dependencies";
  }

  return std::nullopt;
}

Stmt *Parser::recurse_scope_block() {
  if (next_if(qPuncSemi)) {
    return make<Block>(BlockItems(), SafetyMode::Unknown);
  } else if (next_if(qOpArrow)) {
    return recurse_block(false, true, SafetyMode::Unknown);
  } else {
    return recurse_block(true, false, SafetyMode::Unknown);
  }
}

Stmt *Parser::recurse_scope() {
  let scope_name = recurse_scope_name(rd);

  if (let implicit_dependencies = recurse_scope_deps(rd)) {
    let scope_block = recurse_scope_block();

    return make<ScopeStmt>(SaveString(scope_name), scope_block,
                           std::move(implicit_dependencies.value()));
  } else {
    diagnostic << current() << "Expected scope dependencies";
  }

  return mock_stmt(QAST_SCOPE);
}
