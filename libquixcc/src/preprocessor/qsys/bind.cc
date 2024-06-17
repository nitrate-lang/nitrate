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

#include <preprocessor/QSys.h>
#include <quixcc/EngineAPI.h>

#include <unordered_map>

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

/*==================== FILE IO ====================*/
#define QSYS_OPEN_FILE 0x00F0
#define QSYS_CLOSE_FILE 0x00F1
#define QSYS_READ_FILE 0x00F2
#define QSYS_WRITE_FILE 0x00F3
#define QSYS_SEEK_FILE 0x00F4
#define QSYS_TELL_FILE 0x00F5
#define QSYS_FLUSH_FILE 0x00F6
#define QSYS_DELETE_FILE 0x00F7
#define QSYS_RENAME_FILE 0x00F8
#define QSYS_CREATE_DIRECTORY 0x00F9
#define QSYS_DELETE_DIRECTORY 0x00FA
#define QSYS_RENAME_DIRECTORY 0x00FB
#define QSYS_LIST_DIRECTORY 0x00FC
#define QSYS_GET_FILE_ATTRIBUTES 0x00FD
#define QSYS_SET_FILE_ATTRIBUTES 0x00FE

/*==================== SYSTEM ====================*/
#define QSYS_GET_COMPILER_VERSION 0x01F0
#define QSYS_GET_COMPILER_FLAGS 0x01F1
#define QSYS_UNSET_COMPILER_FLAG 0x01F2
#define QSYS_SET_COMPILER_FLAG 0x01F3
#define QSYS_GET_COMPILER_LOG_FILE 0x01F4
#define QSYS_EXECUTE_OS_COMMAND 0x01F5
#define QSYS_ABORT_COMPILATION 0x01F6

/*==================== NETWORK ====================*/
#define QSYS_OPEN_NETWORK_CONNECTION 0x03F0
#define QSYS_CLOSE_NETWORK_CONNECTION 0x03F1
#define QSYS_READ_NETWORK_CONNECTION 0x03F2
#define QSYS_WRITE_NETWORK_CONNECTION 0x03F3

/*==================== LANGUAGE ====================*/
#define QSYS_COMPILE_AND_EXECUTE_QUIX 0x0FF0
#define QSYS_GET_TYPE_OF_ITEM 0x0FF1
#define QSYS_UNDEFINE_FUNCTION 0x0FF2
#define QSYS_UNDEFINE_MACRO 0x0FF3
#define QSYS_UNDECLARE_ITEM 0x0FF4
#define QSYS_GET_MACRO_BODY 0x0FF5
#define QSYS_GET_FUNCTION_BODY 0x0FF6
#define QSYS_GET_TYPE_BODY 0x0FF7
#define QSYS_SET_GLOBAL_META_VARIABLE 0x0FF8
#define QSYS_GET_GLOBAL_META_VARIABLE 0x0FF9

void libquixcc::qsys::bind_qsyscalls(quixcc_job_t *job) {
  /// TODO: Implement this function
  const std::unordered_map<uint32_t, quixcc_qsys_impl_t> qsyscalls = {
      /*==================== TEXT IO ====================*/
      {QSYS_WRITE_STDOUT, qsys_write_stdout},
      {QSYS_WRITE_STDERR, qsys_write_stderr},
      {QSYS_READ_STDIN, qsys_read_stdin},
      {QSYS_CLEAR_TERMINAL, qsys_clear_terminal},
      {QSYS_SET_TERMINAL_TITLE, qsys_set_terminal_title},
      {QSYS_SET_TERMINAL_COLOR, qsys_set_terminal_color},
      {QSYS_SET_CURSOR_POSITION, qsys_set_cursor_position},
      {QSYS_SET_CURSOR_VISIBILITY, qsys_set_cursor_visibility},
      {QSYS_SET_CURSOR_BLINK_RATE, qsys_set_cursor_blink_rate},
      {QSYS_ENABLE_TERMINAL_ECHO, qsys_enable_terminal_echo},
      {QSYS_BELL, qsys_bell},
      {QSYS_SET_TERMINAL_SIZE, qsys_set_terminal_size},
      {QSYS_SET_TERMINAL_FONT, qsys_set_terminal_font},

      /*==================== FILE IO ====================*/
      {QSYS_OPEN_FILE, qsys_open_file},
      {QSYS_CLOSE_FILE, qsys_close_file},
      {QSYS_READ_FILE, qsys_read_file},
      {QSYS_WRITE_FILE, qsys_write_file},
      {QSYS_SEEK_FILE, qsys_seek_file},
      {QSYS_TELL_FILE, qsys_tell_file},
      {QSYS_FLUSH_FILE, qsys_flush_file},
      {QSYS_DELETE_FILE, qsys_delete_file},
      {QSYS_RENAME_FILE, qsys_rename_file},
      {QSYS_CREATE_DIRECTORY, qsys_create_directory},
      {QSYS_DELETE_DIRECTORY, qsys_delete_directory},
      {QSYS_RENAME_DIRECTORY, qsys_rename_directory},
      {QSYS_LIST_DIRECTORY, qsys_list_directory},
      {QSYS_GET_FILE_ATTRIBUTES, qsys_get_file_attributes},
      {QSYS_SET_FILE_ATTRIBUTES, qsys_set_file_attributes},

      /*==================== SYSTEM ====================*/
      {QSYS_GET_COMPILER_VERSION, qsys_get_compiler_version},
      {QSYS_GET_COMPILER_FLAGS, qsys_get_compiler_flags},
      {QSYS_UNSET_COMPILER_FLAG, qsys_unset_compiler_flag},
      {QSYS_SET_COMPILER_FLAG, qsys_set_compiler_flag},
      {QSYS_GET_COMPILER_LOG_FILE, qsys_get_compiler_log_file},
      {QSYS_EXECUTE_OS_COMMAND, qsys_execute_os_command},
      {QSYS_ABORT_COMPILATION, qsys_abort_compilation},

      /*==================== NETWORK ====================*/
      {QSYS_OPEN_NETWORK_CONNECTION, qsys_open_network_connection},
      {QSYS_CLOSE_NETWORK_CONNECTION, qsys_close_network_connection},
      {QSYS_READ_NETWORK_CONNECTION, qsys_read_network_connection},
      {QSYS_WRITE_NETWORK_CONNECTION, qsys_write_network_connection},

      /*==================== LANGUAGE ====================*/
      {QSYS_COMPILE_AND_EXECUTE_QUIX, qsys_compile_and_execute_quix},
      {QSYS_GET_TYPE_OF_ITEM, qsys_get_type_of_item},
      {QSYS_UNDEFINE_FUNCTION, qsys_undefine_function},
      {QSYS_UNDEFINE_MACRO, qsys_undefine_macro},
      {QSYS_UNDECLARE_ITEM, qsys_undeclare_item},
      {QSYS_GET_MACRO_BODY, qsys_get_macro_body},
      {QSYS_GET_FUNCTION_BODY, qsys_get_function_body},
      {QSYS_GET_TYPE_BODY, qsys_get_type_body},
      {QSYS_SET_GLOBAL_META_VARIABLE, qsys_set_global_meta_variable},
      {QSYS_GET_GLOBAL_META_VARIABLE, qsys_get_global_meta_variable},
  };

  for (const auto &[num, impl] : qsyscalls) {
    quixcc_qsys_add(job, num, impl);
  }
}