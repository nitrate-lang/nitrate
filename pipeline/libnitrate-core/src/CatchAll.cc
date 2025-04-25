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

// This translation unit must be compiled with exceptions enabled.

#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>

using namespace ncc;

#ifndef NCC_CATCH_ALL_ENABLED_EXCEPTIONS
#error "This file requires exceptions to be enabled."
#endif

NCC_EXPORT auto ncc::detail::CatchAllAny(const std::function<std::any()>& expr) -> std::optional<std::any> {
  try {
    return expr();
  } catch (...) {
    return std::nullopt;
  }
}

NCC_EXPORT auto ncc::detail::CatchAllVoid(const std::function<void()>& expr) -> bool {
  try {
    expr();
    return true;
  } catch (...) {
    return false;
  }
}

#include <boost/throw_exception.hpp>

[[maybe_unused]] NCC_EXPORT void boost::throw_exception(std::exception const& m, boost::source_location const&) {
  Log << Trace << "boost::throw_exception: " << m.what();
  throw m;
}

[[maybe_unused]] NCC_EXPORT void boost::throw_exception(std::exception const& m) {
  Log << Trace << "boost::throw_exception: " << m.what();
  throw m;
}
