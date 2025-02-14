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

#include <core/ANSI.hh>
#include <core/Logger.hh>
#include <iostream>
#include <mutex>
#include <string_view>
#include <unordered_map>

struct LogConfig {
  bool m_use_color = true;
  bool m_show_debug = false;
};

static LogConfig GLogConfig;
static std::mutex GLogMutex;

void no3::core::SetColorMode(bool use_color) {
  std::lock_guard<std::mutex> lock(GLogMutex);
  GLogConfig.m_use_color = use_color;
}

void no3::core::SetDebugMode(bool debug) {
  std::lock_guard<std::mutex> lock(GLogMutex);
  GLogConfig.m_show_debug = debug;
}

auto no3::core::GetDebugMode() -> bool {
  std::lock_guard<std::mutex> lock(GLogMutex);
  return GLogConfig.m_show_debug;
}

void no3::core::MyLogSink::send(google::LogSeverity severity, const char*, const char* base_filename, int line,
                                const struct tm* tm, const char* message_ptr, std::size_t message_len) {
  static const std::unordered_map<google::LogSeverity, uint32_t> sev_colors = {
      {google::GLOG_INFO, ansi::FG_GREEN},
      {google::GLOG_WARNING, ansi::FG_YELLOW},
      {google::GLOG_ERROR, ansi::FG_RED},
      {google::GLOG_FATAL, ansi::FG_RED | ansi::BOLD},
  };

  static const std::unordered_map<google::LogSeverity, std::string_view> sev_prefix = {
      {google::GLOG_INFO, "I: "},
      {google::GLOG_WARNING, "W: "},
      {google::GLOG_ERROR, "E: "},
      {google::GLOG_FATAL, "F: "},
  };

  std::string_view message(message_ptr, message_len);
  bool color;
  bool debug;

  {
    std::lock_guard<std::mutex> lock(GLogMutex);
    color = GLogConfig.m_use_color;
    debug = GLogConfig.m_show_debug;
  }

  {
    static std::mutex lock;
    std::lock_guard<std::mutex> guard(lock);

    if (color) {
      using namespace ansi;
      AnsiOut out(*m_out);

      out.SetStyle(sev_colors.at(severity));
      out << sev_prefix.at(severity);
      out.SetStyle(RESET);

      if (debug) {
        out.SetStyle(FG_DEFAULT);

        out << "[" << base_filename << ":" << line << " ";

        { /* Get timestamp */
          std::array<char, 64> timestamp;
          strftime(timestamp.data(), timestamp.size(), "%b %d %H:%M:%S", tm);
          out << timestamp.data();
        }
        out << "] ";

        out.SetStyle(RESET);
      }

      out.SetStyle(sev_colors.at(severity));
      out << message;
      out.SetStyle(RESET);
      out.Newline();
    } else {
      *m_out << sev_prefix.at(severity);

      if (debug) {
        *m_out << "[" << base_filename << ":" << line << " ";

        { /* Get timestamp */
          std::array<char, 64> timestamp;
          strftime(timestamp.data(), timestamp.size(), "%b %d %H:%M:%S", tm);
          *m_out << timestamp.data();
        }

        *m_out << "] ";
      }

      *m_out << message << "\n";
      m_out->flush();
    }
  }
}

auto no3::core::MyLogSink::RedirectToStream(std::unique_ptr<std::ostream> new_stream) -> std::unique_ptr<std::ostream> {
  std::lock_guard<std::mutex> lock(GLogMutex);
  std::unique_ptr<std::ostream> old_stream = std::move(m_out);
  m_out = std::move(new_stream);
  return old_stream;
}
