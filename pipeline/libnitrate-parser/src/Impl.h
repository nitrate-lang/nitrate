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

#ifndef __NITRATE_PARSER_IMPL_H__
#define __NITRATE_PARSER_IMPL_H__

#define __QPARSE_IMPL__

#include <ParseReport.h>
#include <nitrate-parser/Config.h>
#include <nitrate-parser/Node.h>

#include <optional>
#include <vector>

struct qparse_impl_t {
  qparse_impl_t() {}
  ~qparse_impl_t() = default;

  qparse::diag::DiagnosticManager diag;
};

struct qparse_conf_t {
private:
  std::vector<qparse_setting_t> m_data;

  bool verify_prechange(qparse_key_t key, qparse_val_t value) const {
    (void)key;
    (void)value;

    return true;
  }

public:
  qparse_conf_t() = default;
  ~qparse_conf_t() = default;

  bool SetAndVerify(qparse_key_t key, qparse_val_t value) {
    auto it = std::find_if(m_data.begin(), m_data.end(),
                           [key](const qparse_setting_t &setting) { return setting.key == key; });

    if (!verify_prechange(key, value)) {
      return false;
    }

    if (it != m_data.end()) {
      m_data.erase(it);
    }

    m_data.push_back({key, value});

    return true;
  }

  std::optional<qparse_val_t> Get(qparse_key_t key) const {
    auto it = std::find_if(m_data.begin(), m_data.end(),
                           [key](const qparse_setting_t &setting) { return setting.key == key; });

    if (it == m_data.end()) {
      return std::nullopt;
    }

    return it->value;
  }

  const qparse_setting_t *GetAll(size_t &count) const {
    count = m_data.size();
    return m_data.data();
  }

  void ClearNoVerify() {
    m_data.clear();
    m_data.shrink_to_fit();
  }

  bool has(qparse_key_t option, qparse_val_t value) const;
};

#endif  // __NITRATE_PARSER_IMPL_H__
