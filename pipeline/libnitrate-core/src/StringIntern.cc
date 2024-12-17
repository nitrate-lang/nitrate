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

#include <nitrate-core/StringIntern.hh>

using namespace qcore;

CPP_EXPORT StringMemory::Storage StringMemory::StringMemory::m_storage;

CPP_EXPORT std::string_view str_alias::get() const {
  return StringMemory::from_id(m_id);
}

str_alias StringMemory::get(std::string_view str) {
  std::lock_guard lock(m_storage.m_mutex);

  if (auto it = m_storage.m_bimap.right.find(str);
      it != m_storage.m_bimap.right.end()) {
    return str_alias::get(it->second);
  }

  auto ref_it = m_storage.m_strings.insert(std::string(str));
  const std::string &str_ref = *ref_it.first;
  uint64_t new_id = m_storage.m_next_id++;

  m_storage.m_bimap.insert({new_id, str_ref});

  return str_alias::get(new_id);
}

CPP_EXPORT void StringMemory::save(std::string_view str) { (void)get(str); }

std::string_view StringMemory::from_id(uint64_t id) {
  std::lock_guard lock(m_storage.m_mutex);

  auto it = m_storage.m_bimap.left.find(id);

  if (it == m_storage.m_bimap.left.end()) [[unlikely]] {
    qcore_panicf("Unknown interned string ID: %lu", id);
  }

  return it->second;
}
