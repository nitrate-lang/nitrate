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

#ifndef __QUIX_QXIR_IMPL_H__
#define __QUIX_QXIR_IMPL_H__

#define __QPARSE_IMPL__

#include <QXIRReport.h>
#include <quix-qxir/Config.h>
#include <quix-qxir/Node.h>

#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

struct qxir_impl_t {
  std::unordered_set<std::string> strings;

  qxir_impl_t() {}
  ~qxir_impl_t() = default;

  qxir::diag::DiagnosticManager diag;

  std::string_view push_string(std::string_view sv) {
    for (const auto &str : strings) {
      if (str == sv) {
        return str;
      }
    }

    return strings.insert(std::string(sv)).first->c_str();
  }
};

class qxir_conf_t {
  std::vector<qxir_setting_t> m_data;

  bool verify_prechange(qxir_key_t key, qxir_val_t value) const {
    (void)key;
    (void)value;

    return true;
  }

public:
  qxir_conf_t() = default;
  ~qxir_conf_t() = default;

  bool SetAndVerify(qxir_key_t key, qxir_val_t value) {
    auto it = std::find_if(m_data.begin(), m_data.end(),
                           [key](const qxir_setting_t &setting) { return setting.key == key; });

    if (!verify_prechange(key, value)) {
      return false;
    }

    if (it != m_data.end()) {
      m_data.erase(it);
    }

    m_data.push_back({key, value});

    return true;
  }

  std::optional<qxir_val_t> Get(qxir_key_t key) const {
    auto it = std::find_if(m_data.begin(), m_data.end(),
                           [key](const qxir_setting_t &setting) { return setting.key == key; });

    if (it == m_data.end()) {
      return std::nullopt;
    }

    return it->value;
  }

  const qxir_setting_t *GetAll(size_t &count) const {
    count = m_data.size();
    return m_data.data();
  }

  void ClearNoVerify() {
    m_data.clear();
    m_data.shrink_to_fit();
  }

  bool has(qxir_key_t option, qxir_val_t value) const;
};

#endif  // __QUIX_QXIR_IMPL_H__
