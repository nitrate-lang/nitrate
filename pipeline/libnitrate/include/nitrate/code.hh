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
#include <nitrate/stream.h>

#include <cstdbool>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace nitrate {
  class Stream final {
    nit_stream_t *m_stream;

    Stream(const Stream &) = delete;
    Stream &operator=(const Stream &) = delete;

  public:
    Stream(FILE *file, bool auto_close = false) {
      m_stream = nit_from(file, auto_close);
    }

    Stream(const std::vector<FILE *> &merge_files, bool auto_close = false) {
      m_stream = nit_njoin(auto_close, merge_files.size(), merge_files.data());
    }

    Stream(std::span<FILE *> merge_files, bool auto_close = false) {
      m_stream = nit_njoin(auto_close, merge_files.size(), merge_files.data());
    }

    Stream(Stream &&other) {
      this->m_stream = other.m_stream;
      other.m_stream = nullptr;
    }

    ~Stream() {
      if (m_stream) {
        nit_fclose(m_stream);
      }
    };

    bool is_open() const { return m_stream != nullptr; }

    nit_stream_t *get() const { return m_stream; }
  };

  using DiagnosticFunc =
      std::function<void(std::string_view message, std::string_view by)>;

  std::future<bool> pipeline(
      std::shared_ptr<Stream> in, std::shared_ptr<Stream> out,
      const std::vector<std::string> &options,
      std::optional<DiagnosticFunc> diag = [](std::string_view message,
                                              std::string_view) {
        std::cerr << message;
        std::cerr.flush();
      });

  std::future<bool> pipeline(
      Stream in, Stream out, const std::vector<std::string> &options,
      std::optional<DiagnosticFunc> diag = [](std::string_view message,
                                              std::string_view) {
        std::cerr << message;
        std::cerr.flush();
      });
}  // namespace nitrate

#endif  // __LIBNITRATE_CODE_HH__
