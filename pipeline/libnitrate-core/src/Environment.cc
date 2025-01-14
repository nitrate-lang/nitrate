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

using namespace ncc;

void Environment::setup_default_env() {
  { /* Generate unique ID for this compilation unit */
    boost::uuids::random_generator gen;
    boost::uuids::uuid uuid = gen();
    std::string uuid_str = boost::uuids::to_string(uuid);
    set("this.job", uuid_str.c_str());
  }

  { /* Set the compiler start time */
    let now = std::chrono::system_clock::now();
    let ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());

    set("this.created_at", std::to_string(ms.count()).c_str());
  }

  set("FILE", "<stdin>");
}

NCC_EXPORT Environment::Environment() { setup_default_env(); }

NCC_EXPORT bool Environment::contains(std::string_view key) {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_data.contains(key);
}

NCC_EXPORT std::optional<std::string_view> Environment::get(
    std::string_view key) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (auto it = m_data.find(key); it != m_data.end()) {
    return it->second;
  }
  return std::nullopt;
}

NCC_EXPORT void Environment::set(std::string_view key,
                                 std::optional<std::string_view> value, bool) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (value.has_value()) {
    m_data[key] = *value;
  } else {
    m_data.erase(key);
  }
}
