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

#include <nitrate-ir/Config.h>

#include <boost/bimap.hpp>
#include <core/Config.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>

namespace nr::conf {
  extern std::vector<nr_setting_t> default_settings;
}

template <typename L, typename R>
boost::bimap<L, R> make_bimap(
    std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const boost::bimap<nr_key_t, std::string> options_bimap =
    make_bimap<nr_key_t, std::string>({
        {QQK_UNKNOWN, "QQK_UNKNOWN"},
        {QQK_CRASHGUARD, "-fcrashguard"},
        {QQV_FASTERROR, "-ffasterror"},
    });

static const boost::bimap<nr_val_t, std::string> values_bimap =
    make_bimap<nr_val_t, std::string>({
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

C_EXPORT nr_conf_t *nr_conf_new(bool use_defaults) {
  nr_conf_t *obj = new nr_conf_t();

  if (use_defaults) {
    assign_default_options(*obj);
  }

  return obj;
}

C_EXPORT void nr_conf_free(nr_conf_t *conf) { delete conf; }

C_EXPORT bool nr_conf_setopt(nr_conf_t *conf, nr_key_t key, nr_val_t value) {
  return conf->SetAndVerify(key, value);
}

C_EXPORT bool nr_conf_getopt(nr_conf_t *conf, nr_key_t key, nr_val_t *value) {
  auto val = conf->Get(key);

  if (!val.has_value()) {
    return false;
  }

  if (value != nullptr) {
    *value = val.value();
  }

  return true;
}

C_EXPORT nr_setting_t *nr_conf_getopts(nr_conf_t *conf, size_t *count) {
  if (!count) {
    qcore_panic(
        "nr_conf_getopts: Contract violation: 'count' parameter cannot be "
        "NULL.");
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

C_EXPORT void nr_conf_clear(nr_conf_t *conf) { conf->ClearNoVerify(); }

bool nr_conf_t::has(nr_key_t option, nr_val_t value) const {
  for (const auto &dat : m_data) {
    if (dat.key == option && dat.value == value) {
      return true;
    }
  }

  return false;
}
