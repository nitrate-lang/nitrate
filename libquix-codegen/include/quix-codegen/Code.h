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

#ifndef __QUIX_CODEGEN_CODE_H__
#define __QUIX_CODEGEN_CODE_H__

#include <quix-codegen/Config.h>
#include <quix-qxir/TypeDecl.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  QCODE_C11,     /* Generate C 11 Source Code */
  QCODE_CXX11,   /* Generate C++ 11 Source Code */
  QCODE_TS,      /* Generate TypeScript Source Code */
  QCODE_RUST,    /* Generate Rust Source Code */
  QCODE_PYTHON3, /* Generate Python3 Source Code */
  QCODE_CSHARP,  /* Generate C# Source Code */
} qcode_lang_t;

typedef enum {
  QCODE_MINIFY,
  QCODE_GOOGLE,
} qcode_style_t;

/**
 * @brief Transcompile the QXIR module to the target source language.
 *
 * @param module QXIR module to transcompile.
 * @param conf Configuration for the transcompiler.
 * @param lang Target source language.
 * @param style Code style to use.
 * @param err Write human readable error messages to this file or NULL for suppression.
 * @param out Write the transcompiled source code to this file or NULL for suppression.
 * @return true if the transcompilation was successful, false otherwise.
 *
 * Both `err` and `out` will be flushed before returning, irrespective of the return value.
 */
bool qcode_transcode(qmodule_t* module, qcode_conf_t* conf, qcode_lang_t lang, qcode_style_t style,
                     FILE* err, FILE* out);

///==============================================================================

bool qcode_ir(qmodule_t* module, qcode_conf_t* conf, FILE* err, FILE* out);
bool qcode_asm(qmodule_t* module, qcode_conf_t* conf, FILE* err, FILE* out);
bool qcode_obj(qmodule_t* module, qcode_conf_t* conf, FILE* err, FILE* out);

///==============================================================================

#ifdef __cplusplus
}
#endif

#endif  // __QUIX_CODEGEN_CODE_H__
