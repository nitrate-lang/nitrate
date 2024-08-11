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
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <core/Adapter.hh>
#include <core/Collector.hh>
#include <core/Writer.hh>

std::string qpkg::core::FormatAdapter::format(const std::string &msg, qpkg::core::Level lvl,
                                              float weight) {
  struct State {
    float m_progress;
  };

  static std::mutex m_mutex;
  std::lock_guard<std::mutex> lock(m_mutex);

  static State state = {0.0f};

  state.m_progress += weight;

  if (state.m_progress < 0.0f)
    state.m_progress = 0.0f;
  else if (state.m_progress > 1.0f)
    state.m_progress = 1.0f;

  int percent = static_cast<int>(state.m_progress * 100.0f);

  std::stringstream ss;

  ss << "[";
  if (percent < 10)
    ss << "  ";
  else if (percent < 100)
    ss << " ";
  ss << percent << "%] ";

  switch (lvl) {
    case qpkg::core::Level::DEBUG:
      ss << msg;
      break;
    case qpkg::core::Level::GOOD:
      ss << "\x1b[32;49m" << msg << "\x1b[0m";
      break;
    case qpkg::core::Level::BOLD:
      ss << "\x1b[32;49;1m" << msg << "\x1b[0m";
      break;
    case qpkg::core::Level::INFO:
      ss << msg;
      break;
    case qpkg::core::Level::WARN:
      ss << "\x1b[36;49m" << msg << "\x1b[0m";
      break;
    case qpkg::core::Level::ERROR:
      ss << "\x1b[31;49;1m" << msg << "\x1b[0m";
      break;
    case qpkg::core::Level::FATAL:
      ss << "\x1b[31;49;1;4m" << msg << "\x1b[0m";
      break;
  }

  return ss.str();
}

std::string qpkg::core::FormatAdapter::format_nocolor(const std::string &msg, qpkg::core::Level lvl,
                                                      float weight) {
  struct State {
    float m_progress;
  };

  static std::mutex m_mutex;
  std::lock_guard<std::mutex> lock(m_mutex);

  static State state = {0.0f};

  state.m_progress += weight;

  if (state.m_progress < 0.0f)
    state.m_progress = 0.0f;
  else if (state.m_progress > 1.0f)
    state.m_progress = 1.0f;

  int percent = static_cast<int>(state.m_progress * 100.0f);

  std::stringstream ss;

  ss << "[";
  if (percent < 10)
    ss << "  ";
  else if (percent < 100)
    ss << " ";
  ss << percent << "%] ";

  switch (lvl) {
    case qpkg::core::Level::DEBUG:
      ss << msg;
      break;
    case qpkg::core::Level::GOOD:
      ss << "[OKAY   " << msg << "]";
      break;
    case qpkg::core::Level::BOLD:
      ss << "[!OKAY  " << msg << "]";
      break;
    case qpkg::core::Level::INFO:
      ss << msg;
      break;
    case qpkg::core::Level::WARN:
      ss << "[WARN   " << msg << "]";
      break;
    case qpkg::core::Level::ERROR:
      ss << "[!ERROR " << msg << "]";
      break;
    case qpkg::core::Level::FATAL:
      ss << "[!PANIC " << msg << "]";
      break;
  }

  return ss.str();
}

void qpkg::core::FormatAdapter::push(const std::string &msg, qpkg::core::Level lvl, float weight,
                                     bool use_colors, bool debug) {
  if (lvl == qpkg::core::Level::DEBUG && !debug) return;

  std::string f;
  if (use_colors)
    f = format(msg, lvl, weight);
  else
    f = format_nocolor(msg, lvl, weight);

  /* We must call format because empty messages may update internal state
   * (weight). */
  if (msg.empty()) return;

  if (lvl == qpkg::core::Level::ERROR || lvl == qpkg::core::Level::FATAL) {
    log_ewrite(f);
  } else {
    log_owrite(f);
  }
}

void qpkg::core::FormatAdapter::PluginAndInit(bool debug, bool use_colors) {
  auto push_f = [use_colors, debug](const std::string &msg, qpkg::core::Level lvl, float weight) {
    push(msg, lvl, weight, use_colors, debug);
  };

  auto &glog = GlobalCollector::get();
  glog.setWriter(push_f);
}