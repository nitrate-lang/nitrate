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

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <nitrate-ir/Config.h>

#include <boost/bimap.hpp>
#include <core/Config.hh>

namespace nr::conf {
  extern std::vector<nr_setting_t> default_settings;
}

template <typename L, typename R>
boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const boost::bimap<nr_key_t, std::string_view> options_bimap =
    make_bimap<nr_key_t, std::string_view>({
        {QQK_UNKNOWN, "QQK_UNKNOWN"},
        {QQK_CRASHGUARD, "-fcrashguard"},
        {QQV_FASTERROR, "-ffasterror"},
    });

static const boost::bimap<nr_val_t, std::string_view> values_bimap =
    make_bimap<nr_val_t, std::string_view>({
        {QQV_UNKNOWN, "QQV_UNKNOWN"},
        {QQV_TRUE, "true"},
        {QQV_FALSE, "false"},
    });

std::ostream &operator<<(std::ostream &os, const nr_key_t &key) {
  return os << options_bimap.left.at(key);
}

std::ostream &operator<<(std::ostream &os, const nr_val_t &val) {
  return os << values_bimap.left.at(val);
}

static void assign_default_options(nr_conf_t &conf) {
  for (const auto &setting : nr::conf::default_settings) {
    nr_conf_setopt(&conf, setting.key, setting.value);
  }
}

LIB_EXPORT nr_conf_t *nr_conf_new(bool use_defaults) {
  nr_conf_t *obj = new nr_conf_t();

  if (use_defaults) {
    assign_default_options(*obj);
  }

  return obj;
}

LIB_EXPORT void nr_conf_free(nr_conf_t *conf) { delete conf; }

LIB_EXPORT bool nr_conf_setopt(nr_conf_t *conf, nr_key_t key, nr_val_t value) {
  return conf->SetAndVerify(key, value);
}

LIB_EXPORT bool nr_conf_getopt(nr_conf_t *conf, nr_key_t key, nr_val_t *value) {
  auto val = conf->Get(key);

  if (!val.has_value()) {
    return false;
  }

  if (value != nullptr) {
    *value = val.value();
  }

  return true;
}

LIB_EXPORT nr_setting_t *nr_conf_getopts(nr_conf_t *conf, size_t *count) {
  if (!count) {
    qcore_panic("nr_conf_getopts: Contract violation: 'count' parameter cannot be NULL.");
  }

  const nr_setting_t *ptr = conf->GetAll(*count);

  if (!ptr) {
    return nullptr;
  }

  size_t size = *count * sizeof(nr_setting_t);

  nr_setting_t *copy = static_cast<nr_setting_t *>(malloc(size));
  if (!copy) {
    return nullptr;
  }

  memcpy(copy, ptr, size);

  return copy;
}

LIB_EXPORT void nr_conf_clear(nr_conf_t *conf) { conf->ClearNoVerify(); }

LIB_EXPORT size_t nr_conf_dump(nr_conf_t *conf, FILE *stream, const char *field_delim,
                               const char *line_delim) {
  if (!stream) {
    qcore_panic("nr_conf_dump: Contract violation: 'stream' parameter cannot be NULL.");
  }

  if (!field_delim) {
    field_delim = "=";
  }

  if (!line_delim) {
    line_delim = "\n";
  }

  const nr_setting_t *settings = nullptr;
  size_t count = 0, bytes = 0;

  settings = nr_conf_getopts(conf, &count);

  for (size_t i = 0; i < count; ++i) {
    if (options_bimap.left.find(settings[i].key) == options_bimap.left.end()) {
      qcore_panic("nr_conf_dump: Unhandled nr_key_t value.");
    }

    if (values_bimap.left.find(settings[i].value) == values_bimap.left.end()) {
      qcore_panic("nr_conf_dump: Unhandled nr_val_t value.");
    }

    bytes += fprintf(stream, "%s%s%s%s", options_bimap.left.at(settings[i].key).data(), field_delim,
                     values_bimap.left.at(settings[i].value).data(), line_delim);
  }

  return bytes;
}

bool nr_conf_t::has(nr_key_t option, nr_val_t value) const noexcept {
  for (const auto &dat : m_data) {
    if (dat.key == option && dat.value == value) {
      return true;
    }
  }

  return false;
}
