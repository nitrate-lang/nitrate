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

#include <Impl.h>
#include <quix-core/Error.h>
#include <quix-parser/Config.h>

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "LibMacro.h"

namespace qparse::conf {
  extern std::vector<qparse_setting_t> default_settings;
}

static std::string_view qkey_to_string(qparse_key_t key) {
  switch (key) {
      /// TODO: Implement lookup table for qparse_key_t values.
    default:
      qcore_panic("qkey_to_string: Unhandled qparse_key_t value.");
  }
}

static std::string_view qval_to_string(qparse_val_t val) {
  switch (val) {
    /// TODO: Implement lookup table for qparse_val_t values.
    default:
      qcore_panic("qval_to_string: Unhandled qparse_val_t value.");
  }
}

std::ostream &operator<<(std::ostream &os, const qparse_key_t &key) {
  os << qkey_to_string(key);
  return os;
}

std::ostream &operator<<(std::ostream &os, const qparse_val_t &val) {
  os << qval_to_string(val);
  return os;
}

static void assign_default_options(qparse_conf_t &conf) {
  for (const auto &setting : qparse::conf::default_settings) {
    qparse_conf_setopt(&conf, setting.key, setting.value);
  }
}

LIB_EXPORT qparse_conf_t *qparse_conf_new(bool use_defaults) {
  qparse_conf_t *obj = new qparse_conf_t();

  if (use_defaults) {
    assign_default_options(*obj);
  }

  return obj;
}

LIB_EXPORT void qparse_conf_free(qparse_conf_t *conf) { delete conf; }

LIB_EXPORT bool qparse_conf_setopt(qparse_conf_t *conf, qparse_key_t key, qparse_val_t value) {
  return conf->SetAndVerify(key, value);
}

LIB_EXPORT bool qparse_conf_getopt(qparse_conf_t *conf, qparse_key_t key, qparse_val_t *value) {
  auto val = conf->Get(key);

  if (!val.has_value()) {
    return false;
  }

  if (value != nullptr) {
    *value = val.value();
  }

  return true;
}

LIB_EXPORT const qparse_setting_t *qparse_conf_getopts(qparse_conf_t *conf, size_t *count) {
  if (!count) {
    qcore_panic("qparse_conf_getopts: Contract violation: 'count' parameter cannot be NULL.");
  }

  return conf->GetAll(*count);
}

LIB_EXPORT void qparse_conf_clear(qparse_conf_t *conf) { conf->ClearNoVerify(); }

LIB_EXPORT void qparse_conf_dump(qparse_conf_t *conf, FILE *stream, const char *field_delim,
                                 const char *line_delim) {
  if (!field_delim || !line_delim) {
    qcore_panic(
        "qparse_conf_dump: Contract violation: 'field_delim' and 'line_delim' parameters cannot be "
        "NULL.");
  }

  const qparse_setting_t *settings = nullptr;
  size_t count = 0;

  settings = qparse_conf_getopts(conf, &count);

  for (size_t i = 0; i < count; ++i) {
    fprintf(stream, "%s%s%s%s", qkey_to_string(settings[i].key).data(), field_delim,
            qval_to_string(settings[i].value).data(), line_delim);
  }
}

bool qparse_conf_t::has(const char *option, const char *value) const {
  /// TODO: Implement logic to check if the configuration has a specific option.

  return false;
}