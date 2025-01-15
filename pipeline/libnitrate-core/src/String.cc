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

#include <boost/unordered_map.hpp>
#include <mutex>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <sparsehash/dense_hash_map>

using namespace ncc;

static NCC_FORCE_INLINE constexpr std::vector<char> FromStr(
    std::string_view str) {
  std::vector<char> vec(str.size());
  std::copy(str.begin(), str.end(), vec.begin());
  return vec;
}

static NCC_FORCE_INLINE constexpr std::string_view FromVec(
    const std::vector<char>& vec) {
  return std::string_view(vec.data(), vec.size());
}

struct Storage {
  std::vector<std::vector<char>> m_data;
  google::dense_hash_map<std::string_view, uint64_t> m_map;

  Storage() {
    m_map.set_empty_key("");
    m_data.reserve(4096);
  }
};

static Storage m_storage;
static std::mutex m_storage_mutex;

NCC_EXPORT std::string_view auto_intern::get() const {
  if (m_id == 0) {
    return "";
  }

  bool sync = EnableSync;
  if (sync) {
    m_storage_mutex.lock();
  }

  std::string_view str;

  if (m_id < m_storage.m_data.size()) [[likely]] {
    str = FromVec(m_storage.m_data[m_id]);
  } else {
    str = "";
  }

  if (sync) {
    m_storage_mutex.unlock();
  }

  return str;
}

NCC_EXPORT uint64_t StringMemory::FromString(std::string_view str) {
  assert(!str.empty());

  bool sync = EnableSync;

  if (sync) {
    m_storage_mutex.lock();
  }

  uint64_t id;
  if (auto it = m_storage.m_map.find(str); it != m_storage.m_map.end()) {
    id = it->second;
  } else {
    id = m_storage.m_data.size();
    m_storage.m_data.emplace_back(FromStr(str));
    m_storage.m_map[FromVec(m_storage.m_data.back())] = id;
  }

  if (sync) {
    m_storage_mutex.unlock();
  }

  return id;
}

NCC_EXPORT uint64_t StringMemory::FromString(std::string&& str) {
  assert(!str.empty());

  bool sync = EnableSync;

  if (sync) {
    m_storage_mutex.lock();
  }

  uint64_t id;
  if (auto it = m_storage.m_map.find(str); it != m_storage.m_map.end()) {
    id = it->second;
  } else {
    id = m_storage.m_data.size();
    m_storage.m_data.emplace_back(FromStr(std::move(str)));
    m_storage.m_map[FromVec(m_storage.m_data.back())] = id;
  }

  if (sync) {
    m_storage_mutex.unlock();
  }

  return id;
}

NCC_EXPORT void StringMemory::Reset() {
  bool sync = EnableSync;

  if (sync) {
    m_storage_mutex.lock();
  }

  m_storage.m_map.clear();
  m_storage.m_data.clear();

  if (sync) {
    m_storage_mutex.unlock();
  }
}
