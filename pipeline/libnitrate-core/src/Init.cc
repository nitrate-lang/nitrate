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

#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>

using namespace ncc;

CPP_EXPORT LibraryRC<CoreLibrarySetup> ncc::CoreLibrary;

CPP_EXPORT bool CoreLibrarySetup::Init() {
  // Nothing to do here for now.

  qcore_print(QCORE_DEBUG, "Initialized Nitrate Core Library");

  return true;
}

CPP_EXPORT void CoreLibrarySetup::Deinit() {
  qcore_print(QCORE_DEBUG, "Deinitialing Nitrate Core Library...");

  /* After nitrate-core is deinitialized, all str_aliases are invalid. */
  StringMemory::Clear();
}

CPP_EXPORT std::string_view CoreLibrarySetup::GetVersionId() {
  return __TARGET_VERSION;
}

#define BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>
#include <iostream>

namespace boost {
  CPP_EXPORT void throw_exception(std::exception const& m,
                                  boost::source_location const&) {
    std::cerr << "boost::throw_exception: " << m.what();
    std::terminate();
  }

  CPP_EXPORT void throw_exception(std::exception const& m) {
    std::cerr << "boost::throw_exception: " << m.what();
    std::terminate();
  }
}  // namespace boost
