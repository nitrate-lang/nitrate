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

#include <QXIRImpl.h>
#include <quix-core/Error.h>
#include <quix-qxir/Config.h>

#include <boost/bimap.hpp>

#include "LibMacro.h"

namespace qxir::conf {
  extern std::vector<qxir_setting_t> default_settings;
}

template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const boost::bimap<qxir_key_t, std::string_view> options_bimap =
    make_bimap<qxir_key_t, std::string_view>({
        {QQK_UNKNOWN, "QQK_UNKNOWN"},
        {QQK_CRASHGUARD, "-fcrashguard"},
        {QQV_FASTERROR, "-ffasterror"},
    });

static const boost::bimap<qxir_val_t, std::string_view> values_bimap =
    make_bimap<qxir_val_t, std::string_view>({
        {QQV_UNKNOWN, "QQV_UNKNOWN"},
        {QQV_TRUE, "true"},
        {QQV_FALSE, "false"},
    });

std::ostream &operator<<(std::ostream &os, const qxir_key_t &key) {
  if (options_bimap.left.find(key) != options_bimap.left.end()) {
    os << options_bimap.left.at(key);
  } else {
    qcore_panic("operator<<: Unhandled qxir_key_t value.");
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const qxir_val_t &val) {
  if (values_bimap.left.find(val) != values_bimap.left.end()) {
    os << values_bimap.left.at(val);
  } else {
    qcore_panic("operator<<: Unhandled qxir_val_t value.");
  }

  return os;
}

static void assign_default_options(qxir_conf_t &conf) {
  for (const auto &setting : qxir::conf::default_settings) {
    qxir_conf_setopt(&conf, setting.key, setting.value);
  }
}

LIB_EXPORT qxir_conf_t *qxir_conf_new(bool use_defaults) {
  try {
    qxir_conf_t *obj = new qxir_conf_t();

    if (use_defaults) {
      assign_default_options(*obj);
    }

    return obj;
  } catch (...) {
    return nullptr;
  }
}

LIB_EXPORT void qxir_conf_free(qxir_conf_t *conf) {
  try {
    delete conf;
  } catch (...) {
    qcore_panic("qxir_conf_free: Internal error: failed to free qxir_conf_t object.");
  }
}

LIB_EXPORT bool qxir_conf_setopt(qxir_conf_t *conf, qxir_key_t key, qxir_val_t value) {
  try {
    return conf->SetAndVerify(key, value);
  } catch (...) {
    return false;
  }
}

LIB_EXPORT bool qxir_conf_getopt(qxir_conf_t *conf, qxir_key_t key, qxir_val_t *value) {
  try {
    auto val = conf->Get(key);

    if (!val.has_value()) {
      return false;
    }

    if (value != nullptr) {
      *value = val.value();
    }

    return true;
  } catch (...) {
    qcore_panic("qxir_conf_getopt: Internal error: unable to retrieve qxir_val_t value.");
  }
}

LIB_EXPORT const qxir_setting_t *qxir_conf_getopts(qxir_conf_t *conf, size_t *count) {
  try {
    if (!count) {
      qcore_panic("qxir_conf_getopts: Contract violation: 'count' parameter cannot be NULL.");
    }

    return conf->GetAll(*count);
  } catch (...) {
    return nullptr;
  }
}

LIB_EXPORT void qxir_conf_clear(qxir_conf_t *conf) {
  try {
    conf->ClearNoVerify();
  } catch (...) {
    qcore_panic("qxir_conf_clear: Internal error: failed to clear qxir_conf_t object.");
  }
}

LIB_EXPORT size_t qxir_conf_dump(qxir_conf_t *conf, FILE *stream, const char *field_delim,
                                 const char *line_delim) {
  try {
    if (!field_delim || !line_delim) {
      qcore_panic(
          "qxir_conf_dump: Contract violation: 'field_delim' and 'line_delim' parameters cannot be "
          "NULL.");
    }

    const qxir_setting_t *settings = nullptr;
    size_t count = 0, bytes = 0;

    settings = qxir_conf_getopts(conf, &count);

    for (size_t i = 0; i < count; ++i) {
      if (options_bimap.left.find(settings[i].key) == options_bimap.left.end()) {
        qcore_panic("qxir_conf_dump: Unhandled qxir_key_t value.");
      }

      if (values_bimap.left.find(settings[i].value) == values_bimap.left.end()) {
        qcore_panic("qxir_conf_dump: Unhandled qxir_val_t value.");
      }

      bytes += fprintf(stream, "%s%s%s%s", options_bimap.left.at(settings[i].key).data(),
                       field_delim, values_bimap.left.at(settings[i].value).data(), line_delim);
    }

    return bytes;
  } catch (...) {
    return 0;
  }
}

bool qxir_conf_t::has(qxir_key_t option, qxir_val_t value) const {
  try {
    for (const auto &dat : m_data) {
      if (dat.key == option && dat.value == value) {
        return true;
      }
    }

    return false;
  } catch (...) {
    return false;
  }
}
