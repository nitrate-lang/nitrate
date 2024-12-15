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

#ifndef __LIBNITRATE_CODE_HH__
#define __LIBNITRATE_CODE_HH__

#include <nitrate/code.h>

#include <cstdbool>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>

namespace nitrate {
  using DiagnosticFunc = std::function<void(std::string_view message)>;

  std::future<bool> pipeline(
      std::istream &in, std::ostream &out,
      const std::vector<std::string> &options,
      std::optional<DiagnosticFunc> diag = [](std::string_view message) {
        std::cerr << message << std::endl;
      });

  template <typename T>
  static inline std::future<bool> pipeline(
      const T &in, std::string &out, const std::vector<std::string> &options,
      std::optional<DiagnosticFunc> diag = [](std::string_view message) {
        std::cerr << message << std::endl;
      }) {
    std::stringstream out_stream, in_stream(in);
    auto future = pipeline(in_stream, out_stream, options, diag);
    future.wait();

    out = out_stream.str();
    return future;
  }

}  // namespace nitrate

#endif  // __LIBNITRATE_CODE_HH__
