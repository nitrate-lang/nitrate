////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
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

#include <quixcc/plugin/EngineAPI.h>
#include <quixcc/preprocessor/QSys.h>

#include <iostream>

#ifndef __linux__
#error "This implementation of QSys is only supported on Linux"
#endif

QSYS_DEFINE(qsys_write_stdout, "Write to the compiler's standard output") {
  QSYS_ARGASSERT(qsys_write_stdout, 3);

  QSYS_ARG_STRING(qsys_write_stdout, c_msg, 0);
  QSYS_ARG_INT64(qsys_write_stdout, formatted, 1);
  QSYS_ARG_INT64(qsys_write_stdout, flush, 2);

  std::string msg(c_msg, c_msg_len);

  if (!formatted) {
    std::cout << msg;
    if (flush) {
      std::cout.flush();
    }

    return strdup(std::to_string(msg.size()).c_str());
  }

  /*================ FORMATTED OUTPUT =================*/
  quixcc_message_t ch;

  if (msg.starts_with("DEBUG:")) {
    ch = QUIXCC_MESSAGE_DEBUG;
    msg = msg.substr(6);
  } else if (msg.starts_with("INFO:")) {
    ch = QUIXCC_MESSAGE_INFO;
    msg = msg.substr(5);
  } else if (msg.starts_with("WARNING:")) {
    ch = QUIXCC_MESSAGE_WARNING;
    msg = msg.substr(8);
  } else if (msg.starts_with("ERROR:")) {
    ch = QUIXCC_MESSAGE_ERROR;
    msg = msg.substr(6);
  } else if (msg.starts_with("FATAL:")) {
    ch = QUIXCC_MESSAGE_FATAL;
    msg = msg.substr(6);
  } else {
    ch = QUIXCC_MESSAGE_INFO;
  }

  quixcc_engine_message(e, ch, "%s", msg.c_str());

  return strdup(std::to_string(msg.size()).c_str());
}

QSYS_DEFINE(qsys_write_stderr, "Write to the compiler's standard error") {
  QSYS_ARGASSERT(qsys_write_stdout, 3);

  QSYS_ARG_STRING(qsys_write_stdout, c_msg, 0);
  QSYS_ARG_INT64(qsys_write_stdout, formatted, 1);
  QSYS_ARG_INT64(qsys_write_stdout, flush, 2);

  std::string msg(c_msg, c_msg_len);

  if (!formatted) {
    std::cerr << msg;
    if (flush) {
      std::cerr.flush();
    }

    return strdup(std::to_string(msg.size()).c_str());
  }

  /*================ FORMATTED OUTPUT =================*/
  quixcc_message_t ch;

  if (msg.starts_with("DEBUG:")) {
    ch = QUIXCC_MESSAGE_DEBUG;
    msg = msg.substr(6);
  } else if (msg.starts_with("INFO:")) {
    ch = QUIXCC_MESSAGE_INFO;
    msg = msg.substr(5);
  } else if (msg.starts_with("WARNING:")) {
    ch = QUIXCC_MESSAGE_WARNING;
    msg = msg.substr(8);
  } else if (msg.starts_with("ERROR:")) {
    ch = QUIXCC_MESSAGE_ERROR;
    msg = msg.substr(6);
  } else if (msg.starts_with("FATAL:")) {
    ch = QUIXCC_MESSAGE_FATAL;
    msg = msg.substr(6);
  } else {
    ch = QUIXCC_MESSAGE_INFO;
  }

  quixcc_engine_message(e, ch, "%s", msg.c_str());

  return strdup(std::to_string(msg.size()).c_str());
}

QSYS_DEFINE(qsys_read_stdin, "Read bytes from stdin") {
  QSYS_ARGASSERT(qsys_read_stdin, 2);
  QSYS_ARG_INT64(qsys_read_stdin, count, 0);
  QSYS_ARG_INT64(qsys_read_stdin, term_char, 1);

  std::string input;
  char ch;

  while (count--) {
    ch = std::cin.get();

    if (std::cin.eof() || ch == term_char) {
      break;
    }

    input.push_back(ch);
  }

  return strdup(input.c_str());
}

QSYS_DEFINE(qsys_clear_terminal, "Clear the terminal screen") {
  QSYS_ARGASSERT(qsys_clear_terminal, 0);

  std::cout << "\x1b\x5b\x48\x1b\x5b\x32\x4a\x1b\x5b\x33\x4a";

  return strdup("");
}

QSYS_NOT_IMPLEMENTED(qsys_set_terminal_title);
QSYS_NOT_IMPLEMENTED(qsys_set_terminal_color);
QSYS_NOT_IMPLEMENTED(qsys_set_cursor_position);
QSYS_NOT_IMPLEMENTED(qsys_set_cursor_visibility);
QSYS_NOT_IMPLEMENTED(qsys_set_cursor_blink_rate);
QSYS_NOT_IMPLEMENTED(qsys_enable_terminal_echo);

QSYS_DEFINE(qsys_bell, "Use ANSI escape codes to ring the terminal bell") {
  QSYS_ARGASSERT(qsys_bell, 1);
  QSYS_ARG_INT64(qsys_bell, count, 0);

  while (count--) {
    std::cout << "\x07";
  }

  std::cout.flush();

  return strdup("");
}

QSYS_NOT_IMPLEMENTED(qsys_set_terminal_size);
QSYS_NOT_IMPLEMENTED(qsys_set_terminal_font);
