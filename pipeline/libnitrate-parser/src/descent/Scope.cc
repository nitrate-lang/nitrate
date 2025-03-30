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
#include <nitrate-parser/ASTStmt.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

static auto RecurseScopeDeps(GeneralParser::Context& m) -> std::vector<string> {
  std::vector<string> dependencies;

  if (!m.NextIf<PuncLBrk>()) {
    return dependencies;
  }

  while (true) {
    if (m.IsEof()) [[unlikely]] {
      Log << ParserSignal << m.Current() << "Unexpected EOF in scope dependencies";
      return dependencies;
    }

    if (m.NextIf<PuncRBrk>()) {
      return dependencies;
    }

    if (auto dependency_name = m.RecurseName()) {
      dependencies.push_back(dependency_name);
    } else {
      Log << ParserSignal << m.Next() << "Expected dependency name";
    }

    m.NextIf<PuncComa>();
  }
}

static auto RecurseScopeBlock(GeneralParser::Context& m) -> FlowPtr<Expr> {
  if (m.NextIf<PuncSemi>()) {
    return m.CreateBlock();
  }

  return m.RecurseBlock(true, false, BlockMode::Unknown);
}

auto GeneralParser::Context::RecurseScope() -> FlowPtr<Expr> {
  auto scope_name = RecurseName();
  bool has_colon = NextIf<PuncColn>().has_value();
  auto dependencies = RecurseScopeDeps(*this);

  bool is_colon_required = !dependencies.empty() && !scope_name->empty();
  if (!has_colon && is_colon_required) [[unlikely]] {
    Log << ParserSignal << Current() << "Expected ':' after scope name";
  }

  auto scope_block = RecurseScopeBlock(*this);

  return CreateScope(scope_name, scope_block, dependencies);
}
