////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * NITRATE PACKAGE MANAGER - The official app for the Nitrate language. ///
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

thread_local qpkg::ansi::AnsiCerr qpkg::ansi::acout;

qpkg::ansi::AnsiCerr::AnsiCerr() { style = Style::FG_DEFAULT | Style::BG_DEFAULT; }

qpkg::ansi::AnsiCerr qpkg::ansi::AnsiCerr::newline() {
  std::cerr << std::endl;
  return *this;
}

qpkg::ansi::AnsiCerr &qpkg::ansi::AnsiCerr::operator<<(const std::string &str) {
  std::stringstream ansi_str;

  ansi_str << "\x1b[";
  static const std::string_view reset = "\x1b[0m";

  switch (style & Style::COLOR_MASK) {
    case Style::FG_BLACK:
      ansi_str << "30";
      break;
    case Style::FG_RED:
      ansi_str << "31";
      break;
    case Style::FG_GREEN:
      ansi_str << "32";
      break;
    case Style::FG_YELLOW:
      ansi_str << "33";
      break;
    case Style::FG_BLUE:
      ansi_str << "34";
      break;
    case Style::FG_PURPLE:
      ansi_str << "35";
      break;
    case Style::FG_CYAN:
      ansi_str << "36";
      break;
    case Style::FG_WHITE:
      ansi_str << "37";
      break;
    case Style::FG_DEFAULT:
      ansi_str << "39";
      break;
    default:
      break;
  };

  switch (style & Style::BG_COLOR_MASK) {
    case Style::BG_BLACK:
      ansi_str << ";40";
      break;
    case Style::BG_RED:
      ansi_str << ";41";
      break;
    case Style::BG_GREEN:
      ansi_str << ";42";
      break;
    case Style::BG_YELLOW:
      ansi_str << ";43";
      break;
    case Style::BG_BLUE:
      ansi_str << ";44";
      break;
    case Style::BG_PURPLE:
      ansi_str << ";45";
      break;
    case Style::BG_CYAN:
      ansi_str << ";46";
      break;
    case Style::BG_WHITE:
      ansi_str << ";47";
      break;
    case Style::BG_DEFAULT:
      ansi_str << ";49";
      break;
    default:
      break;
  }

  if ((style & Style::BOLD) != 0) ansi_str << ";1";
  if ((style & Style::ILTALIC) != 0) ansi_str << ";3";
  if ((style & Style::UNDERLINE) != 0) ansi_str << ";4";
  if ((style & Style::STRIKE) != 0) ansi_str << ";9";

  ansi_str << "m";

  std::cerr << ansi_str.str() << str << reset;
  std::cerr.flush();

  return *this;
}