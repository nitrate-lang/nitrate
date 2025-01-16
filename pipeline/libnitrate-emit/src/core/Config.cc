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

#include <nitrate-emit/Config.h>

#include <boost/bimap.hpp>
#include <core/Config.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>

namespace codegen::conf {
  extern std::vector<QcodeSettingT> DefaultSettings;
}

template <typename L, typename R>
auto MakeBimap(
    std::initializer_list<typename boost::bimap<L, R>::value_type> list) -> boost::bimap<L, R> {
  return boost::bimap<L, R>(list.begin(), list.end());
}

static const boost::bimap<QcodeKeyT, std::string> OPTIONS_BIMAP =
    MakeBimap<QcodeKeyT, std::string>({
        {QCK_UNKNOWN, "QCK_UNKNOWN"},
        {QCK_CRASHGUARD, "-fcrashguard"},
        {QCV_FASTERROR, "-ffasterror"},
    });

static const boost::bimap<QcodeValT, std::string> VALUES_BIMAP =
    MakeBimap<QcodeValT, std::string>({
        {QCV_UNKNOWN, "QCV_UNKNOWN"},
        {QCV_TRUE, "true"},
        {QCV_FALSE, "false"},
    });

auto operator<<(std::ostream &os, const QcodeKeyT &key) -> std::ostream & {
  if (OPTIONS_BIMAP.left.find(key) != OPTIONS_BIMAP.left.end()) {
    os << OPTIONS_BIMAP.left.at(key);
  } else {
    qcore_panic("operator<<: Unhandled QcodeKeyT value.");
  }
  return os;
}

auto operator<<(std::ostream &os, const QcodeValT &val) -> std::ostream & {
  if (VALUES_BIMAP.left.find(val) != VALUES_BIMAP.left.end()) {
    os << VALUES_BIMAP.left.at(val);
  } else {
    qcore_panic("operator<<: Unhandled QcodeValT value.");
  }

  return os;
}

static void AssignDefaultOptions(QCodegenConfig &conf) {
  for (const auto &setting : codegen::conf::DefaultSettings) {
    QcodeConfSetopt(&conf, setting.m_key, setting.m_value);
  }
}

extern "C" NCC_EXPORT auto QcodeConfNew(bool use_defaults) -> QCodegenConfig * {
  QCodegenConfig *obj = new QCodegenConfig();

  if (use_defaults) {
    AssignDefaultOptions(*obj);
  }

  return obj;
}

extern "C" NCC_EXPORT void QcodeConfFree(QCodegenConfig *conf) { delete conf; }

extern "C" NCC_EXPORT auto QcodeConfSetopt(QCodegenConfig *conf, QcodeKeyT key,
                                           QcodeValT value) -> bool {
  return conf->SetAndVerify(key, value);
}

extern "C" NCC_EXPORT auto QcodeConfGetopt(QCodegenConfig *conf, QcodeKeyT key,
                                           QcodeValT *value) -> bool {
  auto val = conf->Get(key);

  if (!val.has_value()) {
    return false;
  }

  if (value != nullptr) {
    *value = val.value();
  }

  return true;
}

extern "C" NCC_EXPORT auto QcodeConfGetopts(QCodegenConfig *conf,
                                                      size_t *count) -> QcodeSettingT * {
  if (!count) {
    qcore_panic(
        "qcode_conf_getopts: Contract violation: 'count' parameter cannot be "
        "NULL.");
  }

  const QcodeSettingT *ptr = conf->GetAll(*count);

  if (!ptr) {
    return nullptr;
  }

  size_t size = *count * sizeof(QcodeSettingT);

  QcodeSettingT *copy = static_cast<QcodeSettingT *>(malloc(size));
  if (!copy) {
    return nullptr;
  }

  memcpy(copy, ptr, size);

  return copy;
}

extern "C" NCC_EXPORT void QcodeConfClear(QCodegenConfig *conf) {
  conf->ClearNoVerify();
}

auto QCodegenConfig::Has(QcodeKeyT option, QcodeValT value) const -> bool {
  for (const auto &dat : m_data) {
    if (dat.m_key == option && dat.m_value == value) {
      return true;
    }
  }

  return false;
}
