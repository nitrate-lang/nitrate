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
#include <nitrate-core/Error.h>
#include <quix-parser/Config.h>

#include <boost/bimap.hpp>

#include "LibMacro.h"

namespace qparse::conf {
  extern std::vector<qparse_setting_t> default_settings;
}

template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const boost::bimap<qparse_key_t, std::string_view> options_bimap =
    make_bimap<qparse_key_t, std::string_view>({
        {QPK_UNKNOWN, "QPK_UNKNOWN"},
        {QPK_VERBOSE, "-fverbose"},
        {QPK_CRASHGUARD, "-fcrashguard"},
        {QPV_FASTERROR, "-ffasterror"},
    });

static const boost::bimap<qparse_val_t, std::string_view> values_bimap =
    make_bimap<qparse_val_t, std::string_view>({
        {QPV_UNKNOWN, "QPV_UNKNOWN"},
        {QPV_TRUE, "true"},
        {QPV_FALSE, "false"},
    });

std::ostream &operator<<(std::ostream &os, const qparse_key_t &key) {
  if (options_bimap.left.find(key) != options_bimap.left.end()) {
    os << options_bimap.left.at(key);
  } else {
    qcore_panic("operator<<: Unhandled qparse_key_t value.");
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const qparse_val_t &val) {
  if (values_bimap.left.find(val) != values_bimap.left.end()) {
    os << values_bimap.left.at(val);
  } else {
    qcore_panic("operator<<: Unhandled qparse_val_t value.");
  }

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
    if (options_bimap.left.find(settings[i].key) == options_bimap.left.end()) {
      qcore_panic("qparse_conf_dump: Unhandled qparse_key_t value.");
    }

    if (values_bimap.left.find(settings[i].value) == values_bimap.left.end()) {
      qcore_panic("qparse_conf_dump: Unhandled qparse_val_t value.");
    }

    fprintf(stream, "%s%s%s%s", options_bimap.left.at(settings[i].key).data(), field_delim,
            values_bimap.left.at(settings[i].value).data(), line_delim);
  }
}

bool qparse_conf_t::has(qparse_key_t option, qparse_val_t value) const {
  for (const auto &dat : m_data) {
    if (dat.key == option && dat.value == value) {
      return true;
    }
  }

  return false;
}