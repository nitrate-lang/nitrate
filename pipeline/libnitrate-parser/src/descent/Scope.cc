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

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto Parser::PImpl::RecurseScopeDeps() -> std::optional<ScopeDeps> {
  ScopeDeps dependencies;

  if (!next_if(PuncColn)) {
    return dependencies;
  }

  if (next_if(PuncLBrk)) [[likely]] {
    while (true) {
      if (next_if(EofF)) [[unlikely]] {
        Log << SyntaxError << current()
            << "Unexpected EOF in scope dependencies";
        break;
      }

      if (next_if(PuncRBrk)) {
        return dependencies;
      }

      if (auto dependency_name = RecurseName(); !dependency_name->empty()) {
        dependencies.push_back(dependency_name);
      } else {
        Log << SyntaxError << next() << "Expected dependency name";
      }

      next_if(PuncComa);
    }
  } else {
    Log << SyntaxError << current()
        << "Expected '[' at start of scope dependencies";
  }

  return std::nullopt;
}

auto Parser::PImpl::RecurseScopeBlock() -> FlowPtr<Stmt> {
  if (next_if(PuncSemi)) {
    return make<Block>(BlockItems(), SafetyMode::Unknown)();
  }

  if (next_if(OpArrow)) {
    return RecurseBlock(false, true, SafetyMode::Unknown);
  }

  return RecurseBlock(true, false, SafetyMode::Unknown);
}

auto Parser::PImpl::RecurseScope() -> FlowPtr<Stmt> {
  auto scope_name = RecurseName();

  if (auto dependencies = RecurseScopeDeps()) [[likely]] {
    auto scope_block = RecurseScopeBlock();

    return make<ScopeStmt>(scope_name, scope_block, dependencies.value())();
  } else {
    Log << SyntaxError << current() << "Expected scope dependencies";
  }

  return MockStmt(QAST_SCOPE);
}
