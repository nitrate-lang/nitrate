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

#ifndef __NITRATE_QXIR_QXIR_H__
#define __NITRATE_QXIR_QXIR_H__

#include <nitrate-ir/TypeDecl.h>
#include <nitrate-lexer/Lexer.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qparse_node_t qparse_node_t;

/**
 * @brief Create QModule instance.
 *
 * @param lexer Source code lexer instance or NULL.
 * @param conf Module configuration.
 * @param name Module name or NULL.
 *
 * @return On success, a new QModule instance is returned. On failure, NULL is returned.
 *
 * @warning The QModule instance must be freed using `nr_free`.
 *
 * @note If `lexer` is NULL, the diagnostics subsystem will be unable to assign
 * source locations to its reports. However, the diagnostics subsystem will still
 * detect issues normally. In addition, all nodes created by the QModule will have
 * no source location information, rendering all later phase of compilation unable
 * to assign source locations to their reports.
 *
 * @note External mutation of the configuration object after passing it to this
 * function is not recommended, but technically supported. However, the semantic
 * consequences of doing so are undefined.
 *
 * @note Obviously, the `lexer` and `conf` lifetimes must exceed the lifetime of the
 * returned QModule instance.
 *
 * @warning This library supports a maximum of `nr_max_modules()` QModule instances
 * at once. Attempting to create more than this number will result in the function
 * returning NULL.
 *
 * @note This function is thread safe.
 */
qmodule_t *nr_new(qlex_t *lexer, nr_conf_t *conf, const char *name);

/**
 * @brief Free a QModule instance and ALL of its associated resources.
 *
 * @param module Pointer to the QModule instance to free.
 *
 * @note If `!module`, this function is a no-op.
 *
 * @note This function will not free the lexer instance passed to `nr_new`.
 * @note This function will not free the configuration instance passed to `nr_new`.
 *
 * @note This function is thread safe.
 */
void nr_free(qmodule_t *nr);

typedef enum nr_serial_t {
  QXIR_SERIAL_CODE = 0, /* Human readable ASCII text */
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
 * @param ... Additional arguments to pass to the serialization function. See the
 * documentation for the specific serialization mode for more information.
 *
 * @return True if the serialization was successful, false otherwise.
 *
 * @note This function is thread safe.
 */
bool nr_write(qmodule_t *mod, const nr_node_t *node, nr_serial_t mode, FILE *out, size_t *outlen,
              uint32_t argcnt, ...);

/**
 * @brief Deserialize a QModule instance from a FILE stream.
 *
 * @param module Pointer to the module to deserialize into.
 * @param in The FILE stream to deserialize from.
 * @param inlen Number of bytes read from the stream (if not NULL).
 * @param argcnt Number of additional variadic arguments. 0 is valid always.
 * @param ... Additional arguments to pass to the deserialization function. See the
 * documentation for the specific serialization mode for more information.
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
 * @param diagnostics Whether to enable diagnostics.
 *
 * @return True if the lowering was successful, false otherwise.
 * @note If `!base` or `!mod`, false is returned.
 *
 * @note This function is thread safe.
 */
bool nr_lower(qmodule_t **mod, qparse_node_t *base, bool diagnostics);

typedef void (*nr_node_cb)(nr_node_t *cur, uintptr_t userdata);

typedef enum nr_audit_t {
  QXIR_AUDIT_NONE = 0,     /* No audit */
  QXIR_AUDIT_WILLCOMP = 1, /* Minimum to determine if the module will compile; g++ disables some */
  QXIR_AUDIT_STD = 2,      /* Standard audit checks; What happens here will vary; g++ default */
  QXIR_AUDIT_STRONG = 3,   /* Strong audit checks; What happens here will vary; g++ -Wextra */
  QXIR_AUDIT_GIGACHAD = 4, /* A bunch of checks that are probably unnecessary; g++ -Wall */
  QXIR_AUDIT_MAX = 5,      /* Maximum audit */
  QXIR_AUDIT_DEFAULT = QXIR_AUDIT_STD,
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
  QXIR_LEVEL_DEBUG = 0,
  QXIR_LEVEL_INFO = 1,
  QXIR_LEVEL_WARN = 2,
  QXIR_LEVEL_ERROR = 3,
  QXIR_LEVEL_FATAL = 4,
  QXIR_LEVEL_MAX = 5,
  QXIR_LEVEL_DEFAULT = QXIR_LEVEL_WARN,
} nr_level_t;

/**
 * @brief A callback function to facilitate the communication of a report generated by the QModule
 * diagnostics subsystem.
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
typedef void (*nr_report_cb)(const uint8_t *utf8text, size_t size, nr_level_t level,
                             uintptr_t data);

typedef enum nr_diag_format_t {
  /**
   * @brief Code decimal serialization of the error code.
   * @example `801802`
   * @format <code>
   */
  QXIR_DIAG_ASCII_ECODE,

  /**
   * @brief Code decimal serialization of the error code and source location.
   * @example `801802:1:1:/path/to/filename.q`
   * @format <code>:<line>:<col>:<path>
   *
   * @note UTF-8 characters in the path are preserved.
   */
  QXIR_DIAG_UTF8_ECODE_LOC,

  /**
   * @brief Code decimal serialization of the error code and UTF-8 error message.
   * @example `801802:This is an UTF-8 error message.`
   * @format <code>:<utf8_message>
   */
  QXIR_DIAG_UTF8_ECODE_ETEXT,

  /**
   * @brief Unspecified format.
   * @note No-ANSI colors are included.
   * @note Includes source location information as well as source code snippets (if available).
   * @note Includes error messages and suggestions.
   * @note Basically, everything you expect from a mainstream compiler (except without color).
   */
  QXIR_DIAG_NOSTD_TTY_UTF8,

  /**
   * @brief Unspecified format.
   * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
   */
  QXIR_DIAG_NONSTD_ANSI16_UTF8_FULL,

  /**
   * @brief Unspecified format.
   * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
   */
  QXIR_DIAG_NONSTD_ANSI256_UTF8_FULL,

  /**
   * @brief Unspecified format.
   * @note Similar to `QXIR_DIAG_NOSTD_TTY_UTF8`, but with undocumented differences.
   */
  QXIR_DIAG_NONSTD_ANSIRGB_UTF8_FULL,

  /**
   * @brief Display in a modern terminal emulator with UTF-8, RGB colors, ANSI-box drawing, and
   * full diagnostics with source highlighting.
   * @note Includes everything the user would expect from a mainstream compiler.
   */
  QXIR_DIAG_COLOR = QXIR_DIAG_NONSTD_ANSIRGB_UTF8_FULL,

  /**
   * @brief Display in a modern terminal emulator with UTF-8 and no colors.
   * @note Includes everything the user would expect from a mainstream compiler.
   */
  QXIR_DIAG_NOCOLOR = QXIR_DIAG_NOSTD_TTY_UTF8,
} nr_diag_format_t;

/**
 * @brief Read diagnostic reports generated by the QModule diagnostics subsystem.
 *
 * @param nr QModule instance to read diagnostics from.
 * @param format Format for the diagnostics reporting.
 * @param cb Callback function to call for each report.
 * @param data User supplied data to pass to the callback function.
 *
 * @return Number of reports processed.
 * @note `data` is arbitrary it will be passed to the callback function.
 * @note If `!cb`, the number of reports that would have been processed is returned.
 *
 * @note This function will not diapose of any reports. Calling this function multiple times
 * with the same arguments will result in the same reports being emitted.
 *
 * @warning The order that the reports are emitted in currently unspecified. It may be inconsistent
 * between calls to this function.
 *
 * @note This function is thread safe.
 */
size_t nr_diag_read(qmodule_t *nr, nr_diag_format_t format, nr_report_cb cb, uintptr_t data);

/**
 * @brief Clear diagnostic reports generated by the QModule diagnostics subsystem.
 * @param nr QModule instance to clear diagnostics from.
 *
 * @return Number of reports cleared.
 */
size_t nr_diag_clear(qmodule_t *nr);

/**
 * @brief Return the maximum number of modules that can exist at once.
 *
 * @return The maximum number of modules that can exist at once.
 *
 * @note This function is thread safe.
 */
size_t nr_max_modules(void);

/**
 * @brief Performs type inference on a QXIR node.
 *
 * @param node Node to perform type inference on.
 * @param PtrSizeBytes Size of a pointer on the target platform
 * @return Type of the node or NULL if inference failed.
 *
 * @note This function is thread-safe.
 */
nr_node_t *nr_infer(nr_node_t *node, uint32_t PtrSizeBytes);

/**
 * @brief Clone a QXIR node. Optionally into a different module.
 *
 * @param node The node to clone.
 *
 * @return nr_node_t* The cloned node.
 *
 * @note If `node` NULL, the function will return NULL.
 * @note This clone is a deep copy.
 */
nr_node_t *nr_clone(const nr_node_t *node);

qlex_t *nr_get_lexer(qmodule_t *mod);
nr_node_t *nr_base(qmodule_t *mod);
nr_conf_t *nr_get_conf(qmodule_t *mod);

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_QXIR_QXIR_H__
