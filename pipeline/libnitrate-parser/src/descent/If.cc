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

FlowPtr<Stmt> Parser::PImpl::RecurseIfThen() {
  if (next_if(OpArrow)) {
    return RecurseBlock(false, true, SafetyMode::Unknown);
  } else {
    return RecurseBlock(true, false, SafetyMode::Unknown);
  }
}

NullableFlowPtr<Stmt> Parser::PImpl::RecurseIfElse() {
  if (next_if(Else)) {
    if (next_if(OpArrow)) {
      return RecurseBlock(false, true, SafetyMode::Unknown);
    } else if (next_if(If)) {
      return RecurseIf();
    } else {
      return RecurseBlock(true, false, SafetyMode::Unknown);
    }
  } else {
    return std::nullopt;
  }
}

FlowPtr<Stmt> Parser::PImpl::RecurseIf() {
  auto cond = RecurseExpr({
      Token(Punc, PuncLCur),
      Token(Oper, OpArrow),
  });
  auto then = RecurseIfThen();
  auto ele = RecurseIfElse();

  return make<IfStmt>(cond, then, ele)();
}
