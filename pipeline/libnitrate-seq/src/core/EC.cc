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

#include <core/EC.hh>

using namespace ncc;
using namespace ncc::seq;

static const std::array<std::string_view, Sev_MaxValue + 1> ANSI_PREFIXES = {
    "\x1b[1mtrace:\x1b[0m ",         "\x1b[1mdebug:\x1b[0m ",      "\x1b[37;1minfo:\x1b[0m ",
    "\x1b[37;1mnotice:\x1b[0m ",     "\x1b[35;1mwarning:\x1b[0m ", "\x1b[31;1merror:\x1b[0m ",
    "\x1b[31;1;4mcritical:\x1b[0m ", "\x1b[31;1;4malert:\x1b[0m ", "\x1b[31;1;4memergency:\x1b[0m "};

NCC_EXPORT auto ncc::seq::ec::Formatter(std::string_view msg, Sev sev) -> std::string {
  if (sev == Sev::Raw) {
    return std::string(msg);
  }

  std::stringstream ss;

  ss << "\x1b[0m\x1b[37;1m[\x1b[0m\x1b[32;1mMeta \x1b[0m\x1b[37;1m]: " << ANSI_PREFIXES[sev] << msg;

  return ss.str();
}
