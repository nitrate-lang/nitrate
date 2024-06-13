////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <IR/Q/Variable.h>

bool libquixcc::ir::q::Local::print_impl(std::ostream &os,
                                         PState &state) const {
  os << "%" << name << "(";
  if (!type->print(os, state)) return false;
  os << ") = ";

  if (value) {
    if (!value->print(os, state)) return false;
  } else {
    os << "undef";
  }

  return true;
}

bool libquixcc::ir::q::Global::print_impl(std::ostream &os,
                                          PState &state) const {
  if (_extern) os << "extern ";

  os << "@" << name << "(";
  if (!type->print(os, state)) return false;
  os << ") = ";

  if (value) {
    if (!value->print(os, state)) return false;
  } else {
    os << "undef";
  }

  return true;
}

bool libquixcc::ir::q::Number::print_impl(std::ostream &os,
                                          PState &state) const {
  os << value;
  return true;
}

static std::string escape(const std::string &str) {
  std::string out;
  for (char c : str) {
    switch (c) {
      case '\n':
        out += "\\n";
        break;
      case '\t':
        out += "\\t";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\0':
        out += "\\0";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\"':
        out += "\\\"";
        break;
      default:
        if (c >= 32 && c <= 126)
          out += c;
        else
          out += "\\x" + std::to_string((int)c);
        break;
    }
  }
  return out;
}

bool libquixcc::ir::q::String::print_impl(std::ostream &os,
                                          PState &state) const {
  os << "\"" << escape(value) << "\"";
  return true;
}

bool libquixcc::ir::q::Char::print_impl(std::ostream &os,
                                        libquixcc::ir::PState &state) const {
  os << "'" << escape(value) << "'";
  return true;
}

bool libquixcc::ir::q::List::print_impl(std::ostream &os,
                                        libquixcc::ir::PState &state) const {
  os << "[";
  for (size_t i = 0; i < values.size(); i++) {
    if (!values[i]->print(os, state)) return false;

    if (i != values.size() - 1) os << ", ";
  }
  os << "]";
  return true;
}