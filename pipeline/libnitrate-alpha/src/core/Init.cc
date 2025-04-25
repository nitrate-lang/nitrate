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

#include <nitrate-alpha/core/Init.hh>
#include <nitrate-alpha/core/Logging.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/Init.hh>

using namespace ncc;
using namespace ncc::alpha::core;

NCC_EXPORT ncc::LibraryRC<AlphaLibrarySetup> ncc::alpha::core::AlphaLibrary;

auto AlphaLibrarySetup::Init() -> bool {
  Log << Core << Trace << "libnitrate-alpha initializing...";

  if (!ncc::CoreLibrary.InitRC()) [[unlikely]] {
    Log << Core << "libnitrate-alpha failed init: libnitrate-core failed to initialize";
    return false;
  }

  if (!parse::ParseLibrary.InitRC()) [[unlikely]] {
    Log << Core << "libnitrate-alpha failed init: libnitrate-parser failed to initialize";
    return false;
  }

  /// FIXME: Depends on mangling library

  Log << Core << Trace << "libnitrate-alpha initialized";

  return true;
}

void AlphaLibrarySetup::Deinit() {
  Log << Core << Trace << "libnitrate-alpha deinitializing...";

  parse::ParseLibrary.DeinitRC();
  ncc::CoreLibrary.DeinitRC();

  Log << Core << Trace << "libnitrate-alpha deinitialized";
}

auto AlphaLibrarySetup::GetSemVersion() -> std::array<uint32_t, 3> {
  return {__TARGET_MAJOR_VERSION, __TARGET_MINOR_VERSION, __TARGET_PATCH_VERSION};
}

auto AlphaLibrarySetup::BuildId() -> ncc::BuildId {
  return {__TARGET_COMMIT_HASH, __TARGET_COMMIT_DATE, __TARGET_COMMIT_BRANCH};
}
