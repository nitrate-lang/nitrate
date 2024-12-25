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

#ifndef __NITRATE_CODEGEN_CODE_H__
#define __NITRATE_CODEGEN_CODE_H__

#include <nitrate-emit/Config.h>
#include <stdbool.h>

#include <nitrate-ir/IRFwd.hh>

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
 * @brief Transcompile the NR module to the target source language.
 *
 * @param module NR module to transcompile.
 * @param conf Configuration for the transcompiler.
 * @param lang Target source language.
 * @param style Code style to use.
 * @param err Write human readable error messages to this file or NULL for
 * suppression.
 * @param out Write the transcompiled source code to this file or NULL for
 * suppression.
 * @return true if the transcompilation was successful, false otherwise.
 *
 * Both `err` and `out` will be flushed before returning, irrespective of the
 * return value.
 */
bool qcode_transcode(ncc::ir::qmodule_t* module, qcode_conf_t* conf,
                     qcode_lang_t lang, qcode_style_t style, FILE* err,
                     FILE* out);

///==============================================================================

bool qcode_ir(ncc::ir::qmodule_t* module, qcode_conf_t* conf, FILE* err,
              FILE* out);
bool qcode_asm(ncc::ir::qmodule_t* module, qcode_conf_t* conf, FILE* err,
               FILE* out);
bool qcode_obj(ncc::ir::qmodule_t* module, qcode_conf_t* conf, FILE* err,
               FILE* out);

///==============================================================================

#endif  // __NITRATE_CODEGEN_CODE_H__
