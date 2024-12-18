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

#ifndef __NITRATE_NR_NR_H__
#define __NITRATE_NR_NR_H__

#include <nitrate-ir/TypeDecl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

namespace npar {
  struct Base;
}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Free a QModule instance and ALL of its associated resources.
 *
 * @param module Pointer to the QModule instance to free.
 *
 * @note If `!module`, this function is a no-op.
 *
 * @note This function will not free the lexer instance.
 * @note This function will not free the configuration instance.
 *
 * @note This function is thread safe.
 */
void nr_free(qmodule_t *nr);

typedef enum nr_serial_t {
  NR_SERIAL_CODE = 0, /* Human readable ASCII text */
} nr_serial_t;

/**
 * @brief Serialize a QModule instance to a FILE stream.
 *
 * @param mod The QModule or NULL.
 * @param node Pointer to the node to serialize.
 * @param mode The serialization mode.
 * @param out The FILE stream to serialize to.
 * @param outlen Number of bytes written to the stream (if not NULL).
 * @param argcnt Number of additional variadic arguments. 0 is valid always.
 * @param ... Additional arguments to pass to the serialization function. See
 * the documentation for the specific serialization mode for more information.
 *
 * @return True if the serialization was successful, false otherwise.
 *
 * @note This function is thread safe.
 */
bool nr_write(qmodule_t *mod, const nr_node_t *node, nr_serial_t mode,
              FILE *out, size_t *outlen, uint32_t argcnt, ...);

/**
 * @brief Deserialize a QModule instance from a FILE stream.
 *
 * @param module Pointer to the module to deserialize into.
 * @param in The FILE stream to deserialize from.
 * @param inlen Number of bytes read from the stream (if not NULL).
 * @param argcnt Number of additional variadic arguments. 0 is valid always.
 * @param ... Additional arguments to pass to the deserialization function. See
 * the documentation for the specific serialization mode for more information.
 *
 * @return True if the deserialization was successful, false otherwise.
 *
 * @note This function is thread safe.
 */
bool nr_read(qmodule_t *mod, FILE *in, size_t *inlen, uint32_t argcnt, ...);

/**
 * @brief Lower a parse tree into a QModule.
 *
 * @param[out] mod Pointer to a QModule struct that will store the module.
 * @param base The base node of the parse tree to lower.
 * @param name Module name or nullptr to use the default
 * @param diagnostics Whether to enable diagnostics.
 *
 * @return True if the lowering was successful, false otherwise.
 * @note If `!base` or `!mod`, false is returned.
 * @note A module is always stored in `*mod`, unless it is null. Ensure that
 * nr_free is called to dispose of it when done.
 *
 * @note This function is thread safe.
 */
bool nr_lower(qmodule_t **mod, npar::Base *base, const char *name,
              bool diagnostics);

typedef void (*nr_node_cb)(nr_node_t *cur, uintptr_t userdata);

typedef enum nr_audit_t {
  NR_AUDIT_NONE = 0,     /* No audit */
  NR_AUDIT_WILLCOMP = 1, /* Minimum to determine if the module will compile;
                              g++ disables some */
  NR_AUDIT_STD =
      2, /* Standard audit checks; What happens here will vary; g++ default */
  NR_AUDIT_STRONG =
      3, /* Strong audit checks; What happens here will vary; g++ -Wextra */
  NR_AUDIT_GIGACHAD =
      4, /* A bunch of checks that are probably unnecessary; g++ -Wall */
  NR_AUDIT_MAX = 5, /* Maximum audit */
  NR_AUDIT_DEFAULT = NR_AUDIT_STD,
} nr_audit_t;

/**
 * @brief Perform semantic analysis a QModule instance.
 *
 * @param nr The QModule instance to analyze.
 * @param level The level of audit to perform.
 *
 * @return True if the analysis was successful, false otherwise.
 *
 * @note If `!nr`, false is returned.
 *
 * @note This function is thread safe.
 */
bool nr_audit(qmodule_t *nr, nr_audit_t level);

typedef enum {
  NR_LEVEL_DEBUG = 0,
  NR_LEVEL_INFO = 1,
  NR_LEVEL_WARN = 2,
  NR_LEVEL_ERROR = 3,
  NR_LEVEL_FATAL = 4,
  NR_LEVEL_MAX = 5,
  NR_LEVEL_DEFAULT = NR_LEVEL_WARN,
} nr_level_t;

/**
 * @brief A callback function to facilitate the communication of a report
 * generated by the QModule diagnostics subsystem.
 *
 * @param utf8text UTF-8 encoded text of the report (null terminated).
 * @param size Size of the report in bytes.
 * @param level The severity level of the report.
 * @param data User supplied data.
 *
 * @note `utf8text` is not valid after the callback function returns.
 *
 * @note This function shall be thread safe.
 */
typedef void (*nr_report_cb)(const uint8_t *utf8text, size_t size,
                             nr_level_t level, uintptr_t data);

typedef enum nr_diag_format_t {
  /**
   * @brief Code decimal serialization of the error code.
   * @example `801802`
   * @format <code>
   */
  NR_DIAG_ASCII_ECODE,

  /**
   * @brief Code decimal serialization of the error code and source location.
   * @example `801802:1:1:/path/to/filename.q`
   * @format <code>:<line>:<col>:<path>
   *
   * @note UTF-8 characters in the path are preserved.
   */
  NR_DIAG_UTF8_ECODE_LOC,

  /**
   * @brief Code decimal serialization of the error code and UTF-8 error
   * message.
   * @example `801802:This is an UTF-8 error message.`
   * @format <code>:<utf8_message>
   */
  NR_DIAG_UTF8_ECODE_ETEXT,

  /**
   * @brief Unspecified format.
   * @note No-ANSI colors are included.
   * @note Includes source location information as well as source code snippets
   * (if available).
   * @note Includes error messages and suggestions.
   * @note Basically, everything you expect from a mainstream compiler (except
   * without color).
   */
  NR_DIAG_NOSTD_TTY_UTF8,

  /**
   * @brief Unspecified format.
   * @note Similar to `NR_DIAG_NOSTD_TTY_UTF8`, but with undocumented
   * differences.
   */
  NR_DIAG_NONSTD_ANSI16_UTF8_FULL,

  /**
   * @brief Unspecified format.
   * @note Similar to `NR_DIAG_NOSTD_TTY_UTF8`, but with undocumented
   * differences.
   */
  NR_DIAG_NONSTD_ANSI256_UTF8_FULL,

  /**
   * @brief Unspecified format.
   * @note Similar to `NR_DIAG_NOSTD_TTY_UTF8`, but with undocumented
   * differences.
   */
  NR_DIAG_NONSTD_ANSIRGB_UTF8_FULL,

  /**
   * @brief Display in a modern terminal emulator with UTF-8, RGB colors,
   * ANSI-box drawing, and full diagnostics with source highlighting.
   * @note Includes everything the user would expect from a mainstream compiler.
   */
  NR_DIAG_COLOR = NR_DIAG_NONSTD_ANSIRGB_UTF8_FULL,

  /**
   * @brief Display in a modern terminal emulator with UTF-8 and no colors.
   * @note Includes everything the user would expect from a mainstream compiler.
   */
  NR_DIAG_NOCOLOR = NR_DIAG_NOSTD_TTY_UTF8,
} nr_diag_format_t;

/**
 * @brief Read diagnostic reports generated by the QModule diagnostics
 * subsystem.
 *
 * @param nr QModule instance to read diagnostics from.
 * @param format Format for the diagnostics reporting.
 * @param cb Callback function to call for each report.
 * @param data User supplied data to pass to the callback function.
 *
 * @note `data` is arbitrary it will be passed to the callback function.
 * @note If `!cb`, the number of reports that would have been processed is
 * returned.
 *
 * @note This function will not dispose of any reports. Calling this function
 * multiple times with the same arguments will result in the same reports being
 * emitted.
 *
 * @warning The order that the reports are emitted is unspecified. It may be
 * inconsistent between calls to this function.
 *
 * @note This function is thread safe.
 */
void nr_diag_read(qmodule_t *nr, nr_diag_format_t format, nr_report_cb cb,
                  uintptr_t data);

/**
 * @brief Clear diagnostic reports generated by the QModule diagnostics
 * subsystem.
 * @param nr QModule instance to clear diagnostics from.
 */
void nr_diag_clear(qmodule_t *nr);

/**
 * @brief Return the maximum number of modules that can exist at once.
 *
 * @return The maximum number of modules that can exist at once.
 *
 * @note This function is thread safe.
 */
size_t nr_max_modules(void);

/**
 * @brief Performs type inference on a NR node.
 *
 * @param node Node to perform type inference on.
 * @param res Should be NULL
 * @return Type of the node or NULL if inference failed.
 *
 * @note This function is thread-safe.
 */
nr_node_t *nr_infer(const nr_node_t *node, void *res);

/**
 * @brief Clone a NR node. Optionally into a different module.
 *
 * @param node The node to clone.
 *
 * @return nr_node_t* The cloned node.
 *
 * @note If `node` NULL, the function will return NULL.
 * @note This clone is a deep copy.
 */
nr_node_t *nr_clone(const nr_node_t *node);

nr_node_t *nr_base(qmodule_t *mod);
nr_conf_t *nr_get_conf(qmodule_t *mod);

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_NR_NR_H__
