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
#include <iostream>
#include <optional>
#include <sstream>
#include <string_view>

namespace nitrate {
  using DiagnosticFunc = std::function<void(std::string_view message)>;

  template <typename T>
  class LazyResult {
    std::function<T()> m_func;
    std::optional<T> m_value;

  public:
    LazyResult(std::function<T()> func) : m_func(func) {}

    auto Get() -> T {
      if (!m_value.has_value()) {
        m_value = m_func();
      }

      return m_value.value();
    }

    void Wait() { Get(); }

    auto GetFunctor() -> std::function<T()> { return m_func; }
  };

  auto Pipeline(std::istream &in, std::ostream &out, std::vector<std::string> options) -> LazyResult<bool>;

  template <typename T>
  static inline auto Pipeline(const T &in, std::string &out, std::vector<std::string> options) -> LazyResult<bool> {
    std::stringstream out_stream;
    std::stringstream in_stream(in);
    auto unit = Pipeline(in_stream, out_stream, std::move(options));
    unit.Wait();
    out.assign(out_stream.str());

    return unit;
  }

  static inline auto Pipeline(std::string_view in, std::string &out,
                              std::vector<std::string> options) -> LazyResult<bool> {
    std::stringstream out_stream;
    std::stringstream in_stream((std::string(in)));
    auto unit = Pipeline(in_stream, out_stream, std::move(options));
    unit.Wait();
    out.assign(out_stream.str());

    return unit;
  }

  using ChainOptions = std::vector<std::vector<std::string>>;

  auto Chain(std::istream &in, std::ostream &out, ChainOptions operations, bool select = false) -> LazyResult<bool>;

  static inline auto Chain(const auto &in, std::string &out, ChainOptions operations) -> LazyResult<bool> {
    std::stringstream out_stream;
    std::stringstream in_stream(in);
    auto unit = Chain(in_stream, out_stream, std::move(operations), false);
    unit.Wait();

    out.assign(out_stream.str());

    return unit;
  }

  static inline auto Chain(std::string_view in, std::string &out, ChainOptions operations) -> LazyResult<bool> {
    std::stringstream out_stream;
    std::stringstream in_stream((std::string(in)));
    auto unit = Chain(in_stream, out_stream, std::move(operations), false);
    unit.Wait();

    out.assign(out_stream.str());

    return unit;
  }
}  // namespace nitrate

#endif  // __LIBNITRATE_CODE_HH__
