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

#include <IR/Q/Type.h>

bool libquixcc::ir::q::I1::print_impl(std::ostream &os, PState &state) const {
  return os << "i1", true;
}

bool libquixcc::ir::q::I8::print_impl(std::ostream &os, PState &state) const {
  return os << "i8", true;
}

bool libquixcc::ir::q::I16::print_impl(std::ostream &os, PState &state) const {
  return os << "i16", true;
}

bool libquixcc::ir::q::I32::print_impl(std::ostream &os, PState &state) const {
  return os << "i32", true;
}

bool libquixcc::ir::q::I64::print_impl(std::ostream &os, PState &state) const {
  return os << "i64", true;
}

bool libquixcc::ir::q::I128::print_impl(std::ostream &os, PState &state) const {
  return os << "i128", true;
}

bool libquixcc::ir::q::U8::print_impl(std::ostream &os, PState &state) const {
  return os << "u8", true;
}

bool libquixcc::ir::q::U16::print_impl(std::ostream &os, PState &state) const {
  return os << "u16", true;
}

bool libquixcc::ir::q::U32::print_impl(std::ostream &os, PState &state) const {
  return os << "u32", true;
}

bool libquixcc::ir::q::U64::print_impl(std::ostream &os, PState &state) const {
  return os << "u64", true;
}

bool libquixcc::ir::q::U128::print_impl(std::ostream &os, PState &state) const {
  return os << "u128", true;
}

bool libquixcc::ir::q::F32::print_impl(std::ostream &os, PState &state) const {
  return os << "f32", true;
}

bool libquixcc::ir::q::F64::print_impl(std::ostream &os, PState &state) const {
  return os << "f64", true;
}

bool libquixcc::ir::q::Void::print_impl(std::ostream &os, PState &state) const {
  return os << "void", true;
}

bool libquixcc::ir::q::Ptr::print_impl(std::ostream &os, PState &state) const {
  if (!type->print(os, state)) return false;
  return os << "*", true;
}

bool libquixcc::ir::q::Array::print_impl(std::ostream &os,
                                         PState &state) const {
  os << "[";
  if (!type->print(os, state)) return false;
  return os << "; " << size << "]", true;
}

bool libquixcc::ir::q::Vector::print_impl(std::ostream &os,
                                          libquixcc::ir::PState &state) const {
  os << "[";
  if (!type->print(os, state)) return false;
  return os << "]", true;
}

bool libquixcc::ir::q::FType::print_impl(std::ostream &os,
                                         PState &state) const {
  os << "[";
  for (size_t i = 0; i < params.size(); i++) {
    if (!params[i]->print(os, state)) return false;
    if (i + 1 < params.size())
      os << ", ";
    else if (m_variadic)
      os << ", ...";
  }
  os << "]->";
  return ret->print(os, state);
}

bool libquixcc::ir::q::Region::print_impl(std::ostream &os,
                                          libquixcc::ir::PState &state) const {
  os << "region " << name;
  return true;
}

bool libquixcc::ir::q::Union::print_impl(std::ostream &os,
                                         libquixcc::ir::PState &state) const {
  os << "union " << name;
  return true;
}

bool libquixcc::ir::q::Opaque::print_impl(std::ostream &os,
                                          libquixcc::ir::PState &state) const {
  os << "opaque " << name;
  return true;
}

bool libquixcc::ir::q::IntrinsicType::print_impl(std::ostream &os, libquixcc::ir::PState &state) const
{
  os << "#" << name;
  return true;
}