////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * NITRATE PACKAGE MANAGER - The official app for the Nitrate language. ///
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
  bool use_color = true;
  bool show_debug = false;
};

static LogConfig g_log_config;
static std::mutex g_log_mutex;

void qpkg::core::SetColorMode(bool use_color) {
  std::lock_guard<std::mutex> lock(g_log_mutex);
  g_log_config.use_color = use_color;
}

void qpkg::core::SetDebugMode(bool debug) {
  std::lock_guard<std::mutex> lock(g_log_mutex);
  g_log_config.show_debug = debug;
}

void qpkg::core::MyLogSink::send(google::LogSeverity severity, const char*,
                                 const char* base_filename, int line, const struct tm* tm,
                                 const char* message_ptr, std::size_t message_len) {
  static const std::unordered_map<google::LogSeverity, ansi::Style> sev_colors = {
      {google::GLOG_INFO, ansi::Style::FG_GREEN},
      {google::GLOG_WARNING, ansi::Style::FG_YELLOW},
      {google::GLOG_ERROR, ansi::Style::FG_RED},
      {google::GLOG_FATAL, ansi::Style::FG_RED | ansi::Style::BOLD},
  };

  static const std::unordered_map<google::LogSeverity, std::string_view> sev_prefix = {
      {google::GLOG_INFO, "I: "},
      {google::GLOG_WARNING, "W: "},
      {google::GLOG_ERROR, "E: "},
      {google::GLOG_FATAL, "F: "},
  };

  std::string_view message(message_ptr, message_len);
  bool color, debug;

  {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    color = g_log_config.use_color;
    debug = g_log_config.show_debug;
  }

  {
    static std::mutex lock;
    std::lock_guard<std::mutex> guard(lock);

    if (color) {
      using namespace ansi;
      AnsiCerr out;

      out.set_style(sev_colors.at(severity));
      out << sev_prefix.at(severity);
      out.set_style(Style::RESET);

      if (debug) {
        out.set_style(Style::FG_DEFAULT);

        out << "[" << base_filename << ":" << line << " ";

        { /* Get timestamp */
          char timestamp[64];
          strftime(timestamp, sizeof(timestamp), "%b %d %H:%M:%S", tm);
          out << timestamp;
        }
        out << "] ";

        out.set_style(Style::RESET);
      }

      out.set_style(sev_colors.at(severity));
      out << message;
      out.set_style(Style::RESET);
      out << std::endl;
    } else {
#define out std::cerr

      out << sev_prefix.at(severity);

      if (debug) {
        out << "[" << base_filename << ":" << line << " ";

        { /* Get timestamp */
          char timestamp[64];
          strftime(timestamp, sizeof(timestamp), "%b %d %H:%M:%S", tm);
          out << timestamp;
        }

        out << "] ";
      }

      out << message << std::endl;
    }
  }
}
