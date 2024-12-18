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

    T get() {
      if (!m_value.has_value()) {
        m_value = m_func();
      }

      return m_value.value();
    }

    void wait() { get(); }

    std::function<T()> _get_functor() { return m_func; }
  };

  static inline void default_diagnostic(std::string_view message) {
    std::cerr << message << std::endl;
  }

  LazyResult<bool> pipeline(
      std::istream &in, std::ostream &out, std::vector<std::string> options,
      std::optional<DiagnosticFunc> diag = default_diagnostic);

  template <typename T>
  static inline LazyResult<bool> pipeline(
      const T &in, std::string &out, std::vector<std::string> options,
      std::optional<DiagnosticFunc> diag = default_diagnostic) {
    std::stringstream out_stream, in_stream(in);
    auto unit = pipeline(in_stream, out_stream, std::move(options), diag);
    unit.wait();
    out.assign(out_stream.str());

    return unit;
  }

  using ChainOptions = std::vector<std::vector<std::string>>;

  LazyResult<bool> chain(
      std::istream &in, std::ostream &out, ChainOptions operations,
      std::optional<DiagnosticFunc> diag = default_diagnostic,
      bool select = false);

  static inline LazyResult<bool> chain(
      const auto &in, std::string &out, ChainOptions operations,
      std::optional<DiagnosticFunc> diag = default_diagnostic) {
    std::stringstream out_stream, in_stream(in);
    auto unit =
        chain(in_stream, out_stream, std::move(operations), diag, false);
    unit.wait();

    out.assign(out_stream.str());

    return unit;
  }
}  // namespace nitrate

#endif  // __LIBNITRATE_CODE_HH__
