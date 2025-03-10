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

auto GeneralParser::PImpl::RecurseScopeDeps() -> std::vector<string> {
  std::vector<string> dependencies;

  if (!NextIf<PuncLBrk>()) {
    return dependencies;
  }

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << SyntaxError << Current() << "Unexpected EOF in scope dependencies";
      return dependencies;
    }

    if (NextIf<PuncRBrk>()) {
      return dependencies;
    }

    if (auto dependency_name = RecurseName()) {
      dependencies.push_back(dependency_name);
    } else {
      Log << SyntaxError << Next() << "Expected dependency name";
    }

    NextIf<PuncComa>();
  }
}

auto GeneralParser::PImpl::RecurseScopeBlock() -> FlowPtr<Expr> {
  if (NextIf<PuncSemi>()) {
    return m_fac.CreateBlock();
  }

  return RecurseBlock(true, false, BlockMode::Unknown);
}

auto GeneralParser::PImpl::RecurseScope() -> FlowPtr<Expr> {
  auto scope_name = RecurseName();
  bool has_colon = NextIf<PuncColn>().has_value();
  auto dependencies = RecurseScopeDeps();

  bool is_colon_required = !dependencies.empty() && !scope_name->empty();
  if (!has_colon && is_colon_required) [[unlikely]] {
    Log << SyntaxError << Current() << "Expected ':' after scope name";
  }

  auto scope_block = RecurseScopeBlock();

  return m_fac.CreateScope(scope_name, scope_block, dependencies);
}
