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

#include <core/cli/Interpreter.hh>

using namespace ncc;

auto no3::Interpreter::PImpl::CommandClean(ConstArguments, const MutArguments& argv) -> bool {
  (void)argv;

  /**
   * .no3
   * ├── cache
   * │   ├── metadata
   * │   │   ├── dependency-info.db
   * │   │   └── package-index.db
   * │   ├── optimization
   * │   │   ├── alpha-conv
   * │   │   ├── beta-conv
   * │   │   ├── gamma-conv
   * │   │   ├── llvm-conv
   * │   │   ├── ptree-conv
   * │   │   └── this.db
   * │   └── translation
   * │       ├── alpha-conv
   * │       ├── beta-conv
   * │       ├── gamma-conv
   * │       ├── llvm-conv
   * │       ├── ptree-conv
   * │       └── this.db
   * ├── emit
   * │   ├── debug
   * │   ├── rapid
   * │   └── release
   * └── unit
   */

  /**
   * Instead of complex command line options, support the execution of Lua scripts
   * to perform predicate-based cache eviction. This will allow users to define
   * their own cache eviction policies. The default will be to evict all package cache
   * entries for simplicity.

  BEGIN SAMPLE SCRIPT
    local no3 = {}
    no3.clean = {}

    no3.clean.evict_if = function(cache_name, object)
    if cache_name == "translation" then
      return true
    end

    local size = object.size()
    local last_used = object.last_used().timestamp()

    local size_limit = 65535
    local max_unused_age = os.time() - (60 * 60 * 24 * 3) -- 3 days

    return size > size_limit or last_used < max_unused_age
    end

    no3.clean.evict_if("test", {
    size = function()
      print("size-of")
      return 65536
    end
    })
  END SAMPLE SCRIPT
  */

  /// TODO: Implement package cleaning

  Log << "Package cleaning is not implemented yet.";

  return false;
}
