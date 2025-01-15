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

struct qcode_conf_t {
private:
  std::vector<qcode_setting_t> m_data;

  bool verify_prechange(qcode_key_t, qcode_val_t) const { return true; }

public:
  qcode_conf_t() = default;
  ~qcode_conf_t() = default;

  bool SetAndVerify(qcode_key_t key, qcode_val_t value) {
    auto it = std::find_if(
        m_data.begin(), m_data.end(),
        [key](const qcode_setting_t &setting) { return setting.key == key; });

    if (!verify_prechange(key, value)) {
      return false;
    }

    if (it != m_data.end()) {
      m_data.erase(it);
    }

    m_data.push_back({key, value});

    return true;
  }

  std::optional<qcode_val_t> Get(qcode_key_t key) const {
    auto it = std::find_if(
        m_data.begin(), m_data.end(),
        [key](const qcode_setting_t &setting) { return setting.key == key; });

    if (it == m_data.end()) {
      return std::nullopt;
    }

    return it->value;
  }

  const qcode_setting_t *GetAll(size_t &count) const {
    count = m_data.size();
    return m_data.data();
  }

  void ClearNoVerify() {
    m_data.clear();
    m_data.shrink_to_fit();
  }

  bool has(qcode_key_t option, qcode_val_t value) const;
};

#endif  // __NITRATE_CODEGEN_CORE_CONFIG_H__
