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
#include <nitrate-core/SmartLock.hh>
#include <nitrate-core/String.hh>

using namespace ncc;

NCC_EXPORT LibraryRC<CoreLibrarySetup> ncc::CoreLibrary;
NCC_EXPORT std::atomic<bool> ncc::EnableSync = true;

NCC_EXPORT auto CoreLibrarySetup::Init() -> bool {
  // Nothing to do here for now.

  Log << Debug << "Initialized Nitrate Core Library";

  return true;
}

NCC_EXPORT void CoreLibrarySetup::Deinit() {
  Log << Debug << "Deinitialing Nitrate Core Library...";

  Log.UnsubscribeAll();
  Log.ClearFilters();
  Log.Enable();

  String::ResetInstances();
}

NCC_EXPORT auto CoreLibrarySetup::GetSemVersion() -> std::array<uint32_t, 3> {
  return {__TARGET_MAJOR_VERSION, __TARGET_MINOR_VERSION, __TARGET_PATCH_VERSION};
}

NCC_EXPORT auto CoreLibrarySetup::BuildId() -> ncc::BuildId {
  return {__TARGET_COMMIT_HASH, __TARGET_COMMIT_DATE, __TARGET_COMMIT_BRANCH};
}

#define BOOST_NO_EXCEPTIONS
#include <boost/throw_exception.hpp>
#include <iostream>

[[maybe_unused]] NCC_EXPORT void boost::throw_exception(std::exception const& m, boost::source_location const&) {
  std::cerr << "boost::throw_exception: " << m.what();
  std::terminate();
}

[[maybe_unused]] NCC_EXPORT void boost::throw_exception(std::exception const& m) {
  std::cerr << "boost::throw_exception: " << m.what();
  std::terminate();
}
