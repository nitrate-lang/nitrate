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

#include <core/Context.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/Context.hh>
#include <sstream>

using namespace ncc::parse;

std::string ncc::parse::mint_clang16_message(const DiagMessage &msg) {
  std::stringstream ss;
  ss << "\x1b[37;1m" << qlex_filename(diagnostic->lexer) << ":";
  uint32_t line = qlex_line(diagnostic->lexer, qlex_begin(&msg.tok));
  uint32_t col = qlex_col(diagnostic->lexer, qlex_begin(&msg.tok));

  if (line != QLEX_EOFF) {
    ss << line << ":";
  } else {
    ss << "?:";
  }

  if (col != QLEX_EOFF) {
    ss << col << ":\x1b[0m ";
  } else {
    ss << "?:\x1b[0m ";
  }

  ss << "\x1b[37;1m" << msg.msg << " [";

  ss << "SyntaxError";

  ss << "]\x1b[0m";

  uint32_t offset;
  char *snippet = qlex_snippet(diagnostic->lexer, msg.tok, &offset);
  if (!snippet) {
    return ss.str();
  }

  ss << "\n" << snippet << "\n";
  for (uint32_t i = 0; i < offset; i++) {
    ss << " ";
  }
  ss << "\x1b[32;1m^\x1b[0m";
  free(snippet);

  return ss.str();
}
