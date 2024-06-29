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

#include <core/Config.hh>

#if QPKG_DEV_TOOLS

#include <cmath>
#include <core/ANSI.hh>
#include <dev/test/test.hh>
#include <iomanip>
#include <iostream>

#define PROGRAM_STRING "Quix Compiler Suite (qpkg) - " QUIX_BUILD_ID
#define COPYRIGHT_STRING "Copyright (C) 2024 Wesley C. Jones"

using namespace qpkg::ansi;

qpkg::dev::test::Progress::Progress(const std::string &title) {
  m_title = title;
  m_all_okay = true;

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
  acout << " Running tests for ";
  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << title;
  acout |= Style::RESET;
  acout << "..." << std::endl;
  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " This may take a while. Status updates will be printed"
        << std::endl;
  acout << "  periodically.\n" << std::endl;
}

void qpkg::dev::test::Progress::begin_result() {
  acout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void qpkg::dev::test::Progress::result(const std::string &msg, Result type) {
  if (type == Result::FAIL) {
    m_all_okay = false;
  }

  acout << "┃ ";
  switch (type) {
    case Result::PASS:
      acout |= Style::BOLD | Style::FG_GREEN | Style::BG_DEFAULT;
      acout << "PASS";
      acout |= Style::RESET;
      break;
    case Result::FAIL:
      acout |= Style::BOLD | Style::FG_RED | Style::BG_DEFAULT;
      acout << "FAIL";
      acout |= Style::RESET;
      break;
  }

  acout << " " << msg << std::endl;
}

void qpkg::dev::test::Progress::end_result() {
  if (m_all_okay) {
    acout << "┃\n┃ ";
    acout |= Style::BOLD | Style::FG_GREEN | Style::BG_DEFAULT;
    acout << "* ";
    acout |= Style::RESET;
    acout |= Style::BOLD;
    acout << "All tests passed!" << std::endl;
    acout |= Style::RESET;
  } else {
    acout << "┃\n┃ ";
    acout |= Style::BOLD | Style::FG_RED | Style::BG_DEFAULT;
    acout << "* ";
    acout |= Style::RESET;
    acout |= Style::BOLD | Style::FG_RED;
    acout << "Some tests failed" << std::endl;
    acout |= Style::RESET;
  }

  acout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
}

void qpkg::dev::test::Progress::done(const std::string &outfile_name) {
  acout |= Style::BOLD | Style::FG_CYAN | Style::BG_DEFAULT;
  acout << "*";
  acout |= Style::RESET;
  acout << " Done running tests for ";
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

#endif // QPKG_DEV_TOOLS
