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

#include <cmath>
#include <core/ANSI.hh>
#include <dev/bench/bench.hh>
#include <iomanip>
#include <iostream>

#define PROGRAM_STRING "Quix Compiler Suite (qpkg) - " QUIX_BUILD_ID
#define COPYRIGHT_STRING "Copyright (C) 2024 Wesley C. Jones"

using namespace qpkg::ansi;

qpkg::dev::bench::Progress::Progress(const std::string &title) {
  m_title = title;

  acout |= Style::BOLD | Style::FG_WHITE | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " " PROGRAM_STRING << std::endl;

  acout |= Style::BOLD | Style::FG_WHITE | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " " COPYRIGHT_STRING << std::endl;

  acout << std::endl;

  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " Running benchmarks for ";
  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << title;
  acout |= Style::RESET;
  acout << "..." << std::endl;
  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " This may take a while. Status updates will be printed" << std::endl;
  acout << "  periodically.\n" << std::endl;

  acout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void qpkg::dev::bench::Progress::print(double percent, const std::string &msg) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2) << percent * 100.0;
  std::string percent_str = ss.str();

  acout << "┃ ";
  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " Progress: ";
  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << percent_str << "%";
  acout |= Style::RESET;

  if (!msg.empty())
    acout << "\t- " << msg;

  acout << std::endl;
}

void qpkg::dev::bench::Progress::update(double percent, const std::string &msg) {
  auto now = std::chrono::high_resolution_clock::now();
  if (now - m_last_print < m_print_interval) {
    m_queue.emplace(msg, percent);
    return;
  }
  m_last_print = now;

  while (!m_queue.empty()) {
    auto [msg, percent] = m_queue.front();
    m_queue.pop();
    if (!msg.empty()) {
      if (msg == m_last_msg)
        continue;
      print(percent, msg);
      m_last_msg = msg;
    }
  }

  print(percent, msg);
}

void qpkg::dev::bench::Progress::begin_result(qpkg::dev::bench::Progress::Result type) {
  switch (type) {
  case Result::THROUGHPUT:
    acout << "\n┏━━━━━┫ ";
    acout |= Style::BOLD | Style::FG_YELLOW | Style::BG_DEFAULT;
    acout << "THROUGHPUT PERFORMANCE";
    acout |= Style::RESET;
    acout << " ┣━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    break;
  case Result::MEMORY:
    acout << "\n┏━━━━━┫ ";
    acout |= Style::BOLD | Style::FG_YELLOW | Style::BG_DEFAULT;
    acout << "MEMORY USAGE";
    acout |= Style::RESET;
    acout << " ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    break;
  case Result::TIME:
    acout << "\n┏━━━━━┫ ";
    acout |= Style::BOLD | Style::FG_YELLOW | Style::BG_DEFAULT;
    acout << "SPEED";
    acout |= Style::RESET;
    acout << " ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    break;
  }
}

void qpkg::dev::bench::Progress::result(const std::string &msg) {
  acout << "┃ ";
  acout |= Style::BOLD | Style::FG_PURPLE | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " " << msg << std::endl;
}

void qpkg::dev::bench::Progress::end_result() {
  acout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
}

void qpkg::dev::bench::Progress::done(const std::string &outfile_name) {
  bool do_last = false;
#define F_CMP(a, b) (fabs((a) - (b)) < 1e-6)

  while (!m_queue.empty()) {
    auto [msg, percent] = m_queue.front();
    m_queue.pop();
    if (!msg.empty() || m_queue.empty()) {
      if (msg != m_last_msg) {
        print(percent, msg);
        m_last_msg = msg;
      }
    }
    if (m_queue.empty() && !F_CMP(percent, 1.0))
      do_last = true;
  }

  if (do_last)
    print(1.0, "");

  acout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;

  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " Done running benchmarks for ";
  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << m_title;
  acout |= Style::RESET;
  acout << "." << std::endl;

  if (!outfile_name.empty()) {
    acout |= Style::BOLD | Style::FG_GREEN | Style::BG_DEFAULT;
    acout << "*";
    acout |= Style::RESET;
    acout << " Results saved to ";
    acout |= Style::BOLD | Style::FG_GREEN | Style::BG_DEFAULT;
    acout << outfile_name;
    acout |= Style::RESET;
    acout << "." << std::endl;
  }
}