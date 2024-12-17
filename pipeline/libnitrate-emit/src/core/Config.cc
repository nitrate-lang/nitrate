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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-emit/Config.h>

#include <Config.hh>
#include <boost/bimap.hpp>

namespace codegen::conf {
  extern std::vector<qcode_setting_t> default_settings;
}

template <typename L, typename R>
boost::bimap<L, R> make_bimap(
    std::initializer_list<typename boost::bimap<L, R>::value_type> list) {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const boost::bimap<qcode_key_t, std::string> options_bimap =
    make_bimap<qcode_key_t, std::string>({
        {QCK_UNKNOWN, "QCK_UNKNOWN"},
        {QCK_CRASHGUARD, "-fcrashguard"},
        {QCV_FASTERROR, "-ffasterror"},
    });

static const boost::bimap<qcode_val_t, std::string> values_bimap =
    make_bimap<qcode_val_t, std::string>({
        {QCV_UNKNOWN, "QCV_UNKNOWN"},
        {QCV_TRUE, "true"},
        {QCV_FALSE, "false"},
    });

std::ostream &operator<<(std::ostream &os, const qcode_key_t &key) {
  if (options_bimap.left.find(key) != options_bimap.left.end()) {
    os << options_bimap.left.at(key);
  } else {
    qcore_panic("operator<<: Unhandled qcode_key_t value.");
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const qcode_val_t &val) {
  if (values_bimap.left.find(val) != values_bimap.left.end()) {
    os << values_bimap.left.at(val);
  } else {
    qcore_panic("operator<<: Unhandled qcode_val_t value.");
  }

  return os;
}

static void assign_default_options(qcode_conf_t &conf) {
  for (const auto &setting : codegen::conf::default_settings) {
    qcode_conf_setopt(&conf, setting.key, setting.value);
  }
}

C_EXPORT qcode_conf_t *qcode_conf_new(bool use_defaults) {
  qcode_conf_t *obj = new qcode_conf_t();

  if (use_defaults) {
    assign_default_options(*obj);
  }

  return obj;
}

C_EXPORT void qcode_conf_free(qcode_conf_t *conf) { delete conf; }

C_EXPORT bool qcode_conf_setopt(qcode_conf_t *conf, qcode_key_t key,
                                qcode_val_t value) {
  return conf->SetAndVerify(key, value);
}

C_EXPORT bool qcode_conf_getopt(qcode_conf_t *conf, qcode_key_t key,
                                qcode_val_t *value) {
  auto val = conf->Get(key);

  if (!val.has_value()) {
    return false;
  }

  if (value != nullptr) {
    *value = val.value();
  }

  return true;
}

C_EXPORT qcode_setting_t *qcode_conf_getopts(qcode_conf_t *conf,
                                             size_t *count) {
  if (!count) {
    qcore_panic(
        "qcode_conf_getopts: Contract violation: 'count' parameter cannot be "
        "NULL.");
  }

  const qcode_setting_t *ptr = conf->GetAll(*count);

  if (!ptr) {
    return nullptr;
  }

  size_t size = *count * sizeof(qcode_setting_t);

  qcode_setting_t *copy = static_cast<qcode_setting_t *>(malloc(size));
  if (!copy) {
    return nullptr;
  }

  memcpy(copy, ptr, size);

  return copy;
}

C_EXPORT void qcode_conf_clear(qcode_conf_t *conf) { conf->ClearNoVerify(); }

bool qcode_conf_t::has(qcode_key_t option, qcode_val_t value) const {
  for (const auto &dat : m_data) {
    if (dat.key == option && dat.value == value) {
      return true;
    }
  }

  return false;
}
