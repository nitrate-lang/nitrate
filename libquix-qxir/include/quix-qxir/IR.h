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

#ifndef __QUIX_QXIR_QXIR_H__
#define __QUIX_QXIR_QXIR_H__

#include <quix-lexer/Lexer.h>
#include <quix-parser/Node.h>
#include <quix-qxir/TypeDecl.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create QModule instance.
 *
 * @param lexer Source code lexer instance or NULL.
 * @param conf Module configuration.
 * @param name Module name or NULL.
 *
 * @return On success, a new QModule instance is returned. On failure, NULL is returned.
 *
 * @warning The QModule instance must be freed using `qxir_free`.
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
 * @warning This library supports a maximum of `qxir_max_modules()` QModule instances
 * at once. Attempting to create more than this number will result in the function
 * returning NULL.
 *
 * @note This function is thread safe.
 */
qmodule_t *qxir_new(qlex_t *lexer, qxir_conf_t *conf, const char *name);

/**
 * @brief Free a QModule instance and ALL of its associated resources.
 *
 * @param module Pointer to the QModule instance to free.
 *
 * @note If `!module`, this function is a no-op.
 *
 * @note This function will not free the lexer instance passed to `qxir_new`.
 * @note This function will not free the configuration instance passed to `qxir_new`.
 *
 * @note This function is thread safe.
 */
void qxir_free(qmodule_t *qxir);

typedef enum qxir_serial_t {
  QXIR_SERIAL_CODE = 0,     /* Human readable ASCII text */
  QXIR_SERIAL_CODE_MIN = 1, /* Minified human readable ASCII text */
  QXIR_SERIAL_B10 = 2,      /* Binary representation 1.0 */
} qxir_serial_t;

/**
 * @brief Serialize a QModule instance to a FILE stream.
 *
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
bool qxir_write(const qxir_node_t *node, qxir_serial_t mode, FILE *out, size_t *outlen,
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
bool qxir_read(qmodule_t *mod, FILE *in, size_t *inlen, uint32_t argcnt, ...);

/**
 * @brief Lower a parse tree into a QModule.
 *
 * @param mod Pointer to the QModule instance to lower into.
 * @param base The base node of the parse tree to lower.
 * @param diagnostics Whether to enable diagnostics. Setting this to false may
 * potentially increase performance. Correctness will remain the same.
 *
 * @return True if the lowering was successful, false otherwise.
 * @note If `!base`, false is returned.
 *
 * @details Keep in mind that this function is 'kinda no super fast' and client-side
 * caching/avoidance of repeated calls is recommended.
 *
 * @note This function is thread safe.
 */
bool qxir_lower(qmodule_t *mod, qparse_node_t *base, bool diagnostics);

typedef void (*qxir_node_cb)(qxir_node_t *cur, uintptr_t userdata);

/**
 * @brief Just lower the damn thing and print it.
 *
 * @param base Base node of the parse tree to start from.
 * @param out FILE stream to print to.
 * @param mode The serialization mode to use when printing the IR.
 * @param cb Callback to call for each node in the polymorphic graph while printing or NULL.
 * @param argcnt Number of additional variadic arguments. 0 is valid always.
 * @param ... Additional arguments to pass to the printing function. See the
 * documentation for more information.
 *
 * @return True if the IR was printed successfully, false otherwise.
 *
 * @warning In the event of a non-fatal lowering error, the function will still print the
 * (potentially invalid) qxir to the stream. This is to allow the user to see the IR even if it is
 * invalid. However, this function will only return false if nothing was printed to the stream
 * (which needs not indicate that the input was actually semantically acceptable).
 *
 * @note This function is thread safe.
 */
bool qxir_justprint(qparse_node_t *base, FILE *out, qxir_serial_t mode, qxir_node_cb cb,
                    uint32_t argcnt, ...);

typedef enum qxir_audit_t {
  QXIR_AUDIT_NONE = 0,     /* No audit */
  QXIR_AUDIT_WILLCOMP = 1, /* Minimum to determine if the module will compile; g++ disables some */
  QXIR_AUDIT_STD = 2,      /* Standard audit checks; What happens here will vary; g++ default */
  QXIR_AUDIT_STRONG = 3,   /* Strong audit checks; What happens here will vary; g++ -Wextra */
  QXIR_AUDIT_GIGACHAD = 4, /* A bunch of checks that are probably unnecessary; g++ -Wall */
  QXIR_AUDIT_MAX = 5,      /* Maximum audit; Includes user-defined checks */
  QXIR_AUDIT_DEFAULT = QXIR_AUDIT_STD,
} qxir_audit_t;

typedef uint32_t qxir_audit_ticket_t;

#define QXIR_AUDIT_ALL ((qxir_audit_ticket_t)0x00000000)
#define QXIR_AUDIT_CONV ((qxir_audit_ticket_t)0x00000001)
#define QXIR_AUDIT_LAST ((qxir_audit_ticket_t)0xFFFFFFFF)

/**
 * @brief Perform semantic analysis a QModule instance.
 *
 * @param qxir The QModule instance to analyze.
 * @param level The level of audit to perform.
 * @param ticket Pointer to save the audit ticket to.
 *
 * @return True if the analysis was successful, false otherwise.
 *
 * @note If `!qxir`, false is returned.
 * @note The results will be accessible via the QModule diagnostics API using the audit ticket
 * number.
 *
 * @note This function is thread safe.
 */
bool qxir_audit(qmodule_t *qxir, qxir_audit_t level, qxir_audit_ticket_t *ticket);

typedef enum {
  QXIR_LEVEL_DEBUG = 0,
  QXIR_LEVEL_INFO = 1,
  QXIR_LEVEL_WARN = 2,
  QXIR_LEVEL_ERROR = 3,
  QXIR_LEVEL_FATAL = 4,
  QXIR_LEVEL_MAX = 5,
  QXIR_LEVEL_DEFAULT = QXIR_LEVEL_WARN,
} qxir_level_t;

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
typedef void (*qxir_report_cb)(const uint8_t *utf8text, size_t size, qxir_level_t level,
                               uintptr_t data);

typedef enum qxir_diag_format_t {
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
   * @brief Display in a modern terminal emulator with UTF-8 and RGB colors.
   * @note Includes everything the user would expect from a mainstream compiler.
   */
  QXIR_DIAG_COLOR = QXIR_DIAG_NONSTD_ANSIRGB_UTF8_FULL,

  /**
   * @brief Display in a modern terminal emulator with UTF-8 and no colors.
   * @note Includes everything the user would expect from a mainstream compiler.
   */
  QXIR_DIAG_NOCOLOR = QXIR_DIAG_NOSTD_TTY_UTF8,
} qxir_diag_format_t;

/**
 * @brief Read diagnostic reports generated by the QModule diagnostics subsystem.
 *
 * @param qxir QModule instance to read diagnostics from.
 * @param ticket Audit ticket number.
 * @param format Format for the diagnostics reporting.
 * @param cb Callback function to call for each report.
 * @param data User supplied data to pass to the callback function.
 *
 * @return Number of reports processed.
 * @note If `ticket` is invalid, 0 is returned.
 * @note `data` is arbitrary it will be passed to the callback function.
 * @note If `!cb`, the number of reports that would have been processed is returned.
 *
 * @note This function will not diapose of any reports. Calling this function multiple times
 * with the same arguments will result in the same reports being emitted (unless other reports
 * are generated in the meantime). The ticket number remains valid until the QModule instance is
 * freed.
 *
 * @warning The order that the reports are emitted in currently unspecified. It may be inconsistent
 * between calls to this function.
 *
 * @note This function is thread safe.
 */
size_t qxir_diag_read(qmodule_t *qxir, qxir_audit_ticket_t ticket, qxir_diag_format_t format,
                      qxir_report_cb cb, uintptr_t data);

/**
 * @brief Clear diagnostic reports generated by the QModule diagnostics subsystem.
 * @param qxir QModule instance to clear diagnostics from.
 * @param ticket Audit ticket number.
 *
 * @return Number of reports cleared. 0 may be returned if the ticket does not exist OR if the
 * ticket does not contain any reports.
 *
 * @note If `ticket` is `QXIR_AUDIT_ALL`, all reports are cleared.
 */
size_t qxir_diag_clear(qmodule_t *qxir, qxir_audit_ticket_t ticket);

/**
 * @brief Return the maximum number of modules that can exist at once.
 *
 * @return The maximum number of modules that can exist at once.
 *
 * @note This function is thread safe.
 */
size_t qxir_max_modules(void);

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_QXIR_QXIR_H__
