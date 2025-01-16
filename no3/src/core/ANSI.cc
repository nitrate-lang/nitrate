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

#include <core/ANSI.hh>
#include <iostream>

no3::ansi::AnsiOut no3::ansi::AnsiOut::Newline() {
  m_out << "\n";
  m_out.flush();
  return *this;
}

no3::ansi::AnsiOut &no3::ansi::AnsiOut::operator<<(const std::string &str) {
  std::stringstream ansi_str;

  ansi_str << "\x1b[";
  static const std::string_view reset = "\x1b[0m";

  switch (m_style & COLOR_MASK) {
    case FG_BLACK:
      ansi_str << "30";
      break;
    case FG_RED:
      ansi_str << "31";
      break;
    case FG_GREEN:
      ansi_str << "32";
      break;
    case FG_YELLOW:
      ansi_str << "33";
      break;
    case FG_BLUE:
      ansi_str << "34";
      break;
    case FG_PURPLE:
      ansi_str << "35";
      break;
    case FG_CYAN:
      ansi_str << "36";
      break;
    case FG_WHITE:
      ansi_str << "37";
      break;
    case FG_DEFAULT:
      ansi_str << "39";
      break;
    default:
      break;
  };

  switch (m_style & BG_COLOR_MASK) {
    case BG_BLACK:
      ansi_str << ";40";
      break;
    case BG_RED:
      ansi_str << ";41";
      break;
    case BG_GREEN:
      ansi_str << ";42";
      break;
    case BG_YELLOW:
      ansi_str << ";43";
      break;
    case BG_BLUE:
      ansi_str << ";44";
      break;
    case BG_PURPLE:
      ansi_str << ";45";
      break;
    case BG_CYAN:
      ansi_str << ";46";
      break;
    case BG_WHITE:
      ansi_str << ";47";
      break;
    case BG_DEFAULT:
      ansi_str << ";49";
      break;
    default:
      break;
  }

  if ((m_style & BOLD) != 0) {
    ansi_str << ";1";
  }
  if ((m_style & ILTALIC) != 0) {
    ansi_str << ";3";
  }
  if ((m_style & UNDERLINE) != 0) {
    ansi_str << ";4";
  }
  if ((m_style & STRIKE) != 0) {
    ansi_str << ";9";
  }

  ansi_str << "m";

  m_out << ansi_str.str() << str << reset;
  m_out.flush();

  return *this;
}

bool no3::ansi::IsUsingColors() {
  const char *no_color = getenv("NO_COLOR");
  return no_color == nullptr || no_color[0] == '\0';
}