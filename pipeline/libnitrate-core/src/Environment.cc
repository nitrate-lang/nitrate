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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Macro.hh>
#include <ranges>
#include <shared_mutex>
#include <sstream>
#include <unordered_map>

using namespace ncc;

class Environment::PImpl {
  static auto GetInitialEnvironment() {
    std::unordered_map<string, string> initial;

    for (auto **environ_it = environ; *environ_it != nullptr; ++environ_it) {
      const std::string_view env_var(*environ_it);
      const auto pos = env_var.find('=');
      if (pos != std::string_view::npos) {
        initial[env_var.substr(0, pos)] = env_var.substr(pos + 1);
      }
    }

    return initial;
  }

public:
  std::unordered_map<string, string> m_initial;
  std::unordered_map<string, string> m_current;
  mutable std::shared_mutex m_mutex;

  PImpl() { m_current = m_initial = GetInitialEnvironment(); }
};

Environment::Environment() : m_impl(std::make_unique<PImpl>()) {}

Environment::~Environment() {
  auto &m = *m_impl;
  std::unique_lock lock(m.m_mutex);

  m.m_current.clear();
  m.m_initial.clear();
}

auto Environment::Contains(std::string_view key) const -> bool {
  auto &m = *m_impl;
  std::shared_lock lock(m.m_mutex);

  return m.m_current.contains(key) || key == "this.keys";
}

auto Environment::Get(string key) const -> std::optional<string> {
  const auto &m = *m_impl;
  std::shared_lock lock(m.m_mutex);

  if (key != "this.keys") [[likely]] {
    auto it = m.m_current.find(key);
    return it == m.m_current.end() ? std::nullopt : std::make_optional(it->second);
  }

  std::stringstream keys;
  for (auto const &[k, _] : m.m_current) {
    keys << k->size() << " " << k;
  }

  return keys.str();
}

void Environment::Set(string key, std::optional<string> value) {
  auto &m = *m_impl;
  std::unique_lock lock(m.m_mutex);

  if (key != "this.keys") {
    if (value.has_value()) {
      m.m_current.insert_or_assign(key, *value);
    } else {
      m.m_current.erase(key);
    }
  }
}

void Environment::Reset() {
  auto &m = *m_impl;
  std::unique_lock lock(m.m_mutex);

  m.m_current = m.m_initial;
}

auto Environment::GetKeys() const -> std::vector<string> {
  auto &m = *m_impl;
  std::shared_lock lock(m.m_mutex);

  std::vector<string> keys;
  keys.reserve(m.m_current.size() + 1);
  keys.emplace_back("this.keys");

  for (auto const &[k, _] : m.m_current) {
    keys.push_back(k);
  }

  return keys;
}
