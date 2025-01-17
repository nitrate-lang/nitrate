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
#include <chrono>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Macro.hh>

using namespace ncc;

void Environment::SetupDefaultKeys() {
  { /* Generate unique ID for this compilation unit */
    boost::uuids::random_generator gen;
    boost::uuids::uuid uuid = gen();
    std::string uuid_str = boost::uuids::to_string(uuid);
    m_data["this.job"] = uuid_str;
  }

  { /* Set the compiler start time */
    let now = std::chrono::system_clock::now();
    let ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());

    m_data["this.created_at"] = std::to_string(ms.count());
  }
}

auto Environment::Contains(std::string_view key) -> bool {
  return m_data.contains(string(key));
}

auto Environment::Get(string key) -> std::optional<string> {
  if (auto it = m_data.find(key); it != m_data.end()) {
    return it->second;
  }
  return std::nullopt;
}

void Environment::Set(string key, std::optional<string> value, bool) {
  if (value.has_value()) {
    m_data.insert_or_assign(key, *value);
  } else {
    m_data.erase(key);
  }
}
