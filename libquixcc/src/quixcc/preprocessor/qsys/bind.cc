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

#include <tuple>
#include <vector>

/*==================== TEXT IO ====================*/
#define QSYS_WRITE_STDOUT 0x0001
#define QSYS_WRITE_STDERR 0x0002
#define QSYS_READ_STDIN 0x0003
#define QSYS_CLEAR_TERMINAL 0x0004
#define QSYS_SET_TERMINAL_TITLE 0x0005
#define QSYS_SET_TERMINAL_COLOR 0x0006
#define QSYS_SET_CURSOR_POSITION 0x0007
#define QSYS_SET_CURSOR_VISIBILITY 0x0008
#define QSYS_SET_CURSOR_BLINK_RATE 0x0009
#define QSYS_ENABLE_TERMINAL_ECHO 0x000A
#define QSYS_BELL 0x000B
#define QSYS_SET_TERMINAL_SIZE 0x000C
#define QSYS_SET_TERMINAL_FONT 0x000D

/*==================== SYSTEM ====================*/
#define QSYS_GET_COMPILER_VERSION 0x01F0
#define QSYS_GET_FLAGS 0x01F1
#define QSYS_SET_FLAG 0x01F3
#define QSYS_SET_MIO 0x01F4

/*==================== LANGUAGE ====================*/
#define QSYS_COMPILE_AND_EXECUTE_QUIX 0x0FF0
#define QSYS_GET_TYPE 0x0FF1
#define QSYS_UNDEFINE 0x0FF2
#define QSYS_GET_SOURCE 0x0FF5
#define QSYS_SET 0x0FF8
#define QSYS_GET 0x0FF9
#define QSYS_ABORT 0x0FFA

/*==================== UTILITIES ===================*/
#define QSYS_TIME 0x2000
#define QSYS_SLEEP 0x2001
#define QSYS_CRYPTO_RANDOM 0x2002
#define QSYS_HASH 0x2003
#define QSYS_INSMOD 0x2004
#define QSYS_RMMOD 0x2005
#define QSYS_IOCTL 0x2006

void libquixcc::qsys::bind_qsyscalls(quixcc_cc_job_t *job) {
  const std::vector<std::tuple<uint32_t, quixcc_qsys_impl_t, const char *>> qsyscalls = {
      /*==================== TEXT IO ====================*/
      {QSYS_WRITE_STDOUT, qsys_write_stdout, "write"},
      {QSYS_WRITE_STDERR, qsys_write_stderr, "ewrite"},
      {QSYS_READ_STDIN, qsys_read_stdin, "read"},
      {QSYS_CLEAR_TERMINAL, qsys_clear_terminal, "clear_term"},
      {QSYS_SET_TERMINAL_TITLE, qsys_set_terminal_title, "set_term_title"},
      {QSYS_SET_TERMINAL_COLOR, qsys_set_terminal_color, "set_term_color"},
      {QSYS_SET_CURSOR_POSITION, qsys_set_cursor_position, "set_cursor_pos"},
      {QSYS_SET_CURSOR_VISIBILITY, qsys_set_cursor_visibility, "show_cursor"},
      {QSYS_SET_CURSOR_BLINK_RATE, qsys_set_cursor_blink_rate, "set_cursor_blink_rate"},
      {QSYS_ENABLE_TERMINAL_ECHO, qsys_enable_terminal_echo, "enable_echo"},
      {QSYS_BELL, qsys_bell, "bell"},
      {QSYS_SET_TERMINAL_SIZE, qsys_set_terminal_size, "set_term_size"},
      {QSYS_SET_TERMINAL_FONT, qsys_set_terminal_font, "set_term_font"},

      /*==================== SYSTEM ====================*/
      {QSYS_GET_COMPILER_VERSION, qsys_get_compiler_version, "get_version"},
      {QSYS_GET_FLAGS, qsys_get_flags, "get_flags"},
      {QSYS_SET_FLAG, qsys_set_flag, "set_flag"},

      /*==================== LANGUAGE ====================*/
      {QSYS_COMPILE_AND_EXECUTE_QUIX, qsys_compile_and_execute_quix, "compile"},
      {QSYS_GET_TYPE, qsys_get_type, "get_type"},
      {QSYS_UNDEFINE, qsys_undefine, "undefine"},
      {QSYS_GET_SOURCE, qsys_get_source, "get_source"},
      {QSYS_SET, qsys_set, "set"},
      {QSYS_GET, qsys_get, "get"},
      {QSYS_ABORT, qsys_abort, "abort"},

      /*==================== UTILITIES ===================*/
      {QSYS_TIME, qsys_time, "time"},
      {QSYS_SLEEP, qsys_sleep, "sleep"},
      {QSYS_CRYPTO_RANDOM, qsys_crypto_random, "crypto_random"},
      {QSYS_HASH, qsys_hash, "hash"},
      {QSYS_INSMOD, qsys_insmod, "insmod"},
      {QSYS_RMMOD, qsys_rmmod, "rmmod"},
      {QSYS_IOCTL, qsys_ioctl, "ioctl"},
  };

  for (const auto &[num, impl, name] : qsyscalls) {
    quixcc_qsys_add(job, num, name, impl);
  }
}