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

#ifndef __NITRATE_CODEGEN_CORE_CONFIG_H__
#define __NITRATE_CODEGEN_CORE_CONFIG_H__

#include <nitrate-emit/Config.h>

#include <algorithm>
#include <optional>
#include <vector>

struct QCodegenConfig {
private:
  std::vector<QcodeSettingT> m_data;

  [[nodiscard]] auto VerifyPrechange(QcodeKeyT, QcodeValT) const -> bool { return true; }

public:
  QCodegenConfig() = default;
  ~QCodegenConfig() = default;

  auto SetAndVerify(QcodeKeyT key, QcodeValT value) -> bool {
    auto it = std::find_if(
        m_data.begin(), m_data.end(),
        [key](const QcodeSettingT &setting) { return setting.m_key == key; });

    if (!VerifyPrechange(key, value)) {
      return false;
    }

    if (it != m_data.end()) {
      m_data.erase(it);
    }

    m_data.push_back({key, value});

    return true;
  }

  [[nodiscard]] auto Get(QcodeKeyT key) const -> std::optional<QcodeValT> {
    auto it = std::find_if(
        m_data.begin(), m_data.end(),
        [key](const QcodeSettingT &setting) { return setting.m_key == key; });

    if (it == m_data.end()) {
      return std::nullopt;
    }

    return it->m_value;
  }

  auto GetAll(size_t &count) const -> const QcodeSettingT * {
    count = m_data.size();
    return m_data.data();
  }

  void ClearNoVerify() {
    m_data.clear();
    m_data.shrink_to_fit();
  }

  [[nodiscard]] auto Has(QcodeKeyT option, QcodeValT value) const -> bool;
};

#endif  // __NITRATE_CODEGEN_CORE_CONFIG_H__
