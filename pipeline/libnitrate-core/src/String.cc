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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>

using namespace ncc;

NCC_EXPORT StringMemory::Storage StringMemory::StringMemory::m_storage;

NCC_EXPORT std::string_view auto_intern::get() const {
  return m_id == 0 ? "" : StringMemory::FromID(m_id);
}

NCC_EXPORT uint64_t StringMemory::FromString(std::string_view str) {
  std::lock_guard lock(m_storage.m_mutex);

  if (auto it = m_storage.m_map_b.find(str); it != m_storage.m_map_b.end()) {
    return it->second;
  }

  auto new_id = m_storage.m_next_id++;

  const auto& ref_str =
      m_storage.m_map_a.insert({new_id, std::string(str)}).first->second;

  m_storage.m_map_b.insert({ref_str, new_id});

  return new_id;
}

NCC_EXPORT uint64_t StringMemory::FromString(std::string&& str) {
  std::lock_guard lock(m_storage.m_mutex);

  if (auto it = m_storage.m_map_b.find(str); it != m_storage.m_map_b.end()) {
    return it->second;
  }

  auto new_id = m_storage.m_next_id++;

  const auto& ref_str =
      m_storage.m_map_a.insert({new_id, std::move(str)}).first->second;

  m_storage.m_map_b.insert({ref_str, new_id});

  return new_id;
}

NCC_EXPORT std::string_view StringMemory::FromID(uint64_t id) {
  std::lock_guard lock(m_storage.m_mutex);

  if (auto it = m_storage.m_map_a.find(id); it != m_storage.m_map_a.end())
      [[likely]] {
    return it->second;
  } else {
    qcore_panicf("Unknown interned string ID: %lu", id);
  }
}

NCC_EXPORT void StringMemory::Reset() {
  std::lock_guard lock(m_storage.m_mutex);

  m_storage.m_map_a.clear();
  m_storage.m_map_b.clear();
  m_storage.m_next_id = 0;
}
