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

auto GeneralParser::PImpl::RecurseEscapeBlock() -> void {
  if (!NextIf<PuncLPar>()) {
    Log << ParserSignal << Current() << "Expected '(' to open the escape block annotation";
    return;
  }

  if (!NextIf<Text>()) {
    Log << ParserSignal << Current() << "Expected a string literal token for escape block annotation";
    return;
  }

  if (!NextIf<PuncRPar>()) {
    Log << ParserSignal << Current() << "Expected ')' to close the escape block annotation";
    return;
  }

  if (!NextIf<PuncLCur>()) {
    Log << ParserSignal << Current() << "Expected '{' to open the escape block";
    return;
  }

  size_t depth = 1;

  while (depth > 0) {
    if (m_rd.IsEof()) {
      Log << ParserSignal << Current() << "Unexpected end of file while parsing 'escape_block'";
      return;
    }

    const auto token = Next();
    if (token.Is<PuncLCur>()) {
      depth++;
    } else if (token.Is<PuncRCur>()) {
      depth--;
    }
  }
}
