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

  if (!NextIf<PuncColn>()) {
    return dependencies;
  }

  if (NextIf<PuncLBrk>()) [[likely]] {
    while (true) {
      if (NextIf<EofF>()) [[unlikely]] {
        Log << SyntaxError << Current() << "Unexpected EOF in scope dependencies";
        break;
      }

      if (NextIf<PuncRBrk>()) {
        return dependencies;
      }

      if (auto dependency_name = RecurseName(); !dependency_name.empty()) {
        dependencies.push_back(dependency_name);
      } else {
        Log << SyntaxError << Next() << "Expected dependency name";
      }

      NextIf<PuncComa>();
    }
  } else {
    Log << SyntaxError << Current() << "Expected '[' at start of scope dependencies";
  }

  return std::nullopt;
}

auto Parser::PImpl::RecurseScopeBlock() -> FlowPtr<Stmt> {
  if (NextIf<PuncSemi>()) {
    return CreateNode<Block>(BlockItems(), SafetyMode::Unknown)();
  }

  if (NextIf<OpArrow>()) {
    return RecurseBlock(false, true, SafetyMode::Unknown);
  }

  return RecurseBlock(true, false, SafetyMode::Unknown);
}

auto Parser::PImpl::RecurseScope() -> FlowPtr<Stmt> {
  auto scope_name = RecurseName();

  if (auto dependencies = RecurseScopeDeps()) [[likely]] {
    auto scope_block = RecurseScopeBlock();

    return CreateNode<Scope>(scope_name, scope_block, dependencies.value())();
  } else {
    Log << SyntaxError << Current() << "Expected scope dependencies";
  }

  return MockStmt(QAST_SCOPE);
}
