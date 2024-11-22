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

#ifndef __NITRATE_PARSER_CLASSES_H__
#define __NITRATE_PARSER_CLASSES_H__

#ifndef __cplusplus
#error "This header is for C++ only."
#endif

#include <nitrate-core/Error.h>
#include <nitrate-parser/Parser.h>

class qparse_conf final {
  qparse_conf_t *m_conf;

public:
  qparse_conf(bool use_default = true) {
    if ((m_conf = qparse_conf_new(use_default)) == nullptr) {
      qcore_panic("qparse_conf_new failed");
    }
  }
  ~qparse_conf() { qparse_conf_free(m_conf); }

  qparse_conf_t *get() const { return m_conf; }
};

class qparser final {
  qparse_t *m_parser;

public:
  qparser(qlex_t *scanner, qparse_conf_t *conf, qcore_env_t env) {
    if ((m_parser = qparse_new(scanner, conf, env)) == nullptr) {
      qcore_panic("qparse_new failed");
    }
  }
  ~qparser() { qparse_free(m_parser); }

  qparse_t *get() const { return m_parser; }
};

#endif  // __NITRATE_PARSER_CLASSES_H__
